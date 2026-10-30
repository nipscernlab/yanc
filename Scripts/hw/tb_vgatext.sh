#!/bin/bash
# Runs Scripts/hw/tb_vgatext.v (SAPHO/myVGAText.v) with an 8-bit and a 32-bit
# word (or the widths given: tb_vgatext.sh 8), each twice: the block on
# another clock (LINK_SYNC 0) with the word arriving after T (SKEW 1), and on
# the SAPHO's clock (LINK_SYNC 1). Needs iverilog only. Exit status 1 on any
# mismatch. About 3 minutes a run.
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
rc=0
for N in ${@:-8 32}; do
  for LS in "0 1" "1 0"; do
    set -- $LS
    if iverilog -g2012 -s tb -Ptb.NUBITS=$N -Ptb.LINK_SYNC=$1 -Ptb.SKEW=$2 -o "$TMP/v.vvp" \
                Scripts/hw/tb_vgatext.v SAPHO/myVGAText.v 2>"$TMP/e.log"; then
        out=$(vvp -n "$TMP/v.vvp" | grep -v '\$finish')
        echo "$out"
        case $out in *FAIL*|"") rc=1 ;; esac
    else
        echo "NUBITS=$N LINK_SYNC=$1: ELABORATION FAILED"; head -10 "$TMP/e.log"; rc=1
    fi
  done
done
exit $rc
