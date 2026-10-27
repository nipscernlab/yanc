# Cycle measurements (Scripts/perf)

Instruction count is not cycle count once there are loops: a change can shrink
the assembly and not the run time, or the reverse, and a benchmark written to
isolate a cost overstates the gain (the inliner below). Measure a real program,
in cycles, before calling an optimisation a win.

| file | what |
|---|---|
| `cycles.sh <work dir> <name> <n_outputs> [prof]` | compiles `<work dir>/<name>/<name>.cpp` (output in `<work dir>/<name>/<name>/`) with the toolchain in `.smoke/bin`, builds it under Verilator, prints the instruction count and the cycle of every output; `prof` adds the per-address profile |
| `sim_cyc.cpp` | the Verilator harness (a copy of `Compilers/CPPComp/Tests/Verilator/sim_main.cpp` that prints `CYCLES <n> value <v>` at every output) |
| `sim_prof.cpp` | the same, also counting the cycles the fetch address (`pc_sim_val`, exposed with `+define+YANC_TRACE`) spends on each program word, into `prof.txt` |
| `asm_waste.py <tests dir>` | scans every `golden.asm` for sequences a lean compiler would not emit (reload after SET, jump to a jump, unfused PSH; LOD, ...) |
| `prof_report.py <output dir> <name>` | cycles per function, per source line (through `pc_<name>_mem.txt` and `src_lines.txt`) and per address |

Rebuild the toolchain first, or you measure the OLD compiler:

    bash Scripts/regress.sh --cpp-only --no-sim

The program must read `in(0)` once (Verilator prunes an unread input port and
the harness does not compile). Add it to a COPY, never to the repo's test.
Run everything through the MSYS2 login shell (`$env:MSYSTEM="MINGW64"; bash -lc ...`).

## test46, the reference program

    W=/c/tmp/perf; mkdir -p $W/test46
    cp Compilers/CPPComp/Tests/test46/* $W/test46/
    sed -i 's/^void main(void) {/&\n    in(0);/' $W/test46/test46.cpp
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
| v5.5 and #SHARE, measured at 0b9db20 | 49 678 | 1 613 |
| dead stores of `this` dropped | 49 661 | 1 581 |

The inliner of small accessors (`fdcc070`, then the class-template
`operator[]` and the write path) gave 1.32x on a benchmark of 3232 element
accesses written to isolate it, and 0.13 % on test46 (76 360 -> 76 261):
test46 spends its time in float arithmetic, the linear solve and the
convolution. The zero-fill change right after it gave 15.8 %.

Where test46 spends its cycles at the end (`cycles.sh ... prof`):
`solve_spd` (Cholesky) 35 %, `main` 25 % (the `scratch[2400]` zero-fill),
`inverse_tikhonov_calibrated` 18 %, `sqrt` 11 % (the user's 24-step Newton
loop), `convolve_full` 9 %.

Measured and not done (2026-09-26):
- the backward solve `a[k*n + i]` with a pointer stepped by `n`: it costs
  the same five instructions as `P_LOD k; MLT n; S_ADD` plus the load (`MLT`
  is one cycle like `ADD`). One instruction a turn (18 -> 17) comes only by
  also replacing `b[k]` with a pointer and eliminating `k`, an
  induction-variable elimination in cppcomp, for about 105 cycles (0.2 %);
- the zero-fill at 8 words a turn: about 4.5 instructions a word, for more
  code; fast fill and copy are issue #18, in hardware;
- the Cholesky inner loop is 16 instructions a turn, near the floor of this
  ISA: `k < j` with a variable bound needs `GRE j; LIN; JIZ`.

Declined by Luciano, not to be proposed again: `SHL` for a power-of-two 2D
row size (the multiplier is in most programs anyway).
