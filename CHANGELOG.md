# Changelog

All notable changes to YANC are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project adheres to a loose semantic-versioning scheme on the `v*`
tags consumed by Aurora.

## [Unreleased]

## [v5.2] – 2026-06-08

### Added
- **C++ simulation/GTKWave pipeline parity (`cppcomp`)** — a cppcomp-generated
  design now tracks its variables and source lines in the waveform exactly like a
  C±-generated one. cppcomp emits the two GTKWave-support files cmmcomp always
  produced: `pc_<proc>_mem.txt` (one 20-bit two's-complement source line per
  instruction, written in lockstep with the instruction count; program scaffolding
  tagged −1/−3) and `trad_cmm.txt` (the numbered preprocessed source those line
  numbers index into). `cmm_log.txt` is now restricted to genuine scalar statics —
  `int → 1`, `float → 2`, and anything else (pointer/struct/…) is no longer
  published, so asmcomp never builds a wrong integer mirror at a fixed address; a
  function parameter is logged under its asm label so overloaded-function params
  still reconstruct to the `<func>_<var>` operand asmcomp matches. All of this is
  simulation-only (behind `` `ifdef YANC_SIM_VIS `` in the generated core), so it
  costs nothing in synthesis.
- **`single_proc_cpp.bat`** — end-to-end C++ runner mirroring `single_proc.bat`
  (cpppp → cppcomp → appcomp → asmcomp → iverilog/verilator → GTKWave), with
  `--sim iverilog|verilator` and `--no-view`, plus a minimal `proc_cpp` demo.
- **`out()` warns on a float value (`cppcomp`)** — the hardware OUT port is an
  integer word; `out(port, x)` with a float `x` ships its raw bit pattern (a
  wrong number). cppcomp now emits a compile *warning* pointing at the cast
  (`out(port, (int)x)`) but still compiles and does **not** convert — the
  convention every existing test already follows is the explicit cast. (C±
  warns and truncates via `F2I` and offers `fout`; cppcomp warns without
  converting.)

### Changed
- **Runner scripts moved under `Scripts/`** — the single/multi-proc runners
  (`single_proc.bat`/`.sh`, `multi_proc.bat`/`.sh`, `single_proc_cpp.bat`) now
  live in `Scripts/` next to `env`/`setup` instead of the repo root, and a new
  `Scripts/single_proc_cpp.sh` gives the C++ pipeline a Linux runner to match the
  `.bat`. Aurora's deploy step (`Scripts/aurora.bat`) was trimmed to copy only the
  binaries Aurora actually consumes (no `gen_gtkw`); the release archives are
  unchanged.

### Fixed
- **Runner scripts trusted phantom tool paths (`Scripts/env.bat`)** — an
  IDE/launcher (the VS Code integrated terminal under Aurora) can pre-set
  `IVERILOG`/`VVP`/`GTKWAVE`/`MINGW_BIN` to bundled tools that are not actually
  installed. env.bat only filled a tool *if not defined*, so it trusted those
  non-existent paths and never reached its PATH fallback — every runner died at
  `iverilog` with *"The system cannot find the path specified"* and never built the
  `.vvp`. env.bat now drops any inherited tool path that does not exist (so the
  PATH lookup finds the real MSYS2 tool while a present nipscernlab GTKWave is
  kept), and derives `MINGW_BIN` from a resolved tool when setup's cache is absent.
- **`--version` printed `5.0`** — `yanc_version.h` was left at `5.0` after the v5.1
  release; bumped to match the tag and CHANGELOG.
- **Dead / double-evaluated left operand on `-` `/` `%` `<<` `>>` (`cppcomp`)** —
  the memory-operand fast path evaluated the left operand and only then found
  these non-commutative ops aren't handled there, falling through to the stack
  path which re-evaluated it: a dead `LOD` for a plain variable, and a genuine
  double-evaluation for a side-effecting left operand (`f() - b` called `f()`
  twice). Now gated on the operator, so the left operand is emitted exactly once.
- **Constant-index array access uses the `_V` (base+offset) opcodes (`cppcomp`)** —
  `arr[k]` with a compile-time integer `k` now bakes the offset into the
  instruction instead of computing the index and going through the indirect
  `STI` / `LDI` path: a store `SET_V arr k` (4 → 2 instructions), a load
  `LOD_V arr k` (2 → 1), and when `arr[k]` is the right operand of `+`/`*` the
  fused `ADD_V` / `MLT_V` (and float `F_ADD_V` / `F_MLT_V`) collapses load+op too
  (`g[0] + g[1]` → `LOD_V g 0; ADD_V g 1`, 6 → 2). Non-constant indices keep the
  indirect path. Same opcodes cmmcomp's constant-index path emits. Regress: 51/51 cpp.
- **Unary ops / int↔float casts of a plain variable use the `_M` form (`cppcomp`)** —
  `-x`, `~x`, `!x` and `(int)x` / `(float)x` on a simple scalar memory variable
  now read the operand straight from memory (`NEG_M` / `F_NEG_M` / `INV_M` /
  `LIN_M` / `F2I_M` / `I2F_M`) instead of `LOD x; <op>`, one instruction less
  each. Operands that aren't a plain memory variable (members, references, frame
  locals, expressions) keep the `LOD; <op>` path. Regress: 51/51 cpp.
- **Post-emit peephole (`cppcomp`)** — cppcomp now buffers emitted instructions
  and runs a peephole before writing them (the instruction-selection layer it
  lacked; cmmcomp tracks the accumulator inline). Two passes, no fusion across a
  label or into verbatim inline asm, and the `pc_<proc>_mem.txt` line table is
  regenerated from the fused stream so `num_ins` stays exact:
    1. `LOD <var>; <unary>` → `<unary>_M <var>` — a unary acc op (NEG/ABS/PST/
       NRM/I2F/F2I/INV/LIN and `F_` variants) after loading a *named* variable
       reads it straight from memory. Literal loads (`LOD 5`) are left alone.
    2. bare `PSH` + a load-class op → the op's `P_` / `PF_` variant that pushes
       the accumulator as part of the same instruction (`PSH; LOD x` → `P_LOD x`,
       `PSH; NEG_M x` → `P_NEG_M x`, ...); and a plain `SET x; POP` →
       `SET_P x` (store-then-pop fused), which collapses the argument-unpacking
       in a function prologue (`POP; SET c; POP; SET b; POP; SET a` →
       `POP; SET_P c; SET_P b; SET a`).
  Together they collapse e.g. `a - b` to `LOD a; P_NEG_M b; S_ADD` (3 instrs).
  Regress: 51/51 cpp.
- **Redundant store-then-reload elimination (`cppcomp`)** — a `LOD x` (or
  `LOD_V a k`) immediately after a `SET x` (or `SET_V a k`) of the *same* cell
  is dropped: `SET` leaves the value in the accumulator, so reloading it is a
  no-op. This was pervasive — `int x = e; <use x>` emitted `<e>; SET x; LOD x;
  <use>` — and removing it shed ~470 instructions across the test suite (test50
  alone ~60). New peephole pass 0. Regress: 51/51 cpp.
- **Dead `JMP` to the next line dropped (`cppcomp`)** — a bare `JMP L`
  immediately followed by `@L` (exact label, not a prefix) just falls through,
  so it is removed — e.g. the `JMP main` a helper-function-less program emits
  right before `@main`. Regress: 51/51 cpp.

## [v5.1] – 2026-06-07

### Added
- **Math-library built-ins (`cmmcomp`)** — `pow(x, y)` (an integer-constant
  exponent expands to exact square-and-multiply, an integer variable to a runtime
  multiply loop, otherwise `exp(y·ln x)`); `tan(x)` (a dedicated degree-11 minimax
  fit, range-reduced mod π with a cotangent fold for the pole — *not* `sin/cos`);
  `floor` / `ceil` / `round` (inline via `F2I`, returning an integral float, ties
  away from zero like C `round()`); and the hyperbolics `cosh` / `sinh` / `tanh`
  (composed from the real exponential, `tanh` via the one-`exp` `(e^2x−1)/(e^2x+1)`
  identity). Each is validated by value with a new fixture. No new hardware.
- **Complex arguments across the transcendentals (`cmmcomp`)** — `exp`, `sin`,
  `cos`, `tan`, `log` and `atan` now accept a `comp` argument and return a `comp`
  (joining `sqrt`), composed from real ops via the canonical library identities:
  `exp(a+bi)=eᵃ(cos b+i·sin b)`, `sin(a+bi)=sin a·cosh b+i·cos a·sinh b`,
  `log(a+bi)=½·ln|z|²+i·arg z`, `tan` via the real-denominator closed form
  `(sin 2a+i·sinh 2b)/(cos 2a+cosh 2b)`, and `atan` via
  `½·atan2(2a,1−a²−b²)+i·¼·ln(…)`. The `cosh`/`sinh` of the imaginary part come
  from a single `exp` + reciprocal. Also adds **`conj(z)`** (complex conjugate
  `a−bi`; a real `x` becomes `x+0i`; always returns a `comp`). New `cmm_cexp` /
  `cmm_csincos` / `cmm_clog` / `cmm_catan` / `cmm_ctan` / `cmm_conj` fixtures
  validate every form (const / memory / accumulator) by value.

### Fixed
- **`fase()` / `atan2` returned the wrong quadrant (`cmmcomp`)** — all five `F_LES`
  sign tests in `exec_fase` had been written assuming `F_LES X` is true when
  `acc < X`, but the hardware semantics are `acc > X`, so the routine silently
  computed `fase(−z)` (every quadrant off by ±π). It went unnoticed for a long
  time because the `cmm_comp_fase` golden had been blessed with the wrong output
  (the fixture's own comment had the right values). Fixed by swapping the operand
  on each test; `cmm_comp_fase` now matches the true `atan2`
  (`fase(1+2i)=1107`, `fase(2+0i)=0`, …), and complex `log` / `atan`, which build
  on it, are correct.

### Changed
- **`comp` argument → `comp` result is now explicit in the type pass (`cmmcomp`)** —
  the typecheck annotates `sqrt`/`exp`/`sin`/`cos`/`tan`/`log`/`atan` as `comp`
  when the argument is `comp` (it always said `float` before). Cosmetic — codegen
  already used the function's runtime return type, so no emitted code changed — but
  it makes the rule "real argument → `float`, `comp` argument → `comp`" explicit.
- **Codegen now uses the fused load+op instructions (`cmmcomp`)** — after a full
  audit of the assembler ISA, wasteful `(P_)LOD`/`PSH` + op pairs were collapsed
  into the single-cycle fused forms (`F_NEG_M`, `F_ABS_M`, `PF_NEG_M`, `P_LOD`,
  `I2F_M`, …), shaving an instruction off `cosh`/`sinh`, the `pow` loop, and the
  complex `sqrt`/`log` branches. Values are unchanged (the affected goldens only
  grow — shorter code loops more times in the fixed sim window).

### Changed
- **`float_atan` now uses a minimax polynomial instead of a LUT** — the arctangent
  macro drops the 49-entry table for a degree-11 odd minimax polynomial
  (least-squares fit on `[0,1]`, max abs error ~4.9e-6, ~6 digits vs the table's
  ~4e-5). `|x|` is still folded into `[0,1]` with the `1/x` identity
  (`atan(|x|)=π/2−atan(1/|x|)`), but now a single shared polynomial serves both
  branches instead of duplicating the lookup — so the macro is actually *smaller*
  in code (34 vs 39 instructions) while freeing ~98 words of data memory
  (`Arctan_LUT.txt` removed). A few more instructions execute per call (a Horner
  poly vs a table lookup), negligible on the single-cycle ULA. No new hardware.
  `cmm_trig` gains `atan(1)=π/4`, `atan(2)` (the 1/x branch) and `atan(-1)` (sign);
  `cmm_comp_fase` (atan via `fase`) is unchanged in value. With this, **no
  transcendental macro uses a lookup table anymore**.
- **`float_sin` / `float_cos` now use a minimax polynomial instead of a LUT** — the
  sine macro drops the 152-entry lookup table for a degree-7 odd minimax
  polynomial (least-squares fit on `[0, π/2]`, max abs error ~1.6e-6, ~6 digits
  vs the table's ~3). Range reduction folds to `[-π/2, π/2]` (`k = round(x/π)`,
  `r = x - k·π`) and the `(-1)^k` sign is applied from the parity of `k`. Costs
  just +3 instructions over the LUT version (31 vs 28) while freeing ~152 words
  of data memory (`Sin_LUT.txt` removed), and uses only existing instructions (no
  new HW). `cos` reuses it. `cmm_trig` golden updated (`cos(0)` now 999 — the
  polynomial's uniform 0.9999975 vs the table's lucky-exact node).
- **`float_sin` / `float_cos` range reduction is now O(1)** — the sine macro no
  longer subtracts `2π` in a loop to bring the argument into range; it computes
  `k = round(x/2π)` and does `x -= k·2π` in one shot (round-to-nearest via a
  sign-half bias, since `F2I` truncates), landing in `[-π, π]` before the usual
  table lookup. `cos` reuses it (`sin(x+π/2)`), so both are now fixed-cost
  regardless of argument magnitude — `sin(100 rad)` no longer loops ~16 times.
  Same table, same ~3–4 digit precision; uses only existing instructions (no new
  HW). First by-value sin/cos coverage added as the `cmm_trig` fixture
  (`sin(0)=0`, `sin(π/2)≈1`, `cos(0)=1`, `sin(100)≈-0.506`, `cos(π)≈-1`).
- **`float_exp` / `float_log` range reduction is now O(1) via `F_SCL`/`XPO`** — the
  exp and log macros no longer reduce the argument with a value loop (subtract
  `ln2` / halve repeatedly). `float_exp` computes `n = round(x/ln2)` directly and
  scales `exp(r)` by `2^n` with one `F_SCL`; `float_log` gets the exponent with
  `XPO` and the mantissa `m = x·2^-e` with one `F_SCL`. Both are now fixed-cost
  (~25–33 instructions) regardless of argument magnitude, and the reduction is
  exact (no bit loss from repeated halving), so the inverse round-trips land
  cleaner — `exp(log(8))` now hits 8.000 exactly (was 7.999). `float_sqrt` keeps
  its `F_ROT` estimate. Same values, regenerated `cmm_exp`/`cmm_log` goldens.

### Added
- **ISA: `F_SCL`/`SF_SCL` and `XPO`/`XPO_M` — float exponent surgery (HW + assemblers)** —
  four new ULA operations. `F_SCL X` / `SF_SCL` scale a float by a power of two
  (`acc·2^k`, k a signed int from memory or the stack) by adding k to the
  exponent field (saturating); `XPO` / `XPO_M` extract the base-2 exponent of a
  float as an int (`floor(log2|x|)`, from the accumulator or memory). They are
  the `ldexp`/`frexp` primitives for O(1), exact range reduction in the
  transcendental macros (the value-loop reductions in `float_exp`/`float_log` and
  the `F_ROT` estimate in `float_sqrt` are being migrated to them). Implemented as
  `ula_scl`/`ula_xpo` in `HDL/ula.v` (mux/params/decode in `instr_dec.v`,
  `core.v`, `processor.v`; `F_ROT` kept for now), and taught to the assemblers
  (`ASMComp.l`, `app.l`, with resource-report messages in `opcodes.c`). Verified
  by value through the sim: `F_SCL(1,3)=8`, `F_SCL(8,-3)=1`, `XPO(8)=3`,
  `XPO(16)=4`, `SF_SCL(1,3)=8`, `XPO_M(32)=5`. The exponent field is signed two's
  complement (not IEEE-biased), which makes both ops a thin exponent-field add /
  read.
- **Real `exp(x)` and `log(x)` (cmmcomp)** — two new built-ins: `exp` is e^x and
  `log` is the natural logarithm (ln). Both are backed by new assembly macros
  (`Includes/float_exp.asm`, `Includes/float_log.asm`) auto-included on demand
  through the same `find_opc`/`mac_add` hook as `float_sqrt`/`float_sin`. The
  macros use **no new hardware and no lookup table**: range reduction is done by
  value with a loop (like `float_sin`), independent of the non-IEEE float layout,
  and the reduced interval is evaluated with a Horner polynomial — `exp(r)` a
  degree-8 Taylor series on `[0, ln2)`, `log(m)` the atanh series
  `2u(1+u²/3+u⁴/5+…)` with `u=(m-1)/(m+1)` on `[1, 2)`. `log` guards `x<=0`
  (returns 0). Complex arguments are rejected for now (a later step). Wired
  through the lexer (`exp`/`log` keywords), grammar (`std_exp`/`std_log`),
  AST (`OP_STD_EXP`/`OP_STD_LOG`) and `exec_exp`/`exec_log` in stdlib. Validated
  by value with the new `cmm_exp`/`cmm_log` fixtures (scaled ×1000 since `fout`
  truncates to int): `exp(0)=1`, `exp(1)=e`, `exp(2)=e²`, `exp(-1)=1/e`,
  `log(2)=ln2`, `log(10)=ln10`, and the inverse round-trips `log(exp(3))` and
  `exp(log(8))`.
- **Complex `sqrt(z)` (cmmcomp)** — `sqrt` now accepts a `comp` argument and
  returns the principal complex square root, instead of rejecting it as an error.
  The `comp` branch of `exec_sqrt` is composed entirely from existing real ops
  (no new assembly instructions): with `z = a + b i` and `r = |z| = sqrt(a²+b²)`,
  it emits `sqrt(z) = sqrt((r+a)/2) + sign(b)·sqrt((r-a)/2) i` (`F_SU1` for the
  subtraction, `F_MLT 0.5` for the halving, `F_SGN` for the sign transfer). All
  three argument forms are handled — `comp` constant, `comp` in memory, and a
  `comp` already in the accumulator (spilled to temps with `SET_P`/`SET`, like
  `exec_fase`). The accumulator peephole drops the redundant `LOD csqrt_r` /
  `LOD csqrt_a` after each `SET`, so the live half stays in the acc. Locked down
  by the new `cmm_comp_sqrt` fixture (by-value): `sqrt(3+4i)=2+1i`,
  `sqrt(-3+4i)=1+2i`, `sqrt(z+w)=sqrt(3+4i)=2+1i` (acc path), and the real-axis
  edges `sqrt(4)=2`, `sqrt(-4)=2i` (principal root; `F_SGN(0)` yields `+`).
- **Accumulator-aware redundant-load elimination (cmmcomp codegen)** — the
  streaming peephole in `add_instr` now tracks which operand is in the
  accumulator (`acc_name`, set by a plain `LOD`/`P_LOD`/`SET`, cleared by every
  other op) and drops a `LOD x` whenever the accumulator already holds `x`. This
  generalises the old "`LOD x` right after `SET x`" drop to any basic-block
  position; `acc_name` is reset at every control-flow boundary
  (`emit_peephole_reset`), so a drop never crosses a label. Value-preserving and
  byte-identical on the suite (the existing codegen had no extra redundant loads
  to drop). To turn the new capability into a real win, every `c²+d²` the codegen
  builds — the denominator of all complex divisions (`oper_divi`: comp÷comp,
  scalar÷comp and the acc variants) and the `|comp|²` in `mod2`/`abs`
  (`exec_mod2`) — now goes through one `emit_sq_sum(er, ei)` helper that squares
  the half the accumulator already holds first. `c²+d²` is commutative, so when
  `d` is live (e.g. `r = x / y` or `mod2(x)` right after `x = …`) squaring `d²`
  first lets the peephole drop its `LOD`. So `r = x / y` right after `y = …` is
  21 instructions instead of 22. Value-preserving and byte-identical on the suite
  (no existing test has the divisor/operand live in the acc at the operation).
  Locked down by the new `cmm_comp_div` fixture, which also adds the first
  by-value coverage of `comp_var / comp_var` ((4+2i)/(1+1i)=3-1i,
  (8+6i)/(1-1i)=1+7i, (6+8i)/(2+0i)=3+4i).

### Fixed
- **Walker-time diagnostics report the right line and variable name (cmmcomp)** —
  every warning / info message emitted during the deferred-AST walk (comp/float
  conversions, complex array index, comp-in-condition, comp/float `RECV`, the
  function-call parameter conversions, …) printed the **EOF line** and the
  **mangled `<fn>_<var>`** name (e.g. `line 21: variable 'main_c' …` instead of
  `line 15: variable 'c' …`). Both come from globals that are stale during the
  walk: `line_num` (the lexer line) has run to EOF, and `fname` (the current
  function, used by `rem_fname` to strip the prefix) was reset to `""` at each
  function's end. Fix: the walker wrappers (`stmt_emit` / `ast_emit_expr`) now
  keep `line_num` in sync with the node's source line (mirroring `emit_line`,
  with save/restore so parse-time diagnostics are untouched), and the
  `STMT_FUNC` walker sets a new display-only global `emit_fname` to the
  function's name for `rem_fname`. `emit_fname` is display-only — `exec_id` still
  keys off the global `fname`, so temp-variable names and the **assembly are
  byte-identical**. Messages stay correctly bilingual (`-pt` / `-en`). Verified
  byte-identical on the suite and by a probe in both languages (warnings now read
  the real line and the bare local name).
- **Arithmetic between two `comp` literals is no longer dropped (cmmcomp)** —
  `(1+2i)*(3+4i)` returned `3+4i` (and `(4+2i)/(1+1i)` returned `4+2i`, and
  `(0+2i)+(3+4i)` returned `3+4i`) instead of computing the operation. The
  algebraic-identity folder (`x*1→x`, `x/1→x`, `x+0→x`) used `is_const_value`,
  which only compared the literal's *real-part* cell against the scalar 0/1 — so
  a comp literal like `1+2i` (real part 1) was mistaken for the constant 1 and
  the operation was folded away. `is_const_value` now rejects non-scalar
  literals (`n->type > 2`), so only a genuine int/float literal can be the
  algebraic 0/1; comp literals go through the normal `oper_mult`/`oper_divi`/
  `oper_soma` path. Scalar folding is unchanged (every existing test stays
  byte-identical). Locked down by the new `cmm_comp_arith` fixture (`×`/`÷`/`+`/
  `−` between comp literals, the `0+x`/`1*x` decoy cases, and a comp-variable
  regression — output values checked against hand computation).
- **`fase()` (complex phase) is now a real `atan2` (cmmcomp)** — it previously
  computed just `atan(imag/real)`, a 2-quadrant angle that was wrong for
  `real < 0` (off by ±π) and **divided by zero when `real == 0`** (the sim
  produced no output). `exec_fase` now emits proper `atan2(imag, real)`: it
  branches on the signs of real and imag, computes `atan(imag/real)` only when
  `real ≠ 0` (with the ±π quadrant correction for `real < 0`), and returns ±π/2
  on the imaginary axis and 0 at the origin — correct in all four quadrants and
  on the axes, no division by zero. Uses only existing opcodes (`F_LES`/`F_DIV`/
  `F_ADD`/`CAL float_atan`/`LOD`/`JIZ`/`JMP`), no hardware change. `fase` was
  unused by any test/example, so no golden changed. Locked down by the new
  `cmm_comp_fase` sim fixture (all four quadrants, all four axes and the origin,
  checked as `atan2×1000` against hand computation — e.g. `-2+0i → π`,
  `0±2i → ±π/2`).

### Added
- **`do { } while ()` loop (cmmcomp)** — C± gains the third C loop. The body runs
  first and the condition is tested at the bottom, so it always executes at least
  once. STMT_DO shares the `while` label namespace (`push_while`), so `break`
  (-> `Lwh<n>end`) and `continue` (-> `Lwh<n>cont`, the bottom test) ride the
  same break/continue stacks as while/for — a do-while continue re-evaluates the
  condition. Uses only `JIZ`/`JMP`, no hardware change. Locked down by the
  `cmm_dowhile` sim fixture (body-runs-once, normal loop, break, continue, and a
  nested do-while). `goto` is now the only C control-flow keyword still missing
  from C±.
- **`continue` statement (cmmcomp)** — C± gains `continue`, binding to the
  innermost enclosing loop (a `switch` does not catch it, matching C). It rides
  the same machinery as `break`: a **continue-target stack** in `saltos.c` that
  only loops push to. A `while`-continue jumps to the loop top (re-test the
  condition); a `for`-continue jumps to a step label (`Lwh<n>cont`) placed just
  before the step, so the step still runs and the index advances — a top jump
  would hang the loop. The `for` step now lives on the while node's `else_body`
  and is emitted after the body (instead of glued into it), and the
  `Lwh<n>cont` label is emitted only when a `continue` actually targets that
  loop, so **every existing test stays byte-identical**. Uses only `JMP` — no
  hardware change. Locked down by the `cmm_continue` sim fixture (continue in
  while, in for with the step-runs check, nested inner-only, and inside a switch
  inside a for). `goto` and `do/while` remain unimplemented in C±.
- **Real C `switch` fall-through (cmmcomp)** — a `case` without `break` now falls
  through into the next case body, and empty cases stack onto the following one
  (`case 1: case 2: case 3: foo();` runs `foo` for all three) — standard C
  semantics. The old codegen re-tested `switch_exp` at every case label, so a
  no-break case was *skipped* instead of falling through, and an empty case was
  a syntax error. The `STMT_SWITCH` walker now emits a **dispatch block** (one
  compare per case, jump to that case's body) followed by the **case bodies laid
  out contiguously**, so control falls from one body into the next with no
  re-test. The grammar's switch body became a flat `sw_body` (labels and
  statements interleaved, the C model), which also allows the empty-case stack.
  Uses only existing opcodes (`LOD`/`EQU`/`JIZ`/`JMP`) — no hardware change. Cost
  is about one extra `JMP` per switch; the assembly is no longer byte-identical,
  so the switch goldens and `size_baseline` were regenerated. Locked down by the
  `cmm_switch_fall` sim fixture (cascade, empty-case stacking and partial
  fall-through, output values checked against hand computation). Note: a jump
  table (O(1) dispatch for large dense switches) would need an indirect jump,
  which the ISA does not have — but that is an optimisation, not correctness.
- **Nested `switch` and correct `break` binding (cmmcomp)** — a `switch` can now
  appear inside a `case`, and `break` binds to the innermost enclosing loop *or*
  switch (the standard C rule). Previously the grammar had a separate
  "statements allowed in a case" tier (`stmt_case`) that omitted `switch`, so a
  nested switch was a syntax error; worse, a `break` inside an `if` inside a
  `case` was emitted as a loop break (`JMP Lwh<n>end`) and jumped out of the
  enclosing `while` instead of the switch. The fix replaces the grammar tier
  with a semantic **break-target stack** in `saltos.c`: every loop and switch
  pushes one entry, and `exec_break` resolves to the top, so a case body is now
  an ordinary statement list (blocks and nested switches included). Per-switch
  case numbering moved onto the switch's `stmt_node` so nested switches no longer
  share a global counter. **Byte-identical assembly** for every existing test
  (flat switches, loops and breaks emit exactly as before — verified by a
  zero-diff `--update` and the `size_baseline` ratchet); the new behaviour is
  locked down by the `cmm_switch_nest` sim fixture (nested switch + a
  break-in-if-in-case, output values checked against hand computation).
  (Fall-through was a limitation at this point; it is fixed by the
  fall-through entry above.)
- **Object-like `#define` in `cmmcomp`** — the C± lexer now handles
  `#define NAME body` directly, with no separate preprocessor stage: a later use
  of `NAME` is replaced by re-lexing its body (flex `yy_scan_string` + a buffer
  stack), so `#define LIMIT 256` lets you write `LIMIT` anywhere the literal
  would go. Nested defines expand; a self-referential define is expanded once
  and then left alone (a per-macro active flag prevents an infinite loop).
  Function-like macros, `#ifdef` and `#include` are out of scope. Locked down by
  a positive asm-golden fixture (`cmm_define`) and a recursion-guard negative
  test.
- **CMM negative phase in `regress.sh`** — malformed programs (syntax error,
  missing `main()`, undeclared variable, garbage, recursive `#define`) are now
  asserted to be rejected with a clean non-zero exit and the right diagnostic,
  never a crash or silent accept. Fixtures live in `Compilers/CMMComp/NegTests/`.
- **Sethi-Ullman operand ordering (cmmcomp codegen)** — when a commutative
  integer `+` / `*` has two complex operands, the AST walker now evaluates the
  heavier subtree first, so its result spends less time pushed on the shallow
  NDSTAC hardware stack (fewer pushes / spills). The reorder is value-identical
  (`S_ADD` / `S_MLT` are symmetric) and the AST is what makes it possible — the
  old parse-order emit was locked left-to-right. Validated functionally by the
  `cmm_reorder` sim fixture (output values checked against hand computation, not
  golden.asm). Scoped to int `+`/`*` for now; float is excluded (reassociation
  would change rounding).
- **Direct access for constant array indices (cmmcomp codegen)** — `arr[k]` with
  a compile-time-constant index into an int array now uses the `SET_V` (store)
  and `LOD_V` / `P_LOD_V` (read) pseudo-instructions: the assembler bakes the
  offset into a plain `SET`/`LOD` at `arr_base + k`, so there is no index to
  materialise, no stack push, and no indirect `STI`/`LDI` -- one fewer
  instruction per access, no stack slot, no indirect addressing. The deferred
  AST makes it clean: the index subtree is inspected before the access is
  emitted. Scoped to 1D-forward int arrays (int RHS for stores); 2D / reversed /
  float / comp fall back to the indirect path. Validated by the `cmm_arridx` sim
  fixture, which writes distinct values to distinct constant indices via `SET_V`
  and reads them back via `LOD_V` -- the readback values are checked against
  hand computation, so a wrong offset on either side would be caught.
- **Integer constant folding (cmmcomp codegen)** — a fully-constant int
  subexpression (`+ - * & | ^` of int literals) is evaluated at compile time and
  emitted as a single literal, folding deeply through nesting (`(2+3)*4` -> 20).
  Only when every intermediate stays within the signed NUBITS range, so the
  compile-time arithmetic matches the hardware. A non-negative result becomes a
  literal; a negative one has no literal form, so it is emitted as a `NEG_M` of
  its magnitude (`3-5` -> `NEG_M 2`) -- negatives compose too (`(3-5)+10` folds
  to `8`). An out-of-range/overflowing result falls back to the runtime op
  (which does the NUBITS wraparound), e.g. `30000+30000` is left to the runtime.
  Float is never folded (the YANC float is not IEEE-bit-compatible); bitwise
  ops fold only on non-negative operands. Composes with the constant-index array
  opt (`arr[2+3]` folds to `arr[5]`, then uses `LOD_V` / `SET_V`). Validated by
  the `cmm_cfold` sim fixture (folded output values checked against hand
  computation: 5 20 63 10 35 -7 8).

### Fixed
- **Memory-safety pass over the older compilers** — every `fopen` in
  cmmcomp/asmcomp/appcomp is now NULL-checked (a missing input file is a clean
  error instead of a segfault), and the path-building `sprintf`s became
  bounds-checked `snprintf`.

## [v5.0] – 2026-06-03

YANC v5.0 is the **first cross-platform release** — a big milestone. The whole
toolchain now builds and runs on both **Linux** (native `gcc`) and **Windows**
(MSYS2 / MinGW-w64) from one shared top-level `Makefile`, verified on every push
by CI on Ubuntu *and* Windows, with prebuilt binaries published for both
platforms. It also lands the `gen_gtkw` wave-view generator (replacing the
runtime Tcl formatters), named HDL generate blocks, and a batch of
cross-platform build hardening.

### Added
- **Cross-platform build from a single `Makefile`** — one top-level `Makefile`
  is now the single source of truth for every binary (cmmcomp, appcomp, asmcomp,
  cpppp, cppcomp, comp2gtkw, gen_gtkw). It emits `.exe` under Windows
  (`$(OS)=Windows_NT`) and bare binaries on Linux, and CI builds + smoke-tests
  the full pipeline on **both** Ubuntu and Windows/MSYS2 with `-Werror`.
- **Per-OS setup + runners** — `Scripts/setup.sh` (Linux) and `Scripts/setup.bat`
  (Windows) resolve the build / simulation / GTKWave tools and cache them via
  `env.sh` / `env.bat`; the example runners ship as both `single_proc.{sh,bat}`
  and `multi_proc.{sh,bat}` and pick the simulator with `--sim`. No hardcoded
  tool paths anywhere.
- **Prebuilt Linux binaries** — the release workflow now also builds on Ubuntu
  and publishes `yanc-bin-linux-vX.tar.gz` alongside the Windows
  `yanc-bin-vX.zip`.
- **`.gitattributes`** pins the shell scripts to LF (and the Windows scripts to
  CRLF), so a fresh checkout runs under MSYS2 / git-bash regardless of the
  cloning machine's `core.autocrlf` setting.
- **`Scripts/gen_gtkw.c`** — a C tool that parses a
  VCD header and emits a pre-formatted GTKWave `.gtkw` save file, porting the
  proven logic of Aurora's wave pipeline. It classifies the harness signals by
  name (I/O mirrors, the `valr2` Assembly / `linetabs` C+- tracks with their
  translate files, int/float/comp variables and arrays, the Stack/ALU flag
  groups) and writes the exact `.gtkw` opcodes. Handles **single and multiple
  processors**: every VCD scope owning both `valr2` and `linetabs` is a proc
  instance and gets its own section, with the type derived from the `p_<type>.core`
  sub-scope (so per-type `trad_opcode.txt`/`trad_cmm.txt` resolve) — single-proc
  is the N=1 case. Replaces the runtime `gtk_proc_init.tcl`/`gtk_proj_init.tcl`
  and lets us move to the nipscern GTKWave v4 (which ignores `--script`), opened
  as `gtkwave <vcd> -a <out.gtkw>`.
- **`+HEADER_ONLY` testbench instrumentation** — both the generated `<proc>_tb.v`
  (hdl.c) and the DTW project `top_level_tb.v` now respond to a `+HEADER_ONLY`
  plusarg: they dump, advance one tick, `$dumpflush` and `$finish`, producing a
  VCD that carries only the header (the signal list `gen_gtkw` needs) without the
  multi-gigabyte body. On the DTW project this is 109 KB in 130 ms instead of a
  641 MB full dump, and the resulting `.gtkw` is byte-identical to the one built
  from the full trace. Plusarg-gated, so normal and regression runs are
  unaffected (regress 71/71).

### Changed
- **Runners reorganised** — `go_proc` / `go_proj` are renamed to `single_proc` /
  `multi_proc`, the per-simulator variants are merged behind a
  `--sim {icarus,verilator}` flag, and each ships as both `.bat` and `.sh`. The
  runners now copy only the inputs (the `.cmm` / project inputs), not the whole
  project tree.
- **Build hardening** — `make` is invoked with bare `BISON=bison FLEX=flex` so an
  inherited backslash Windows path can't reach MSYS2's `/bin/sh`; `setup` probes
  for the broken MSYS2 gcc 16.1.0-5 libstdc++ (which kills Verilator) and warns;
  `regress.sh` resolves its tools through `env.sh` and links with `-lm` instead
  of hardcoded paths.
- **Named HDL generate blocks** — `core.v` and `instr_dec.v` name every
  `generate` block, so the simulation hierarchy shows meaningful names instead of
  `genblk2` / `genblkN`.
- **The runner wave flows now use gen_gtkw + the nipscern GTKWave v4.** Each
  builds `gen_gtkw.exe`, writes the `.gtkw` layout from the (header-only) VCD, and
  opens the waveform with `gtkwave --dark --zoom-fit --left-justify <vcd> -a
  <gtkw>` (no `--script`, no `fix.vcd` tab hack, no `--rcvar` — the nipscern fork
  hides the SST pane and reports that rcvar as not found). FST flows
  (the Icarus `single_proc`/`multi_proc` runs and the Verilator `multi_proc
  --sim verilator` run) add a quick `+HEADER_ONLY` pass — Icarus writes the
  header VCD directly; Verilator's tiny header FST is converted with
  `fst2vcd -f`. The `--zoom-fit` restores the whole-wave view the
  Tcl flow did via `Zoom_Best_Fit`.
- **`multi_proc.bat` sets `TMP`/`TEMP`** to its own Temp dir, so it no longer
  inherits a stale temp path from a previous bat run in the same cmd window
  (which broke gcc/iverilog with "cannot create temporary file").

### Fixed
- **`cpppp` realpath buffer overflow** — `path_canon`'s POSIX branch could
  overflow its buffer while canonicalising paths; fixed so the preprocessor is
  safe on Linux.
- **`racc` declared before use in the HDL** — `racc` is now declared ahead of the
  `uic_in2` generate block that references it, so iverilog v13 (which rejects
  forward references) builds the generated Verilog.

### Removed
- **The runtime GTKWave Tcl formatters and their props** — `gtk_proc_init.tcl`,
  `gtk_proj_init.tcl`, `gtk_almost_proj.tcl`, `pos_gtkw.tcl` and `fix.vcd` (the
  empty-tab crash workaround) are gone, replaced by `gen_gtkw.c`. Also removed the
  long-unused `proc2rtl.ys` Yosys script.
- **The old `go_*.bat` runners** (`go_proc` / `go_proj` / `go_proc_vl` /
  `go_proj_vl`) — replaced by `single_proc` / `multi_proc` (`.bat` + `.sh`,
  `--sim`-selected).
- **`HANDOFF.md`** — the merged branch's working notes.

## [v4.4.1] – 2026-06-01

A maintenance release on top of v4.4: a flaky-test fix and a clutter pass over
the Verilator harness in the generated `<proc>.v`. No simulation behaviour
changes (Icarus and Verilator traces are identical to v4.4).

### Fixed
- **Flaky DTW regression** — the multi-proc project pass dumped every signal of
  the heavy `top_level_tb` sim through the FST writer, which intermittently
  crashed `vvp` on Windows (exit 1, empty stderr) even though the design output
  was correct and the regression never reads the waveform. The tb's `$dumpvars`
  is now gated behind a `+WAVE` plusarg: `regress.sh` runs without it (no dump →
  stable and faster), while `go_proj.bat` / `go_proj_vl.bat` pass `+WAVE` to
  keep the GTKWave trace. Verified 0/25 sim failures and 3/3 green regress runs
  (previously ~13% DTW failure rate).

### Changed
- **Less Verilator clutter in the generated `<proc>.v`** — the per-signal
  `/* verilator tracing_off */ … /* verilator tracing_on */` wrappers on the
  `valr` PC-delay registers are now one block fence (valr2 traced, valr1+valr3..10
  fenced together), and the `sm_me2`/`e_me2` float decode helpers are emitted once
  in a single fenced block instead of inline-fenced and duplicated across the
  variable and array loops. The raw `comp` real/imag halves (joined into the
  shown `comp_<name>` wire) are likewise emitted as one fenced block instead of
  fencing each half inline. Pure codegen tidy-up, identical behaviour: regress
  71/71 and the Verilator trace still carries `valr2` (not `valr1/3..10`) with the
  ULA monitors still dead-code-eliminated.
- **`/* verilator public_flat */` only where it's needed** — it was stamped on
  every mirror declaration; now it's dropped from the *traced* signals (the
  waveform dump keeps those on its own) and kept only on the ones fenced out of
  the trace and the hierarchically-referenced `valr10` (the `_tb.v` `$finish`).
  Cuts the attribute roughly in half in the generated `<proc>.v`. Validated by a
  byte-identical Verilator VCD signal diff (same 19 signals on proc_fft) and the
  `$finish` still resolving `proc.valr10`.

## [v4.4] – 2026-05-31

Verilator polish on top of v4.3: a clean lint pass, simulation progress on the
terminal instead of a file, and the visibility/performance trade-off spelled
out.

### Fixed
- **Clean Verilator lint on the sim-visibility code** — the `YANC_SIM_VIS`
  helper signals were assigned across mismatched widths, so Verilator flagged
  `WIDTHEXPAND` (and `REALCVT` on a real `%`). Sized them explicitly with no
  behaviour change (the traced values are identical): `core.v` stack
  `pointeri`; `ula.v` signed mantissa/exponent helpers and the integer-remainder
  `val_mod` (now an integer `%` instead of a real one); and in the generated
  `<proc>.v` the `me2` float decode helpers, the `valr1 <= pc_sim_val` PC tap
  (now zero-extended), and an off-by-one in the `me3` complex `'dx` initialiser.
  The remaining warnings on a project come from user-side HDL (top level,
  wrappers, hand-written testbench), not YANC.

### Changed
- The auto-generated `<proc>_tb.v` now reports progress to the **terminal**
  (`$display "Progress: N% complete"` … `Simulation Complete!`, each in-loop
  line flushed) instead of writing a `progress.txt` file. Tools that drove a
  progress overlay off `progress.txt` (Aurora's `VVPProgressManager`) should
  parse the terminal lines instead — see
  [`docs/aurora-verilator-migration.md`](docs/aurora-verilator-migration.md).
- **The stack/ULA monitor signals are intentionally not in the Verilator VCD.**
  The stack-pointer flags and the ULA rounding-error taps (`fl_max`, `fl_full`,
  `pointeri`, `delta_int`, `delta_float`) sit below the
  `/* verilator tracing_off */` fence, so Verilator drops them — keeping them
  alive would force it to evaluate the costly real-valued ULA monitoring logic
  every cycle, defeating the speed that is the whole point of the Verilator
  backend. They stay available under Icarus (the GTKWave Stack/ALU groups are an
  Icarus-only feature). Documented in the README and the Aurora migration guide.
- Dropped the redundant `YANC_SIM_VIS` guards from the generated testbench — the
  tb is a sim-visibility artifact by definition, so those guards never changed
  anything. No behaviour change.

## [v4.3] – 2026-05-31

The Verilator waveform path now works end-to-end: the same
variables/arrays/PC-line view the Icarus flow always had now appears under
Verilator too. Downstream (Aurora) adopts it with two small changes — see
[`docs/aurora-verilator-migration.md`](docs/aurora-verilator-migration.md).

### Added
- **`go_proc_vl.bat` / `go_proj_vl.bat`** — Verilator siblings of
  `go_proc.bat` / `go_proj.bat`. They feed the generated `<proc>_tb.v` (or the
  project's `top_level_tb`) to Verilator (`--binary --timing --trace[-fst]
  +define+YANC_TRACE`), then open GTKWave the same way the Icarus scripts do,
  keeping each processor's user variables in the waveform.
- `docs/aurora-verilator-migration.md` — migration guide for the Aurora repo
  (pass `+define+YANC_TRACE`; drop the `$finish`-strip workaround).
- A **Simulating with Verilator** section in the README, plus the Verilator 5.x
  MSYS2 install and the `+define+YANC_TRACE` rationale.

### Fixed
- **Waveform visibility under Verilator (GTKWave)** — the sim-visibility
  harness (user variable/array mirrors, the PC→C± line table, the assembly
  opcode tap, the I/O port mirrors) was gated behind `` `ifdef __ICARUS__ ``,
  so Verilator never compiled it and the signals vanished from the trace. The
  harness — and the `pc_sim_val`/`mem_wr`/`mem_addr_wr` plumbing it needs in
  `HDL/processor.v` and `HDL/core.v` — now also compiles under
  `+define+YANC_TRACE` (the new `YANC_SIM_VIS` guard folds `__ICARUS__` *or*
  `YANC_TRACE`). Every mirrored declaration is tagged
  `/* verilator public_flat */`, so Verilator keeps it and the `_tb.v` can
  reach `proc.valr10` hierarchically — which means the **end-of-program
  `$finish` now works under Verilator** (no more running the full cycle budget
  on short programs).
- The **stack-pointer flags** (`core.v`) and the **ULA rounding-error** signals
  (`ula.v`) are no longer Icarus-only — the "Verilator rejects them" premise was
  wrong. They are re-gated to `YANC_SIM_VIS`, so the Stack/ALU waveform groups
  populate under Verilator too.
- **Latch-free under Verilator** — signals that were modelled as
  self-referential `always @(*)` (the `fl_max`/`fl_full` stack high-water marks
  and the `in_sim_*`/`out_sig_*` capture mirrors) are now the clocked
  accumulators / registers they actually are, so Verilator no longer infers
  latches or combinational loops. `fl_max`/`fl_full` evaluate on the pointer's
  next value, so the tracked values are bit-for-bit the same as before, on both
  Icarus and Verilator.

### Changed
- **The Verilator trace now carries only the curated `$dumpvars` set.**
  `--trace` would otherwise dump the whole hierarchy; the generated `<proc>.v`
  and `_tb.v` now fence the CPU internals, the PC-delay intermediates
  (`valr1`, `valr3..valr10`), the raw `comp` halves (`me3_*`), the float decode
  helpers, `linetab`, and the testbench plumbing out of the trace with
  `/* verilator tracing_off */` (no-op comments for Icarus). A `proc_fft` trace
  drops from 1124 signals to 19 — the user variables, `valr2` (Assembly track),
  `comp_*`, `linetabs` and the I/O mirrors. Synthesis and the Icarus flow are
  unchanged.
- Documentation: the language is referred to consistently as **C±** (was a mix
  of "CMM" and "C+-"); the `CMMComp`/`cmmcomp`/`.cmm` names are untouched.

## [v4.2] – 2026-05-28

### Fixed
- **Verilator cleanliness across the HDL** — silence the warning
  storm Verilator emits when running yanc HDL through `--lint-only`
  or the FISTA / Aurora Verilator harness. Each fix is value-
  preserving; iverilog regress stays 71/71 byte-identical.
  - **WIDTHTRUNC in every `generate if`**: Verilog parameters
    default to 32-bit integers, and the HDL uses dozens of
    opcode-enable flags (`ADD`, `F_ADD`, `MLT`, `INN`, `JIZ`, ...)
    as `generate if (FLAG)` conditions. Verilator's GENIF expects
    1 bit. Rewrote every such site (and OR'd combinations like
    `F_ADD | F_SU1 | F_SU2`) to `generate if ((EXPR) != 0)` —
    `HDL/ula.v` (47), `HDL/core.v` (6), `HDL/instr_dec.v` (107).
    Comparison-result conditions (`ITRADD > 0`, `TOAQUIADDR > 0`)
    already produce 1 bit and were left alone.
  - **`HDL/addr_dec.v`**: slice the integer loop variable to the
    index width (`index == i[$clog2(NPORT)-1:0]`) so the EQ no
    longer requires a 32-bit expand on the index side.
  - **`HDL/ula.v` comparison modules** (`ula_les`, `ula_fles`,
    `ula_gre`, `ula_fgre`, `ula_equ`): explicitly zero-extend the
    1-bit comparison result to `NUBITS` (`{{(NUBITS-1){1'b0}}, cmp}`)
    instead of letting an implicit ASSIGNW expand fire.
  - **`ula_denorm`**: zero-pad the MAN-bit mantissas in the COND's
    "false" branch so both ternary arms match the MAN+1-bit signed
    target.
  - **`ula_f2i`**: introduce `m_ext = {{(EXP+1){1'b0}}, m}` so the
    shift operand has the same width as `mag` (MAN+EXP+1).
  - **`su1` / `su2`**: cast the 32-bit `F_SU1` / `F_SU2` parameters
    with `!= 0` so the AND with the 1-bit op-equality stays 1 bit
    and matches the 1-bit LHS.

- **COMBDLY + LATCH on the auto-generated testbench's output
  decoder** — `Compilers/ASMComp/Sources/hdl.c` (`hdl_tb_file`)
  used to emit
  ```
  always @ (*) begin
      if (proc_out_en == N) out_sig_N <= proc_io_out;
      out_en_N = proc_out_en == N;
  end
  ```
  which Verilator flagged twice: `<=` inside a combinational always
  (COMBDLY), and the conditional with no `else` inferred a latch on
  `out_sig_N`. Switched to an unconditional combinational
  `out_sig_N = proc_io_out;` — the file-write block below already
  gates on `out_en_N`, so the captured per-cycle value is
  byte-identical to before, just without the warnings.
  The corresponding proc-side sim block in `hdl_vv_file` is left
  alone: it sits inside `\`ifdef __ICARUS__`, so Verilator never
  sees it, and switching it to the unconditional form broke the
  multi-proc DTW project (its top-level testbench depends on the
  latched per-port semantics of those signals).

### Changed
- The early-`@fim` `$finish` handler and the `integer progress, chrys;`
  declaration now sit AFTER the `// signal registration, progress
  bar and finish` comment in the generated `_tb.v`, grouped with
  the rest of the sim harness. No behavior change — purely a
  layout move so all the simulation-only constructs live in one
  block.

### Release packaging
- `YANC_VERSION` bumped to `"4.2"`.
- Zip content unchanged from v4.1 (25 files, no `Scripts/`).

## [v4.1] – 2026-05-28

### Fixed
- `asmcomp`-generated testbench now closes `progress.txt` on the
  early-`@fim` `$finish` path. The testbench has two `$finish` paths
  — an `always @(posedge clk)` block that fires when the program
  reaches `@fim`, and an `initial` block that runs the cycle-budget
  loop — and only the second one was closing the file handle. On
  programs that actually reach `@fim` (the common case) the
  simulation exited with the file still open. The `integer progress,
  chrys;` declaration was moved above the `@fim` always block so the
  handler can `$fclose` it.

### Changed
- `Scripts/` is no longer copied from yanc into Aurora. Neither the
  local `aurora.bat` deploy nor the release zip ship anything under
  `Scripts/` anymore. Aurora manages its own scripts (`proc2rtl.ys`,
  `copy-components.js`, `download-*.js`, ...), and the GTKWave Tcl
  init / `fix.vcd` path was replaced by `gen_gtkw` emitting a
  static `.gtkw`.
- `aurora.bat` hardened: the cleanup phase now `rmdir` + `del` +
  `mkdir`s each yanc-managed folder (`bin`, `HDL`, `Macros`,
  `Header`), so an interrupted previous run that left a stray FILE
  named `bin` (where the directory should be) is auto-repaired on
  the next invocation. All `move` commands got `/Y` and all `xcopy`
  commands got `/I` so the script never prompts.

### Docs
- README dropped two stale `#USEMAC` references. `appcomp` never had
  a `#USEMAC` directive — that was the CMM user-macro feature
  removed in v4. The `appcomp` row now reads "first pass over the
  `.asm`: records processor params + resolves variable/label
  addresses for `asmcomp`", which is what it actually does.

### Release packaging
- `YANC_VERSION` bumped to `"4.1"`.
- Release zip content shrinks from 31 to 25 entries: `bin/` (6) +
  `HDL/` (6) + `Macros/` (5) + `Header/` (8). No `Scripts/`.

## [v4] – 2026-05-28

### Added
- **CPPComp** — new C++ compiler (`cppcomp`) and preprocessor (`cpppp`)
  targeting YANC's 32-bit / IEEE-754 / 4 K core. Implements tiers 1-3:
  classes, single inheritance, virtual functions + vtables, function
  and class templates with real monomorphization (non-type template
  parameters, mixed type/value args), namespaces and `::` qualified
  names, references, `new`/`delete` over a 4 K heap, RAII (stack
  ctor/dtor), operator overloading (binary / unary / compound /
  subscript / call), `enum class`, `using` aliases, C++ casts,
  range-for, default arguments, in-class method declarations with
  out-of-class definitions, static data members, `static` methods,
  `Class<T>::static`, member-init lists, mini-STL written in the
  language (`Vector<T>`, `unique_ptr<T>`, `std::vector`), and
  `<array>` / `<cstddef>` / `<cstdint>` / `<cstring>` / `<cmath>` /
  `<limits>` / `<bit>` shims under `Compilers/CPPComp/Includes/`.
  End-to-end milestone: FISTA `blind_deconvolve` runs on YANC
  (`Compilers/CPPComp/Tests/test48`, `test49`), validated against a
  reference ARM build (`test50`).
- Single shared `YANC_VERSION` (`Compilers/yanc_version.h`) — bumped
  to `"4.0"`. All five binaries (`cmmcomp`, `cppcomp`, `asmcomp`,
  `cpppp`, `appcomp`) read it for `--version`.
- `cppcomp` long-form aliases for every CLI option.
- CMM `#TOAQUI` directive plus a new `cheguei` processor output pin
  that asserts when PC reaches the marked address. The Verilator
  harness terminates the simulation on that pin (replacement for
  manual `$finish` plumbing in heavy sims).
- CMM `for` loop, desugaring to `while` plus an init / step pair.
- ISA: `LDA` / `STA` base-less indirect addressing and a `LEA`
  pseudo-op.
- Verilator runner for the heavy FISTA tests
  (`Compilers/CPPComp/.work/verilator/`), ~seconds instead of minutes
  on iverilog.
- `regress.sh` now drives the full toolchain end-to-end (cmmcomp →
  appcomp → asmcomp → sim) and diffs the produced output files
  against goldens; multi-proc DTW project covered; `num_ins` ratchet
  prevents future refactors from growing the `.asm`.

### Changed
- **Repo layout** — all compilers now live under `Compilers/`
  (`APPComp/`, `ASMComp/`, `CMMComp/`, `CPPComp/`). `Macros/` and
  `include/` consolidated into a single `Includes/` per compiler.
  `Exemplos/` + `Testes/` folded into `CMMComp/Tests/`. `APP/` and
  `ASM/` renamed to `APPComp/` / `ASMComp/`. `build.bat` (repo root)
  renamed to `Scripts/aurora.bat` with relative paths and no
  hardcoded MSYS2 install root.
- **CMMComp** — expression and statement codegen migrated to a real
  AST: POD `expr` is gone, every expression builds an `expr_node`
  tree that the emit walker traverses; `if` / `while` / `switch` /
  function bodies emit through statement-AST walkers; `emit.c`
  (capture-buffer indirection) retired; symbol table moved to
  `struct symbol` + `v_table`, retiring the SoA parallel storage.
  Added a typecheck pass that annotates `BINOP` / `UNOP` /
  `STDLIB_CALL` / `INNER`, plus algebraic identity folding (`x+/-0`,
  `x*1`, `x/1`).
- **CMMComp** — `#INTERPOINT` keyword renamed to `#PRACA`.
- **CLI** — `cmmcomp` / `asmcomp` `-P` / `--project` renamed to
  `-A` / `--array`; `asmcomp`'s `--array` flag dropped entirely;
  `cppcomp` `-p <proc_dir>` is now the only output mode (the old
  `-o` was removed).
- **HW (breaking)** — `JIZ` now tests the whole accumulator word
  (`if_acc = |ula_out`) instead of just bit 0; counting loops
  (`i++` until zero) now terminate correctly.
- **HDL** — combinational `always` blocks use blocking assignments;
  `asmcomp`-generated testbench flushes output on each write
  (kills regress flakiness from buffered tb output).
- `@fim $finish` moved from the generated processor `.v` to the
  auto-testbench.
- README rewritten to reflect the current toolchain shape;
  GTKWave source-trace screenshot added.

### Removed
- Standalone CNIPS C compiler (added during v4 development, then
  removed once CPPComp covered the C++ — and by extension the C —
  use case).
- CMM `#USEMAC` / `#ENDMAC` user-macro feature (unused).
- `cpppp` `-D NAME[=val]` CLI flag.
- `CPPComp/build.bat` and `CPPComp/regress.sh` shim.

### Release packaging
- `release.yml` now builds and ships `cpppp` + `cppcomp` and ships
  `Header/` (CPPComp `Includes/`) alongside `bin/` / `HDL/` /
  `Macros/`. `Macros/` is sourced from `Compilers/CMMComp/Includes/`
  (the old root `Macros/` no longer exists). `Scripts/` in the zip
  is now a single file (`proc2rtl.ys`) — the gtkwave init Tcls and
  `fix.vcd` are no longer bundled, since `gen_gtkw` emits a static
  `.gtkw` from the VCD.

## [v3] – 2026-05-14

### Added
- MIT `LICENSE` file (was previously "all rights reserved" by default).
- `CONTRIBUTING.md` documenting code, commit, and bilingual-message
  conventions.
- Per-push CI workflow (`.github/workflows/ci.yml`) that builds all four
  binaries (`cmmcomp`, `appcomp`, `asmcomp`, `comp2gtkw`) with
  `-O2 -Wall` on Windows + MSYS2, then smoke-tests the full
  `cmmcomp → appcomp → asmcomp` pipeline against every example in
  `Exemplos/`.
- `MSG_ERR_OUT_OF_MEMORY` bilingual diagnostic (replaces
  `MSG_ERR_TOO_MANY_VARS` / `MSG_ERR_TOO_MANY_LABELS`).
- `-h` / `--help` and `-V` / `--version` on all three compilers, plus
  bilingual `MSG_CLI_*` / `MSG_ERR_CLI_*` diagnostics for malformed
  command lines.

### Changed
- All three compilers now take **named command-line options** instead of
  bare positional arguments (`cmmcomp -i file.cmm -n name -p ... -m ... -t ...`,
  `appcomp -i ... -t ...`,
  `asmcomp -i ... -p ... -d ... -m ... -t ... -f ... -c ...`, with
  `-P` / `--project` for project mode). Each compiler validates that every
  required option is present — and `asmcomp` that `-f` / `-c` are integers —
  printing a usage message and exiting instead of dereferencing a missing
  `argv[]` slot. Parsing lives in a new per-compiler `args.c` / `args.h`.
  `go_proc.bat`, `go_proj.bat`, `build.bat`, and the CI invocations were
  updated to the new flag form.
- Symbol and label tables in all three compilers are now grow-on-demand
  via `realloc` (starting at 256 entries, 128 for the label-nesting
  stack), instead of fixed `NVARMAX=999999` / `NLABMAX=99999` BSS
  arrays. Per-process resident memory drops from ~500 MB of zeroed BSS
  to ~128 KB initial.
- All in-source comments translated from Portuguese to English across
  `APP/`, `ASM/`, `CMMComp/`, `HDL/`, `Scripts/`, `Macros/`, the build
  `.bat` files, and the runnable examples in `Exemplos/`.
- README expanded with pipeline diagram, component table, build
  instructions, CLI usage, a runnable CMM example, and project layout.
- `release.yml` now links the new `args.c` into every compiler build
  (was missing it, which would break the release link step); inline
  comments translated to English.
- `.gitignore` comments translated to English; added `/.smoke/` and
  `.vscode/`.

### Removed
- `.vscode/c_cpp_properties.json` is no longer tracked (IDE-specific
  config with hard-coded MSYS2 paths from the original machine).

## [v2] – 2026-05-13

### Added
- Release workflow now bundles `HDL/` and `Macros/` into the release zip
  alongside `bin/`, so Aurora can extract `components/{bin,HDL,Macros}/`
  in one shot.

## [v1] – 2026-05-13

### Added
- Initial release artifact: tag-driven GitHub Actions workflow that
  builds `cmmcomp`, `appcomp`, `asmcomp`, and `comp2gtkw` with MSYS2 +
  MinGW-w64, packages them in `yanc-bin-vN.zip`, and publishes the zip
  as a release asset.

[Unreleased]: https://github.com/nipscernlab/yanc/compare/v5.0...HEAD
[v5.0]: https://github.com/nipscernlab/yanc/releases/tag/v5.0
[v4.4.1]: https://github.com/nipscernlab/yanc/releases/tag/v4.4.1
[v4.4]: https://github.com/nipscernlab/yanc/releases/tag/v4.4
[v4.3]: https://github.com/nipscernlab/yanc/releases/tag/v4.3
[v4.2]: https://github.com/nipscernlab/yanc/releases/tag/v4.2
[v4.1]: https://github.com/nipscernlab/yanc/releases/tag/v4.1
[v4]: https://github.com/nipscernlab/yanc/releases/tag/v4
[v3]: https://github.com/nipscernlab/yanc/releases/tag/v3
[v2]: https://github.com/nipscernlab/yanc/releases/tag/v2
[v1]: https://github.com/nipscernlab/yanc/releases/tag/v1
