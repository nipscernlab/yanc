# Cycle measurements (Scripts/perf)

Instruction count is not cycle count once there are loops: a change can shrink
the assembly and not the run time, or the reverse, and a benchmark written to
isolate a cost overstates the gain (TODO.md item 14). Measure a real program,
in cycles, before calling an optimisation a win.

| file | what |
|---|---|
| `cycles.sh <work dir> <name> <n_outputs> [prof]` | compiles `<work dir>/<name>/Software/<name>.cpp` with the toolchain in `.smoke/bin`, builds it under Verilator, prints the instruction count and the cycle of every output; `prof` adds the per-address profile |
| `sim_cyc.cpp` | the Verilator harness (a copy of `Compilers/CPPComp/Tests/Verilator/sim_main.cpp` that prints `CYCLES <n> value <v>` at every output) |
| `sim_prof.cpp` | the same, also counting the cycles the fetch address (`pc_sim_val`, exposed with `+define+YANC_TRACE`) spends on each program word, into `prof.txt` |
| `asm_waste.py <tests dir>` | scans every `golden.asm` for sequences a lean compiler would not emit (reload after SET, jump to a jump, unfused PSH; LOD, ...) |
| `prof_report.py <program dir> <name>` | cycles per function, per source line (through `pc_<name>_mem.txt` and `trad_cmm.txt`) and per address |

Rebuild the toolchain first, or you measure the OLD compiler:

    bash Scripts/regress.sh --cpp-only --no-sim

The program must read `in(0)` once (Verilator prunes an unread input port and
the harness does not compile). Add it to a COPY, never to the repo's test.
Run everything through the MSYS2 login shell (`$env:MSYSTEM="MINGW64"; bash -lc ...`).

## test46, the reference program

    W=/c/tmp/perf; mkdir -p $W/test46/Software
    cp Compilers/CPPComp/Tests/test46/Software/* $W/test46/Software/
    sed -i 's/^void main(void) {/&\n    in(0);/' $W/test46/Software/test46.cpp
    bash Scripts/perf/cycles.sh $W test46 4          # cycles
    bash Scripts/perf/cycles.sh $W test46 4 prof     # + profile

History of test46 (the blind-deconvolution inverse filter, 4 outputs):

| state | cycles | instructions |
|---|---|---|
| before 2026-09-24 | 76 360 | 1 754 |
| zero-fill with one index (e342ef6) | 64 184 | 1 740 |
| loop-invariant address hoisting, `base; ADD idx` (4e8eeba) | 60 890 | 1 682 |
| `for` tested at the bottom, literal memory form (a84db8a, 9da5264) | 56 345 | 1 635 |
| zero-fill four words a turn (f614e65) | 51 540 | 1 657 |
| hoist sees by-value calls and lone variables (d838f52) | 49 710 | 1 667 |
| integer constant folding (0e61b60) | 49 684 | 1 628 |
