# Hardware measurement scripts

Reproducible measurements for changes to `HDL/`. None of them is part of the
regress (they need Yosys or Quartus); run them by hand when a change should
not alter behaviour but may alter cost or timing.

All paths assume the repo at `/c/nipscern/yanc` under an **MSYS2 login shell**
(`$env:MSYSTEM='MINGW64'; bash -lc "..."` from PowerShell).

| script | needs | answers |
|---|---|---|
| `elab.sh` | iverilog, verilator | does the ALU still elaborate at every `#FROUND` level, at 32/23/8 and 16/10/5, and lint clean? (float ops, shifts and both dividers) |
| `elab_single.sh` | iverilog | does each float operator still elaborate **alone** (one `generate` branch at a time)? Catches a block that stopped being gated by the opcodes |
| `area.sh` | yosys | LUT4 count and critical-path depth per operator set |
| `fmax.sh` | Quartus Prime Lite | real Fmax and ALMs of a processor the regress already built |
| `tb_fdiv.v` | iverilog | the divider array against Verilog's `/` and `%`, 20000 random operands |
| `width_sweep.sh` (`width_sweep/`) | iverilog, verilator, python3 | does each simulator compute every integer operator right at a given word width, in continuous assigns and in procedural blocks, against a Python-bigint oracle? Re-run after upgrading Icarus or Verilator: it decides when YANC may go beyond 32 bits (`TODO.md` item 6(b)). **Expected non-zero result:** Verilator still reports one `div` mismatch, `INT_MIN / -1` giving `0` instead of the wrapped `INT_MIN`. That is its runtime guarding the host divide trap, and it is a property of the simulator, not of YANC: `ula_div` names the case and forces the wrapped quotient, so the processor agrees on both (item 11(a), `test72`/`test73`) |
| `tb_alu.sh` (`tb_alu.v`) | iverilog | the shared shifter, `F2I` and the float comparison against references derived in the testbench, in four formats (8/4/3 … 64/52/11) × three `#FROUND` levels. The place to add an operator whose only check today is a blessed golden |

## Typical use

```sh
bash Scripts/hw/elab.sh                 # after any ula.v edit
bash Scripts/hw/elab_single.sh          # after touching a generate guard
NOSHARE=1 bash Scripts/hw/area.sh 2 fadd fmlt all_nodiv     # level 2, three configurations
NUBITS=64 NBMANT=52 NBEXPO=11 NOSHARE=1 bash Scripts/hw/area.sh 0 fadd
bash Scripts/hw/fmax.sh cmm_cexp                            # after Scripts/regress.sh built it
FR=2 TAG=lvl2 bash Scripts/hw/fmax.sh cmm_cexp              # same program forced to #FROUND 2
iverilog -g2012 -s tb -o tb.vvp Scripts/hw/tb_fdiv.v HDL/ula.v && vvp -n tb.vvp
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

## Two traps, both measured

- **Yosys `synth` shares resources** (SAT-based `share`): it merges mutually
  exclusive operators — `F2I`'s shifters with the normaliser's — which inflates
  the reported depth by ~7 levels and is something Quartus does not do. Always
  pass `NOSHARE=1`.
- **`abc` is heuristic**: on a netlist Quartus proves identical, Yosys reported
  4209 vs 3827 LUT4. Deltas under ~10 % are noise; Quartus is the reference.

`area.sh` works on a copy of `HDL/ula.v` (in `.smoke/hw/`, gitignored, where
every result also lands) with the `I2F` part-selects routed through named
wires: Yosys 0.56 asserts (`modules_.count(...) == 0`) when a part-select
feeds a parameterised instance directly. Yosys is a native Windows build, so
the script runs it from that directory with relative paths — an MSYS `/c/...`
path makes it report "file not found".
