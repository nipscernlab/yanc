#!/usr/bin/env bash
# Scripts/regress.sh - regression test for the CMM compiler pipeline.
#
# For every .cmm in Exemplos/ (plus Testes/fixtures/) runs cmmcomp and
# compares the produced .asm against Testes/golden/<prname>.asm.
#
# For a subset of examples it also runs appcomp + asmcomp + iverilog +
# vvp and compares the testbench output files against
# Testes/golden_sim/<prname>/output_*.txt. Catches behavioral
# regressions that pure .asm comparison would miss after future
# optimization-style refactors.
#
# Usage (from repo root, in msys2/git-bash on Windows):
#   Scripts/regress.sh                check against goldens (exit 0 = pass)
#   Scripts/regress.sh --update       regenerate goldens (review diff!)
#   Scripts/regress.sh --update-size  ratchet down Testes/size_baseline.txt
#   Scripts/regress.sh --skip-build   reuse binaries already in .smoke/bin
#   Scripts/regress.sh --no-sim       skip the simulation phase entirely

set -uo pipefail

# When invoked from a non-interactive shell (e.g. PowerShell calling bash),
# msys2 may not inherit TMP/TEMP and gcc/bison choke on C:\WINDOWS\. Pin
# TMPDIR to a writable location if nothing usable is set.
if [ -z "${TMPDIR:-}" ] && [ -z "${TMP:-}" ] && [ -z "${TEMP:-}" ]; then
    export TMPDIR=/tmp
fi

UPDATE=0
UPDATE_SIZE=0
SKIP_BUILD=0
NO_SIM=0
for arg in "$@"; do
    case "$arg" in
        --update)      UPDATE=1 ;;
        --update-size) UPDATE_SIZE=1 ;;
        --skip-build)  SKIP_BUILD=1 ;;
        --no-sim)      NO_SIM=1 ;;
        -h|--help)
            sed -n '1,/^$/p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "regress: unknown option: $arg" >&2
            exit 2
            ;;
    esac
done

ROOT="$(pwd)"
SCRATCH="$ROOT/.smoke"
BIN_DIR="$SCRATCH/bin"
WORK_DIR="$SCRATCH/work"
TMP_DIR="$SCRATCH/tmp"
GOLDEN_DIR="$ROOT/Testes/golden"
GOLDEN_SIM_DIR="$ROOT/Testes/golden_sim"

CMMCOMP="$BIN_DIR/cmmcomp.exe"
APPCOMP="$BIN_DIR/appcomp.exe"
ASMCOMP="$BIN_DIR/asmcomp.exe"
MACROS="$ROOT/Macros"
HDL="$ROOT/HDL"
SIZE_BASELINE_FILE="$ROOT/Testes/size_baseline.txt"

# load the size ratchet (prname -> num_ins). Lines starting with '#' are skipped.
declare -A SIZE_BASELINE
if [ -f "$SIZE_BASELINE_FILE" ]; then
    while IFS= read -r line || [ -n "$line" ]; do
        [ -z "$line" ] && continue
        [ "${line:0:1}" = "#" ] && continue
        name=$(echo "$line" | awk '{print $1}')
        val=$(echo "$line"  | awk '{print $2}')
        [ -n "$name" ] && [ -n "$val" ] && SIZE_BASELINE[$name]="$val"
    done < "$SIZE_BASELINE_FILE"
fi
declare -A SIZE_CURRENT

# iverilog / vvp paths can be overridden via env (CI will need different ones)
: "${IVERILOG:=/c/nipscern/Aurora/components/Packages/iverilog/bin/iverilog.exe}"
: "${VVP:=/c/nipscern/Aurora/components/Packages/iverilog/bin/vvp.exe}"

# Examples to exclude from the standalone simulation phase (still run cmmcomp
# .asm compare AND appcomp/asmcomp so their .v + .mif are available for any
# downstream project-level link):
#   procBlind          - takes too long to simulate
#   sw_test            - synthetic .cmm fixture; testbench would loop on while(1)
#   ArcTan/Seno/Sqrt   - Math benchmarks: compute internally with no out(...) calls
#   ProcDTW/ZeroCross  - real testbench lives in the multi-proc DTW project pass
SIM_SKIP=("procBlind"            "ArcTan" "Seno" "Sqrt" "ProcDTW" "ZeroCross")

# Examples to also skip the appcomp/asmcomp build for (their .v / .mif are
# never consumed elsewhere - no standalone sim, no project link). Keeps the
# regression cheap.
BUILD_SKIP=("procBlind"            "ArcTan" "Seno" "Sqrt")
build_skipped() {
    local name="$1"
    for s in "${BUILD_SKIP[@]}"; do [ "$s" = "$name" ] && return 0; done
    return 1
}

sim_skipped() {
    local name="$1"
    for s in "${SIM_SKIP[@]}"; do [ "$s" = "$name" ] && return 0; done
    return 1
}

# ---- 1. build the three compilers -----------------------------------------

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "==> building cmmcomp + appcomp + asmcomp"
    mkdir -p "$BIN_DIR"

    # Wipe any stale binaries first: a silently-failing gcc must NOT leave the
    # previous build in place to fool the regress into testing dead code.
    rm -f "$CMMCOMP" "$APPCOMP" "$ASMCOMP"

    # `set -e` here so each tool's non-zero exit aborts the script. Without
    # this, a gcc -Werror failure would print to the terminal but the regress
    # would happily continue and run all examples against the (now-missing)
    # binary, hiding the real problem.
    set -e

    pushd "$ROOT/CMMComp/Sources" >/dev/null
    bison -y -d CMMComp.y
    flex CMMComp.l
    gcc -O2 -Wall -Werror -o "$CMMCOMP" \
        ast.c data_assign.c data_declar.c data_use.c itr.c diretivas.c \
        funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c \
        variaveis.c array_index.c global.c macros.c messages.c args.c \
        y.tab.c
    rm -f lex.yy.c y.tab.c y.tab.h
    popd >/dev/null

    pushd "$ROOT/APP/Sources" >/dev/null
    flex -o app.c app.l
    gcc -O2 -Wall -Werror -o "$APPCOMP" \
        app.c eval.c variaveis.c messages.c args.c
    rm -f app.c
    popd >/dev/null

    pushd "$ROOT/ASM/Sources" >/dev/null
    flex -o ASMComp.c ASMComp.l
    gcc -O2 -Wall -Werror -o "$ASMCOMP" \
        ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c \
        hdl.c simulacao.c array.c messages.c args.c
    rm -f ASMComp.c
    popd >/dev/null

    set +e
fi

# ---- 2. run on each example and compare -----------------------------------

mkdir -p "$WORK_DIR" "$TMP_DIR" "$GOLDEN_DIR" "$GOLDEN_SIM_DIR"

pass=0
fail=0
failed_names=()

# globstar avoids depending on `find` (msys2's find may be shadowed by
# Windows DOS find.exe in PATH when invoked from PowerShell)
shopt -s globstar nullglob
cmm_files=(Exemplos/**/*.cmm Testes/fixtures/**/*.cmm)
IFS=$'\n' cmm_sorted=($(printf '%s\n' "${cmm_files[@]}" | sort))

for cmm in "${cmm_sorted[@]}"; do
    proc_dir_rel="$(dirname "$(dirname "$cmm")")"
    prname="$(basename "$proc_dir_rel")"
    filename="$(basename "$cmm")"

    work_proc="$WORK_DIR/$prname"
    rm -rf "$work_proc"
    cp -r "$proc_dir_rel" "$work_proc"
    mkdir -p "$work_proc/Hardware" "$work_proc/Simulation"

    tmp="$TMP_DIR/$prname"
    rm -rf "$tmp"
    mkdir -p "$tmp"

    asm_file="$work_proc/Software/$prname.asm"
    golden="$GOLDEN_DIR/$prname.asm"

    # cmmcomp step ----------------------------------------------------------

    if ! "$CMMCOMP" -en -i "$filename" -n "$prname" -p "$work_proc" -m "$MACROS" -t "$tmp" >/dev/null 2>&1; then
        echo "FAIL ($prname): cmmcomp exited non-zero"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi
    [ -s "$asm_file" ] || { echo "FAIL ($prname): asm missing/empty"; fail=$((fail+1)); failed_names+=("$prname"); continue; }

    # size ratchet ---------------------------------------------------------
    # cmmcomp writes "num_ins N" at the end of cmm_log.txt - the real
    # instruction count (labels, directives and macros excluded). Compare to
    # the per-example baseline; refactors that grow num_ins for any example
    # are rejected unless --update-size is passed explicitly.
    num_ins=$(grep "^num_ins " "$tmp/cmm_log.txt" 2>/dev/null | awk '{print $2}')
    SIZE_CURRENT[$prname]="${num_ins:-0}"
    baseline="${SIZE_BASELINE[$prname]:-}"
    size_grew=0
    if [ -z "$baseline" ]; then
        size_msg="  [size: ${num_ins:-?}, no baseline]"
    elif [ "${num_ins:-0}" -gt "$baseline" ]; then
        size_msg="  [size: $num_ins, GREW +$((num_ins - baseline)) vs $baseline]"
        size_grew=1
    elif [ "${num_ins:-0}" -lt "$baseline" ]; then
        size_msg="  [size: $num_ins, -$((baseline - num_ins)) vs $baseline]"
    else
        size_msg="  [size: $num_ins]"
    fi
    if [ "$size_grew" -eq 1 ] && [ "$UPDATE_SIZE" -eq 0 ]; then
        echo "FAIL ($prname):$size_msg"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    # golden .asm capture or compare ---------------------------------------

    if [ "$UPDATE" -eq 1 ]; then
        cp "$asm_file" "$golden"
        asm_status="UPDATED"
    elif [ ! -f "$golden" ]; then
        echo "FAIL ($prname): no golden at $golden (run --update?)"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    elif cmp -s "$asm_file" "$golden"; then
        asm_status="PASS"
    else
        echo "FAIL ($prname): asm differs from golden"
        diff "$golden" "$asm_file" | head -20 | sed 's/^/    /'
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    # simulation phase -----------------------------------------------------

    if [ "$NO_SIM" -eq 1 ] || build_skipped "$prname"; then
        echo "$asm_status ($prname)  [sim skipped]$size_msg"
        pass=$((pass + 1))
        continue
    fi

    # appcomp pre-processes the .asm in place
    if ! "$APPCOMP" -en -i "$asm_file" -t "$tmp" >/dev/null 2>&1; then
        echo "FAIL ($prname): appcomp exited non-zero"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    # asmcomp generates <prname>.v, .mif and <prname>_tb.v
    if ! "$ASMCOMP" -en -i "$asm_file" -p "$work_proc" -d "$HDL" -m "$MACROS" -t "$tmp" -f 100 -c 100000 >/dev/null 2>&1; then
        echo "FAIL ($prname): asmcomp exited non-zero"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    # Procs whose real testbench lives at the project level (DTW) - build their
    # .v/.mif here so the project pass below can link them, but stop short of
    # standalone iverilog/vvp (would link the wrong testbench).
    if sim_skipped "$prname"; then
        echo "$asm_status ($prname)  [standalone sim skipped]$size_msg"
        pass=$((pass + 1))
        continue
    fi

    uproc="$work_proc/Hardware/$prname"
    tb="$tmp/${prname}_tb.v"   # asmcomp drops the auto-testbench in the temp dir
    if [ ! -s "$uproc.v" ] || [ ! -s "$tb" ]; then
        echo "FAIL ($prname): asmcomp artifacts missing"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    # iverilog: HDL core + generated processor .v + auto-testbench
    if ! "$IVERILOG" -s "${prname}_tb" -o "$tmp/$prname.vvp" \
            "$tb" "$uproc.v" \
            "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
            "$HDL/core.v" "$HDL/ula.v" >/dev/null 2>&1; then
        echo "FAIL ($prname): iverilog exited non-zero"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    # vvp runs in tmp/ - .mif files must live there for $readmemh
    cp "${uproc}_data.mif" "${uproc}_inst.mif" "$tmp/"
    pushd "$tmp" >/dev/null
    "$VVP" "$tmp/$prname.vvp" >/dev/null 2>&1
    vvp_status=$?
    popd >/dev/null
    if [ $vvp_status -ne 0 ]; then
        echo "FAIL ($prname): vvp exited non-zero"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    # collect output_*.txt produced by the testbench
    sim_dir="$work_proc/Simulation"
    golden_sim="$GOLDEN_SIM_DIR/$prname"

    out_files=("$sim_dir"/output_*.txt)
    if [ "${#out_files[@]}" -eq 0 ] || [ ! -e "${out_files[0]}" ]; then
        echo "FAIL ($prname): testbench produced no output_*.txt"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    if [ "$UPDATE" -eq 1 ]; then
        rm -rf "$golden_sim"
        mkdir -p "$golden_sim"
        for f in "${out_files[@]}"; do cp "$f" "$golden_sim/"; done
        echo "$asm_status ($prname)  [sim UPDATED, ${#out_files[@]} file(s)]$size_msg"
        pass=$((pass + 1))
        continue
    fi

    if [ ! -d "$golden_sim" ]; then
        echo "FAIL ($prname): no sim golden at $golden_sim (run --update?)"
        fail=$((fail + 1)); failed_names+=("$prname"); continue
    fi

    sim_fail=0
    for f in "${out_files[@]}"; do
        base="$(basename "$f")"
        if ! cmp -s "$f" "$golden_sim/$base"; then
            echo "FAIL ($prname): $base differs from sim golden"
            sim_fail=1
        fi
    done
    if [ $sim_fail -eq 0 ]; then
        echo "$asm_status ($prname)  [sim OK]$size_msg"
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("$prname")
    fi
done

# ---- 3. project pass: multi-proc DTW with top-level testbench --------------

if [ "$NO_SIM" -eq 0 ]; then
    proj="DTW"
    proj_top="$ROOT/Exemplos/$proj/TopLevel"
    proj_tmp="$TMP_DIR/$proj"
    golden_proj="$GOLDEN_SIM_DIR/$proj"
    procs=("ProcDTW" "ZeroCross")

    rm -rf "$proj_tmp"; mkdir -p "$proj_tmp"

    # collect each proc's .v + .mif into the project scratch dir
    project_ok=1
    proc_vs=()
    for p in "${procs[@]}"; do
        if [ ! -s "$WORK_DIR/$p/Hardware/$p.v" ]; then
            echo "FAIL ($proj): $p artifacts missing - did its build pass?"
            project_ok=0; break
        fi
        proc_vs+=("$WORK_DIR/$p/Hardware/$p.v")
        cp "$WORK_DIR/$p/Hardware/$p.v"          "$proj_tmp/"
        cp "$WORK_DIR/$p/Hardware/${p}_data.mif" "$proj_tmp/"
        cp "$WORK_DIR/$p/Hardware/${p}_inst.mif" "$proj_tmp/"
    done

    if [ "$project_ok" -eq 1 ]; then
        # input .txt for the testbench ($fopen uses relative paths)
        cp "$proj_top"/*.txt "$proj_tmp/" 2>/dev/null || true

        # iverilog: HDL core + proc .v files + every .v under TopLevel/
        if ! "$IVERILOG" -s top_level_tb -o "$proj_tmp/$proj.vvp" \
                "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                "$HDL/core.v" "$HDL/ula.v" "$HDL/myFIFO.v" \
                "${proc_vs[@]}" \
                "$proj_top"/*.v >/dev/null 2>&1; then
            echo "FAIL ($proj): iverilog exited non-zero"
            fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
        fi
    fi

    if [ "$project_ok" -eq 1 ]; then
        pushd "$proj_tmp" >/dev/null
        "$VVP" "$proj_tmp/$proj.vvp" >/dev/null 2>&1
        vvp_status=$?
        popd >/dev/null
        if [ $vvp_status -ne 0 ]; then
            echo "FAIL ($proj): vvp exited non-zero"
            fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
        fi
    fi

    if [ "$project_ok" -eq 1 ]; then
        out_files=("$proj_tmp"/output_*.txt)
        if [ "${#out_files[@]}" -eq 0 ] || [ ! -e "${out_files[0]}" ]; then
            echo "FAIL ($proj): testbench produced no output_*.txt"
            fail=$((fail + 1)); failed_names+=("$proj")
        elif [ "$UPDATE" -eq 1 ]; then
            rm -rf "$golden_proj"; mkdir -p "$golden_proj"
            for f in "${out_files[@]}"; do cp "$f" "$golden_proj/"; done
            echo "UPDATED ($proj)  [sim UPDATED, ${#out_files[@]} file(s)]"
            pass=$((pass + 1))
        elif [ ! -d "$golden_proj" ]; then
            echo "FAIL ($proj): no sim golden at $golden_proj (run --update?)"
            fail=$((fail + 1)); failed_names+=("$proj")
        else
            sim_fail=0
            for f in "${out_files[@]}"; do
                base="$(basename "$f")"
                if ! cmp -s "$f" "$golden_proj/$base"; then
                    echo "FAIL ($proj): $base differs from sim golden"
                    sim_fail=1
                fi
            done
            if [ $sim_fail -eq 0 ]; then
                echo "PASS ($proj)  [sim OK]"
                pass=$((pass + 1))
            else
                fail=$((fail + 1)); failed_names+=("$proj")
            fi
        fi
    fi
fi

echo ""
echo "===== $pass passed, $fail failed ====="
if [ "$fail" -ne 0 ]; then
    echo "failed: ${failed_names[*]}"
    exit 1
fi

# write the new baseline if --update-size was requested
if [ "$UPDATE_SIZE" -eq 1 ]; then
    {
        echo "# num_ins ratchet: each line is \"<prname> <count>\"."
        echo "# Captures the number of real instructions cmmcomp emits per example"
        echo "# (excludes labels, directives and macros). regress.sh fails any run where"
        echo "# a count grows. To intentionally update after a refactor that shrinks"
        echo "# things, run: Scripts/regress.sh --update-size"
        # bash's `set -u` raises "unbound variable" on ${assoc[$key]} even
        # when the key is in ${!assoc[@]} (seen on bash 5.2). Build the
        # output lines first, then sort, then print - keeps the lookup
        # inside a single tight loop and avoids unrelated re-expansion.
        if [ "${#SIZE_CURRENT[@]}" -gt 0 ]; then
            set +u
            lines=""
            for name in "${!SIZE_CURRENT[@]}"; do
                lines+=$(printf "%-10s %s" "$name" "${SIZE_CURRENT[$name]}")$'\n'
            done
            set -u
            printf "%s" "$lines" | sort
        fi
    } > "$SIZE_BASELINE_FILE"
    echo ""
    echo "===== size baseline rewritten ($SIZE_BASELINE_FILE) ====="
fi
exit 0
