#!/bin/bash
# Lints SAPHO with Verilator (-Wall) on sapho_all, the program that builds
# every block it can, with the waivers of Scripts/hw/lint.vlt, each of which
# says why the warning is meant (GitHub issue #35). Exit status 1 on any
# warning left, 2 when Verilator or the tools are missing.
#
# Usage: bash Scripts/hw/lint.sh          (bin/yanc built, verilator on PATH)
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 2
VL=$(command -v verilator_bin || command -v verilator_bin.exe || command -v verilator)
[ -n "$VL" ] || { echo "lint: verilator not found"; exit 2; }
# verilator_bin finds its own include files through VERILATOR_ROOT, which a
# Windows build does not always know (MSYS2 UCRT64: <prefix>/share/verilator)
if [ -z "$VERILATOR_ROOT" ]; then
    vr="$(dirname "$VL")/../share/verilator"
    [ -d "$vr" ] && export VERILATOR_ROOT=$(cd "$vr" && { pwd -W 2>/dev/null || pwd; })
fi
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

if ! bin/yanc -o "$TMP/sapho_all" Compilers/CMMComp/Tests/sapho_all/sapho_all.cmm >"$TMP/yanc.log" 2>&1; then
    echo "lint: sapho_all does not build"; cat "$TMP/yanc.log"; exit 2
fi
H=SAPHO
if "$VL" --lint-only -Wall --top-module sapho_all Scripts/hw/lint.vlt \
        "$TMP/sapho_all/sapho_all.v" $H/processor.v $H/core.v $H/ula.v \
        $H/addr_dec.v $H/instr_dec.v >"$TMP/lint.log" 2>&1; then
    echo "lint: SAPHO is clean (sapho_all, Verilator -Wall, Scripts/hw/lint.vlt)"
    exit 0
fi
grep -E "^%(Warning|Error)" "$TMP/lint.log"
exit 1
