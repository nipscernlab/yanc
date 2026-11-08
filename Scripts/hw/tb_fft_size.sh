#!/bin/bash
# The FFT's bit-reversed addressing (v[k), ILI) at the edge of the data
# memory (#168), with the programs of Scripts/hw/fft_size/:
#   fit.asm    an 8-word array and #FFTSIZ 3, the whole data memory: it has
#              to elaborate and read v in bit-reversed order (10 14 12 16
#              11 15 13 17); it used to stop Icarus with a reversed
#              part-select
#   short.asm  a 4-word memory and #FFTSIZ 3: asmcomp has to refuse it with
#              its message
# Needs bin/yanc and iverilog. Exit status 1 on any failure.
#
# Usage: bash Scripts/hw/tb_fft_size.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
cp Scripts/hw/fft_size/* "$TMP/"
rc=0
H="$ROOT/SAPHO"
if (cd "$TMP" && "$ROOT/bin/yanc" fit.asm >fit.log 2>&1 \
     && iverilog -s fit_tb -o fit.vvp fit_tb.v fit.v "$H/addr_dec.v" "$H/instr_dec.v" "$H/processor.v" "$H/core.v" "$H/ula.v" 2>>fit.log \
     && vvp fit.vvp >/dev/null 2>&1); then
    got=$(tr -d '\r' < "$TMP/output_0.txt" | tr '\n' ' ')
    if [ "$got" = "10 14 12 16 11 15 13 17 " ]; then echo "tb_fft_size: ok (an FFT array that is the whole memory)"
    else echo "FAIL tb_fft_size: fit.asm read '$got'"; rc=1; fi
else
    echo "FAIL tb_fft_size: fit.asm does not build or elaborate"; tail -5 "$TMP/fit.log"; rc=1
fi
if (cd "$TMP" && YANC_LANG=en "$ROOT/bin/yanc" short.asm >short.log 2>&1); then
    echo "FAIL tb_fft_size: short.asm was accepted"; rc=1
elif grep -q "#FFTSIZ 3 reverses 3 bits" "$TMP/short.log"; then
    echo "tb_fft_size: ok (a memory smaller than the FFT array is refused)"
else
    echo "FAIL tb_fft_size: short.asm refused without the message"; tail -3 "$TMP/short.log"; rc=1
fi
exit $rc
