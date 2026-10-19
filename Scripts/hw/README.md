# Hardware measurement scripts

Reproducible measurements for changes to `SAPHO/`. None of them is part of the
regress (they need Yosys or Quartus); run them by hand when a change should
not alter behaviour but may alter cost or timing.

All paths assume the repo at `/c/nipscern/yanc` under an **MSYS2 login shell**
(`$env:MSYSTEM='MINGW64'; bash -lc "..."` from PowerShell).

| script | needs | answers |
|---|---|---|
| `elab.sh` | iverilog, verilator | does the ALU still elaborate at every `#FROUND` level, at 32/23/8 and 16/10/5, and lint clean? (float ops, shifts and both dividers) |
| `elab_single.sh` | iverilog | does each float operator still elaborate **alone** (one `generate` branch at a time)? Catches a block that stopped being gated by the opcodes |
| `area.sh` | yosys | LUT4 count and critical-path depth per operator set |
| `fmax.sh` | Quartus Prime Lite | real Fmax and ALMs of a processor the regress already built, its 10 worst register-to-register paths, and the worst one grouped by block (`path_blocks.py`): memory read, operand select, each ALU operator, output mux, jump decision, PC. Time the whole design on `sapho_all` (every block in, `#FROUND 2`) |
| `clock_exp/` (`make_variants.py`, `eq_norm.sh`, `tb_norm.v`) | python3, iverilog, Quartus | the clock experiments of 2026-10-15 (four variants of `SAPHO/` built under `.smoke/hw/clock_exp/`, timed with `HDL=<dir> fmax.sh`), and the proof that the tree leading-zero count gives the same normaliser output; see `clock_exp/README.md` |
| `path_groups.py` | python3 | the worst path of each destination block in a long timing report (`many_paths.txt`, written by `fmax.sh` and `vfmax.sh`): the first thousands of paths can all end at one register bank and hide the next block that limits. To look behind a block, rerun `quartus_sta` on the fitted project with that block's registers as false paths (lines in `docs/design/pipeln-and-division.md` 4.1); no new fit |
| `tb_fdiv.v` | iverilog | the divider array against Verilog's `/` and `%`, 20000 random operands |
| `tb_fifo.v` | iverilog | `SAPHO/myFIFO.v` against what a FIFO between SAPHOs needs: the word of the `wrreq` cycle, a full FIFO refusing a write, a read right after a write. The FIFO before its fix failed all three; the regress `FIFO` pass runs it (`docs/design/myfifo.md`) |
| `tb_dcfifo.v` | iverilog | `SAPHO/myDCFIFO.v`, the FIFO between two clocks: the word of the `wrreq` cycle, LENGTH words fit and the next is refused, a word reaches the reader, 300 words with random pauses on both sides arrive once and in order. Run it both ways round (`WH`/`RH`, the half periods); the regress `FIFO` pass does |
| `width_sweep.sh` (`width_sweep/`) | iverilog, verilator, python3 | does each simulator compute every integer operator right at a given word width, in continuous assigns and in procedural blocks, against a Python-bigint oracle? Re-run after upgrading Icarus or Verilator: it decides when YANC may go beyond 32 bits (`TODO.md` item 6(b)). **Expected non-zero result:** Verilator still reports one `div` mismatch, `INT_MIN / -1` giving `0` instead of the wrapped `INT_MIN`. That is its runtime guarding the host divide trap, and it is a property of the simulator, not of YANC: `ula_div` names the case and forces the wrapped quotient, so the processor agrees on both (item 11(a), `test72`/`test73`) |
| `vfmax.sh` (`vfmax.tcl`) | Vivado (2025.2 ML Standard) | the same as `fmax.sh` on AMD/Xilinx, out of context (no pins), default part the ZYBO's `xc7z010clg400-1`; both memories asked into block RAM in a copy of `SAPHO/`, as Quartus puts them |
| `fmax_link.sh`, `vfmax_link.sh` (`vfmax_link.tcl`, `linkall_top.v`) | Quartus / Vivado | Fmax of two linked processors on two clocks: the LINKALL pair (`sapho_all_a` -> `sapho_all_b` over a synchronized `toma`/`cade` link) in `linkall_top.v`, the top a board would hold, with the two-clock constraint a board needs (`set_clock_groups -asynchronous`); one Fmax per clock, the worst path of each and the clock crossings (`docs/design/toma-and-cade-links.md` section 10). Run the regress first |
| `est/` | yosys, python3 | the per-operator delay table over the base, on several families (see `est/README.md`) |
| `opmix/` (`run_mix.sh`, `probe.v`, `cost.py`) | iverilog, python3 | how often each ALU operation runs: re-simulates every program the regress built with a probe on `id_ula_op` (stopped at `@fim`), then `cost.py [budget_ns ...]` prints the stages each operation would need and the extra cycles per program for a pipeline with a global pause (TODO item 19). Run `Scripts/regress.sh` first |
| `tb_ula.sh` (`tb_ula.v`) | iverilog | the whole ALU (`ula`, every opcode on) through each operation number, against references derived from what each operation means, in 16/10/5, 23/16/6 and 32/23/8 × three `#FROUND` levels; the regress runs it (step 4a0). The operations that round and the divisions are TODO 6(a) steps 2 and 3 |
| `tb_keyps2.sh` (`tb_keyps2.v`) | iverilog | `SAPHO/myKeyPS2.v` against frames sent with a keyboard's timing: every key of both layouts with and without Shift, releases, extended keys, a bad parity, a cut frame (the 1 ms timeout), a full queue; US and ABNT2, at 2 and 25 MHz, queue of 1, 4 and 16. The regress runs it (step 4a0) |
| `tb_alu.sh` (`tb_alu.v`) | iverilog | the shared shifter, `F2I` and the float comparison against references derived in the testbench, in four formats (8/4/3 … 64/52/11) × three `#FROUND` levels. The place to add an operator whose only check today is a blessed golden |

## Typical use

```sh
bash Scripts/hw/elab.sh                 # after any ula.v edit
bash Scripts/hw/elab_single.sh          # after touching a generate guard
NOSHARE=1 bash Scripts/hw/area.sh 2 fadd fmlt all_nodiv     # level 2, three configurations
NUBITS=64 NBMANT=52 NBEXPO=11 NOSHARE=1 bash Scripts/hw/area.sh 0 fadd
bash Scripts/hw/fmax.sh cmm_cexp                            # after Scripts/regress.sh built it
HDL=$PWD/.smoke/hw/clock_exp/addr TAG=exp bash Scripts/hw/fmax.sh sapho_all 5CSEBA6U23I7   # a modified copy of SAPHO/
FR=2 TAG=lvl2 bash Scripts/hw/fmax.sh cmm_cexp              # same program forced to #FROUND 2
iverilog -g2012 -s tb -o tb.vvp Scripts/hw/tb_fdiv.v SAPHO/ula.v && vvp -n tb.vvp
iverilog -g2012 -s tb_fifo -o f.vvp Scripts/hw/tb_fifo.v SAPHO/myFIFO.v && vvp -n f.vvp
iverilog -g2012 -s tb_dcfifo -o d.vvp Scripts/hw/tb_dcfifo.v SAPHO/myDCFIFO.v && vvp -n d.vvp
iverilog -g2012 -s tb_dcfifo -Ptb_dcfifo.WH=13.65 -Ptb_dcfifo.RH=10 -o d.vvp Scripts/hw/tb_dcfifo.v SAPHO/myDCFIFO.v && vvp -n d.vvp
bash Scripts/hw/tb_alu.sh                                   # after any ula.v edit
bash Scripts/hw/tb_alu.sh 50000                             # longer run
bash Scripts/hw/width_sweep.sh                              # after a simulator upgrade (32 64 128)
```

`tb_alu.v` prints an `info:` count of comparisons that disagree with the true
value for words **outside** the float format's invariant (an unnormalized
mantissa at a non-minimum exponent). Those are not failures: no operator and no
constant produces such a word — see the comment on `ula_fcmp`. A non-zero count
there is expected and roughly width-dependent (100/4000 at 8/4/3, 3/4000 at
64/52/11).

## Three traps

- **The `.mif` must sit beside the project** (deduced, 2026-10-14; no fit was
  run without them). Since v7.1 the generated Verilog names its memories by
  bare name. `fmax.sh` and `vfmax.sh` copy both `.mif` into the folder the
  tool runs in and stop if the tool reports one missing; without them the
  program memory would be empty and the synthesis free to prune the
  processor, giving a false Fmax.
- **Yosys `synth` shares resources** (measured; SAT-based `share`): it merges mutually
  exclusive operators — `F2I`'s shifters with the normaliser's — which inflates
  the reported depth by ~7 levels and is something Quartus does not do. Always
  pass `NOSHARE=1`.
- **`abc` is heuristic** (measured): on a netlist Quartus proves identical, Yosys reported
  4209 vs 3827 LUT4. Deltas under ~10 % are noise; Quartus is the reference.

`area.sh` works on a copy of `SAPHO/ula.v` (in `.smoke/hw/`, gitignored, where
every result also lands) with the `I2F` part-selects routed through named
wires: Yosys 0.56 asserts (`modules_.count(...) == 0`) when a part-select
feeds a parameterised instance directly. Yosys is a native Windows build, so
the script runs it from that directory with relative paths — an MSYS `/c/...`
path makes it report "file not found".
