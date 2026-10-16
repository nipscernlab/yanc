# Normalising a float on an FPGA: the known methods

Survey of 2026-10-15 (Luciano's request, TODO 29, note
[pipeln-and-division.md](pipeln-and-division.md) 4.3): the ways to
normalise a floating-point result found in the literature, which serve every
operation of SAPHO's shared normaliser and which serve the add alone, which
fit a Cyclone V, and which to try. Each line says whether it is read in a
source, measured here, or deduced (and from what).

**Status: FINISHED (2026-10-15).** Eight methods built and proven equal to
`pnorm` (sections 2.2 and 4), two fitted (`nsr`, `rshort`), none faster.
Decision (Luciano): `pnorm` stays, the normaliser unchanged. The next study
is the add itself and the exponent alignment
([fadd-survey.md](fadd-survey.md)).

## 1. The block being replaced

`ula_norm` in `SAPHO/ula.v` (fact, read in the code): a leading-zero count
over `W = MAN+G` bits (26 at `#FROUND 2`: 23 + 3 extra bits), a left shift
by that count, the exponent minus the count, then at `#FROUND 2` a
round-to-nearest-even increment on the mantissa (a carry chain) with the
range check in carry select. Eight operations go through it: `F_ADD`,
`F_SU1`, `F_SU2`, `I2F`, `I2F_M`, `F_ROT`, and `F_MLT`/`F_DIV` (at
`#FROUND 0` through the count, at 1 and 2 through the direct path). The
format has no hidden bit, so the leading one is stored.

Where it stands (measured, note 4.3): on `sapho_all` with `pnorm`
(32.61 MHz, seed 1) the normaliser is 12.1 ns and 18 cells of the worst
path; in `addr` the `F_ADD` path to the accumulator was denorm 7.5 + add 2.6
+ normaliser 14.8 ns. Seed noise is about 1 % (32.61 against 32.25), so a
change has to win by more than that on several seeds.

## 2. The methods

| # | method | serves | Cyclone V | tried here |
|---|---|---|---|---|
| 1 | leading-zero count as a chain, then a shift | all | yes | original |
| 2 | leading-zero count as a tree (Oklobdzija) | all | yes | `lzc`, kept in every later variant |
| 3 | leading-zero count fitted to the FPGA cell | all | must be redone for the ALM | no |
| 4 | normalising shifter (count and shift by stages 16/8/4/2/1) | all | yes | `nshift`, `nsr`: lost |
| 5 | rounding in carry select | all | yes | `rpar`, `nsr`: lost |
| 6 | shift through a DSP multiplier (shift by k = product by 2^k) | all | yes, 109 DSP blocks free | no |
| 7 | rounding only where it can be non-zero (short shifts) | all (deduced) | yes | no |
| 8 | leading-zero anticipation (LZA / LOP) | the add | yes | no |
| 9 | two-path adder (near / far) | the add | yes | no |
| 10 | shifters in the routing multiplexers | all | no (architecture change) | -- |
| 11 | hard floating-point DSP blocks | all | no (Arria 10 and later) | -- |
| 12 | fused datapath, fewer normalisations | -- | changes the format | -- |
| 13 | coarse normalisation by groups of bits | all | not bit-exact | -- |

### 2.1 Count, then shift (1, 2, 3)

The tree count is the classic one (Oklobdzija, IEEE Trans. VLSI 2(1),
124-128, 1994). Here it is proven equal to the chain on 1.8 million
outputs (`eq_norm.sh`) and gave +2 % on its fit (`lzc`, note 4.3).

Method 3, read in the sources: Zahir, Ullah, Reviriego and Hassnain (IEEE
Embedded Systems Letters 14(1), 35-38, 2022) map the count to Xilinx 6-input
LUTs and report 33 % fewer LUTs at 32 bits; Morse, Kozsik and Rakyta (ICCS
2023) keep that area and cut the count's own delay by 6, 14 and 19 % at 16,
32 and 64 bits. Both use Xilinx parts (LUT6-2, MUXF7/MUXF8), which the
Cyclone V does not have; its ALM is a different fracturable cell, so the
mapping would have to be worked out again. Deduced from the `nsr` fit (the
synthesis already built the count and the shift well, note 4.3) and from
the size of the reported gain (14 % of the count, which is only part of
the 12.1 ns): expect little.

### 2.2 Count and shift together (4)

This is what FloPoCo's `Normalizer` does (read in its source,
`ShiftersEtc/Normalizer.cpp`: "leading zero/one counter + shifter +
optional sticky bit"; stage i tests the top 2^i bits and shifts by 2^i or
not, so the count comes out bit by bit). It is `nshift` here. One fit:
the normaliser took 14.6 ns against 12.1 in `pnorm` (measured, note 4.3).
The three-seed batch is still to run.

### 2.3 The shift in a DSP multiplier (6)

Read in the sources: a multiplier block shifts left by k when the other
operand is 2^k (Gigliotti, Xilinx XAPP195, "Implementing barrel shifters
using multipliers", 2004). Jamieson and Rose (IEEE NEWCAS 2005) map
multiplexers onto unused multipliers and measured it on Stratix with
Quartus: 7.3 % fewer logic elements on average, but in designs that had no
multiplier the clock fell by 54 % on average, because a multiplier is
slower than a small soft mux (on Stratix, 9x9 at 308 MHz against a soft 9:1
mux at 422 MHz). Moctar et al. (FPGA 2012) list it as known (their refs 8
and 12).

Why it might still pay here (deduced, not measured):

- The shift being replaced is not a 9:1 mux but a 26-bit shift by 0 to 25
  (five levels of logic plus their wires), on a path that is 55 % wire.
  One 27x27 block (Cyclone V has that mode natively) takes W = 26 in one
  piece. Yosys's simulation model of the Cyclone V 27x27 multiplier
  (`techlibs/intel_alm/common/dsp_sim.v`) carries 3732 and 3928 as its
  input-to-output delays, units not stated (if picoseconds, under 4 ns);
  not confirmed in Intel's datasheet, Intel publishes no combinational
  figure. Only a fit tells.
- `sapho_all` uses 3 of the 112 DSP blocks of the 5CSEBA6 (read in the
  `pnorm` fit summary).
- The multiplier needs 2^k as a one-hot word, not the count k. That word can
  come without a decoder: with r the mantissa bit-reversed, `r & -r` keeps
  only the lowest set bit of r, which is the leading one of the mantissa
  mirrored, i.e. bit k (the `x & -x` identity, Warren, Hacker's Delight,
  ch. 2). One negation on the carry chain and an AND. The count k for the
  exponent is then an encoder of the same one-hot word, beside the
  multiplier, off the mantissa path.
- The product m * 2^k is exact, so the variant is bit-exact by construction
  (a zero mantissa gives r = 0, a zero product, and `zman` as today); still
  to be proven with `eq_norm.sh` like the others.

Risk, read in Jamieson and Rose: the multiplier's fixed delay and the wires
to and from the DSP column may cost more than the soft shift saves.

### 2.4 Rounding only where it can be non-zero (7)

Deduced from the operators in `SAPHO/ula.v`, not read in a source in this
form: at `#FROUND 2` the rounding increment can be non-zero only when the
normalising shift is 0 or 1.

- `F_ADD`: if the alignment shift d is 2 or more, at most one bit cancels,
  so the normalising shift is at most 1. If d is 0 or 1, the only non-zero
  bit below the mantissa is at most the guard bit, and any left shift of 1 or
  more moves it into the mantissa: guard and sticky are zero, the increment
  is 0. This is the observation behind the near/far adder (9): on the near
  path the result is exact.
- `I2F`/`I2F_M`: the operator pre-aligns the magnitude; a shift in the
  normaliser happens only when the integer fits the mantissa, and then it
  is exact.
- `F_ROT`: exact (the G extra bits are zero, says the code).
- `F_MLT`/`F_DIV` at `#FROUND` 1 and 2 take the direct path (no shift).

If that holds, the increment can be computed on the two candidates (shift 0
and shift 1) in parallel with the count, and the long-shift result needs no
rounding: the rounding carry chain leaves the end of the path. Injection
rounding (rounding before normalising) is the textbook family of this idea
(Seidel and Even, ARITH 2001 "On the design of fast IEEE floating-point
adders", later IEEE Trans. Computers 2004; several patents, e.g. US 6963896).
Caveat: `eq_norm.sh` feeds the normaliser random inputs, which break this
property; the proof needs inputs that come out of the operators (a testbench
over `ula_fadd`, `ula_i2f` and the rest), and the regress.

### 2.5 The add alone (8, 9)

Leading-zero anticipation predicts the count from the operands in parallel
with the subtraction, off by at most one, corrected after (Hokenek and
Montoye, 1990; Bruguera and Lang, IEEE Trans. Computers 48(10), 1999, with
the correction done concurrently; survey: Schmookler and Nowka, "Leading
zero anticipation and detection: a comparison of methods", ARITH 2001).
FPGA measurement, read in a University of Saskatchewan thesis on Virtex-2
Pro ("Design tradeoff analysis of floating-point adder in FPGAs", 2005):
the LOP adder was 6.5 % faster than the plain one for 38 % more area.

The two-path adder (Seidel and Even, above) is note 4.2 item 2.

Both shorten `F_ADD` only; `I2F_M` through the same normaliser is right
behind it (note 4.2: 27.3 against 25.8 MHz on the old placement). They come
after the methods that serve every operation.

### 2.6 Not for SAPHO (10 to 13)

- Shifters in the routing multiplexers (Moctar, George, Parandeh-Afshar,
  Ienne, Lemieux, Brisk, FPGA 2012): 25 to 32 % less area in float adders,
  but it changes the FPGA's routing fabric; no device has it.
- Hard floating-point DSP blocks: Arria 10 and later Intel families; not in
  the Cyclone V nor in the Zynq-7000 of the ZYBO.
- Fused datapath (Langhammer, "Floating point datapath synthesis for
  FPGAs", FPL 2008; Langhammer and VanCourt, FCCM 2009): extra guard bits
  and no normalisation between chained operators, logic and latency down to
  40 % (their slides, HPEC 2008). SAPHO stores every result as a word in
  memory, and the normalised word is the format; dropping it changes every
  golden and the meaning of comparisons.
- Coarse normalisation by groups of 4 to 6 bits (Langhammer, patent
  US 8244789): the leading one ends anywhere in the group, which loses
  precision; not bit-exact.

## 3. What to try, in order

1. **Method 6, the shift in a DSP block**, from `pnorm`: the only one
   specific to FPGAs, it serves all eight operations, and the one-hot word
   needs no decoder. `eq_norm.sh`, then one fit; three seeds only if it
   beats `pnorm`.
2. **Method 7, rounding only on the short shifts**: also serves every
   operation at `#FROUND 2`; first a testbench through the operators to
   prove the "no rounding after a shift of 2 or more" property, then the
   variant.
3. Methods 8 and 9 (LZA, two-path), only if `F_ADD` is still the worst path
   after 1 and 2.
4. Method 3 (count fitted to the ALM), last: the expected gain is small.

The three-seed batch of `nshift`, `rpar` and `nsr`
(`fit_normalisers_resume.sh`) is independent of this list and still to run.

## 4. Built and checked (2026-10-15)

Luciano asked for the Verilog of each buildable method and a quick check
that each gives the right values, before any fit. The five are built from
`pnorm` by `Scripts/hw/clock_exp/make_variants.py` (`dsp`, `alm`, `rshort`,
`lza`, `twopath`; nothing in `SAPHO/` changed) and checked by
`eq_fops.sh`: the whole ALU, eight operations (`F_ADD`, `F_SU1`, `F_SU2`,
`F_MLT`, `F_DIV`, `I2F`, `I2F_M`, `F_ROT`), 20000 operand pairs, five
formats and rounding levels (32 bits at `#FROUND` 0, 1, 2; 16 bits at 0
and 2), each print compared with `pnorm`'s. Two kinds of operands:
normalised floats as programs make them, biased to exponents 0 to 2 apart
and to close mantissas (cancellation), plus integers for `I2F`; and any bit
pattern, unnormalised floats included.

| variant | what it is | normalised operands | any bit pattern |
|---|---|---|---|
| `dsp` | shift as `man * (r & -r)` | equal, all 10 runs | equal |
| `alm` | radix-4 leading-zero tree | equal | equal |
| `rshort` | rounding only for a shift of 0 or 1 | equal | differs at `#FROUND 2` only, in `F_ADD`/`F_SU1`/`F_SU2` (5 to 6 % of their 20000 outputs) |
| `lza` | `F_ADD` with anticipation (`f_i = (a_i ^ b_i) & ~(d_(i-1) = -1)`), one-bit correction, direct path | equal | equal |
| `twopath` | near/far `F_ADD`, direct path | equal | differs in `F_ADD`/`F_SU1`/`F_SU2` (11 to 12 % of their 20000 outputs, 23 to 25 % at `#FROUND 0`) |

So `dsp`, `alm` and `lza` are exact for any input; `rshort` and
`twopath` are exact only when the operands of an add are normalised, as
their derivation assumed (2.4 above; the far path normalises by at most 2
bits). Whether an unnormalised float can reach an add in a real program
(an integer pattern read as a float, a value from an input port) was not
checked: it decides whether those two can be used. The check found two
faults of the testbench, not of the variants, before it was trusted: the
integer samples went to `F_ADD` too (they are unnormalised floats), and a
paused older run wrote the same directories (lesson in memory).

Luciano: every operand that enters the ALU is normalised; only the add
denormalises, inside (2026-10-15). So `rshort` and `twopath` are valid too.

Yosys estimate (`-noshare`, LUT4, longest path of the ALU with `F_ADD`,
`F_SU1`, `F_SU2`, `I2F`, `I2F_M`, `F_ROT`), `#FROUND 2`: `pnorm` 45 levels /
1268 LUT4, `twopath` 40 / 1733, `rshort` 41 / 1332, `rpar` 44 / 1280, `lza`
45 / 1541, `nsr` 46 / 1237, `alm` 47 / 1242, `nshift` 49 / 1217 (`dsp` not
meaningful: Yosys builds the product in LUTs). A proxy: `nsr` was one level
from `pnorm` here and 2.5 ns slower in its fit.

Fit of `rshort` (`sapho_all`, DE10-Nano, seed 1, `PERIOD=30
EFFORT=BALANCED`, against `pnorm` in the same conditions): **no gain**.

| variant | Fmax | ALMs | worst path | normaliser on it |
|---|---|---|---|---|
| `pnorm` | 32.37 MHz | 3,543 | 28.69 ns | 12.5 ns |
| `rshort` | 32.03 MHz | 3,629 | 28.66 ns | 10.3 ns |

The normaliser did get 2.2 ns shorter, but the add on the same path grew
from 3.1 to 4.5 ns in this placement and the path total is the same; the
1 % in clock is seed noise. Same `F_ADD` -> normaliser path in both.

**Decision (Luciano, 2026-10-15): `pnorm` stays.** The normaliser is not
changed. Not fitted, and parked: `twopath`, `dsp`, `lza` (still built by
`make_variants.py` and checked by `eq_fops.sh` if the question comes back). `lza` and `twopath` also
take `F_ADD` out of the leading-zero path, so with them the count serves
only `I2F`, `I2F_M`, `F_ROT` and `F_MLT`/`F_DIV` at `#FROUND 0`.

## Sources

- V. G. Oklobdzija, "An algorithmic and novel design of a leading zero
  detector circuit: comparison with logic synthesis", IEEE Trans. VLSI
  Systems 2(1):124-128, 1994, doi:10.1109/92.273153.
- N. Zahir, S. Ullah, P. Reviriego, S. Hassnain, "Efficient leading zero
  count (LZC) implementations for Xilinx FPGAs", IEEE Embedded Systems
  Letters 14(1):35-38, 2022.
  <https://e-archivo.uc3m.es/entities/publication/42df6019-2159-47e6-b091-babb0266560e>
- G. Morse, T. Kozsik, P. Rakyta, "Minimal path delay leading zero counters
  on Xilinx FPGAs", ICCS 2023, doi:10.1007/978-3-031-36024-4_48.
  <https://www.iccs-meeting.org/archive/iccs2023/papers/140750606.pdf>
- FloPoCo, `Normalizer.cpp`.
  <https://gitlab.com/flopoco/flopoco/-/raw/master/code/VHDLOperators/src/ShiftersEtc/Normalizer.cpp>
- P. Gigliotti, "Implementing barrel shifters using multipliers", Xilinx
  XAPP195, 2004. <https://docs.amd.com/v/u/en-US/xapp195>
- P. Jamieson, J. Rose, "Mapping multiplexers onto hard multipliers in
  FPGAs", IEEE NEWCAS 2005, doi:10.1109/NEWCAS.2005.1496692.
  <https://www.eecg.utoronto.ca/~jayar/pubs/jamieson/jamiesonnewcas05.pdf>
- Y. O. M. Moctar et al., "Reducing the cost of floating-point mantissa
  alignment and normalization in FPGAs", FPGA 2012.
  <https://www.epfl.ch/labs/lap/wp-content/uploads/2018/05/MoctarFeb12_ReducingTheCostOfFloatingPointMantissaAlignmentandNormalizationInFpgas_FPGA12.pdf>
- Yosys, Cyclone V DSP simulation model `dsp_sim.v` (delay figures, units
  not stated).
  <https://git.libre-chip.org/mirrors/yosys/src/commit/656a608bb13218d2f6edb749e43c911970510dc4/techlibs/intel_alm/common/dsp_sim.v>
- Intel, Cyclone V Device Handbook vol. 1, variable-precision DSP block.
  <https://www.intel.com/content/www/us/en/docs/programmable/683375/current/input-register-bank.html>
- M. S. Schmookler, K. J. Nowka, "Leading zero anticipation and detection:
  a comparison of methods", ARITH 2001.
  <https://userpages.cs.umbc.edu/phatak/645/supl/lza/lza-survey-arith01.pdf>
- J. D. Bruguera, T. Lang, "Leading-one prediction with concurrent position
  correction", IEEE Trans. Computers 48(10), 1999 (cited through patents;
  page range not confirmed).
- "Design tradeoff analysis of floating-point adder in FPGAs", M.Sc. thesis,
  University of Saskatchewan, 2005.
  <https://harvest.usask.ca/handle/10388/etd-08182005-161614>
- P.-M. Seidel, G. Even, "On the design of fast IEEE floating-point
  adders", ARITH 2001.
  <https://cris.tau.ac.il/en/publications/on-the-design-of-fast-ieee-floating-point-adders/>
- M. Langhammer, "Floating point datapath synthesis for FPGAs", FPL 2008,
  doi:10.1109/FPL.2008.4629963; M. Langhammer, T. VanCourt, "FPGA floating
  point datapath compiler", FCCM 2009, doi:10.1109/FCCM.2009.54; slides
  HPEC 2008.
  <https://archive.ll.mit.edu/HPEC/agendas/proc08/Day2/B12-Vancourt-precis.pdf>
- M. Langhammer, "Normalization of floating point operations in a
  programmable integrated circuit device", US 8244789 B1.
  <https://patents.google.com/patent/US8244789B1/en>
- H. S. Warren, Hacker's Delight, ch. 2 (`x & -x`): from memory, not
  re-read in this survey.

Not read in full here (only abstracts, citations or slides): Zahir et al.,
Bruguera and Lang, the Saskatchewan thesis, Seidel and Even, Langhammer's
FPL and FCCM papers, Schmookler and Nowka. The numbers quoted from them come
from their abstracts or from papers that cite them.

## For the documentation (item 21)

Nothing for the manual yet: none of this is in `SAPHO/`. If a method is
adopted, the manual's hardware page gets one line (the normaliser's
structure) and the instruction page nothing (same results).
