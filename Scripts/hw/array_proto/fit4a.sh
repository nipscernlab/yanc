#!/bin/bash
# Step 4a: sapho_all fitted on the DE10-Nano with the array memory off, then on,
# one fit at a time (about an hour each: mains power).
D=$(dirname "$0")
W=$(cd "$D/../../.." && pwd)
A=$W/.smoke/array_proto/arrtest
for mode in off arr; do
    if [ $mode = off ]; then bash "$D/arrtest.sh" sapho_all off; else bash "$D/arrtest.sh" sapho_all; fi || exit 1
    rm -rf "$W/.smoke/work/sapho_all"; mkdir -p "$W/.smoke/work/sapho_all"
    cp -r "$A/sapho_all/sapho_all" "$W/.smoke/work/sapho_all/"
    grep -c 'ARRMEM' "$W/.smoke/work/sapho_all/sapho_all/sapho_all.v"
    date
    TAG=$mode bash "$W/Scripts/hw/fmax.sh" sapho_all 5CSEBA6U23I7 || exit 1
    date
done
