#!/usr/bin/env bash
# CNIPS/regress.sh - regression test for the CNIPS C compiler pipeline.
#
# For every CNIPS/examples/test*.c it runs the full toolchain
#   cnipspp -> cnips -> appcomp -> asmcomp -> iverilog -> vvp
# and compares the simulation output against CNIPS/Testes/golden/<name>.txt.
#
# Usage (from the repo root, in msys2/git-bash on Windows):
#   bash CNIPS/regress.sh              check against goldens (exit 0 = pass)
#   bash CNIPS/regress.sh --update     regenerate goldens (review the diff!)
#   bash CNIPS/regress.sh --skip-build reuse binaries already in CNIPS/.bin

set -uo pipefail

if [ -z "${TMPDIR:-}" ] && [ -z "${TMP:-}" ] && [ -z "${TEMP:-}" ]; then
    export TMPDIR=/tmp
fi

UPDATE=0
SKIP_BUILD=0
for arg in "$@"; do
    case "$arg" in
        --update)     UPDATE=1 ;;
        --skip-build) SKIP_BUILD=1 ;;
        -h|--help) sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "regress: unknown option: $arg" >&2; exit 2 ;;
    esac
done

# locate the repo root from this script's location
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

CNIPS="$ROOT/CNIPS"
BIN="$CNIPS/.bin"
WORK="$CNIPS/.work"
GOLDEN="$CNIPS/Testes/golden"
HDL="$ROOT/HDL"
MACROS="$ROOT/Macros"

# YANC target defaults baked into cnips.exe (overridable per-source via #pragma)
: "${CFG_NUBITS:=16}"; : "${CFG_NBMANT:=10}"; : "${CFG_NBEXPO:=5}"
: "${CFG_NUGAIN:=128}"; : "${CFG_NDSTAC:=8}"; : "${CFG_SDEPTH:=8}"
: "${CFG_NUIOIN:=1}";   : "${CFG_NUIOOU:=1}"; : "${CFG_FFTSIZ:=3}"
DEFS="-DCFG_NUBITS=$CFG_NUBITS -DCFG_NBMANT=$CFG_NBMANT -DCFG_NBEXPO=$CFG_NBEXPO \
      -DCFG_NUGAIN=$CFG_NUGAIN -DCFG_NDSTAC=$CFG_NDSTAC -DCFG_SDEPTH=$CFG_SDEPTH \
      -DCFG_NUIOIN=$CFG_NUIOIN -DCFG_NUIOOU=$CFG_NUIOOU -DCFG_FFTSIZ=$CFG_FFTSIZ"

: "${IVERILOG:=/c/nipscern/Aurora/components/Packages/iverilog/bin/iverilog.exe}"
: "${VVP:=/c/nipscern/Aurora/components/Packages/iverilog/bin/vvp.exe}"

CNIPSPP="$BIN/cnipspp.exe"
CNIPSC="$BIN/cnips.exe"
APPCOMP="$BIN/appcomp.exe"
ASMCOMP="$BIN/asmcomp.exe"

# ---- 1. build the four binaries -------------------------------------------

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "==> building cnipspp + cnips + appcomp + asmcomp"
    mkdir -p "$BIN"
    rm -f "$CNIPSPP" "$CNIPSC" "$APPCOMP" "$ASMCOMP"
    set -e

    # cnipspp (standalone preprocessor)
    gcc -O2 -Wall -o "$CNIPSPP" "$CNIPS/Sources/cnipspp.c"

    # cnips (flex/bison compiler)
    pushd "$CNIPS/Sources" >/dev/null
    bison -y -d CNIPSComp.y
    flex CNIPSComp.l
    gcc -O2 -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function \
        $DEFS -o "$CNIPSC" \
        main.c messages.c types.c symtab.c ast.c codegen.c lex.yy.c y.tab.c
    rm -f lex.yy.c y.tab.c y.tab.h
    popd >/dev/null

    # appcomp
    pushd "$ROOT/APP/Sources" >/dev/null
    flex -o app.c app.l
    gcc -O2 -Wall -o "$APPCOMP" app.c eval.c variaveis.c messages.c args.c
    rm -f app.c
    popd >/dev/null

    # asmcomp
    pushd "$ROOT/ASM/Sources" >/dev/null
    flex -o ASMComp.c ASMComp.l
    gcc -O2 -Wall -o "$ASMCOMP" \
        ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c \
        simulacao.c array.c messages.c args.c
    rm -f ASMComp.c
    popd >/dev/null
    set +e
fi

# ---- 2. run each example ---------------------------------------------------

mkdir -p "$WORK" "$GOLDEN"
pass=0; fail=0; failed=()

shopt -s nullglob
for src in "$CNIPS"/examples/test*.c; do
    base="$(basename "$src" .c)"

    # processor/program name comes from #pragma yanc prname, else the basename
    prname="$(grep -oE '#pragma[ \t]+yanc[ \t]+prname[ \t]+[A-Za-z0-9_]+' "$src" | awk '{print $NF}' | head -1)"
    [ -z "$prname" ] && prname="$base"

    proc="$WORK/$base"
    tmp="$WORK/$base/_tmp"
    rm -rf "$proc"
    mkdir -p "$proc/Software" "$proc/Hardware" "$proc/Simulation" "$tmp"

    asm="$proc/Software/$prname.asm"

    if ! "$CNIPSPP" -i "$src" -o "$tmp/pp.c" >/dev/null 2>&1; then
        echo "FAIL ($base): cnipspp"; fail=$((fail+1)); failed+=("$base"); continue
    fi
    if ! "$CNIPSC" -i "$tmp/pp.c" -o "$asm" -t "$tmp" >/dev/null 2>&1; then
        echo "FAIL ($base): cnips"; fail=$((fail+1)); failed+=("$base"); continue
    fi
    if ! "$APPCOMP" -en -i "$asm" -t "$tmp" >/dev/null 2>&1; then
        echo "FAIL ($base): appcomp"; fail=$((fail+1)); failed+=("$base"); continue
    fi
    if ! "$ASMCOMP" -en -i "$asm" -p "$proc" -d "$HDL" -m "$MACROS" -t "$tmp" -f 100 -c 200000 >/dev/null 2>&1; then
        echo "FAIL ($base): asmcomp"; fail=$((fail+1)); failed+=("$base"); continue
    fi

    uproc="$proc/Hardware/$prname"
    tb="$tmp/${prname}_tb.v"
    if [ ! -s "$uproc.v" ] || [ ! -s "$tb" ]; then
        echo "FAIL ($base): asmcomp artifacts missing"; fail=$((fail+1)); failed+=("$base"); continue
    fi

    if ! "$IVERILOG" -s "${prname}_tb" -o "$tmp/$prname.vvp" \
            "$tb" "$uproc.v" \
            "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
            "$HDL/core.v" "$HDL/ula.v" >/dev/null 2>&1; then
        echo "FAIL ($base): iverilog"; fail=$((fail+1)); failed+=("$base"); continue
    fi

    cp "${uproc}_data.mif" "${uproc}_inst.mif" "$tmp/" 2>/dev/null
    pushd "$tmp" >/dev/null
    "$VVP" "$tmp/$prname.vvp" >/dev/null 2>&1
    popd >/dev/null

    out="$proc/Simulation/output_0.txt"
    if [ ! -f "$out" ]; then
        echo "FAIL ($base): no simulation output"; fail=$((fail+1)); failed+=("$base"); continue
    fi

    gold="$GOLDEN/$base.txt"
    if [ "$UPDATE" -eq 1 ]; then
        cp "$out" "$gold"
        echo "UPDATED ($base): $(tr '\n' ' ' < "$out")"
        pass=$((pass+1))
    elif [ ! -f "$gold" ]; then
        echo "FAIL ($base): no golden at $gold (run --update?)"; fail=$((fail+1)); failed+=("$base")
    elif cmp -s "$out" "$gold"; then
        echo "PASS ($base): $(tr '\n' ' ' < "$out")"
        pass=$((pass+1))
    else
        echo "FAIL ($base): output differs from golden"
        echo "    golden: $(tr '\n' ' ' < "$gold")"
        echo "    got:    $(tr '\n' ' ' < "$out")"
        fail=$((fail+1)); failed+=("$base")
    fi
done

echo ""
echo "===== $pass passed, $fail failed ====="
if [ "$fail" -ne 0 ]; then
    echo "failed: ${failed[*]}"
    exit 1
fi
exit 0
