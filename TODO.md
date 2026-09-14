# TODO

Open work items for YANC that are not tracked elsewhere. This file is the
**list**: what is wrong, why it matters, what "done" looks like. The evidence
(file:line, measurements, the full inventory of 32-bit assumptions) lives in
[`docs/precision-and-width-review.md`](docs/precision-and-width-review.md);
each item links to its section there. Remove an item when it lands (the
history stays in git and in the CHANGELOG).

Items 1–4 are HDL, 5–6 toolchain, 7 HDL scaling, 8 libraries. The suggested
order is 1 → 3 → 4 (one golden re-bless), then 2, then 5 → 6, then 7 → 8.

---

## Decisions needed before item 5

- **Word width tier.** Up to 64 bits (`long long` in the tools, float encoder
  written without a C-type ceiling) or arbitrary from the start (bignum
  integers too)? Recommended: 64 now, encoder future-proof. [§3, decision A]
- **Hidden mantissa bit.** IEEE-style implicit one gives +1 bit at every width
  but changes the format everywhere (HDL, encoder, decoders, all goldens).
  Decide before the shared encoder is written. [§1.8]

---

## 1. Float operators drop one mantissa bit before normalisation

**Status:** open · **Area:** `HDL/ula.v` · **Evidence:** [§1.1](docs/precision-and-width-review.md#11-one-mantissa-bit-is-thrown-away-before-normalisation-bug-verified)

`ula_fadd`, `ula_fmlt` and `ula_fdiv` slice their wide intermediate at a fixed
position and only then hand it to `ula_norm`, which shifts left and fills the
LSB with 0. Whenever the result does not occupy the top bit (same-sign add
without carry, every subtraction, half of all products, `m1 < m2` in a
division) the operation effectively works with `NBMANT-1` bits. Measured:
`(1+2^-22) - 1 = 0`, `(1+2^-22) * 1.0 = 1.0`, `1/1.5` low by ~1.3 ULP.
Sterbenz's lemma fails, so a Kahan sum does not work either.

**Done when:** each operator keeps its full-width result and the normaliser
picks the slice (a mux per operator; no incrementer, no critical-path cost);
`x*1.0 == x` and `a - b` exact for `a/2 <= b <= 2a` in a new fixture; every
float golden re-blessed once, together with items 3 and 4.

## 2. Round-to-nearest mode (`#FROUND 1`) + full-range `I2F` + saturating `F2I`

**Status:** open · **Area:** `HDL/ula.v`, `cmmcomp`/`cppcomp` directive · **Evidence:** [§1.2](docs/precision-and-width-review.md#12-truncation-instead-of-rounding-todomd-item-2), [§1.4](docs/precision-and-width-review.md#14-i2f-uses-only-nbmant-bits-todomd-item-2-and-rounds-nothing)

Even after item 1 the operators truncate toward zero; the error is biased and
grows linearly with the number of operations. Two group projects have hit it
(a CNN kernel with same-sign residuals; a Farrow resampler whose phase error
grows with run time at 60·2^-22 Hz ≈ 14 uHz). `I2F` also reads only
`in[NBMANT-1:0]`, so an `int` outside ±2^(NBMANT-1) converts to garbage, and
`F2I` wraps for |x| ≥ 2^(NUBITS-1).

**Done when:** an opt-in `#FROUND 1` (default `0`, bit-identical to today's
goldens) carries guard/round/sticky bits through `ula_denorm`, the product low
half and the division remainder, rounds to nearest-even *after* normalisation
with carry-out handling; `I2F` normalises the full `NUBITS` word and rounds
the same way; `F2I` saturates. A fixture accumulates `x = x + c` for N steps
with both settings against a double reference: `0` drifts ~N ULP, `1` has
mean ≈ 0 and size ~sqrt(N) ULP; `delta_float` shows zero mean. README
documents the directive and its cost.

## 3. Exponent overflow / underflow wraps silently

**Status:** open · **Area:** `HDL/ula.v` · **Evidence:** [§1.3](docs/precision-and-width-review.md#13-exponent-overflow--underflow-silently-wraps-bug)

Only `ula_scl` saturates. `ula_fmlt` (`e1+e2+MAN`), `ula_fdiv`, `ula_fadd`
(`e+1`) and `ula_norm` (`exp-sh`) compute the exponent modulo 2^NBEXPO: the
product of two small numbers (e ≈ -120 each) comes out with e = +39. With
`#NBEXPO 5` (the 16-bit fixtures) the cliff is a few decades away.

**Done when:** overflow saturates to the largest magnitude, underflow flushes
to canonical zero, in every float operator; a fixture exercises both edges;
`initial if (NUBITS != NBMANT+NBEXPO+1 || NBMANT >= 2**(NBEXPO-1)) $error`
guards the parameters in `ula.v`.

## 4. Negative zero is not equal to zero

**Status:** open · **Area:** `HDL/ula.v` · **Evidence:** [§1.7](docs/precision-and-width-review.md#17-negative-zero-is-not-equal-to-zero)

`EQU` is bitwise; `F_NEG`/`F_SGN` produce `{1, 100…0, 0}`, so `-0.0 == 0.0`
is false. **Done when:** zero is canonical out of every float operator (or
`EQU` ignores the sign when the magnitude is zero) and a fixture checks it.

## 5. One shared, exact constant encoder

**Status:** open · **Area:** `ASMComp/t2t.c`, `CMMComp/{variaveis,t2t}.c`, `CPPComp/codegen.c`, the three lexers · **Evidence:** [§1.5](docs/precision-and-width-review.md#15-constant-encoding-on-the-host-loses-bits-today)

The only encoder, `f2mf`, starts from a host `float` (24 bits) — `#NBMANT`
above 23 gains nothing, below 23 rounds twice — rounds half-up without sticky
bits, returns an `unsigned int`, and has a second copy in cmmcomp. Today's
bugs at any width: cmmcomp splits a complex literal through `sprintf("%f")`
(6 decimals: `(1e-8, 2e-8)` becomes `(0, 0)`); cppcomp prints floats with
`%.20f` (anything below 1e-20 becomes `0.0`) because the asm/app lexers
reject exponent notation.

**Done when:** `Compilers/common/yanc_num.c` converts decimal text (with
exponent) to `{s, e, m}` exactly, ties-to-even, with overflow/underflow
flags, and produces the `.mif` bit string without a C integer ceiling; both
`f2mf` copies, the `%f` complex split, the `%.20f` emission and the
host-`float` range checks are gone; the lexers accept `1e-5`; asmcomp
validates `NBMANT < 2^(NBEXPO-1)` and the supported ceiling; a fixture with
`#NBMANT 40` shows constants correct to 40 bits.

## 6. Word width beyond 32 bits (tier 1: up to 64)

**Status:** open · **Area:** all four tools, Verilator harness, `Scripts/comp2gtkw.c` · **Evidence:** [§2.2–2.5](docs/precision-and-width-review.md#22-asmcomp--the-actual-ceiling)

`itob(int)`, `v_val` (`int`), cmmcomp's `long` constant folder, cppcomp's
`long ival` / `(long)strtoull` / `1L << (g_nubits-1)`, `sim_main.cpp`'s
`(int)top->out` and `comp2gtkw`'s `int` mantissa all cap the word at 32 bits
(`long` is 32-bit on Windows). cppcomp also seals struct/bitfield layout with
a fallback of **16**, not `CFG_NUBITS`. APPComp and the HDL need nothing.

**Done when:** integers are `long long` end to end, the Verilator harness
reads/writes `NUBITS`-bit words, `-Wno-WIDTH` is off while the work is done,
and fixtures at 48/40/7 and 64/52/11 pass through both front ends under Icarus
and Verilator, with a host-double reference for the 64-bit case (like
`test50`).

## 7. HDL scaling for wide mantissas

**Status:** open · **Area:** `HDL/ula.v`, `HDL/core.v` · **Evidence:** [§2.1](docs/precision-and-width-review.md#21-hdl--parametric-with-a-handful-of-scaling-issues)

`ula_norm` is a linear mux chain (depth O(MAN), area O(MAN²)); `ula_fdiv` is
a combinational `(2·MAN-1)/MAN`-bit divider — neither closes timing at 52
bits. `NUGAIN` is an untyped (32-bit) parameter in `processor.v`/`core.v`;
the default parameter sets of `processor.v`, `core.v`, `ula.v` and the three
tools disagree.

**Done when:** log-depth leading-zero count + barrel shifter; a multi-cycle
divider behind a parameter (the combinational one stays for ≤ 32 bits to keep
goldens); `NUGAIN` typed `signed [NUBITS-1:0]` everywhere; one default set;
Fmax reported for 32/23/8 and 64/52/11.

## 8. Library accuracy keyed on `NBMANT`

**Status:** open · **Area:** `CMMComp/Includes/float_*.asm`, `CMMComp/stdlib.c`, `CPPComp/Includes/cmath` · **Evidence:** [§1.6](docs/precision-and-width-review.md#16-library-accuracy-is-pinned-to-23-bits)

The polynomial fits are accurate to ~1e-6 (≈20 bits), π/e constants have
10–12 digits, and cppcomp's `sqrt` runs a fixed 24 Newton iterations; cppcomp
has no `exp/log/sin/cos/pow` at all.

**Done when:** tables/iterations are selected by `nbmant` (a second, wider
set for > 23), constants carry ≥ 40 digits, and the transcendental fixtures
compare against a double reference at 32 and 64 bits.

---

## Workarounds until items 1–2 land (worth a line in the README)

- Keep long-running accumulators (phase, time base, counters) in **integer /
  fixed point**: `ADD` is exact, and wrap-around modulo 2^NUBITS gives a free
  "mod 2*pi" for binary angles.
- Accumulate **deviations** from a nominal value instead of absolute values.
- Raise `#NBMANT` for the processor that accumulates (up to 23 — item 5).
- A compensated (Kahan) sum does **not** help until item 1 is fixed: it relies
  on `t - sum` being exact, which today loses a bit.
