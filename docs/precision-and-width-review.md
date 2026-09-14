# Precision and word-width review

Date: 2026-09-14 · Scope: `HDL/*.v`, `Compilers/{CMMComp,CPPComp,APPComp,ASMComp}`,
`Scripts/`, the Verilator harness. This is the evidence behind the items in
[`TODO.md`](../TODO.md); the list of work and the "done when" criteria live
there, not here.

Two questions were asked of the code base:

1. **What is needed to compute with more precision?**
2. **What is needed to let a processor use any `NBMANT` / `NBEXPO`?** Today
   `NUBITS` cannot exceed 32 because of the compilers, not the HDL.

Every claim below points at a file and line. Nothing has been changed yet.

---

## 0. How a number travels through the toolchain

```
source literal ──cmmcomp/cppcomp──▶ .asm (decimal TEXT, unchanged) ──appcomp──▶ asmcomp
                                                                              │
                                              t2t.c:f2mf  (host float, 32-bit int)
                                                                              ▼
                                              .mif  (bit string, itob(int, w))
                                                                              ▼
                                              HDL  (fully parametric in NUBITS/NBMANT/NBEXPO)
```

Neither front end ever builds a target bit pattern: cmmcomp keeps every constant
as its source string (`CMMComp/Headers/variaveis.h:6-16`), cppcomp keeps
`long ival` / `double fval` (`CPPComp/Headers/ast.h:58-59`) and re-prints floats
with `%.20f` (`CPPComp/Sources/codegen.c:825`). The **only** encoder is
`ASMComp/Sources/t2t.c:39 f2mf()`, and it is the 32-bit ceiling of the whole
chain. The HDL itself has no literal 32 on the datapath.

Format reminder (`HDL/ula.v`): value = `m · 2^e`, `m` an explicit `NBMANT`-bit
magnitude normalised to `[2^(NBMANT-1), 2^NBMANT)`, `e` a two's-complement
`NBEXPO`-bit exponent (no bias), sign-magnitude. There is **no hidden bit**, so
the default 23/8 format carries 22 fractional bits where IEEE binary32 carries
23. Zero is `{s, 100…0, 0…0}` (most negative exponent, mantissa 0).

---

## 1. More precision

Ordered by impact. Items 1.1–1.4 are HDL; 1.5–1.7 are toolchain; 1.8 is a
format change.

### 1.1 One mantissa bit is thrown away *before* normalisation (bug, verified)

All three operators pick a fixed slice of a wider result and only then send it
to `ula_norm`, which left-shifts and fills the LSB with 0:

| Op | Where | When the bit is lost |
|---|---|---|
| F_ADD/F_SU1/F_SU2 | `ula.v:322` `m_out = m[MAN:1]` | whenever the sum has no carry (same-sign add without overflow, every subtraction) |
| F_MLT | `ula.v:373` `m_out = mult[2*MAN-1:MAN]` | whenever the top bit of the product is 0 (about half the cases) |
| F_DIV | `ula.v:418` `m_out = div[MAN-1:0]` | whenever `m1 < m2` |

Measured with a testbench on `ula.v` (MAN=23):

| operation | YANC | exact |
|---|---|---|
| (1+2⁻²²) − 1 | **0** | 2.38e-7 |
| (1+2⁻²²) × 1.0 | **1.0** | 1.00000024 |
| 1 / 1.5 | 0.6666665077 (≈1.3 ULP low) | 0.6666666667 |

Consequences: `x*1.0 != x`; two adjacent floats subtract to zero (Sterbenz's
lemma fails, so a Kahan sum does not work either); the phase accumulator of
`TODO.md` item 2 effectively lives on a
2⁻²¹ grid, which is why the measured drift is ~1 ULP/step instead of the ~0.5
ULP that plain truncation would give.

Fix: keep the full-width intermediate and let the normaliser choose the slice
(a mux per operator, no incrementer, no critical-path cost): F_ADD keeps
`m[MAN+1:0]` and shifts right by `carry ? 1 : 0`; F_MLT keeps `mult[2*MAN-1:0]`
and normalises before slicing; F_DIV computes one extra quotient bit
(`m1 << MAN` instead of `<< MAN-1`). **This changes every float golden.**

### 1.2 Truncation instead of rounding (`TODO.md` item 2)

Even with 1.1 fixed the operators truncate toward zero; the error is biased and
accumulates linearly. Needs guard/round/sticky bits out of `ula_denorm`
(`ula.v:185-186`), the product low half (`ula.v:372`) and the division
remainder (`ula.v:414`), then round-to-nearest-even *after* normalisation,
with carry-out handling (mantissa 1000…0, exponent+1). Do 1.1 first: rounding
on top of a lost bit is still wrong by up to 1 ULP.

### 1.3 Exponent overflow / underflow silently wraps (bug)

Only `ula_scl` saturates (`ula.v:921-925`). Everywhere else the exponent is
computed modulo 2^NBEXPO:

- `ula_fmlt` `ula.v:368` `e_out = e1 + e2 + MAN` — two small numbers (e ≈ −120
  each) multiply to e = −217, which wraps to **+39**: a tiny product becomes
  huge.
- `ula_fdiv` `ula.v:417`, `ula_fadd` `ula.v:321` (`e_in + 1` at the top of the
  range), `ula_norm` `ula.v:233` `exp - sh` (a small difference of two small
  numbers wraps upward).
- `ula_f2i` `ula.v:629` `m_ext << shift` wraps for |x| ≥ 2^(NUBITS−1).

Needed: overflow → saturate to the largest magnitude; underflow → flush to
canonical zero; F2I → saturate. Cheap (a compare on the wide exponent sum, as
`ula_scl` already does). Without this, "more exponent bits" only moves the
cliff; with a short `NBEXPO` (the 16-bit fixtures use 5) the cliff is close.

### 1.4 `I2F` uses only `NBMANT` bits (`TODO.md` item 2) and rounds nothing

`ula.v:1195,1201` pass `in[NBMANT-1:0]`; `ula_i2f` (`ula.v:591-606`) negates and
normalises. Converting the full `NUBITS` range means normalising a
`NUBITS`-bit magnitude and rounding it to `NBMANT` bits — the same
round-after-normalise block as 1.2, so implement them together.

### 1.5 Constant encoding on the host loses bits today

`ASMComp/Sources/t2t.c:39-100 f2mf()` and its copy
`CMMComp/Sources/variaveis.c:164-204` (diagnostics only):

- `t2t.c:41` `float f = atof(va)` — the literal is rounded to a **24-bit**
  host float first. `#NBMANT` above 23 gains nothing; below 23 the value is
  rounded twice (`atof` to 24 bits, then `t2t.c:76-79` to `nbmant`).
- `t2t.c:77-79` rounds half-**up** on a single carry bit, with no sticky bits
  below it — not round-to-nearest-even.
- `t2t.c:45-51` IEEE unpack with hard 31/23/0xFF/127/0x007FFFFF; `t2t.c:70`
  special-cases `nbmant == 23`; `t2t.c:76` `sh = 23-nbmant+sh` goes negative
  (UB) for `nbmant > 23`.
- `t2t.c:39,98-100` the result is an `unsigned int` — the word cannot exceed 32
  bits regardless of anything else.
- `CMMComp/Sources/t2t.c:63-68` — a **complex** literal is split with
  `sscanf("%f %f")` + `sprintf("%f")`: 6 decimal places. `comp c = (1e-8, 2e-8)`
  becomes `(0.000000, 0.000000)` today, at any `NBMANT`. Split the source text
  instead of parsing it.
- `CPPComp/Sources/codegen.c:825` prints floats as `%.20f`: anything below
  ~1e-20 becomes `0.0` (`Includes/limits:24` `min() = 1e-30f` is already 0),
  anything above ~1e75 overflows `buf[96]`.
- `ASMComp/Sources/ASMComp.l:38` and `APPComp/Sources/app.l:10` `FLNUM` do not
  accept an exponent, which is *why* cppcomp prints `%.20f`. Accepting
  `[eE][-+]?[0-9]+` in both lexers (and `CMMComp.l:60`) removes the constraint.

Needed: one shared, correctly rounded (ties-to-even) decimal→target encoder,
independent of host `float`/`double`, used by asmcomp (encoding) and by
cmmcomp/cppcomp (range checks, diagnostics, the `f2mf(0.0)` pattern that
`codegen.c:337-338` hard-codes as `0x40000000`). See §3.

### 1.6 Library accuracy is pinned to ~23 bits

- `CMMComp/Includes/float_{sin,exp,log,atan,tan,sqrt}.asm` — polynomial fits
  with stated errors of 1.6e-6 … 6.5e-7 (≈20 bits), coefficients with 10–12
  significant digits (≈35–40 bits). `stdlib.c:1626,2276,2298` π/2, π with
  10–12 digits.
- `CPPComp/Includes/cmath:12` — `sqrt` = Newton with a fixed **24** iterations;
  no `exp/log/sin/cos/pow` at all in the C++ runtime.

Needed: coefficient tables and iteration counts selected by `nbmant` (or a
second, wider table set), and π/e/ln2 with ≥ 40 digits. Independent of the
HDL work; only matters once 1.1–1.5 are in.

### 1.7 Negative zero is not equal to zero

`EQU` is bitwise (`ula.v:824`). `F_NEG`/`F_SGN` produce `{1, 100…0, 0}`, so
`-0.0 == 0.0` is false and `x - x` (positive zero) does not compare equal to
`-(x - x)`. Canonicalise zero in `ula_fneg`/`ula_fsgn`, or mask the sign in
`EQU` when the magnitude is zero.

### 1.8 (Optional, format change) hidden bit

A normalised mantissa always has its top bit set; storing it wastes one bit.
IEEE-style implicit-one would give +1 bit for free at every width, but changes
the format everywhere (HDL, `f2mf`, `mf2f`, `comp2gtkw`, the `real` monitors,
all goldens). Worth deciding *now*, before the encoder of §3 is written, and
before anyone generates hardware that other projects store data from.

### 1.9 Diagnostics that keep working

`delta_float`/`delta_int` (`ula.v:1400-1444`) and the float mirrors emitted by
`hdl.c:311-351` reconstruct values as Verilog `real` (a 53-bit double): exact
up to `NBMANT ≤ 53`, approximate above — acceptable for a waveform aid.
`regress.sh:703` compares decimal text byte-for-byte, so it is width-agnostic.

---

## 2. Generalising `NBMANT` / `NBEXPO` — what breaks, by layer

### 2.1 HDL — parametric, with a handful of scaling issues

Everything on the datapath is sized by `NUBITS`/`NBMANT`/`NBEXPO`
(`processor.v:115-117` → `core.v:857-861` → `ula.v:961-966`). What remains:

| Item | Where | Effect |
|---|---|---|
| Normaliser is a linear mux chain with widening comparators | `ula.v:236-244` | depth O(MAN), area O(MAN²); at 52 bits it dominates Fmax. Replace with a log-depth leading-zero counter + barrel shifter |
| Combinational divider `(2·MAN−1)/MAN` bits | `ula.v:413-414` | 103/52-bit combinational division at MAN=52 is not synthesisable at any useful clock; needs a multi-cycle (or pipelined) divider and a `busy` handshake in `core.v` |
| Combinational multiplier `MAN×MAN` | `ula.v:372` | fine for DSP blocks up to ~54 bits (several DSPs); check Fmax |
| `NUGAIN` untyped parameter | `processor.v:129`, `core.v:490` | a 32-bit `integer`; `ula.v:966` declares it `signed [NUBITS-1:0]`. Type it in all three |
| Three inconsistent default sets | `processor.v:115` NUBITS=16, `core.v:477` 32, `ula.v:963` 32; asmcomp `eval.c:37-39` 23/16/6; cmmcomp `diretivas.c:21-22` 16/6; cppcomp `config.h:15-22` 32/23/8 | harmless while every flow passes explicit values; unify anyway |
| No invariant check in HDL | — | add `initial if (NUBITS != NBMANT+NBEXPO+1 \|\| NBMANT >= 2**(NBEXPO-1)) $error(...)` — `ula_fmlt` needs `e1+e2+MAN` representable |
| Verilog `real` monitors | `ula.v:1400-1402`, `hdl.c:325,351,424` | exact only to 53 bits (see 1.9) |
| `hdl.c:374,447` comp mirror header `{8'd nbmant, 8'd nbexpo, …}` | caps at 255 bits each — fine; `comp2gtkw.c:54-64` reads it |
| `hdl.c:405` `real … = %f` with `mf2f()` | float-array initial values printed with 6 decimals from a host `float`; emit `sm*2.0**e` instead |
| `hdl.c:694` `$fscanf("%d")` into a `[nubits-1:0]` reg | verify with Icarus for > 32 bits (it should work; Verilator path is the harness, §2.6) |
| `ula_scl` `EMAX = (1 <<< (EXP-1)) - 1` | `ula.v:922` | 32-bit integer shift; only breaks for NBEXPO > 32 — irrelevant |
| Opcode / ALU-op fields | `instr_dec.v:167,403`, `core.v:704` 6-bit `ula_op`; `instr_dec.v:181-342` 7-bit opcode literals | unrelated to word width; noted because 108/128 opcodes and 52/64 ALU ops are used |

### 2.2 ASMComp — the actual ceiling

| Item | Where |
|---|---|
| `itob(int x, int w)` — every `.mif` word passes through a host `int`; `w > 31` only sign-extends | `t2t.c:17-32` (identical copy `CMMComp/Sources/t2t.c:20-38`) |
| `f2mf` — host `float`, `unsigned int` result, IEEE constants, `nbmant==23` special case, negative shift for `nbmant>23`, `1 << (nbmant+nbexpo-1)` UB at ≥ 34 bits | `t2t.c:39-100` |
| `mf2f` — `char exb[64]`, `char mab[64]`, `int m = strtol(mab, 2)`, host `float` result | `t2t.c:104-129` |
| every constant's encoded word stored as `int` | `variaveis.c:21,56-63,87,92-96` |
| int-literal range check `(int)pow(2, nubits-1)` — exact at 32, garbage above | `array.c:48-53` |
| float-literal range check in host `float` — saturates to ±inf once the target exceeds host range (nbexpo ≳ 9) | `array.c:76-78,168-169` |
| the only format validation: `nubits != nbmant+nbexpo+1` — no maximum, no `NBMANT < 2^(NBEXPO-1)` | `eval.c:336` |
| `nbopr = ceil(log2(...))` — fine; `NBITS_OPC 7` — fine | `eval.c:6,254` |
| no exponent notation in `FLNUM` | `ASMComp.l:38`, `app.l:10` |

APPComp itself has **no** width dependence (`app.l:24-26`, `eval.c:86-88` pass
the directives through as strings).

### 2.3 CMMComp

| Item | Where |
|---|---|
| second `f2mf` copy (diagnostics + range check), host `float` | `variaveis.c:164-204,212-227` |
| int-literal max `(int)(pow(2, nbmant+nbexpo)-1)`, `atoi` | `variaveis.c:146-147` |
| constant folder in `long` (32-bit on Windows), `atol`, `(long)1 << (nbmant+nbexpo)` — UB from 31 bits | `ast.c:391-431` |
| `pow(x, n)` unroll driven by host `long` | `stdlib.c:966,988-1001` |
| complex literal through `%f` (6 decimals) | `t2t.c:58-70` |
| `NUBITS` directive value is discarded (`t = 0`); only `nbmant`/`nbexpo` are kept, no validation | `CMMComp.y:179-181`, `diretivas.c:36-44` |
| polynomial tables / π digits | `Includes/float_*.asm`, `stdlib.c:1626,2276,2298` (see 1.6) |
| 20-bit source-line table (`itob(…,20)`) | `global.c:265,320` — unrelated to the word, fine up to 1M lines |

### 2.4 CPPComp

| Item | Where |
|---|---|
| constants are `long ival` / `double fval`; literals via `(long)strtoull` — on Windows `long` is 32-bit, so `0xFFFFFFFF` → `-1` and anything wider is truncated silently | `ast.h:58-59`, `CPPComp.l:233-235`, `CPPComp.y:755-756` |
| emitted with `%ld`; `1L << (g_nubits-1)` for unsigned compare and `udivmod` | `codegen.c:817,1160,1311,1400,1482-1483,2671` |
| `const_eval` and the `#if` evaluator fold in host `long` with host semantics (no wrap to NUBITS, cast ignored, arithmetic `>>` for unsigned) | `CPPComp.y:683-742`, `cpppp.c:350-458` |
| struct/bitfield layout sealed at parse time with fallback **16**, not `CFG_NUBITS` | `CPPComp.y:495,567,604,1012,1040,1073,1351,1378`, `types.c:138` |
| `derive_ieee(w)` already knows 16/32/64 splits; `NBMANT`/`NBEXPO` pragmas are otherwise pure pass-through, unvalidated | `codegen.c:2436-2456`, `CPPComp.l:124-126` |
| `%.20f` float emission | `codegen.c:818-833` (see 1.5) |
| `double`/`long long` silently become one word — fine when the word is wide, but keep it documented | `CPPComp.y:664-677` |
| `Includes/`: `limits` hard-codes 1e30/1e-30/1e-7; `cstdint` all `int`; `cstring` assumes 4 bytes/word (`n >> 2`) while `sizeof` is in words; `cmath` sqrt 24 iterations | `Includes/limits:22-26`, `cstdint:5-12`, `cstring:8,14`, `cmath:12` |
| `NTP_BASE 0x7F000000` sentinel collides with real literals in a wider word | `ast.h:162` |

### 2.5 Scripts, harness, tests

| Item | Where |
|---|---|
| Verilator harness: inputs/outputs are host `int`; `(int)top->out` truncates; `top->in`/`top->out` change C++ type with width (`IData`→`QData`→`VlWide`) | `Tests/Verilator/sim_main.cpp:45-48,55,57,79,102-103` |
| `-Wno-WIDTH` silences every width mismatch — turn it on while widening | `run_verilator_step.ps1:45-51`, `regress.sh:670-680` |
| `comp2gtkw.c`: mantissa in `int` (`strtol`), result in host `float`, `re/im[64]` | `Scripts/comp2gtkw.c:10-34,45-46` |
| `regress.sh` `CFG_NUBITS/NBMANT/NBEXPO` defaults 32/23/8 for the cppcomp build | `Scripts/regress.sh:109-116` |
| fixtures only exercise 16/10/5, 23/16/6, 32/23/8 — nothing above 32 | `Compilers/CMMComp/Tests/*/Software/*.cmm` |
| project top-levels hard-code `[31:0]` (DTW, PulseSim, ResetCheck) — project-specific, leave | `Tests/DTW/TopLevel/*.v`, `Tests/PulseSim/TopLevel/generate_random_32.v` |

---

## 3. Proposed plan

### Decision A — how wide is "any"?

- **Tier 1: up to 64 bits.** `long long`/`uint64_t` for integers everywhere
  (`itob`, `v_val`, `ival`, `const_eval`, range checks, harness), exact
  decimal→binary for floats. Covers 40/48/64-bit words and the
  double-like 52/11 format. Cheap, one afternoon per tool.
- **Tier 2: arbitrary.** A small bignum (bit-string) value type in the
  assembler and both front ends. Only needed beyond 64 bits.

Recommendation: Tier 1 now, with the float encoder written so that its
mantissa width is not tied to a C type (it only ever produces a bit string),
so Tier 2 later is a change in the *integer* paths only.

### Decision B — hidden bit (1.8), yes or no, before anything else is touched.

### Order of work

1. **HDL correctness, default path** (changes goldens once): 1.1 keep the lost
   bit; 1.3 exponent saturate/flush; 1.7 canonical zero; the invariant
   `$error`. Re-bless goldens, then freeze.
2. **`#FROUND 1`** (1.2 + 1.4 together): GRS bits, round-after-normalise,
   `I2F` on the full word, `F2I` saturation; a fixture that accumulates
   `x = x + c` and checks mean error ≈ 0. Default `#FROUND 0` keeps step-1
   goldens bit-identical.
3. **Shared number encoder** (`Compilers/common/yanc_num.c`, used by asmcomp,
   cmmcomp, cppcomp): exact decimal (with exponent) → `{s, e, m}` with
   ties-to-even, overflow/underflow flags, `mf2f`-style decode to text for
   diagnostics; replaces both `f2mf` copies, the `%f` complex split, the
   `%.20f` emission and the host-`float` range checks. Lexers accept
   exponent notation. Add `NBMANT < 2^(NBEXPO-1)` and the tool ceiling to the
   validation at `eval.c:336`.
4. **Integer widening to 64** (Tier 1): `itob`/`v_val`/`ival`/`const_eval`/
   `#if`/harness/`comp2gtkw`; struct-seal default → `CFG_NUBITS`; `-Wno-WIDTH`
   off during the work.
5. **HDL scaling**: log-depth normaliser; multi-cycle divider behind a
   parameter (keep the combinational one for ≤ 32 to stay golden-identical);
   Fmax check on the 52/11 configuration.
6. **Fixtures at 48/40/7 and 64/52/11** through both front ends, Icarus and
   Verilator; a host-reference comparison like `test50` for the 64-bit case.
7. **Library accuracy** (1.6): tables/iterations keyed on `nbmant`.
8. README: directive list (`#FROUND`), supported widths, cost table.

### Not needed

- APPComp: nothing to change.
- Opcode/ALU-op field widths: unrelated to the word width.
- The 20-bit line table and the 8-bit comp header: ample.
