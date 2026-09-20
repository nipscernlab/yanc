# TODO

Open work items for YANC that are not tracked elsewhere. This file is the
**list**: what is wrong, why it matters, what "done" looks like. The evidence
(file:line, measurements, the full inventory of 32-bit assumptions) lives in
[`docs/precision-and-width-review.md`](docs/precision-and-width-review.md) and
[`docs/hdl-architecture-audit.md`](docs/hdl-architecture-audit.md); each item
links to its section there. The measurements themselves are reproducible with
[`Scripts/hw/`](Scripts/hw/README.md) (elaboration, Yosys area/depth, Quartus
Fmax, the divider testbench). Remove an item when it lands (the
history stays in git and in the CHANGELOG).

Items 1–4 are HDL, 5–6 toolchain, 7–8 HDL scaling/timing, 9 libraries,
10 architecture hardening (from the HDL audit), 11 consistency at 32 bits,
12 a run-time exception strobe (parked, noted 2026-09-20).
Items 1, 3 and 4 landed as `#FROUND 1` and item 2 as `#FROUND 2` (see the
CHANGELOG); the default level `0` keeps the legacy datapath, so no C± golden
moved.

**Where the work stands (2026-09-18).** Item **8 is closed** — all six steps,
each measured; step 4b was withdrawn by the measurement rather than done. The
hygiene of item **7** (audit 1.7) is done except the invariant guard, which
item 3 had parked for the same reason: it needs a form Icarus, Verilator,
Yosys *and* Quartus all accept silently. The two decisions below are taken:
**YANC stays at 32 bits** and everything above it is parked. **Next in the
suggested order: 11 → 5 → 6(a) → 9**, all of them about making 32 bits
consistent.

---

## Decisions taken (2026-09-18)

- **Word width: 32 bits, for now.** The width sweep
  ([`Scripts/hw/width_sweep.sh`](Scripts/hw/width_sweep.sh)) found Icarus 13.0
  miscomputing division above 32 bits: unsigned `/` from 36 bits up, signed `/`
  in a procedural block above 64. The one form the HDL uses (signed `/` in a
  continuous assign) is right at every width, but a design that only works
  because it avoids three simulator bugs is not a base to build on. Nothing
  above 32 bits is maintained until the simulators are fixed: item 6(b) and
  item 7 are parked, and no code has to be written to be ready for them.
  **Re-run the sweep whenever Icarus or Verilator is upgraded**; 64 bits
  becomes possible when every operator at 64 comes back with 0 errors in both
  simulators. [§3, decision A]
- **Hidden mantissa bit: no.** The format keeps its explicit leading one.
  [§1.8]

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
- (a) ~~`F_DIV` sticky is approximate~~ — closed by the explicit divider
  array (item 8 step 4): the remainder gives the exact sticky.
- (b) **"Unbiased on varying data" is only shown on the host.** The core was
  checked with a constant addend (`cmm_fround2`, 1000 × 0.001), which is
  correlated and drifts +78 ULP — exactly what IEEE `float` does on a PC.
  A fixture accumulating varying data (e.g. a sine table) at level 2 should
  show mean error ≈ 0 and size ~sqrt(N) ULP against a double reference, and
  `delta_float` zero-mean on the waveform.

**Done when:** (b) has its fixture.

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
validates `NBMANT < 2^(NBEXPO-1)` and refuses `NUBITS > 32`; keeps the
flush-to-zero of sub-normal constants at `#FROUND >= 1` (landed in the old
`f2mf`, with the `cmmcomp` warning); a fixture shows constants correct to the
last mantissa bit at 32/23/8, at a 32-bit format with more mantissa than a
host `float` (e.g. 32/25/6), and at 16/10/5.

## 6. ULA unit testbench (a); word width beyond 32 bits (b, parked)

### 6(a). Self-checking ULA unit testbench

**Status:** open · **Area:** `Scripts/hw/tb_alu.v`, `Scripts/regress.sh`

A directed regress pass like `ResetCheck`: every float operator, including
`F_ROT`, `F_SGN`, `F_LES`/`F_GRE`, the `_M` variants, `I2F`/`F2I` at the
range edges, and the integer operators at their signed edges, at `FROUND`
0/1/2 and at 16/10/5 and 32/23/8, with the expected values derived in the
testbench rather than blessed.
[`Scripts/hw/tb_alu.sh`](Scripts/hw/tb_alu.sh) is the seed of it: shifts,
`F2I`, the float comparison, `DIV`/`MOD`, four formats × three levels,
references derived, mutation-checked. What is left is the other operators and
wiring it into `regress.sh`. Until then levels 1 and 2 are covered only by
`cmm_fround1/2` and (level 2) the C++ tests.

**Done when:** every ULA operator is in `tb_alu.v` and `regress.sh` runs it.

### 6(b). Word width beyond 32 bits — parked

**Status:** parked by decision (see *Decisions taken*): the simulators do not
compute division right above 32 bits. Do not write code for it. **Watch:**
re-run [`Scripts/hw/width_sweep.sh`](Scripts/hw/width_sweep.sh) after every
Icarus or Verilator upgrade (last run: Icarus 13.0, Verilator 5.048,
2026-09-18); reopen when 64 bits comes back with 0 errors in both. The
evidence is in [§2.2–2.5](docs/precision-and-width-review.md#22-asmcomp--the-actual-ceiling)
and [§2.7](docs/precision-and-width-review.md#27-alu-efficiency-review-area-and-depth-operator-by-operator).

What reopening it will involve, measured once so it need not be redone:
`itob(int)`, `v_val` (`int`), cmmcomp's `long` constant folder, cppcomp's
`long ival` / `(long)strtoull` / `1L << (g_nubits-1)`, `sim_main.cpp`'s
`(int)top->out` and `comp2gtkw`'s `int` mantissa all cap the word at 32 bits
(`long` is 32-bit on Windows); cppcomp seals struct/bitfield layout with a
fallback of **16**, not `CFG_NUBITS`; APPComp and the HDL need nothing.
Fixtures at 48/40/7 and 64/52/11 through both front ends under both
simulators, with a host-double reference like `test50`; `tb_alu.v`'s
procedural division reference must be replaced above 64 bits.

## 7. HDL scaling for wide mantissas

**Status:** parked with item 6(b) · **Area:** `HDL/ula.v`, `HDL/core.v` · **Evidence:** [§2.1](docs/precision-and-width-review.md#21-hdl--parametric-with-a-handful-of-scaling-issues)

The structural fixes that make a wide mantissa feasible at all (log-depth
leading-zero tree, explicit divider arrays, single shifters) are item 8 and
are done; so are the typed `NUGAIN` and the one default parameter set
(items 8.6 and 10.7). What remains only matters above 32 bits: nobody has yet
synthesised a 52-bit mantissa.

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

**Done when (on reopening):** the 64/52/11 configuration is synthesised and
its depth/Fmax reported next to 32/23/8 (the 105/52-bit divider array is the
expected limit — its cost is the user's input for choosing `NBMANT` per
project).

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
1. ~~sign-magnitude adder, parallel `e1-e2`/`e2-e1`, one denormaliser
   shifter~~ — **done** (`58944d9`: level 0 45.1 → 51.0 MHz);
2. ~~`F_MLT`/`F_DIV` skip the LZC; carry-select exponent and range check;
   thermometer-mask sticky~~ — **done** (level 2 on a division-free
   processor 35.0 → 40.3 MHz; a log-depth leading-zero tree was measured
   and reverted — `abc` already balances the chain, see §2.7);
3. ~~`F_LES`/`F_GRE` as a lexicographic compare (no denormaliser)~~ —
   **done** (comparisons alone 314 → 132 LUT4 and 20 → 10 levels at level 0,
   403 → 132 / 18 → 10 at level 2; the full float ALU pays ~2 % area at the
   same depth. Levels 0/1 now order a value against a much larger one instead
   of calling them equal after the alignment shifted it out);
4. ~~explicit restoring divider array for `F_DIV` (exact sticky → closes
   item 2(a))~~ — **done**. The follow-up "one array for `DIV`+`MOD`,
   ≈ −50 %" is **withdrawn: measured, the tools already share one divider**
   (`DIV` 1796 LUT4, `MOD` 1863, both 1914 — 6 % marginal). What an explicit
   array would still buy is *defined division by zero*, which a cheap ternary
   guard cannot give (+80 %: the guard breaks that sharing). Decide with
   item 10.5, not here;
5. ~~one shared right shifter for `SHL`/`SHR`/`SRS`, one for `F2I`~~ —
   **done** (`SHL`+`SHR`+`SRS` 504 → 363 LUT4 at the same depth; `F2I`
   387 → 350; the integer no-divider ALU −5 %, the whole no-divider ALU
   −3.5 % / −5.6 % at levels 0 / 2. A processor with a single shift opcode
   builds exactly what it built before — the selects are parameter
   constants then);
6. ~~`NUGAIN` restricted to a power of two, validated by `cmmcomp`/`asmcomp`~~
   — **done**: `asmcomp` refuses it (the gate every front end passes
   through) and `cmmcomp` refuses it with the source line; `NUGAIN` typed
   `signed [NUBITS-1:0]` in `processor.v`/`core.v`/`ula_nrm`. Measured
   before deciding: 64 → 152 LUT4 / 12 levels, 128 → 147 / 11, **100 → 406 /
   70**, 3 → 264 / 38 — not the full divider feared, but 2–3× the depth of
   the whole no-divider ALU (36), i.e. the clock halved.

Targets: full no-divider ALU ≈ 30 levels at every level, ≈ −15 % LUT4;
`F_DIV` ≈ 260 levels, ≈ −50 % LUT4; every golden unchanged.

## 9. Library accuracy keyed on `NBMANT`

**Status:** open · **Area:** `CMMComp/Includes/float_*.asm`, `CMMComp/stdlib.c`, `CPPComp/Includes/cmath` · **Evidence:** [§1.6](docs/precision-and-width-review.md#16-library-accuracy-is-pinned-to-23-bits)

The polynomial fits are accurate to ~1e-6 (≈20 bits), π/e constants have
10–12 digits, and cppcomp's `sqrt` runs a fixed 24 Newton iterations; cppcomp
has no `exp/log/sin/cos/pow` at all.

**Done when:** tables/iterations are selected by `nbmant` and accurate to the
last mantissa bit of every 32-bit format (up to 32/25/6), constants carry
enough digits for it, and the transcendental fixtures compare against a double
reference at 32/23/8 and 16/10/5. Wider sets wait for item 6(b).

---

## 10. Architecture hardening (HDL audit)

**Status:** open · **Area:** `HDL/*`, tooling · **Evidence:** [`docs/hdl-architecture-audit.md`](docs/hdl-architecture-audit.md)

The core is sound where it matters (opcode-driven allocation, one instruction
per cycle, bypass by construction, single clock / sync reset — keep all of
it). The audit found the weak points in the surroundings. In order:

1. **ISS + random differential test** — an instruction-set simulator in C
   and a random program generator; compare its trace against Icarus/Verilator
   on thousands of programs. Catches control bugs the goldens cannot; home of
   the ULA unit testbench planned with item 6.
2. **Single-source ISA table** — mnemonic / opcode / ALU op / stack and I/O
   effects in one file; generate `ASMComp.l` rules, `instr_dec.v` compares and
   `ula_op` table, `opcodes.c` names and `docs/isa.md`; or at least a regress
   check that the four hand-written copies agree (`core.v` also hard-codes
   `JMP`/`JIZ`/`CAL`/`RET` as 5-bit literals).
3. **Interrupt** — today a level-sensitive PC override: the vector instruction
   re-executes every cycle the level is held, no PC save, no mask. Behind
   `ITRADD`: one-shot edge detect, optional PC push so the handler can `RET`,
   a mask bit.
4. **Stacks sized by the compilers** (static call/expression depth) with
   `#NDSTAC`/`#SDEPTH` as overrides the assembler validates, plus a sticky
   overflow pin like `cheguei`.
5. **Defined behaviour** for integer `DIV`/`MOD` by zero (with the shared
   `DIV`+`MOD` array of item 8 step 4b), address and jump-target truncation,
   documented.
6. **I/O contract** documented; a status-port convention (`empty`/`full`
   readable with `in()`); valid/ready with a global stall only as a parameter
   option if streaming peripherals become a goal.
7. **Hygiene** — ~~one default parameter set~~ (done: `cppcomp`'s set,
   the one the 81 C++ tests actually run on, now in `asmcomp`, `cmmcomp` and
   the three HDL files, named at the top of `processor.v`), ~~`NUGAIN`
   typed~~, ~~unused `NBOPCO` parameter of `ula`~~; still open: the invariant
   guard (`NUBITS == NBMANT+NBEXPO+1`, `NBMANT < 2^(NBEXPO-1)`, `NUGAIN` a
   power of two — it needs a form every simulator *and* synthesizer in the
   flow accepts silently, which is why item 3 parked it too), lint-clean
   Verilator, `mem_instr`'s fake write commented or removed, `myFIFO` with a
   registered read so it maps to block RAM.
8. **Registered-branch option** — only if Fmax ever matters more than the
   one-cycle branch; an ISA change with compiler support, never the default.

**Done when:** 1–7 landed; the ISA reference is generated, not hand-written;
the interrupt and I/O contracts are in `docs/isa.md`; every undefined case in
the audit's table has a defined, documented result.

## 11. Everything consistent at 32 bits

**Status:** open · **Area:** `HDL/ula.v`, `Compilers/CPPComp` · **Evidence:** [`Scripts/hw/width_sweep.sh`](Scripts/hw/width_sweep.sh), this item

With the width fixed at 32 (see *Decisions taken*), 32 bits has to be right
everywhere: both simulators, both front ends, the host reference. Known so far:

- (a) **`DIV` of `INT_MIN` by `-1` differs between the simulators.** Icarus
  returns `INT_MIN` (the wrapped quotient); Verilator returns `0` (its runtime
  guards the host's divide trap at exactly 32 and 64 bits). Measured by the
  width sweep at 32 bits. The same program therefore prints different results
  under `--sim icarus` and `--sim verilator`. Pick one result and make the HDL
  produce it on both simulators; decide together with the by-zero case of
  item 10.5, since both are "defined behaviour for `DIV`".
- (b) **I/O of an `unsigned` at or above 2^31.** The testbench writes every
  output word as signed decimal (`hdl.c:772` `%0d`; the Verilator harness
  `sim_main.cpp:102` `%d`), so `out(0, 3000000000u)` puts `-1294967296` in
  `output_0.txt`: the bits are right, a comparison with a host printout is
  not. Input is parsed with `%d` on both sides (`hdl.c:695` `$fscanf`,
  `sim_main.cpp:48` `fscanf` into an `int`, undefined above `INT_MAX`), so
  the two simulators may read such an input differently — to verify. Decide
  whether this is documented or the port learns the signedness it carries.
- (d) **A `static` local object is constructed at program start**, with the
  globals, not when control first reaches its declaration. Only a constructor
  with side effects shows it (`test70` compares a construction count, not the
  order). C++'s rule needs a guard flag and a test at every entry.
- (e) **`T x(N::v);` parses as a function prototype** when the first
  argument is a namespace-qualified variable; C++ decides by name lookup and
  declares an object. A plain variable (`T x(v);`) works since `test71`.

**Done when:** (a) gives the same result under both simulators, with a
fixture that runs under both; (b) is decided and documented; (d) and (e) have
fixtures that match the host.

## 12. Run-time exception strobe (pin + error code)

**Status:** open, not now (noted 2026-09-20) · **Area:** `HDL/*`, `ASMComp`, both front ends · **Evidence:** this item

Today a run-time failure is silent: `malloc`/`new` returns `0` when the
`__heap` arena (fixed `CFG_HEAPSZ` = 2048 words, no analysis of what the
program will allocate) is exhausted, the software call stack `__cstk` (fixed
1024 words) and the hardware stacks (`#NDSTAC`/`#SDEPTH`) overflow without a
trace, `DIV`/`MOD` by zero and exponent overflow (item 3) give whatever the
datapath gives. Nothing outside the processor can tell that it happened.

**Design (Luciano, 2026-09-20):** an output pair — a **strobe pin** high for
exactly one clock cycle, and an **error-code word** valid on that cycle —
decoding *which* exception occurred. One mechanism, many exceptions: heap
exhausted, call-stack overflow, data/return-stack overflow (subsumes the
sticky pin of item 10.4), integer divide by zero (item 10.5), float
overflow/underflow, `F2I` saturation, ... each with its own code. It costs
hardware (comparators, the code register, the pins), so it is **opt-in
through a new directive** (`#EXCEPT`-style, default off): a processor that
does not ask for it builds exactly what it builds today ([pay only for what
you use](docs/hdl-architecture-audit.md)). Same spirit as `#TOAQUI`/`cheguei`.

Points to settle when it is taken up: which exceptions are raised by the
hardware (stack overflow, divide by zero, float range) and which by the
runtime (heap exhausted — the compiler needs an instruction or an I/O
convention to fire the strobe with a code from software); the code table,
shared by the HDL and both compilers (goes into the single-source ISA table
of item 10.2); whether the processor halts, traps to `ITRADD`, or just
signals and continues; how the testbenches and the Verilator harness report
it (an `output_err.txt`, or a line in `app_log.txt`); how Aurora shows it.

**Done when:** the directive exists and is off by default; with it on, each
listed exception fires the strobe once with its code in a fixture under both
simulators; with it off, area and depth are unchanged (`Scripts/hw/area.sh`).

## Workarounds at `#FROUND 0` (worth a line in the README)

- Keep long-running accumulators (phase, time base, counters) in **integer /
  fixed point**: `ADD` is exact, and wrap-around modulo 2^NUBITS gives a free
  "mod 2*pi" for binary angles.
- Accumulate **deviations** from a nominal value instead of absolute values.
- Raise `#NBMANT` for the processor that accumulates (up to 23 — item 5).
- A compensated (Kahan) sum does **not** help at level 0: it relies on
  `t - sum` being exact, which loses a bit there. It works from level 1 on.
