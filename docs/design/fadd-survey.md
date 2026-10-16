# The float add and the exponent alignment: the known methods

Survey of 2026-10-15 (Luciano's request, after
[normalisation-survey.md](normalisation-survey.md) closed with the
normaliser unchanged): the ways to build the alignment (`ula_denorm`) and
the add (`ula_fadd`) found in the literature, and which could shorten
`sapho_all`'s worst path. Nothing here is built. Each line says whether it
is read in a source, measured here, or deduced (and from what).

## 1. What limits today

`ula_denorm` (fact, read in `SAPHO/ula.v`): both exponent differences
`d12 = e1-e2` and `d21 = e2-e1` in parallel; the sign of `d12` (`swap`)
picks the larger operand, the shift amount (`d12` or `d21`) and the two
mantissas; one right shifter on the smaller mantissa; the sticky bit from a
thermometer mask of the shift, beside the shifter. `ula_fadd`: sum and both
differences of the magnitudes in parallel, a mux keeps the non-negative one
(no complement of the result).

The worst path of the `pnorm` fit (measured, `sapho_all`, DE10-Nano,
`PERIOD=30 EFFORT=BALANCED`, 32.37 MHz), inside these two blocks:

| step | ns |
|---|---|
| exponent subtraction (carry chain, 8 cells) | 1.4 |
| its sign = `swap`, fanout 90; the shift-amount mux (`shift[1]`, fanout 28) | 1.1 |
| the right shifter, 3 cells | 3.9 |
| `ula_fadd` (carry chain) | 3.1 |

`ula_denorm` is 7.2 ns of the 28.7 ns path, `ula_fadd` 3.1, the normaliser
12.5. The sticky bit is not on it.

## 2. The methods

| # | method | attacks | tried |
|---|---|---|---|
| 1 | swap by exponent, both differences in the adder | -- | current |
| 2 | swap by full magnitude compare, one subtractor | adder area | no |
| 3 | shift both mantissas at once, choose after | the swap mux before the shifter | no |
| 4 | the whole datapath for both orders, choose at the end | `swap` off the path | no |
| 5 | shift amount from its low bits, saturation flag aside | the wait for the full subtraction | no |
| 6 | shifter and sticky combined | area | no |
| 7 | sticky from the trailing zeros | the sticky | no |
| 8 | two paths (near / far) | the normaliser on the add | `twopath`, built, not fitted |
| 9 | the alignment shift in a DSP block | the shifter | no |
| 10 | hard floating-point blocks, fused rounding | all | not in Cyclone V |
| 11 | higher-radix exponent (hexadecimal) | alignment and normalising | changes the format |

**2 (read in FloPoCo, `FPAddSinglePath.cpp`).** FloPoCo swaps by comparing
exception, exponent and fraction together, so the larger magnitude is
always X and one subtraction gives a non-negative result; its comment: an
exponent-only compare "could give a negative" result, needing an absolute
value and an extra mux. SAPHO does the reverse (method 1, as the duplicated
adder of the ARM patent below): the swap is a short exponent compare and
the adder computes both differences. For SAPHO (deduced): a 31-bit compare
decides later than the 9-bit subtraction, and the swap is at the head of
the worst path, so 2 would lengthen it; it would save area in `ula_fadd`.
The exponent is two's complement, not biased: a magnitude compare as an
integer needs its sign bit inverted.

**3 and 4 (read in ARM's patent US 9009208, Nystad).** The patent gives two
options for the swap: muxes after the exponent compare (little area, "an
increase in the latency") or the adder duplicated, one copy per order, the
right one picked when the compare is known ("less of an increase in
latency", "a higher gate count"); and both shift amounts `EA-EB`, `EB-EA`
can start before the order is known. For SAPHO (deduced from the path
above): method 3 shifts `m2` by `d12` and `m1` by `d21` at once, each
shifter fed straight from its subtractor, and the order picks the shifted
mantissa after. It takes the 1.1 ns shift-amount mux and the fanout-90
`swap` off the head of the shifter; the pick moves behind it, where `swap`
is long ready. Cost: a second 26-bit shifter. Method 4 goes further
(both orders through shifter and adder) and costs a second adder set too.

**5 (FloPoCo saturates the shift at wF+3).** Only the low 5 bits of the
difference move bits (W = 26); the higher ones only say "everything is
shifted out". The low bits come early out of the carry chain. With 3 the
shifter can start on them; without 3 the mux waits for `swap` anyway.
Synthesis may already do part of this (not checked).

**6, 7 (FloPoCo; ARM patent; US 5742537).** FloPoCo's combined
shifter+sticky: 324 LUTs and 11.8 ns against 404 LUTs and 12.3 ns for
shifter then sticky (its comment, Vivado on Kintex). The patents compute the
sticky from the trailing-zero count of the smaller operand compared with the
shift, beside the shifter. SAPHO's sticky is already beside the shifter and
off the worst path: area at most.

**8 (Farmwald, patent US 4639887; Seidel and Even, ARITH 2001).** Built as
`twopath` ([normalisation-survey.md](normalisation-survey.md) 4): proven
equal to `pnorm` with normalised operands, Yosys 40 levels against 45,
+37 % LUT4 in the ALU. Its far path keeps the alignment and the add of
today; only the normaliser leaves the path. An FPGA thesis (University of
Saskatchewan, 2005, Virtex-2 Pro) reports the far/close adder faster than
the plain one (19 %, from a truncated abstract). Not fitted.

**9 (Gigliotti, XAPP195; Amaricai, Boncalo, Sicoe 2013).** A right shift
is a product by 2^(W-k) keeping the high bits; the 2013 paper aligns before
the DSP block of a MAC. Same open point as the normaliser's `dsp`: the
DSP block's delay (not measured), and Jamieson and Rose's warning.

**10, 11.** Intel's patents (US 9098332: sum, sum+1, sum+2 for one
carry-propagate stage) are hard blocks of later families. A BYU thesis
reports a hexadecimal-exponent adder with 30 % lower area-time than binary;
SAPHO's format is fixed, so not applicable.

## 3. What to try, in order

1. **Method 3**, from `pnorm`: two shifters fed straight by `d12`/`d21`,
   the order picked after them; with method 5 inside it. Check with
   `eq_fops.sh` (must be equal for any input: the arithmetic is the same),
   then one fit against `pnorm_fast` (32.37 MHz). Expected gain, deduced:
   up to about 1 ns of the 7.2 in `ula_denorm`, i.e. 3 % of the clock, near
   the seed noise (1 %).
2. **`twopath` fitted** as it is: built and checked already, the only
   method that takes the normaliser off the add.
3. Method 4 only if 3 gains.

Not worth trying for the clock: 2 (longer swap), 6 and 7 (off the path),
10 and 11 (not available / format).

## 4. The add as a separate circuit (2026-10-16)

Luciano's proposal: the add (`F_ADD`, `F_SU1`, `F_SU2`) built as a whole,
with its own normaliser and rounding (a private `ula_norm`), its result
going to the accumulator without the shared normaliser. It cannot enter the
fast mux: that output feeds the `JIZ` zero test and the indirect index
(`LDI`/`ILI`), and the add would come back on those paths (the ones `jz`
and `addr` cleared, 26.4 -> 32 MHz). It enters the final mux as a third
input instead (`out = add ? add's own : normalised ? shared : fast`), still
one cell; the add leaves the shared normaliser's 8-input mux. Two variants
from `pnorm` (`make_variants.py`):

- `sepadd`: today's single-path add, private normaliser;
- `sep2p`: the near/far add of `twopath` inside, the private normaliser only
  rounding and range.

Checked with `eq_fops.sh` (whole ALU against `pnorm`): `sepadd` equal for
any input; `sep2p` equal with normalised operands (the real case), the same
differences as `twopath` with any bit pattern. Yosys (`-noshare`, LUT4,
longest path of the ALU with `F_ADD`, `F_SU1`, `F_SU2`, `I2F`, `I2F_M`,
`F_ROT`):

| variant | levels `#FROUND 2` | LUT4 | levels `#FROUND 0` | LUT4 |
|---|---|---|---|---|
| `pnorm` | 45 | 1268 | 36 | 768 |
| `sepadd` | 45 | 1391 | 36 | 890 |
| `twopath` | 40 | 1733 | 32 | 1183 |
| `sep2p` | 40 | 1676 | 32 | 1176 |

Reading: taking the add out of the shared mux alone (`sepadd`) saves no
level and costs about 120 LUT4; the gain is the two paths, and `sep2p` has
it with 57 LUT4 fewer than `twopath`. The level count is the ALU's longest
path, which with the add shortened may now be another operation (`I2F`
through the shared normaliser): not looked at. No fit yet.

## Sources

- FloPoCo, `FPAddSinglePath.cpp`.
  <https://gitlab.com/flopoco/flopoco/-/raw/master/code/VHDLOperators/src/FPAddSub/FPAddSinglePath.cpp>
- J. Nystad (ARM), "Floating-point adder", US 9009208.
  <https://patents.google.com/patent/US9009208B2/en>
- "Fast determination of floating point sticky bit from input operands",
  US 5742537. <https://patents.justia.com/patent/5742537>
- P. M. Farmwald, "Bifurcated method and apparatus for floating point
  addition with decreased latency time", US 4639887.
  <https://patents.google.com/patent/US4639887>
- P.-M. Seidel, G. Even, "On the design of fast IEEE floating-point
  adders", ARITH 2001.
  <https://cris.tau.ac.il/en/publications/on-the-design-of-fast-ieee-floating-point-adders/>
- "Design tradeoff analysis of floating-point adder in FPGAs", M.Sc.
  thesis, University of Saskatchewan, 2005.
  <https://harvest.usask.ca/handle/10388/etd-08182005-161614>
- P. Gigliotti, Xilinx XAPP195, 2004. <https://docs.amd.com/v/u/en-US/xapp195>
- A. Amaricai, O. Boncalo, C. Sicoe, half-precision FP MAC on FPGA DSP
  blocks, 2013.
  <https://yadda.icm.edu.pl/baztech/element/bwmeta1.element.baztech-f44ba2e3-bb1f-4dde-877b-a9d2aa954374/c/Amaricai_Boncalo_Sicoe_FPGA_4_2013.pdf>
- Intel, "Specialized processing block with fixed- and floating-point
  structures", US 9098332.
  <https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/9098332>
- BYU thesis on hexadecimal floating point for FPGAs (author not confirmed
  in this survey). <https://scholarsarchive.byu.edu/etd/311>
- Oberman and Flynn, "Reducing the mean latency of floating-point
  addition", Theoretical Computer Science 196, 1998 (citation only).

Read in full: FloPoCo's source; the ARM patent (first 100,000 characters).
The rest from abstracts, citations or search summaries.

## For the documentation (item 21)

Nothing yet: none of this is in `SAPHO/`.
