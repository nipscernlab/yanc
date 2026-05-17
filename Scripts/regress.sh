#!/usr/bin/env bash
# Scripts/regress.sh - byte-equality regression test for cmmcomp.
#
# For every .cmm in Exemplos/, runs cmmcomp and compares the produced
# .asm against the version checked into Testes/golden/. Refactors that
# shouldn't change codegen (e.g. AST migration) must keep this green.
#
# Usage (from repo root, in an msys2/git-bash shell on Windows):
#   Scripts/regress.sh                check against goldens (exit 0 = pass)
#   Scripts/regress.sh --update       regenerate goldens (review diff!)
#   Scripts/regress.sh --skip-build   reuse cmmcomp already in .smoke/bin

set -uo pipefail

# When invoked from a non-interactive shell (e.g. PowerShell calling bash),
# msys2 may not inherit TMP/TEMP and gcc/bison choke on C:\WINDOWS\. Pin
# TMPDIR to a writable location under the scratch dir if nothing usable is set.
if [ -z "${TMPDIR:-}" ] && [ -z "${TMP:-}" ] && [ -z "${TEMP:-}" ]; then
    export TMPDIR=/tmp
fi

UPDATE=0
SKIP_BUILD=0
for arg in "$@"; do
    case "$arg" in
        --update)     UPDATE=1 ;;
        --skip-build) SKIP_BUILD=1 ;;
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

CMMCOMP="$BIN_DIR/cmmcomp.exe"
MACROS="$ROOT/Macros"

# ---- 1. build cmmcomp ------------------------------------------------------

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "==> building cmmcomp"
    mkdir -p "$BIN_DIR"
    pushd "$ROOT/CMMComp/Sources" >/dev/null
    bison -y -d CMMComp.y
    flex CMMComp.l
    gcc -O2 -Wall -Werror -o "$CMMCOMP" \
        data_assign.c data_declar.c data_use.c itr.c diretivas.c \
        funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c \
        variaveis.c array_index.c global.c macros.c messages.c args.c \
        y.tab.c
    rm -f lex.yy.c y.tab.c y.tab.h
    popd >/dev/null
fi

# ---- 2. run on each example and compare to golden --------------------------

mkdir -p "$WORK_DIR" "$TMP_DIR" "$GOLDEN_DIR"

pass=0
fail=0
failed_names=()

# globstar avoids depending on `find` (msys2's find may be shadowed by
# Windows DOS find.exe in PATH when invoked from PowerShell)
shopt -s globstar nullglob
cmm_files=(Exemplos/**/*.cmm)
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

    if ! "$CMMCOMP" -en -i "$filename" -n "$prname" -p "$work_proc" -m "$MACROS" -t "$tmp" >/dev/null 2>&1; then
        echo "FAIL ($prname): cmmcomp exited non-zero"
        fail=$((fail + 1))
        failed_names+=("$prname")
        continue
    fi
    [ -s "$asm_file" ] || { echo "FAIL ($prname): asm missing/empty"; fail=$((fail+1)); failed_names+=("$prname"); continue; }

    if [ "$UPDATE" -eq 1 ]; then
        cp "$asm_file" "$golden"
        echo "UPDATED ($prname)"
        pass=$((pass + 1))
        continue
    fi

    if [ ! -f "$golden" ]; then
        echo "FAIL ($prname): no golden at $golden (run --update?)"
        fail=$((fail + 1))
        failed_names+=("$prname")
        continue
    fi

    if cmp -s "$asm_file" "$golden"; then
        echo "PASS ($prname)"
        pass=$((pass + 1))
    else
        echo "FAIL ($prname): asm differs from golden"
        diff "$golden" "$asm_file" | head -20 | sed 's/^/    /'
        fail=$((fail + 1))
        failed_names+=("$prname")
    fi
done

echo ""
echo "===== $pass passed, $fail failed ====="
if [ "$fail" -ne 0 ]; then
    echo "failed: ${failed_names[*]}"
    exit 1
fi
exit 0
