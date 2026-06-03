#!/usr/bin/env bash
# Scripts/regress.sh - single regression suite for the whole repo.
#
# Runs both compiler pipelines off a shared set of binaries:
#
#   1. CMM phase: for every Compilers/CMMComp/Tests/<prname>/Software/<prname>.cmm
#      run cmmcomp and compare the produced .asm against
#      Compilers/CMMComp/Tests/<prname>/golden.asm. For the sim-enabled subset also
#      run appcomp + asmcomp + iverilog + vvp and compare output_*.txt
#      against Compilers/Compilers/CMMComp/Tests/<prname>/golden_sim/. Plus a project pass for
#      the multi-proc DTW example (Compilers/CMMComp/Tests/DTW/TopLevel/).
#
#   2. CPP phase: for every Compilers/CPPComp/Tests/testN/Software/testN.cpp run
#      cpppp -> cppcomp -> appcomp -> asmcomp -> iverilog (or Verilator
#      if testN/Software/testN.in is present) and compare the resulting
#      Simulation/output_0.txt against Compilers/CPPComp/Tests/testN/golden.txt.
#      Tests follow the cmmcomp pipeline layout: <proc>/Software/ for
#      source, <proc>/Hardware/ for the generated .v/.mif, and
#      <proc>/Simulation/ for the sim output (all three are created on
#      demand by cppcomp + asmcomp).
#
#   3. CMM negative phase: for every fixture in Compilers/CMMComp/NegTests/
#      (listed in NegTests/manifest.txt) run cmmcomp and assert it REJECTS the
#      malformed program with a clean non-zero exit and the expected diagnostic
#      -- never a crash, never a silent accept. Error-path coverage the golden
#      phases don't give. (Runs with the CMM phase; skipped by --cpp-only.)
#
# Build phase builds all 5 binaries once (cmmcomp + cpppp + cppcomp +
# appcomp + asmcomp), all phases reuse them.
#
# Usage (from repo root, in msys2/git-bash on Windows):
#   Scripts/regress.sh                check against goldens (exit 0 = pass)
#   Scripts/regress.sh --update       regenerate goldens (review diff!)
#   Scripts/regress.sh --update-size  ratchet down Compilers/CMMComp/Tests/size_baseline.txt
#   Scripts/regress.sh --skip-build   reuse binaries already in .smoke/bin
#   Scripts/regress.sh --no-sim       skip every simulation step
#   Scripts/regress.sh --cmm-only     skip the CPP phase
#   Scripts/regress.sh --cpp-only     skip the CMM phase

set -uo pipefail

# Force TMP/TEMP to a writable Windows path. mingw g++ -- invoked deep
# inside Verilator's makefile (verilator -> make -> g++) -- needs a
# Windows-style temp dir for its intermediate .s files; otherwise it
# silently falls back to C:\WINDOWS\ where unprivileged users can't write
# ("Cannot create temporary file in C:\WINDOWS\: Permission denied").
# Setting all three unconditionally is the only thing that survives the
# PowerShell -> bash -> make -> g++ env-inheritance chain.
export TMP="C:/packs/msys64/tmp"
export TEMP="C:/packs/msys64/tmp"
export TMPDIR="C:/packs/msys64/tmp"
# Create the dir via its POSIX form. MSYS mkdir reads the bare drive prefix
# "C:" as a *relative* directory name and would leave a junk "./C:" folder
# (the ':' shown as a box) in the repo root on every run -- cygpath turns the
# Windows path into the mount path (here /tmp) so the real dir is used.
mkdir -p "$(cygpath -u "$TMP" 2>/dev/null || echo "$TMP")"

UPDATE=0
UPDATE_SIZE=0
SKIP_BUILD=0
NO_SIM=0
CMM_ONLY=0
CPP_ONLY=0
for arg in "$@"; do
    case "$arg" in
        --update)       UPDATE=1 ;;
        --update-size)  UPDATE_SIZE=1 ;;
        --skip-build)   SKIP_BUILD=1 ;;
        --no-sim)       NO_SIM=1 ;;
        --cmm-only)     CMM_ONLY=1 ;;
        --cpp-only)     CPP_ONLY=1 ;;
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

# Resolve repo root from the script's own location so it works no matter
# where you invoke it from.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

SCRATCH="$ROOT/.smoke"
BIN_DIR="$SCRATCH/bin"
WORK_DIR="$SCRATCH/work"          # CMM phase scratch
WORK_CPP="$SCRATCH/work_cpp"      # CPP phase scratch
TMP_DIR="$SCRATCH/tmp"            # CMM phase tmps

CMMCOMP="$BIN_DIR/cmmcomp.exe"
CPPPP="$BIN_DIR/cpppp.exe"
CPPC="$BIN_DIR/cppcomp.exe"
APPCOMP="$BIN_DIR/appcomp.exe"
ASMCOMP="$BIN_DIR/asmcomp.exe"
MACROS="$ROOT/Compilers/CMMComp/Includes"
HDL="$ROOT/HDL"
CMM_ROOT="$ROOT/Compilers/CMMComp"
SIZE_BASELINE_FILE="$CMM_ROOT/Tests/size_baseline.txt"

CPP_ROOT="$ROOT/Compilers/CPPComp"
SIMMAIN="$CPP_ROOT/Tests/Verilator/sim_main.cpp"

# CPPComp build flags (target params burned into cppcomp.exe). Override
# via env when building a non-default-target binary.
: "${CFG_NUBITS:=32}"; : "${CFG_NBMANT:=23}"; : "${CFG_NBEXPO:=8}"
: "${CFG_NUGAIN:=128}"; : "${CFG_NDSTAC:=128}"; : "${CFG_SDEPTH:=128}"
: "${CFG_NUIOIN:=1}";   : "${CFG_NUIOOU:=1}"; : "${CFG_FFTSIZ:=3}"
: "${CFG_HEAPSZ:=2048}"
CPP_DEFS="-DCFG_NUBITS=$CFG_NUBITS -DCFG_NBMANT=$CFG_NBMANT -DCFG_NBEXPO=$CFG_NBEXPO \
          -DCFG_NUGAIN=$CFG_NUGAIN -DCFG_NDSTAC=$CFG_NDSTAC -DCFG_SDEPTH=$CFG_SDEPTH \
          -DCFG_NUIOIN=$CFG_NUIOIN -DCFG_NUIOOU=$CFG_NUIOOU -DCFG_FFTSIZ=$CFG_FFTSIZ \
          -DCFG_HEAPSZ=$CFG_HEAPSZ"

# Resolve iverilog / vvp / verilator the same way the runners do: Scripts/env.sh
# loads the setup.sh cache (tools.local.sh) and falls back to a PATH lookup, so
# there are no machine-specific hardcoded tool paths here. A pre-set env var
# (e.g. CI, or a non-default Verilator) still wins.
. "$ROOT/Scripts/env.sh"

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

# Examples to exclude from the standalone simulation phase (still run cmmcomp
# .asm compare AND appcomp/asmcomp so their .v + .mif are available for any
# downstream project-level link):
#   procBlind/procBlindOpt - take too long to simulate (heavy FISTA)
#   sw_test            - synthetic .cmm fixture; testbench would loop on while(1)
#   ArcTan/Seno/Sqrt   - Math benchmarks: compute internally with no out(...) calls
#   ProcDTW/ZeroCross  - real testbench lives in the multi-proc DTW project pass
#   cmm_define         - synthetic #define fixture; while(1) testbench loop,
#                        asm-golden compare is enough to lock the lexer expansion
SIM_SKIP=("procBlind" "procBlindOpt" "ArcTan" "Seno" "Sqrt" "ProcDTW" "ZeroCross" "cmm_define")

# Examples to also skip the appcomp/asmcomp build for (their .v / .mif are
# never consumed elsewhere - no standalone sim, no project link).
BUILD_SKIP=("procBlind" "procBlindOpt" "ArcTan" "Seno" "Sqrt" "cmm_define")
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

# ---- 1. build all 5 binaries -----------------------------------------------

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "==> building cmmcomp + cpppp + cppcomp + appcomp + asmcomp"
    mkdir -p "$BIN_DIR"

    # Wipe stale binaries first: a silently-failing gcc must NOT leave the
    # previous build in place to fool the regress into testing dead code.
    rm -f "$CMMCOMP" "$CPPPP" "$CPPC" "$APPCOMP" "$ASMCOMP"

    # Per-binary `set -e` so a build error aborts the script. Without it,
    # a gcc -Werror failure would print to the terminal but the regress
    # would happily continue and run all examples against the (now-missing)
    # binary, hiding the real problem.
    set -e

    pushd "$ROOT/Compilers/CMMComp/Sources" >/dev/null
    bison -y -d CMMComp.y
    flex CMMComp.l
    gcc -O2 -Wall -Werror -o "$CMMCOMP" \
        ast.c data_assign.c data_declar.c data_use.c itr.c diretivas.c \
        funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c \
        variaveis.c array_index.c global.c macros.c messages.c args.c \
        y.tab.c -lm
    rm -f lex.yy.c y.tab.c y.tab.h
    popd >/dev/null

    gcc -O2 -Wall -o "$CPPPP" "$CPP_ROOT/Sources/cpppp.c" -lm

    pushd "$CPP_ROOT/Sources" >/dev/null
    bison -y -d CPPComp.y
    flex CPPComp.l
    gcc -O2 -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function \
        $CPP_DEFS -o "$CPPC" \
        main.c messages.c types.c symtab.c ast.c codegen.c lex.yy.c y.tab.c -lm
    rm -f lex.yy.c y.tab.c y.tab.h
    popd >/dev/null

    pushd "$ROOT/Compilers/APPComp/Sources" >/dev/null
    flex -o app.c app.l
    gcc -O2 -Wall -Werror -o "$APPCOMP" \
        app.c eval.c variaveis.c messages.c args.c -lm
    rm -f app.c
    popd >/dev/null

    pushd "$ROOT/Compilers/ASMComp/Sources" >/dev/null
    flex -o ASMComp.c ASMComp.l
    gcc -O2 -Wall -Werror -o "$ASMCOMP" \
        ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c \
        hdl.c simulacao.c array.c messages.c args.c -lm
    rm -f ASMComp.c
    popd >/dev/null

    set +e
fi

mkdir -p "$WORK_DIR" "$WORK_CPP" "$TMP_DIR"

# Accumulators for the combined summary at the end.
pass=0
fail=0
failed_names=()

# ---- 2. CMM phase ----------------------------------------------------------

if [ "$CPP_ONLY" -eq 0 ]; then
    echo ""
    echo "==> CMM phase"

    # Each test lives in its own folder under Compilers/CMMComp/Tests/<prname>/,
    # mirroring the Compilers/CPPComp/Tests/ layout: Software/<prname>.cmm is the
    # entry, golden.asm + golden_sim/ live alongside. The multi-proc DTW
    # and PulseSim entries (project-pass wrappers without a top-level .cmm)
    # are filtered out by the Software/<prname>.cmm existence check below.
    shopt -s globstar nullglob
    cmm_files=("$CMM_ROOT"/Tests/*/Software/*.cmm)
    IFS=$'\n' cmm_sorted=($(printf '%s\n' "${cmm_files[@]}" | sort))

    for cmm in "${cmm_sorted[@]}"; do
        proc_dir_rel="$(dirname "$(dirname "$cmm")")"
        prname="$(basename "$proc_dir_rel")"
        filename="$(basename "$cmm")"

        work_proc="$WORK_DIR/$prname"
        rm -rf "$work_proc"
        cp -r "$proc_dir_rel" "$work_proc"

        tmp="$TMP_DIR/$prname"
        rm -rf "$tmp"
        mkdir -p "$tmp"

        asm_file="$work_proc/Software/$prname.asm"
        golden="$proc_dir_rel/golden.asm"

        # cmmcomp step ------------------------------------------------------
        if ! "$CMMCOMP" -en -i "$filename" -n "$prname" -p "$work_proc" -m "$MACROS" -t "$tmp" >/dev/null 2>&1; then
            echo "FAIL ($prname): cmmcomp exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi
        [ -s "$asm_file" ] || { echo "FAIL ($prname): asm missing/empty"; fail=$((fail+1)); failed_names+=("$prname"); continue; }

        # size ratchet ------------------------------------------------------
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

        # .asm capture or compare -------------------------------------------
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

        # simulation phase --------------------------------------------------
        if [ "$NO_SIM" -eq 1 ] || build_skipped "$prname"; then
            echo "$asm_status ($prname)  [sim skipped]$size_msg"
            pass=$((pass + 1))
            continue
        fi

        if ! "$APPCOMP" -en -i "$asm_file" -t "$tmp" >/dev/null 2>&1; then
            echo "FAIL ($prname): appcomp exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi
        if ! "$ASMCOMP" -en -i "$asm_file" -p "$work_proc" -d "$HDL" -m "$MACROS" -t "$tmp" -f 100 -c 100000 >/dev/null 2>&1; then
            echo "FAIL ($prname): asmcomp exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        # Procs whose real testbench lives at the project level (DTW): build their
        # .v/.mif here for the project pass to link, skip standalone iverilog.
        if sim_skipped "$prname"; then
            echo "$asm_status ($prname)  [standalone sim skipped]$size_msg"
            pass=$((pass + 1))
            continue
        fi

        uproc="$work_proc/Hardware/$prname"
        tb="$tmp/${prname}_tb.v"
        if [ ! -s "$uproc.v" ] || [ ! -s "$tb" ]; then
            echo "FAIL ($prname): asmcomp artifacts missing"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        if ! "$IVERILOG" -s "${prname}_tb" -o "$tmp/$prname.vvp" \
                "$tb" "$uproc.v" \
                "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                "$HDL/core.v" "$HDL/ula.v" >/dev/null 2>&1; then
            echo "FAIL ($prname): iverilog exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        cp "${uproc}_data.mif" "${uproc}_inst.mif" "$tmp/"
        pushd "$tmp" >/dev/null
        "$VVP" "$tmp/$prname.vvp" >/dev/null 2>&1
        vvp_status=$?
        popd >/dev/null
        if [ $vvp_status -ne 0 ]; then
            echo "FAIL ($prname): vvp exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        sim_dir="$work_proc/Simulation"
        golden_sim="$proc_dir_rel/golden_sim"
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

    # ---- 3. project pass: multi-proc DTW with top-level testbench ----------

    if [ "$NO_SIM" -eq 0 ]; then
        proj="DTW"
        proj_top="$CMM_ROOT/Tests/$proj/TopLevel"
        proj_tmp="$TMP_DIR/$proj"
        golden_proj="$CMM_ROOT/Tests/$proj/golden_sim"
        procs=("ProcDTW" "ZeroCross")

        rm -rf "$proj_tmp"; mkdir -p "$proj_tmp"

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
            cp "$proj_top"/*.txt "$proj_tmp/" 2>/dev/null || true

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
            # No waveform here: the tb gates $dumpvars behind +WAVE, so we run
            # without it. Dumping every signal of this heavy multi-proc sim
            # through the FST writer intermittently crashed vvp on Windows
            # (exit 1, empty stderr) -- a flaky DTW failure -- and the dump is
            # pure overhead since this pass only compares the output_*.txt logs,
            # never the trace. multi_proc (--sim iverilog|verilator) passes +WAVE
            # for the interactive GTKWave flow.
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

                # Correctness anchor: a working DTW emits 459908 on its
                # first DTW output port at @out_en. The byte-equal golden
                # compare above catches regressions only if someone hasn't
                # rerun --update; this assertion stays load-bearing even
                # then, and gives a useful failure message tied to the
                # algorithm rather than to a blob diff.
                anchor_file="$proj_tmp/output_dtw_1.txt"
                anchor_expect="459908"
                anchor_got=$(tr -d '[:space:]' < "$anchor_file" 2>/dev/null)
                if [ "$anchor_got" != "$anchor_expect" ]; then
                    echo "FAIL ($proj): output_dtw_1 expected '$anchor_expect' got '$anchor_got'"
                    sim_fail=1
                fi

                if [ $sim_fail -eq 0 ]; then
                    echo "PASS ($proj)  [sim OK, output_dtw_1=$anchor_expect]"
                    pass=$((pass + 1))
                else
                    fail=$((fail + 1)); failed_names+=("$proj")
                fi
            fi
        fi
    fi
fi

# ---- 4. CPP phase ----------------------------------------------------------

if [ "$CMM_ONLY" -eq 0 ]; then
    echo ""
    echo "==> CPP phase"

    shopt -s nullglob
    for entry in "$CPP_ROOT"/Tests/test*/; do
        base="$(basename "$entry")"
        # Sources live in testN/Software/ to match the cmmcomp pipeline layout.
        src="${entry%/}/Software/$base.cpp"
        if [ ! -f "$src" ]; then
            echo "FAIL ($base): folder layout needs $base/Software/$base.cpp as entry"
            fail=$((fail+1)); failed_names+=("$base"); continue
        fi
        local_inc=("-I" "${entry%/}/Software")

        prname="$(grep -oE '#pragma[ \t]+yanc[ \t]+prname[ \t]+[A-Za-z0-9_]+' "$src" | awk '{print $NF}' | head -1)"
        [ -z "$prname" ] && prname="$base"

        proc="$WORK_CPP/$base"
        tmp="$WORK_CPP/$base/_tmp"
        rm -rf "$proc"
        mkdir -p "$tmp"      # cppcomp -p creates Software/, asmcomp creates Hardware/Simulation/

        asm="$proc/Software/$prname.asm"

        if ! "$CPPPP" -i "$src" -o "$tmp/pp.cpp" -I "$CPP_ROOT/Includes" "${local_inc[@]}" >/dev/null 2>&1; then
            echo "FAIL ($base): cpppp"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi
        # cppcomp -p <proc> writes Software/<prname>.asm into the proc folder,
        # creating Software/ if missing (cmmcomp-style pipeline layout).
        if ! "$CPPC" -i "$tmp/pp.cpp" -p "$proc" -n "$prname" -t "$tmp" >/dev/null 2>&1; then
            echo "FAIL ($base): cppcomp"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi
        if ! "$APPCOMP" -en -i "$asm" -t "$tmp" >/dev/null 2>&1; then
            echo "FAIL ($base): appcomp"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi
        if ! "$ASMCOMP" -en -i "$asm" -p "$proc" -d "$HDL" -m "$MACROS" -t "$tmp" -f 100 -c 5000000 >/dev/null 2>&1; then
            echo "FAIL ($base): asmcomp"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi

        uproc="$proc/Hardware/$prname"
        tb="$tmp/${prname}_tb.v"
        if [ ! -s "$uproc.v" ] || [ ! -s "$tb" ]; then
            echo "FAIL ($base): asmcomp artifacts missing"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi

        out="$proc/Simulation/output_0.txt"
        infile="${entry%/}/Software/$base.in"
        if [ -f "$infile" ]; then
            if [ "$NO_SIM" -eq 1 ]; then
                echo "PASS ($base)  [verilator sim skipped]"
                pass=$((pass+1)); continue
            fi
            # heavy / input-driven test: simulate with Verilator (iverilog too slow).
            # Verilator's make->g++ chain wants env-propagated TMP/TEMP, which
            # the bash->Win32 boundary sometimes strips; delegate to a
            # PowerShell helper which propagates env reliably.
            gold="${entry%/}/golden.txt"
            exp=0; [ -f "$gold" ] && exp=$(grep -c '' "$gold")
            clocks_file="${infile%.in}.clocks"
            clocks=200000000
            [ -f "$clocks_file" ] && clocks=$(cat "$clocks_file" | tr -d '[:space:]')
            # On Windows the PowerShell helper works around bash->make env
            # stripping under MSYS2; elsewhere (Linux) Verilator runs straight
            # from bash -- no env boundary, no hardcoded paths.
            if command -v powershell.exe >/dev/null 2>&1; then
                if ! powershell.exe -ExecutionPolicy Bypass -NoProfile \
                        -File "$CPP_ROOT/Tests/Verilator/run_verilator_step.ps1" \
                        -Prname  "$prname" \
                        -TmpDir  "$tmp" \
                        -UprocV  "$uproc.v" \
                        -SimMain "$SIMMAIN" \
                        -HdlDir  "$HDL" \
                        -InFile  "$infile" \
                        -OutFile "$out" \
                        -Clocks  "$clocks" \
                        -Expected "$exp" >/dev/null 2>&1; then
                    echo "FAIL ($base): verilator"; fail=$((fail+1)); failed_names+=("$base"); continue
                fi
            else
                vldir="$tmp/vl"
                if ! "$VERILATOR" --cc --exe --build --top-module "$prname" --prefix Vtop \
                        -o "sim_$prname" -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN \
                        -Wno-BLKANDNBLK -Wno-WIDTH -Wno-CASEINCOMPLETE -Wno-IMPLICIT \
                        -Wno-COMBDLY --no-timing --Mdir "$vldir" \
                        "$SIMMAIN" "$uproc.v" \
                        "$HDL/processor.v" "$HDL/core.v" "$HDL/ula.v" \
                        "$HDL/addr_dec.v" "$HDL/instr_dec.v" >/dev/null 2>&1 \
                   || [ ! -x "$vldir/sim_$prname" ]; then
                    echo "FAIL ($base): verilator"; fail=$((fail+1)); failed_names+=("$base"); continue
                fi
                # Output is compared separately; the sim's idle-detector can exit
                # non-zero on programs that legitimately stall between out() calls.
                "$vldir/sim_$prname" "$infile" "$out" "$clocks" "$exp" >/dev/null 2>&1 || true
            fi
        else
            if ! "$IVERILOG" -s "${prname}_tb" -o "$tmp/$prname.vvp" \
                    "$tb" "$uproc.v" \
                    "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                    "$HDL/core.v" "$HDL/ula.v" >/dev/null 2>&1; then
                echo "FAIL ($base): iverilog"; fail=$((fail+1)); failed_names+=("$base"); continue
            fi

            cp "${uproc}_data.mif" "${uproc}_inst.mif" "$tmp/" 2>/dev/null
            pushd "$tmp" >/dev/null
            "$VVP" "$tmp/$prname.vvp" >/dev/null 2>&1
            popd >/dev/null
        fi
        if [ ! -f "$out" ]; then
            echo "FAIL ($base): no simulation output"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi

        gold="${entry%/}/golden.txt"
        if [ "$UPDATE" -eq 1 ]; then
            cp "$out" "$gold"
            echo "UPDATED ($base): $(tr '\n' ' ' < "$out")"
            pass=$((pass+1))
        elif [ ! -f "$gold" ]; then
            echo "FAIL ($base): no golden at $gold (run --update?)"; fail=$((fail+1)); failed_names+=("$base")
        elif cmp -s "$out" "$gold"; then
            echo "PASS ($base): $(tr '\n' ' ' < "$out")"
            pass=$((pass+1))
        else
            echo "FAIL ($base): output differs from golden"
            echo "    golden: $(tr '\n' ' ' < "$gold")"
            echo "    got:    $(tr '\n' ' ' < "$out")"
            fail=$((fail+1)); failed_names+=("$base")
        fi
    done
fi

# ---- 4b. CMM negative phase (error reporting) ------------------------------
# Error-path coverage the golden phases lack: each fixture is a malformed
# program that cmmcomp must REJECT with a clean non-zero exit and the right
# diagnostic -- never a crash, never a silent accept. Fixtures live in
# NegTests/ (a sibling of Tests/) so the CI smoke `find ... Tests ... .cmm`
# never tries to compile them.
if [ "$CPP_ONLY" -eq 0 ]; then
    echo ""
    echo "==> CMM negative phase (error reporting)"
    neg_dir="$CMM_ROOT/NegTests"
    neg_work="$SCRATCH/work_neg"
    while IFS='|' read -r fixture expect; do
        fixture="${fixture%$'\r'}"; expect="${expect%$'\r'}"   # tolerate CRLF manifest
        case "$fixture" in ''|\#*) continue ;; esac            # skip blanks/comments
        name="neg:${fixture%.cmm}"
        if [ ! -f "$neg_dir/$fixture" ]; then
            echo "FAIL ($name): fixture missing ($neg_dir/$fixture)"
            fail=$((fail+1)); failed_names+=("$name"); continue
        fi
        rm -rf "$neg_work"; mkdir -p "$neg_work/Software" "$neg_work/tmp"
        cp "$neg_dir/$fixture" "$neg_work/Software/p.cmm"
        out="$("$CMMCOMP" -en -i p.cmm -n p -p "$neg_work" -m "$MACROS" -t "$neg_work/tmp" 2>&1)"
        rc=$?
        if [ "$rc" -eq 0 ]; then
            echo "FAIL ($name): compiled (exit 0) but should have been rejected"
            fail=$((fail+1)); failed_names+=("$name")
        elif [ "$rc" -ge 128 ]; then
            echo "FAIL ($name): crashed (exit $rc) instead of a clean error"
            fail=$((fail+1)); failed_names+=("$name")
        elif ! printf '%s' "$out" | grep -qF "$expect"; then
            echo "FAIL ($name): exit $rc but missing diagnostic '$expect'"
            echo "    got: $(printf '%s' "$out" | head -1)"
            fail=$((fail+1)); failed_names+=("$name")
        else
            echo "PASS ($name): rejected (exit $rc) -- \"$expect\""
            pass=$((pass+1))
        fi
    done < "$neg_dir/manifest.txt"
    rm -rf "$neg_work"
fi

# ---- 5. summary ------------------------------------------------------------

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
        # output lines first, then sort, then print.
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
