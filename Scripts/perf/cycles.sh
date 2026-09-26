#!/bin/bash
# cycles.sh <work dir> <name> <n_outputs> [prof]
#
# Cycle count of a C++ program on YANC, under Verilator (instruction count is
# not cycle count once there are loops: measure here before calling an
# optimisation a win). <work dir>/<name>/Software/<name>.cpp is compiled with
# the toolchain in .smoke/bin (rebuild it first -- `bash Scripts/regress.sh
# --cpp-only --no-sim` -- or you measure the OLD compiler), built with
# Verilator and run; prints the static instruction count and the cycle of
# every output. With `prof`, the harness also counts the cycles the fetch
# address spends on each program word and prof_report.py prints them per
# function, per source line and per address.
#
# The program must read in(0) once: Verilator prunes an unread input port and
# the harness then fails to compile ("Vtop has no member 'in'"). Add it to a
# COPY of a repo test, never to the test itself. Inputs come from
# <work dir>/in.txt (created with a single 0 if missing).
# Run through the MSYS2 login shell ($env:MSYSTEM="MINGW64"; bash -lc ...).
set -u
R="$(cd "$(dirname "$0")/../.." && pwd)"; B="$R/.smoke/bin"; P="$R/Scripts/perf"
D="$(cd "$1" && pwd)"; v="$2"; nout="${3:-1}"; mode="${4:-}"
p="$D/$v"
[ -f "$D/in.txt" ] || echo 0 > "$D/in.txt"
rm -rf "$p/proc" "$p/vl"; mkdir -p "$p/proc/Software"
cd "$R"
"$B/cpppp.exe"   -i "$p/Software/$v.cpp" -o "$p/pp.cpp" -I Compilers/CPPComp/Includes -I "$p/Software" || exit 1
"$B/cppcomp.exe" -i "$p/pp.cpp" -p "$p/proc" -n "$v" -t "$p" > /dev/null || exit 1
a="$p/proc/Software/$v.asm"
echo "instructions: $(grep -vc '^#' "$a")"
"$B/appcomp.exe" -en -i "$a" -t "$p" > /dev/null 2>&1 || { echo "appcomp failed"; exit 1; }
"$B/asmcomp.exe" -en -i "$a" -p "$p/proc" -d SAPHO -m Compilers/CMMComp/Includes -t "$p" -f 100 -c 5000000 > /dev/null 2>&1 || { echo "asmcomp failed"; exit 1; }
if [ "$mode" = prof ]; then
    sed "s/PCSIG/${v}__DOT__pc_sim_val/" "$P/sim_prof.cpp" > "$p/sim_prof_$v.cpp"
    harness="$p/sim_prof_$v.cpp"; extra="+define+YANC_TRACE --public-flat-rw"
else
    harness="$P/sim_cyc.cpp"; extra=""
fi
verilator --cc --exe --build --top-module "$v" --prefix Vtop -o "sim_$v" $extra \
  -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH \
  -Wno-CASEINCOMPLETE -Wno-IMPLICIT -Wno-COMBDLY --no-timing --Mdir "$p/vl" \
  "$harness" "$p/proc/Hardware/$v.v" \
  SAPHO/processor.v SAPHO/core.v SAPHO/ula.v SAPHO/addr_dec.v SAPHO/instr_dec.v \
  > "$p/vl.log" 2>&1 || { echo "verilator failed (see $p/vl.log)"; exit 1; }
cd "$p"
"$p/vl/sim_$v" "$D/in.txt" "$p/out.txt" 20000000 "$nout" 2>/dev/null | grep CYCLES
if [ "$mode" = prof ]; then python3 "$P/prof_report.py" "$p" "$v"; fi
