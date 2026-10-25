# Array unit prototype: the tests of steps 4a and 4b

The scripts behind section 7 of `docs/design/array-unit.md` (steps 4a and
4b), on this branch only: the prototype is for measuring, not for `main`.
Run each from an MSYS2 login shell; work files go to `.smoke/array_proto/`.
The array memory is switched on by the environment variable `YANC_ARRMEM`
(asmcomp then numbers the two memories and writes `<name>_arr.mif`).

| script | what it does |
|---|---|
| `arrtest.sh <fixture> [off]` | one C+- fixture through `bin/` and `SAPHO/` of this tree, the array memory on (or `off`), its simulated outputs against `golden_sim` |
| `arrall.sh` | `arrtest.sh` on every C+- fixture with a `golden_sim` and a `.cmm` of its own name (step 4a: 70 of 71) |
| `cppall.sh` | every C++ test simulated by iverilog, the array memory on, against `golden.txt` (step 4a: 81 of 89; the Verilator tests are listed as skipped) |
| `dottest.sh <FROUND> <seed>` | `dot_gen.py`'s program (hand-written `.asm` with `DOT` and `DOB`) simulated, every output against the words of the MAC model in `Scripts/hw/mac_study/mac.py`; the two outputs summed by the float adder are printed for a look |
| `dot_gen.py` | writes that program and the expected words |
| `dotclk.sh` (`dot_probe.v`) | the clocks `DOT` and `DOB` stay decoded for N = 0, 1, 2, 3, 5, 40, and whether the next instruction loses a clock |
| `fit4a.sh` | step 4a: `sapho_all` through `Scripts/hw/fmax.sh` on the DE10-Nano, the array memory off, then on (two fits, about an hour each, on mains power) |
| `fit4b.sh [dot\|dob]` | step 4b: `sapho_all` with a `DOT` (and, with `dob`, a `DOB`) added before `#TOAQUI`, simulated once, then fitted |
| `sweep.sh` | the fitter's seed sweep: `sapho_all` with the array memory and no `DOT`, with `DOT` and `DOB`, and the same on the idea-2 copy, each with `SEEDS` (default 2 3); six fits, five to six hours |
| `mk_ulaout.py <SAPHO> <dir>` | a copy of `SAPHO/` with idea 2 back (the `DOT` result in `ula_out`'s place, dropped in 665ef7f), for `sweep.sh` |
| `arr_ops.py <root>` | which mnemonics take an array name as operand, across every `golden.asm` (decision 10) |

Build the tools of this tree first (`make` at the root). `arrtest.sh`,
`cppall.sh`, `dottest.sh`, `dotclk.sh` and `fit4b.sh` take `HDL=<dir>` to
simulate a modified copy of `SAPHO/`, as `Scripts/hw/fmax.sh` does to fit it.
