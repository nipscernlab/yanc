# TODO

Open work items for YANC that are not tracked elsewhere. This file is the
**list**: what is wrong, why it matters, what "done" looks like. The evidence
(file:line, measurements, the full inventory of 32-bit assumptions) lives in
[`docs/precision-and-width-review.md`](docs/precision-and-width-review.md);
each item links to its section there. Remove an item when it lands (the
history stays in git and in the CHANGELOG).

Items 1–4 are HDL, 5–6 toolchain, 7–8 HDL scaling/timing, 9 libraries.
Items 1, 3 and 4 landed as `#FROUND 1` and item 2 as `#FROUND 2` (see the
CHANGELOG); the default level `0` keeps the legacy datapath, so no C± golden
moved. Suggested order for the rest: 8 → 5 → 6 (+ the ALU testbench) → 7 → 9.

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

**Status:** landed as `#FROUND 1` (the legacy level `0` keeps the bug on
purpose, for bit-identical hardware) · **Evidence:** [§1.1](docs/precision-and-width-review.md#11-one-mantissa-bit-is-thrown-away-before-normalisation-bug-verified)

Left open: nothing. The item stays listed until the next release notes
mention it; remove it then.

## 2. Round to nearest, full-range `I2F`, saturating `F2I`

**Status:** functionality landed; two verification/refinement leftovers ·
**Area:** `HDL/ula.v` · **Evidence:** [§1.2](docs/precision-and-width-review.md#12-truncation-instead-of-rounding-todomd-item-2), [§1.4](docs/precision-and-width-review.md#14-i2f-uses-only-nbmant-bits-todomd-item-2-and-rounds-nothing)

Done (commits `81f5f25`, `755b758`):
- `#FROUND 2`: round to nearest even after normalisation, for `F_ADD`/`F_SU*`,
  `F_MLT`, `F_DIV`, `I2F`. Verified by `cmm_fround2` (tie cases, sticky) and
  by every C++ test (cppcomp always emits level 2); `I2F` matches host
  `float` on the out-of-range cases.
- `#FROUND >= 1`: `I2F` converts the whole `NUBITS` word (was: low `NBMANT`
  bits, garbage beyond ±2^22); `F2I` saturates to `INT_MAX`/`INT_MIN` (was:
  wrap).

Left open:
- (a) **`F_DIV` sticky is approximate.** The divider produces three extra
  quotient bits; the third stands in for the sticky. In ~1/16 of the
  divisions (guard = 1, the next two bits = 0, remainder ≠ 0) the result is
  rounded down instead of up: error ≤ 0.625 ULP instead of ≤ 0.5. The exact
  sticky is `remainder != 0`, which the `/` operator hides; a `%` or a
  multiply-back would cost 1000–2700 LUT4, so it waits for the explicit
  divider array of item 8 (step 4), where the remainder is free.
- (b) **"Unbiased on varying data" is only shown on the host.** The core was
  checked with a constant addend (`cmm_fround2`, 1000 × 0.001), which is
  correlated and drifts +78 ULP — exactly what IEEE `float` does on a PC.
  A fixture accumulating varying data (e.g. a sine table) at level 2 should
  show mean error ≈ 0 and size ~sqrt(N) ULP against a double reference, and
  `delta_float` zero-mean on the waveform.

**Done when:** (a) is closed by item 8 and (b) has its fixture.

## 3. Exponent overflow / underflow wraps silently

**Status:** landed as `#FROUND 1` (saturate / flush to zero) · **Evidence:** [§1.3](docs/precision-and-width-review.md#13-exponent-overflow--underflow-silently-wraps-bug)

Left open: the parameter guard
`initial if (NUBITS != NBMANT+NBEXPO+1 || NBMANT >= 2**(NBEXPO-1)) $error`
in `ula.v` (needs a form that every simulator and synthesizer in the flow
accepts silently).

## 4. Negative zero is not equal to zero

**Status:** landed as `#FROUND 1` (canonical `+0` out of `ula_norm`,
`F_NEG`, `F_SGN`) · **Evidence:** [§1.7](docs/precision-and-width-review.md#17-negative-zero-is-not-equal-to-zero)

Left open: nothing.

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
`test50`). Ships together with a **self-checking ULA unit testbench** (a
directed regress pass like `ResetCheck`): every float operator, including
`F_ROT`, `F_SGN`, `F_LES`/`F_GRE`, the `_M` variants, `I2F`/`F2I` at the
range edges, at `FROUND` 0/1/2 and at 16/10/5, 32/23/8, 64/52/11, with the
expected values derived in the testbench rather than blessed. Until then
levels 1 and 2 are covered only by `cmm_fround1/2` and (level 2) the C++
tests.

## 7. HDL scaling for wide mantissas

**Status:** open · **Area:** `HDL/ula.v`, `HDL/core.v` · **Evidence:** [§2.1](docs/precision-and-width-review.md#21-hdl--parametric-with-a-handful-of-scaling-issues)

The structural fixes that make a wide mantissa feasible at all (log-depth
leading-zero tree, explicit divider arrays, single shifters) are item 8 and
come first. What remains here is the wide-configuration follow-through:
`NUGAIN` is an untyped (32-bit) parameter in `processor.v`/`core.v`; the
default parameter sets of `processor.v`, `core.v`, `ula.v` and the three
tools disagree; and nobody has yet synthesised a 52-bit mantissa.

**Constraint:** the ALU is combinational by design — the processor pipeline
assumes every operation completes in the cycle. No multi-cycle or iterative
divider; wide mantissas pay in Fmax, not in cycles.

**Measured (Yosys `ltp`, 32/23/8, LUT4 levels):** the whole ALU without
dividers is 47 levels deep; `DIV`+`MOD` alone are 387, `F_DIV` alone 508,
the whole ALU with every divider 546 — the combinational dividers are ~10×
deeper than everything else, so a processor that divides runs at roughly a
tenth of the clock of one that does not. `ula_out` also feeds the `JIZ`
decision and the `LDI`/`LDA` address combinationally, so the ALU depth
bounds the fetch path too, not only `racc`. If the constraint is ever
relaxed, a global-stall multi-cycle divider (only `DIV`/`MOD`/`F_DIV`) is
feasible with an enable on ~10 registers of `core.v` and no compiler change
(see [§2.6](docs/precision-and-width-review.md#26-critical-path-depth-of-the-alu)).

**Done when:** with the item-8 structure in place, the 64/52/11 configuration
is synthesised and its depth/Fmax reported next to 32/23/8 (the 105/52-bit
divider array is the expected limit — its cost is the user's input for
choosing `NBMANT` per project); `NUGAIN` typed `signed [NUBITS-1:0]`
everywhere; one default parameter set across `processor.v`, `core.v`,
`ula.v` and the three tools.

## 8. ALU datapath restructuring (depth and area)

**Status:** open · **Area:** `HDL/ula.v` · **Evidence:** [§2.6](docs/precision-and-width-review.md#26-critical-path-depth-of-the-alu), [§2.7](docs/precision-and-width-review.md#27-alu-efficiency-review-area-and-depth-operator-by-operator)

The ALU is combinational, so its depth is the clock. Measured: the float
path is 37 LUT4 levels for `F_ADD` at level 0, 51 at level 2; the whole
no-divider ALU 47 / 57 / 56 (levels 0 / 1 / 2); with `F_DIV` 546. The
efficiency review found five adders in series where one is needed, a linear
leading-zero chain, three pairs of duplicated shifters, `F_MLT`/`F_DIV`
needlessly crossing the leading-zero count, comparisons done by subtraction,
and dividers with twice the rows they need. All of it can change with level
0 staying bit-identical (the C± goldens are the proof). To be done
**before** the 64-bit work (item 6), so the wide datapath is built on the
cheap structure and the ALU testbench of item 6 validates the final one.

**Done when**, in this order (each step measured with `ltp`/`stat` and the
full regress green):
1. sign-magnitude adder (dual subtractor, no two's-complement round trips),
   parallel `e1-e2`/`e2-e1`, one denormaliser shifter with operand swap;
2. LZC + shift moved to the `F_ADD`/`I2F` branch so `F_MLT`/`F_DIV` skip
   it; one folded exponent adder with overflow/underflow decided in
   parallel; level-2 sticky by thermometer mask and carry-select increment
   (a log-depth leading-zero tree was measured and reverted: `abc` already
   balances the chain — see §2.7);
3. `F_LES`/`F_GRE` as a lexicographic compare (no denormaliser) — mind
   `-0.0` at level 0;
4. explicit restoring divider arrays: `F_DIV` with `MAN+1+G` rows (exact
   sticky → closes item 2(a)), one array for `DIV`+`MOD`;
5. one shared right shifter for `SHL`/`SHR`/`SRS`, one for `F2I`;
6. `NUGAIN` restricted to a power of two, validated by `cmmcomp`/`asmcomp`
   (a non-power-of-two infers a 32-bit divider in `ula_nrm`).

Targets: full no-divider ALU ≈ 30 levels at every level, ≈ −15 % LUT4;
`F_DIV` ≈ 260 levels, ≈ −50 % LUT4; every golden unchanged.

## 9. Library accuracy keyed on `NBMANT`

**Status:** open · **Area:** `CMMComp/Includes/float_*.asm`, `CMMComp/stdlib.c`, `CPPComp/Includes/cmath` · **Evidence:** [§1.6](docs/precision-and-width-review.md#16-library-accuracy-is-pinned-to-23-bits)

The polynomial fits are accurate to ~1e-6 (≈20 bits), π/e constants have
10–12 digits, and cppcomp's `sqrt` runs a fixed 24 Newton iterations; cppcomp
has no `exp/log/sin/cos/pow` at all.

**Done when:** tables/iterations are selected by `nbmant` (a second, wider
set for > 23), constants carry ≥ 40 digits, and the transcendental fixtures
compare against a double reference at 32 and 64 bits.

---

## Workarounds at `#FROUND 0` (worth a line in the README)

- Keep long-running accumulators (phase, time base, counters) in **integer /
  fixed point**: `ADD` is exact, and wrap-around modulo 2^NUBITS gives a free
  "mod 2*pi" for binary angles.
- Accumulate **deviations** from a nominal value instead of absolute values.
- Raise `#NBMANT` for the processor that accumulates (up to 23 — item 5).
- A compensated (Kahan) sum does **not** help at level 0: it relies on
  `t - sum` being exact, which loses a bit there. It works from level 1 on.
