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
SKIP_BUILD=0
NO_SIM=0
for arg in "$@"; do
    case "$arg" in
        --update)     UPDATE=1 ;;
        --skip-build) SKIP_BUILD=1 ;;
        --no-sim)     NO_SIM=1 ;;
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

# iverilog / vvp paths can be overridden via env (CI will need different ones)
: "${IVERILOG:=/c/nipscern/Aurora/components/Packages/iverilog/bin/iverilog.exe}"
: "${VVP:=/c/nipscern/Aurora/components/Packages/iverilog/bin/vvp.exe}"

# Examples to exclude from the simulation phase:
#   procBlind          - takes too long to simulate
#   sw_test            - synthetic .cmm fixture; testbench would loop on while(1)
#   ProcDTW/ZeroCross  - multi-proc DTW project, needs top-level wiring
#   ArcTan/Seno/Sqrt   - Math benchmarks: compute internally with no out(...) calls,
#                        so the auto-testbench has no output ports to log
SIM_SKIP=("procBlind" "sw_test" "ProcDTW" "ZeroCross" "ArcTan" "Seno" "Sqrt")

sim_skipped() {
    local name="$1"
    for s in "${SIM_SKIP[@]}"; do [ "$s" = "$name" ] && return 0; done
    return 1
}

# ---- 1. build the three compilers -----------------------------------------

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "==> building cmmcomp + appcomp + asmcomp"
    mkdir -p "$BIN_DIR"

    pushd "$ROOT/CMMComp/Sources" >/dev/null
    bison -y -d CMMComp.y
    flex CMMComp.l
    gcc -O2 -Wall -Werror -o "$CMMCOMP" \
        ast.c emit.c data_assign.c data_declar.c data_use.c itr.c diretivas.c \
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

    if [ "$NO_SIM" -eq 1 ] || sim_skipped "$prname"; then
        echo "$asm_status ($prname)  [sim skipped]"
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
        echo "$asm_status ($prname)  [sim UPDATED, ${#out_files[@]} file(s)]"
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
        echo "$asm_status ($prname)  [sim OK]"
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("$prname")
    fi
done

echo ""
echo "===== $pass passed, $fail failed ====="
if [ "$fail" -ne 0 ]; then
    echo "failed: ${failed_names[*]}"
    exit 1
fi
exit 0
