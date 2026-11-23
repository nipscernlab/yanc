#!/bin/bash
# The random differential test of the whole processor (GitHub issue #32).
#
# For each seed, Scripts/iss/rand_asm.py writes a program, yanc builds its
# processor, and the program runs on the instruction-set simulator
# (Scripts/iss/sapho_iss.c) and on the RTL under Icarus (Scripts/iss/tb_iss.v);
# the two traces, cycle by cycle (the fetch address, every data-memory write,
# input read and output, the depth of both stacks), must be the same. Half the seeds are #ITRAD
# programs, with the interrupt pin raised in one or two cycles the seed picks.
#
# Usage: bash Scripts/iss/iss_diff.sh [-s first_seed] [-n seeds] [-z size]
#                                     [-H sapho_dir] [-k] [-q]
#   -s, -n   seeds first_seed .. first_seed+n-1 (default 1, 50)
#   -z       blocks per program (rand_asm.py --size, default 24)
#   -H       the SAPHO Verilog to simulate (default SAPHO/; a mutant's copy)
#   -k       keep going after the first difference
#   -q       print only the summary
# Needs bin/yanc built (or YANC_BIN set to one), gcc, python and iverilog. Exit status 1 on any
# difference or on a program that does not build or run.
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

S0=1; N=50; SIZE=24; H="$ROOT/SAPHO"; KEEP=0; QUIET=0
while getopts "s:n:z:H:kq" o; do
    case $o in
        s) S0=$OPTARG ;; n) N=$OPTARG ;; z) SIZE=$OPTARG ;;
        H) H=$(cd "$OPTARG" && pwd) ;; k) KEEP=1 ;; q) QUIET=1 ;;
        *) sed -n '2,20p' "$0"; exit 2 ;;
    esac
done

PY=python3; command -v python >/dev/null 2>&1 && python -c "" 2>/dev/null && PY=python
# the yanc to build with: $YANC_BIN (regress.sh passes its own), or bin/yanc
YANC="${YANC_BIN:-$ROOT/bin/yanc}"; [ -x "$YANC" ] || YANC="$YANC.exe"
[ -x "$YANC" ] || { echo "iss_diff: ${YANC_BIN:-bin/yanc} not built (make)"; exit 1; }

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

ISS="$TMP/sapho_iss"
if ! gcc -O2 -Wall -Werror -o "$ISS" Scripts/iss/sapho_iss.c -lm 2>"$TMP/gcc.log"; then
    echo "iss_diff: sapho_iss.c does not build"; cat "$TMP/gcc.log"; exit 1
fi

pass=0; fail=0; failed=()
for ((seed = S0; seed < S0 + N; seed++)); do
    d="$TMP/s$seed"; mkdir -p "$d"
    itr=""; [ $((seed % 2)) -eq 0 ] && itr="--itr"
    if ! $PY -I Scripts/iss/rand_asm.py $seed --size $SIZE $itr > "$d/rnd.asm" 2>"$d/gen.log"; then
        echo "FAIL (seed $seed): rand_asm.py"; cat "$d/gen.log"; fail=$((fail+1)); failed+=($seed)
        [ $KEEP -eq 1 ] && continue || break
    fi
    if ! (cd "$d" && "$YANC" rnd.asm >yanc.log 2>&1); then
        echo "FAIL (seed $seed): the program does not build"; tail -5 "$d/yanc.log"
        fail=$((fail+1)); failed+=($seed); [ $KEEP -eq 1 ] && continue || break
    fi

    # the run's length, and the interrupt cycles inside its first half
    if ! "$ISS" "$d/rnd.v" "$d/rnd_inst.mif" "$d/rnd_data.mif" >/dev/null 2>"$d/iss0.err"; then
        echo "FAIL (seed $seed): sapho_iss"; cat "$d/iss0.err"; fail=$((fail+1)); failed+=($seed)
        [ $KEEP -eq 1 ] && continue || break
    fi
    ncyc=$(awk '/^cycles/ {print $2}' "$d/iss0.err")
    icyc=""; plus=""; idef=""
    if [ -n "$itr" ]; then
        idef="-DITR"
        a=$(( (seed * 7919) % (ncyc / 2 + 1) + 3 ))
        b=$(( (seed * 104729) % (ncyc / 2 + 1) + 3 ))
        # the first pulse lands where it shows the most, every other seed: on
        # an INC (it drops the write) or else where the data stack holds words
        # (it empties it)
        if [ $((seed % 4)) -eq 2 ]; then
            "$ISS" "$d/rnd.v" "$d/rnd_inst.mif" "$d/rnd_data.mif" -h >/dev/null 2>"$d/hot"
            half=$((ncyc / 2))
            hot=$(awk -v h=$half '$1 == "inc" && $2 < h {print $2}' "$d/hot")
            [ -n "$hot" ] || hot=$(awk -v h=$half '$1 == "stk" && $2 < h {print $2}' "$d/hot")
            if [ -n "$hot" ]; then
                set -- $hot
                k=$(( (seed / 4) % $# + 1 )); a=${!k}
            fi
        fi
        icyc="$a"; plus="+ITR0=$a"
        if [ $((seed % 4)) -eq 0 ] && [ $b -ne $a ]; then icyc="$a,$b"; plus="$plus +ITR1=$b"; fi
        # a restart makes the run longer: measure it again with the pulses
        "$ISS" "$d/rnd.v" "$d/rnd_inst.mif" "$d/rnd_data.mif" -i "$icyc" >/dev/null 2>"$d/iss0.err"
        ncyc=$(awk '/^cycles/ {print $2}' "$d/iss0.err")
    fi
    if ! "$ISS" "$d/rnd.v" "$d/rnd_inst.mif" "$d/rnd_data.mif" -n "$ncyc" ${icyc:+-i "$icyc"} \
            > "$d/iss.trace" 2>"$d/iss.err"; then
        echo "FAIL (seed $seed): sapho_iss"; cat "$d/iss.err"; fail=$((fail+1)); failed+=($seed)
        [ $KEEP -eq 1 ] && continue || break
    fi

    nu=$(grep -o '\.NUBITS([0-9]*)' "$d/rnd.v" | grep -o '[0-9]*')
    ni=$(grep -o 'output \[[0-9]*:0\] req_in' "$d/rnd.v" | grep -o '[0-9]*' | head -1)
    no=$(grep -o 'output \[[0-9]*:0\] out_en' "$d/rnd.v" | grep -o '[0-9]*' | head -1)
    cdef=""; grep -q '\.CAL(1)' "$d/rnd.v" && cdef="-DCAL"
    if ! iverilog -g2012 -s tb -DPROC=rnd -DPINST=dut.p_rnd -DNU=$nu -DNI=$((ni+1)) -DNO=$((no+1)) $idef $cdef \
            -o "$d/tb.vvp" Scripts/iss/tb_iss.v "$d/rnd.v" \
            "$H/processor.v" "$H/core.v" "$H/ula.v" "$H/addr_dec.v" "$H/instr_dec.v" 2>"$d/iv.log"; then
        echo "FAIL (seed $seed): elaboration"; head -5 "$d/iv.log"; fail=$((fail+1)); failed+=($seed)
        [ $KEEP -eq 1 ] && continue || break
    fi
    (cd "$d" && vvp -n tb.vvp +N=$ncyc +T=rtl.trace $plus >vvp.log 2>&1)

    # in cycle order, the lines of one cycle in one order on both sides
    LC_ALL=C sort -k1,1n -k2 "$d/iss.trace" > "$d/a"; LC_ALL=C sort -k1,1n -k2 "$d/rtl.trace" > "$d/b" 2>/dev/null
    if cmp -s "$d/a" "$d/b"; then
        pass=$((pass+1))
        [ $QUIET -eq 1 ] || echo "PASS (seed $seed)  [$(sed -n 2p "$d/rnd.asm" | sed 's#// rand_asm.py [0-9]* --size [0-9]*##'), $ncyc cycles${icyc:+, interrupt at $icyc}]"
    else
        fail=$((fail+1)); failed+=($seed)
        # the first difference in cycle order
        first=$(diff "$d/a" "$d/b" | grep '^[<>]' | head -1 | awk '{print $2}')
        echo "FAIL (seed $seed): the ISS and the RTL differ from cycle $first${icyc:+ (interrupt at $icyc)}"
        echo "  ISS:"; awk -v c="$first" '$1 >= c && $1 < c + 3' "$d/a" | sed 's/^/    /'
        echo "  RTL:"; awk -v c="$first" '$1 >= c && $1 < c + 3' "$d/b" | sed 's/^/    /'
        if [ -n "$ISS_KEEP" ]; then mkdir -p "$ISS_KEEP"; cp -r "$d" "$ISS_KEEP/"; echo "  kept in $ISS_KEEP/s$seed"; fi
        [ $KEEP -eq 1 ] || break
    fi
done

echo "iss_diff: $pass passed, $fail failed${failed:+ (seeds ${failed[*]})}"
[ $fail -eq 0 ]
