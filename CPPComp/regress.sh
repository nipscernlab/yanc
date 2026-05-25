#!/usr/bin/env bash
# CPPComp/regress.sh - regression test for the CPPComp C++ compiler pipeline.
#
# For every CPPComp/examples/test*.cpp it runs the full toolchain
#   cpppp -> cppcomp -> appcomp -> asmcomp -> iverilog -> vvp
# and compares the simulation output against CPPComp/Testes/golden/<name>.txt.
#
# Target is fixed: 32-bit word / IEEE-754 single float / sizeof()==1 /
# short&long fold to 32-bit / 4K-word data memory with a dynamic heap.
#
# Usage (from the repo root, in msys2/git-bash on Windows):
#   bash CPPComp/regress.sh              check against goldens (exit 0 = pass)
#   bash CPPComp/regress.sh --update     regenerate goldens (review the diff!)
#   bash CPPComp/regress.sh --skip-build reuse binaries already in CPPComp/.bin

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
        -h|--help) sed -n '2,14p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "regress: unknown option: $arg" >&2; exit 2 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

CPP="$ROOT/CPPComp"
BIN="$CPP/.bin"
WORK="$CPP/.work"
GOLDEN="$CPP/Testes/golden"
HDL="$ROOT/HDL"
MACROS="$ROOT/Macros"

# Fixed CPPComp target: 32-bit / IEEE-754 single (float format derived from
# NUBITS in codegen). Deep data/return stacks since C++ leans on recursion and
# method calls; HEAPSZ reserves the dynamic-allocation arena inside the 4K.
: "${CFG_NUBITS:=32}"; : "${CFG_NBMANT:=23}"; : "${CFG_NBEXPO:=8}"
: "${CFG_NUGAIN:=128}"; : "${CFG_NDSTAC:=128}"; : "${CFG_SDEPTH:=128}"
: "${CFG_NUIOIN:=1}";   : "${CFG_NUIOOU:=1}"; : "${CFG_FFTSIZ:=3}"
: "${CFG_HEAPSZ:=2048}"
DEFS="-DCFG_NUBITS=$CFG_NUBITS -DCFG_NBMANT=$CFG_NBMANT -DCFG_NBEXPO=$CFG_NBEXPO \
      -DCFG_NUGAIN=$CFG_NUGAIN -DCFG_NDSTAC=$CFG_NDSTAC -DCFG_SDEPTH=$CFG_SDEPTH \
      -DCFG_NUIOIN=$CFG_NUIOIN -DCFG_NUIOOU=$CFG_NUIOOU -DCFG_FFTSIZ=$CFG_FFTSIZ \
      -DCFG_HEAPSZ=$CFG_HEAPSZ"

: "${IVERILOG:=/c/nipscern/Aurora/components/Packages/iverilog/bin/iverilog.exe}"
: "${VVP:=/c/nipscern/Aurora/components/Packages/iverilog/bin/vvp.exe}"

# Verilator is used for tests too heavy for iverilog: any examples/<name>.in
# sidecar (an input vector) makes that test compile the synthesizable .v with
# Verilator and drive in()/out() from a small C++ harness instead. Far faster.
: "${VERILATOR:=verilator_bin.exe}"
: "${VERILATOR_ROOT:=C:/packs/msys64/mingw64/share/verilator}"
export VERILATOR_ROOT
# Verilator-generated Makefile invokes `python3 verilator_includer C:/...`.
# msys2 ships TWO python3 binaries: usr/bin/python3 (cygwin, path "/c/...")
# and mingw64/bin/python3 (win32, path "C:/..."). With usr/bin first on PATH
# (msys default), python3 rejects the Windows-style argument as "no such
# file". Prepending mingw64/bin makes python3 resolve to the win32 build
# that does understand `C:/...`. This is the fix that finally lets the
# heavy Verilator-driven tests build on Windows.
export PATH="C:/packs/msys64/mingw64/bin:$PATH"
SIMMAIN="$CPP/Verilator/sim_main.cpp"

CPPPP="$BIN/cpppp.exe"
CPPC="$BIN/cppcomp.exe"
APPCOMP="$BIN/appcomp.exe"
ASMCOMP="$BIN/asmcomp.exe"

# ---- 1. build the four binaries -------------------------------------------

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "==> building cpppp + cppcomp + appcomp + asmcomp"
    mkdir -p "$BIN"
    rm -f "$CPPPP" "$CPPC" "$APPCOMP" "$ASMCOMP"
    set -e

    gcc -O2 -Wall -o "$CPPPP" "$CPP/Sources/cpppp.c"

    pushd "$CPP/Sources" >/dev/null
    bison -y -d CPPComp.y
    flex CPPComp.l
    gcc -O2 -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function \
        $DEFS -o "$CPPC" \
        main.c messages.c types.c symtab.c ast.c codegen.c lex.yy.c y.tab.c
    rm -f lex.yy.c y.tab.c y.tab.h
    popd >/dev/null

    pushd "$ROOT/APP/Sources" >/dev/null
    flex -o app.c app.l
    gcc -O2 -Wall -o "$APPCOMP" app.c eval.c variaveis.c messages.c args.c
    rm -f app.c
    popd >/dev/null

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
# Two test layouts coexist:
#   examples/testN.cpp       - single-file test (most cases)
#   examples/testN/testN.cpp - folder layout for multi-file tests; the entry is
#                              testN/testN.cpp and the directory itself is on
#                              the cpppp include path so the entry can `#include
#                              "..."` its companion .hpp/.cpp files (amalgamation
#                              into one translation unit; there is no linker).
for entry in "$CPP"/examples/test*.cpp "$CPP"/examples/test*/; do
    if [ -d "$entry" ]; then
        base="$(basename "$entry")"
        src="${entry%/}/$base.cpp"
        if [ ! -f "$src" ]; then
            echo "FAIL ($base): folder layout needs $base/$base.cpp as entry"
            fail=$((fail+1)); failed+=("$base"); continue
        fi
        local_inc=("-I" "${entry%/}")
    else
        base="$(basename "$entry" .cpp)"
        src="$entry"
        local_inc=()
    fi

    prname="$(grep -oE '#pragma[ \t]+yanc[ \t]+prname[ \t]+[A-Za-z0-9_]+' "$src" | awk '{print $NF}' | head -1)"
    [ -z "$prname" ] && prname="$base"

    proc="$WORK/$base"
    tmp="$WORK/$base/_tmp"
    rm -rf "$proc"
    mkdir -p "$proc/Software" "$proc/Hardware" "$proc/Simulation" "$tmp"

    asm="$proc/Software/$prname.asm"

    if ! "$CPPPP" -i "$src" -o "$tmp/pp.cpp" -I "$CPP/include" "${local_inc[@]}" >/dev/null 2>&1; then
        echo "FAIL ($base): cpppp"; fail=$((fail+1)); failed+=("$base"); continue
    fi
    if ! "$CPPC" -i "$tmp/pp.cpp" -o "$asm" -t "$tmp" >/dev/null 2>&1; then
        echo "FAIL ($base): cppcomp"; fail=$((fail+1)); failed+=("$base"); continue
    fi
    if ! "$APPCOMP" -en -i "$asm" -t "$tmp" >/dev/null 2>&1; then
        echo "FAIL ($base): appcomp"; fail=$((fail+1)); failed+=("$base"); continue
    fi
    if ! "$ASMCOMP" -en -i "$asm" -p "$proc" -d "$HDL" -m "$MACROS" -t "$tmp" -f 100 -c 5000000 >/dev/null 2>&1; then
        echo "FAIL ($base): asmcomp"; fail=$((fail+1)); failed+=("$base"); continue
    fi

    uproc="$proc/Hardware/$prname"
    tb="$tmp/${prname}_tb.v"
    if [ ! -s "$uproc.v" ] || [ ! -s "$tb" ]; then
        echo "FAIL ($base): asmcomp artifacts missing"; fail=$((fail+1)); failed+=("$base"); continue
    fi

    out="$proc/Simulation/output_0.txt"
    # .in sidecar: examples/<base>.in for single-file tests, or
    # examples/<base>/<base>.in for folder-layout tests.
    if [ -d "$entry" ]; then
        infile="${entry%/}/$base.in"
    else
        infile="$CPP/examples/$base.in"
    fi
    if [ -f "$infile" ]; then
        # heavy / input-driven test: simulate with Verilator (iverilog too slow).
        gold="$GOLDEN/$base.txt"
        exp=0; [ -f "$gold" ] && exp=$(grep -c '' "$gold")
        if ! "$VERILATOR" --cc --exe --build --top-module "$prname" --prefix Vtop \
                -o "sim_$prname" -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN \
                -Wno-BLKANDNBLK -Wno-WIDTH -Wno-CASEINCOMPLETE -Wno-IMPLICIT \
                -Wno-COMBDLY --no-timing --Mdir "$tmp/vl" \
                "$SIMMAIN" "$uproc.v" "$HDL/processor.v" "$HDL/core.v" "$HDL/ula.v" \
                "$HDL/addr_dec.v" "$HDL/instr_dec.v" >/dev/null 2>&1; then
            echo "FAIL ($base): verilator"; fail=$((fail+1)); failed+=("$base"); continue
        fi
        # cycle budget: optional .clocks sidecar next to the .in file lets
        # per-test heavy programs (e.g. blind_deconvolve) override the default.
        # Without a sidecar, 200M is enough for the FISTA-scale tests.
        clocks_file="${infile%.in}.clocks"
        clocks=200000000
        [ -f "$clocks_file" ] && clocks=$(cat "$clocks_file" | tr -d '[:space:]')
        "$tmp/vl/sim_$prname" "$infile" "$out" "$clocks" "$exp" >/dev/null 2>&1
    else
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
    fi
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
