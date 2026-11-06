#!/bin/bash
# Runs Scripts/hw/tb_adv7513text.v (SAPHO/myADV7513Text.v) in the configurations
# given as NUBITS:LINK_SYNC:WAIT_BITS (default: 8:1:13 32:0:11), e.g.
# tb_adv7513text.sh 8:1:11. WAIT_BITS 13 is the block's own, a 262 ms wait
# simulated twice (about 6 minutes); 11 shortens it four times (about 2
# minutes), which the regress uses. Needs iverilog only. Exit status 1 on any
# mismatch.
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
rc=0
for cfg in ${@:-8:1:13 32:0:11}; do
  IFS=: read -r N LS WB <<<"$cfg"
  if iverilog -g2012 -s tb -Ptb.NUBITS=$N -Ptb.LINK_SYNC=$LS -Ptb.WAIT_BITS=$WB -o "$TMP/h.vvp" \
              Scripts/hw/tb_adv7513text.v SAPHO/myADV7513Text.v SAPHO/myVGAText.v 2>"$TMP/e.log"; then
      out=$(vvp -n "$TMP/h.vvp" | grep -v '\$finish')
      echo "$out"
      case $out in *FAIL*|"") rc=1 ;; esac
  else
      echo "NUBITS=$N LINK_SYNC=$LS WAIT_BITS=$WB: ELABORATION FAILED"; head -10 "$TMP/e.log"; rc=1
  fi
done
exit $rc
