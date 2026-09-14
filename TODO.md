# TODO

Open work items for YANC that are not tracked elsewhere. This file is the
**list**: what is wrong, why it matters, what "done" looks like. The evidence
(file:line, measurements, the full inventory of 32-bit assumptions) lives in
[`docs/precision-and-width-review.md`](docs/precision-and-width-review.md);
each item links to its section there. Remove an item when it lands (the
history stays in git and in the CHANGELOG).

Items 1–4 are HDL, 5–6 toolchain, 7 HDL scaling, 8 libraries. Items 1, 3
and 4 landed as `#FROUND 1` and the rounding half of item 2 as `#FROUND 2`
(see the CHANGELOG); the default level `0` keeps the legacy datapath, so no
C± golden moved. Suggested order for the rest: 2 → 5 → 6 → 7 → 8.

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

## 2. Round-to-nearest mode + full-range `I2F` + saturating `F2I`

**Status:** landed as `#FROUND 2` (rounding) and `#FROUND >= 1` (`I2F` on
the whole word, `F2I` saturating); two loose ends · **Area:** `HDL/ula.v` ·
**Evidence:** [§1.2](docs/precision-and-width-review.md#12-truncation-instead-of-rounding-todomd-item-2), [§1.4](docs/precision-and-width-review.md#14-i2f-uses-only-nbmant-bits-todomd-item-2-and-rounds-nothing)

Left open: `F_DIV`'s sticky bit is approximated by a third extra quotient
bit (an exact sticky needs the remainder, i.e. a second divider), and the
claim that level 2 is unbiased on *varying* data has only been checked on
the host, not on the core (`cmm_fround2`'s constant addend is correlated
and still drifts +78 ULP, exactly like IEEE `float` on a PC).

**Done when:** an accumulation of varying data (e.g. a sine table) at level
2 is compared against a double reference (mean ≈ 0, size ~sqrt(N) ULP), and
`delta_float` shows zero mean on it.

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

## Workarounds at `#FROUND 0` (worth a line in the README)

- Keep long-running accumulators (phase, time base, counters) in **integer /
  fixed point**: `ADD` is exact, and wrap-around modulo 2^NUBITS gives a free
  "mod 2*pi" for binary angles.
- Accumulate **deviations** from a nominal value instead of absolute values.
- Raise `#NBMANT` for the processor that accumulates (up to 23 — item 5).
- A compensated (Kahan) sum does **not** help at level 0: it relies on
  `t - sum` being exact, which loses a bit there. It works from level 1 on.
