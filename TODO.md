# TODO

Open work items for YANC that are not tracked elsewhere. This file is the
**list**: what is wrong, why it matters, what "done" looks like. The evidence
(file:line, measurements, the full inventory of 32-bit assumptions) lives in
[`docs/design/precision-and-width-review.md`](docs/design/precision-and-width-review.md) and
[`docs/design/hdl-architecture-audit.md`](docs/design/hdl-architecture-audit.md); each item
links to its section there. The measurements themselves are reproducible with
[`Scripts/hw/`](Scripts/hw/README.md) (elaboration, Yosys area/depth, Quartus
Fmax, the divider testbench). Remove an item when it lands (the
history stays in git and in the CHANGELOG).

## Where things stand (2026-10-17)

**v7.1 released on 2026-10-13** (no v7.0), with AURORA moved to it the same
day. The manual is published from docs/ by each release: GitHub Pages and
nipscern.com/library/yanc. Left from it: the board run of Sabrina and Pedro
Henrique (the `LINKALL` pair with the `.sdc` on both boards; the Fmax is
measured, the link costs no clock).

**Item 29 closed 2026-10-17 (next release):** `sapho_all` 21.8 -> 32.4 MHz
on the DE10-Nano (mean of three seeds), 23.5 -> 37.3 MHz on the ZYBO, the
same area: dividers in 5/4 stages, every mux guided by the opcode
parameters (no `x` left), `JIZ` and the `LDI`/`ILI` index before the float
normaliser (asmcomp inserts the `NOP`), the tree leading-zero count, the
decoder in the ALU's form (CHANGELOG [Unreleased], design note 4.4).
AURORA's text for the release: `C:	mpurora_yanc_divseq.md`.

**Recorded, not to be implemented yet (Luciano, 2026-10-14):** item 28, the
burst (a whole array through a port at one word a clock, stopping while the
FIFO is full); the design note lists the holes to close first.
Also recorded the same day: item 30, SAPHO without showing its
source (parked until a partnership), and item 31, the Dirac array output
(a wasted `MLT 1`, a missing form; open).

**Waiting on a decision:** item 21 (documentation that cannot fall behind
a release): Chrysthofer's proposal arrived on 2026-10-13; Luciano and he
agree on it (its phase 0). The GitHub plan: the proposal says GitHub
Education gives teachers Team for free (Luciano's application).

**Waiting on Luciano, proposed and not started:** `RAMBLK`, a registered
read on `myFIFO`/`myDCFIFO` so a large FIFO goes to block RAM (area, not
clock; docs/design/myfifo.md "Memory"), with a `quartus_map` of it.

**Open items, below:** 2(b) a fixture for unbiased rounding; 3 the parameter
guard in `ula.v`; 5 steps 4-5, the exact constant encoder in cmmcomp (show
Luciano the diff); 6(a) the ULA testbench; 8 the leftovers; 9 library
accuracy keyed on `NBMANT`; 10 architecture hardening (ISS, division by zero,
interrupt, I/O contract); 13 the pre-assembly optimizer and the C++ heap
arena (77 % of its data memory); 14 the zero-fill at 8 words a turn, or close
it; 15 the regress on this machine (mitigated); 17 faster fill/copy in
hardware (measure the index register `X` first; shares its pointer with 28); 24 processors with
different float formats on one link (parked: AURORA's check); 26 arrays in the waveform without a signal per element; 28 the burst (recorded); 31 the Dirac array output; 32 the
divider stage count from the widths (recorded; on the ZYBO `F_DIV` limits); 33 a
float as a condition (study first: C+- truncates, C++ is always true).
**Parked by decision:** 6(b) and 7 (wider than 32 bits), 12 (run-time
exception strobe), the reorganisation of the runner/setup scripts, a macOS
build, 30 (SAPHO without showing its source, until a partnership).

**Closed, in the CHANGELOG:** 1 and 4 (`#FROUND 1`), 11 (consistency at 32
bits), 16, 18 (hardware for code that never runs), 19 (the dividers, v6.0),
20, 22 and 23 (v7.1), 25 (appcomp folded into asmcomp, v7.1), the user names that collide in the `.asm`, 29 (a faster clock, next release).

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

## 2. Round to nearest, full-range `I2F`, saturating `F2I`

**Status:** functionality landed; two verification/refinement leftovers ·
**Area:** `SAPHO/ula.v` · **Evidence:** [§1.2](docs/design/precision-and-width-review.md#12-truncation-instead-of-rounding-todomd-item-2), [§1.4](docs/design/precision-and-width-review.md#14-i2f-uses-only-nbmant-bits-todomd-item-2-and-rounds-nothing)

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

**Status:** landed as `#FROUND 1` (saturate / flush to zero) · **Evidence:** [§1.3](docs/design/precision-and-width-review.md#13-exponent-overflow--underflow-silently-wraps-bug)

Left open: the parameter guard
`initial if (NUBITS != NBMANT+NBEXPO+1 || NBMANT >= 2**(NBEXPO-1)) $error`
in `ula.v` (needs a form that every simulator and synthesizer in the flow
accepts silently).

## 5. One shared, exact constant encoder

**Status:** steps 1-3 of 5 done (2026-09-25), 4-5 left · **Area:** `ASMComp/t2t.c`, `CMMComp/{variaveis,t2t}.c`, `CPPComp/codegen.c`, the three lexers · **Evidence:** [§1.5](docs/design/precision-and-width-review.md#15-constant-encoding-on-the-host-loses-bits-today)

**Where it stands.**
1. Done (`bbb8278`): `Compilers/common/yanc_num.{c,h}`, the exact encoder --
   decimal text with an exponent -> `{s, e, m}` through big integers, to
   nearest, ties to even, flags for overflow / underflow / denormal / flush,
   the word as a bit string. Held to an exact Python model by
   `Scripts/check_yanc_num.py` (3 750 cases, 5 formats), a regress step.
   Limits: `NBMANT <= 63`, `|decimal exponent| <= 1200`.
2. Done (`ed20b8c`): asmcomp's `f2mf` calls it. 45 of 264 test constants
   moved one unit down (the old path rounded twice); the moved outputs were
   each explained (`cmm_tan`, `test63`, `test68`, one DTW value). A
   non-number or an overflowing constant is an error now.
3. Done (`1e8ca03`): the asmcomp and appcomp lexers read an exponent;
   cppcomp prints `%.17g` instead of `%.20f` (`test84`).

**Left, step 4 -- cmmcomp (Luciano's compiler: show the diff first):**
`CMMComp.l:60` `FLNUM` has no exponent (C± cannot write `1e-8`);
`CMMComp/Sources/t2t.c:63-68` splits a complex literal through
`sscanf("%f %f")` + `sprintf("%f")` -- 6 decimals, `(1e-8, 2e-8)` becomes
`(0, 0)`: split the source TEXT instead; `CMMComp/Sources/variaveis.c:164-204`
is a second `f2mf` (diagnostics: precision warnings, the `#FROUND` flush
warning) -- make it call `yanc_num` so both tools agree bit for bit. cmmcomp
builds from the Makefile's `CMMCOMP_C` and from `regress.sh`'s own gcc line:
add `Compilers/common/yanc_num.c` to both, as for asmcomp.

**Left, step 5:** asmcomp refuses `NBMANT >= 2^(NBEXPO-1)` and `NUBITS > 32`
(`f2mf` still returns an `unsigned int`, and `itob` takes an `int`); drop the
host-`float` range checks in the front ends; a fixture that runs the same
constants end to end at 32/23/8, 32/25/6 and 16/10/5 and checks them to the
last mantissa bit (the `.mif` words against `check_yanc_num.py`'s model).

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
evidence is in [§2.2–2.5](docs/design/precision-and-width-review.md#22-asmcomp--the-actual-ceiling)
and [§2.7](docs/design/precision-and-width-review.md#27-alu-efficiency-review-area-and-depth-operator-by-operator).

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

**Status:** parked with item 6(b) · **Area:** `SAPHO/ula.v`, `SAPHO/core.v` · **Evidence:** [§2.1](docs/design/precision-and-width-review.md#21-hdl--parametric-with-a-handful-of-scaling-issues)

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
decision and the `LDI` address combinationally, so the ALU depth
bounds the fetch path too, not only `racc`. If the constraint is ever
relaxed, a global-stall multi-cycle divider (only `DIV`/`MOD`/`F_DIV`) is
feasible with an enable on ~10 registers of `core.v` and no compiler change
(see [§2.6](docs/design/precision-and-width-review.md#26-critical-path-depth-of-the-alu)).

**Done when (on reopening):** the 64/52/11 configuration is synthesised and
its depth/Fmax reported next to 32/23/8 (the 105/52-bit divider array is the
expected limit — its cost is the user's input for choosing `NBMANT` per
project).

## 8. ALU datapath restructuring (depth and area)

**Status:** the six planned steps are done (below); the critical path of
the whole processor was measured under item 29 (closed 2026-10-17: the
dividers, then `F_ADD` -> normaliser into the accumulator and the stack,
which limits now; design note pipeln-and-division.md 4.4) · **Area:** `SAPHO/ula.v`, `SAPHO/core.v` · **Evidence:** [§2.6](docs/design/precision-and-width-review.md#26-critical-path-depth-of-the-alu), [§2.7](docs/design/precision-and-width-review.md#27-alu-efficiency-review-area-and-depth-operator-by-operator)

The ALU is combinational and every instruction takes one cycle, so the ALU
sits on the path that sets the clock. Measured before the steps (Yosys,
`area.sh`, the ALU alone): the float path was 37 LUT4 levels for `F_ADD` at
level 0, 51 at level 2; the whole no-divider ALU 47 / 57 / 56 (levels 0 / 1 /
2); with `F_DIV` 546. The efficiency review found five adders in series where
one is needed, a linear leading-zero chain, three pairs of duplicated
shifters, `F_MLT`/`F_DIV` needlessly crossing the leading-zero count,
comparisons done by subtraction, and dividers with twice the rows they need.
Level 0 stayed bit-identical through all of it (the C± goldens are the proof).

**The ALU alone is a proxy, not the measure.** The clock is set by the worst
register-to-register path of the whole processor: before the ALU the operand
select (data memory, stack top, constant) and the instruction decode, after
it the accumulator write, the bypass and the jump / call logic. A path that is
the worst inside the ALU need not be the worst once those are added, and a
path that never crosses the ALU can be the real one. `area.sh` counts levels
inside `ula.v` only; `fmax.sh` runs Quartus on a processor the regress built,
so its Fmax is the global answer (the MHz in steps 1-2 come from it), but it
prints only the number, not the path.

**Measured 2026-10-02 on `sapho_all`** (`#FROUND 2`, both stacks 8 deep,
Cyclone V 5CSEMA5F31C6, `fmax.sh`, which now prints the worst paths and the
worst one by block): **Fmax 10.66 MHz, 3759 ALMs (12 %)**. Every worst path
starts at the data memory's read port and ends at the **PC**, not at the
accumulator: `ula_out` drives the `JIZ` decision in the same cycle, so the
jump logic and the PC adder sit in series with the whole operation. Cutting
the dividers with `set_false_path` (timing analysis only, no new fit) gives
the next layers:

| worst path | delay | equivalent clock |
|---|---|---|
| through `F_DIV` | 91.8 ns | 10.7 MHz (the real Fmax) |
| without `F_DIV`: integer `DIV` | 87.0 ns | ~11 MHz |
| without any divider: `F_ADD` | 44.4 ns | ~21.5 MHz |

The `F_ADD` path by block: memory read + operand select 4.4 ns (10 %),
denormaliser 11.5 (26 %), adder 3.0 (7 %), normalise + round 19.1 (43 %),
ALU output mux 2.9 (7 %), jump decision + PC adder 4.2 (9 %): about 19 % of
it is outside the ALU. Each divider alone is ~76 ns, twice everything else;
while a program divides, nothing else in the ALU moves the clock.

**Next:** decide from these numbers (the two dividers; the level-2
normaliser; the jump decision in series with the ALU). The targets below
were set on the ALU alone and should be restated against the whole-processor
Fmax. Re-time with `bash Scripts/hw/fmax.sh sapho_all` after any change.

**The steps, in the order they were done** (each measured with `ltp`/`stat` and
the full regress green):
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

Targets set on the ALU alone (not yet re-measured after the steps): full
no-divider ALU ≈ 30 levels at every level, ≈ −15 % LUT4; `F_DIV` ≈ 260
levels, ≈ −50 % LUT4; every golden unchanged.

## 9. Library accuracy keyed on `NBMANT`

**Status:** open · **Area:** `CMMComp/Includes/float_*.asm`, `CMMComp/stdlib.c`, `CPPComp/Includes/cmath` · **Evidence:** [§1.6](docs/design/precision-and-width-review.md#16-library-accuracy-is-pinned-to-23-bits)

The polynomial fits are accurate to ~1e-6 (≈20 bits), π/e constants have
10–12 digits, and cppcomp's `sqrt` runs a fixed 24 Newton iterations; cppcomp
has no `exp/log/sin/cos/pow` at all.

**Done when:** tables/iterations are selected by `nbmant` and accurate to the
last mantissa bit of every 32-bit format (up to 32/25/6), constants carry
enough digits for it, and the transcendental fixtures compare against a double
reference at 32/23/8 and 16/10/5. Wider sets wait for item 6(b).

---

## 10. Architecture hardening (HDL audit)

**Status:** open · **Area:** `SAPHO/*`, tooling · **Evidence:** [`docs/design/hdl-architecture-audit.md`](docs/design/hdl-architecture-audit.md)

The core is sound where it matters (opcode-driven allocation, one instruction
per cycle, bypass by construction, single clock / sync reset — keep all of
it). The audit found the weak points in the surroundings. In order:

1. **ISS + random differential test** — an instruction-set simulator in C
   and a random program generator; compare its trace against Icarus/Verilator
   on thousands of programs. Catches control bugs the goldens cannot; home of
   the ULA unit testbench planned with item 6.
2. **Single-source ISA table** — mnemonic / opcode / ALU op / stack and I/O
   effects in one file; generate `ASMComp.l` rules, `instr_dec.v` compares and
   `ula_op` table, `opcodes.c` names and `docs/isa.md`. The regress check part
   is done (2026-09-25, v5.5): `check_isa.py` holds `ASMComp.l`, the
   `core.v` `OP_*` localparams, every row of `instr_dec.v`'s decode table,
   the `asm_share.c` copy and asmcomp's usage totals to `isa.tsv`. Left: what
   each decode row DOES (its `ula_op` and control lines) is still hand-written
   and checked only by execution; generating it from `isa.tsv` needs the ALU
   operation as a column there.
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

## 12. Run-time exception strobe (pin + error code)

**Status:** open, not now (noted 2026-09-20) · **Area:** `SAPHO/*`, `ASMComp`, both front ends · **Evidence:** this item

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
you use](docs/design/hdl-architecture-audit.md)). Same spirit as `#TOAQUI`/`cheguei`.

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

## 13. Pre-assembly optimizer (whole program), and reusing temporaries

**Status:** open; data-word sharing DONE for C± scalars (2026-09-25) and C++ scalars (2026-09-26), see "What landed" below · **Area:** `Compilers/common/asm_share.c` (sharing), a whole-program pass for the rest · **Evidence:** the measurements below

A separate executable that reads a `.asm` -- from `cmmcomp` or from `cppcomp`,
they meet there -- and removes whatever does not change what the program does.
The peephole part of this belongs in the front ends and is being done there
(a front end knows what the assembly has already forgotten). What genuinely
needs a separate tool is the part that has to see the WHOLE program at once.

**Reusing the memory of temporaries is the prize.** Every local of every
function gets its own word today and never gives it back, so a program pays
for locals that are never alive at the same time. Bounded by the call graph
(two functions' locals can share a word only if neither can be running while
the other is), measured on the current tests:

| program | words of locals | call-graph peak | saving |
|---|---|---|---|
| `test46` | 202 | 100 | 102 |
| `test44` | 186 | 153 | 33 |
| `test70` | 48 | 21 | 27 |
| `test33` | 17 | 9 | 8 |
| `cmm_comp_sqrt`, `proc_fft`, `proc_rls` | 12, 13, 5 | same | 0 |

That table only counts sharing ACROSS functions. Liveness inside a function
is where C± gains: on the C± tests it saves 302 data words over 39 programs
(`sapho_all` 79 -> 58, `cmm_catan` 62 -> 35, `cmm_comp_func` 44 -> 21).

**What landed (2026-09-25).** `asm_share` runs inside cmmcomp on the finished
`.asm` (after the macros are appended) and writes `#SHARE <name> <home>`;
appcomp and asmcomp give `<name>` the address of `<home>`. No instruction is
renamed, added or moved, so the sidecars stay valid, and the waveform is
transparent: asmcomp follows a shared variable by the SETs that write it
(`mem_wr && pc_sim_val == SET+1`), not by its address. Checked by comparing
the user-variable traces of the same `.asm` with and without `#SHARE`
(identical, change by change, Icarus and Verilator). What is left:
- **cppcomp**: DONE 2026-09-26. cppcomp calls `asm_share` on the `.asm` it
  writes: 84 C++ tests, 76 of them smaller, data memory 49375 -> 47665 words
  (-1710; `test50` -241, `test48` -222, `test46` -170, `test44` 291 -> 143).
  A scalar reached through a pointer is safe because cppcomp takes every
  address with `LEA` (pinned); objects, arrays, `__cstk` and `__heap` are
  `#array` (pinned); indirect calls go through `CAL _dispatch`, so the flow
  graph is complete. The same `.asm` with and without `#SHARE` (test29, 39,
  44, 66, 70, Icarus) gives the same output and the same trace for every
  variable except the pointers, which move by a constant (the arrays they
  point into sit lower).
- **Local arrays**: CLOSED 2026-09-26, measured and not worth it. Over the 84
  C++ tests the automatic local arrays and structs are 3114 words; sharing
  them between functions never active together (call graph, greedy) leaves
  3066, 48 saved (0.1 % of the data memory; `test50` -33, `test48` -13). The
  big ones sit in `main`, which is always active (`test46` alone has 2448).
  None of the C± tests has a local array outside `main`. It would also have
  cost a language change (a local array without an initializer starts at 0
  and keeps its contents between calls today; 32 of the 48 words are such)
  and a waveform one (a shared array would show the other's writes).
  Where the C++ data memory is instead: the `__heap` arena, a fixed
  `CFG_HEAPSZ` = 2048 words in every program that allocates -- 18 of the 84
  tests, 36864 words, 77 % of their data memory. Sizing it (a pragma, or from
  the program's measured use) is the saving worth having; not started.
- **An interrupt landing on the cycle of a write** forces the PC to `#ITRAD`,
  and the mirror of a shared variable could miss that one update (the
  program itself is unaffected). Not checked.
- The group keeps the name read before any write, else the first seen; the
  choice no longer shows in the waveform.

**What makes it analysable at all:** the `.asm` is symbolic. Every variable is
a NAME, not an address; `appcomp` assigns addresses later. So merging two
names into one is all the tool has to do, and the memory saving follows.

**What it has to respect:**
- **Indirect addressing.** `LEA`/`LDI`/`STI` make the touched cell
  unknowable, so any name whose address is taken must be pinned. Measured:
  109-144 indirect operations and 4-19 pinned names in the C++ tests, so the
  analysable subset (a scalar whose address is never taken) is most of them.
- **Verification.** A wrong live range corrupts a program silently, and the
  golden tests may not exercise the path. The instruction-set simulator of
  item 10.1 plus a random program generator is the harness this needs; build
  it first or alongside.
- **The instruction table.** `Compilers/common/isa.tsv` already says, per
  mnemonic, what it does to the accumulator, the stack, the word its operand
  names, the control flow and the ports. That is what the analysis reads.
- **The line-mapping sidecars** (`pc_*_mem.txt`, `prog_log.txt`,
  `src_lines.txt`): one entry per instruction. Deleting or merging an
  instruction invalidates them unless the tool rewrites them too.

**Small leftovers found on the way (2026-09-24, cmmcomp goldens):** a
`JMP`/`JIZ` that lands on a `JMP` (13 cases; cycles only), the same index
computed twice in `s[e] = s[e] + ...` (6 cases), `CAL f; RET` -> `JMP f` (2,
procBlind). They fit here or as front-end peepholes;
`Scripts/perf/asm_waste.py Compilers/CMMComp/Tests` is the scanner that found them.

**Done when:** the tool reads a `.asm` and writes a smaller one that
assembles and simulates identically on every fixture, under both simulators;
temporaries share memory by liveness (done for C± scalars; cppcomp next); and
the instruction count and data-word count are reported per program so the
saving is visible.

## 14. Inline small leaf accessors (`cppcomp`): what is left

**Status:** both landed -- the core in `fdcc070`, the two remaining cases on
2026-09-24; the profile is done, what is left is below · **Area:** `Compilers/CPPComp/Sources/codegen.c`

`fdcc070` expands a tiny accessor at the call site, with copy propagation:
1.32x on a loop of element accesses. Two cases still called; both are now
fixed:

- **`operator[]` of a class template.** The instantiated methods are clones
  kept in `g_inst[]`, not in `cg_unit->funcs`, so `func_by_label()` never
  found them. Fix: search `g_inst` too (its declaration moves up next to
  `cg_unit`, and the three inliner helpers get forward declarations).
- **The write path, `a[i] = x`.** `gen_addr`'s own overloaded-subscript site
  now expands too: a reference-returning accessor yields the element's
  address, which is exactly the lvalue's.

Regress with it: the C++ phase 76/76; three C± failures that passed when
re-run (item 15 -- cmmcomp is untouched by this change).

**The measurement that matters:**

| program | before | after |
|---|---|---|
| template benchmark, 3232 accesses | 78 991 cycles | 59 791 |
| `test46` (blind deconvolution), real | 76 360 cycles | **76 261 (-0.13 %)** |

`test46` loses all five `operator[]` calls for +2 instructions, but gains
almost nothing: it spends its time in float arithmetic, the linear solve and
the convolution, and its five accesses sit in loops of fifteen. The change is
correct and nearly free, and it removes a real inconsistency (identical code
expanded for a plain class and not for a template, which is the form real C++
uses) -- but as an optimisation it does not pay on the one real program
measured. Committed for the consistency, with the 0.13 % stated.

**The lesson for every optimisation after this one:** a benchmark written to
isolate a cost overstates the gain. Measure on a real program, in cycles.
The cycle-counting tools are in [`Scripts/perf/`](Scripts/perf/README.md)
(`cycles.sh`, the per-address profile, `prof_report.py`, and the test46
recipe and history).

**The profile of `test46` (2026-09-24).** A Verilator harness counts the
cycles the fetch address (`pc_sim_val`, exposed with `+define+YANC_TRACE`)
spends on each program word; a script maps the words to functions and source
lines through the `.asm`, `pc_*_mem.txt` and `src_lines.txt`. Both are in the
cycle bench (`build_prof.sh`, `prof_report.py`). Of 76 311 cycles:

| function | before | after the zero-fill fix |
|---|---|---|
| `main`, all of it `float scratch[2400] = {0.0f}` | 38.2 % | 26.6 % |
| `solve_spd` (Cholesky) | 28.0 % | 33.2 % |
| `inverse_tikhonov_calibrated` | 15.2 % | 18.0 % |
| `sqrt` (the user's 24-step Newton loop) | 9.0 % | 10.7 % |
| `convolve_full` | 7.9 % | 9.4 % |

The zero-fill loop was 12 instructions a word and is now 7: 76 261 -> 64 184
cycles, 15.8 %, in one small change -- against 0.13 % for the whole inliner.

**Done 2026-09-24:** the Cholesky inner loop recomputed `i*n` and `j*n` for
every `k`. A `for` now hoists a loop-invariant address `p + i*n` into a
temporary (`lih_*` in codegen.c), and a one-word element indexed by a plain
variable is `base; ADD idx`: 64 184 -> 60 890 cycles (-5.1 %), the inner
loop 7 280 -> 4 550.

**Also done 2026-09-24/25** (each in the CHANGELOG): the `for` tests at the
bottom (cppcomp always; cmmcomp only when a literal start already satisfies a
literal bound, so C± code never grows -- Luciano's choice); an int literal
takes the memory form; the zero-fill stores four words a turn; the hoist sees
by-value calls and lone variables; integer constant folding. **test46:
76 261 -> 49 684 cycles (-35 %), 1 754 -> 1 628 instructions.**

**Where test46 spends its 49 734 cycles now** (`Scripts/perf/cycles.sh ...
prof`): `solve_spd` 35 %, `main` 25 % (the `scratch[2400]` zero-fill, 5
instructions a word), `inverse_tikhonov_calibrated` 18 %, `sqrt` 11 % (the
user's 24-step Newton loop: nothing for the compiler), `convolve_full` 9 %.

**Next candidates, smallest first:**
- DONE 2026-09-26: a `<fn>_this` word nothing reads loses every SET (peephole
  pass 0b). It covered the out-of-line methods too (`POP; SET <fn>_this;
  LDI k; RET`), not only the expanded accessors: 384 instructions fewer over
  the C++ tests (26 173 -> 25 789, 37 programs), 12 data words; test46 1 613 ->
  1 581 instructions but 49 678 -> 49 661 cycles, its accessors are not in
  the hot loops;
- ~~the backward solve `a[k*n + i]`: step a pointer by `n` instead of `MLT`~~
  -- measured 2026-09-26, not worth it. The inner loop is 18 instructions a
  turn; a pointer stepped by `n` costs the same five as `P_LOD k; MLT n;
  S_ADD` plus the load (the ALU is combinational, `MLT` is one cycle like
  `ADD`). One instruction a turn comes only by also replacing `b[k]` with a
  pointer and eliminating `k` (the end test on the pointer): 18 -> 17, 105
  turns at n = 15, ~105 of test46's 49 711 cycles (0.2 %), for an induction
  variable elimination in cppcomp;
- the zero-fill at 8 words a turn: ~4.5 instructions a word, for more code;
- the Cholesky inner loop is 16 instructions a turn and near the floor of
  this ISA: `k < j` with a variable bound needs `GRE j; LIN; JIZ` (only JIZ).
Declined, do not propose again: SHL for a power-of-two 2D row size
(Luciano: the multiplier is in most programs anyway).

## 15. The regress is not trustworthy on this machine

**Status:** mitigated 2026-09-25, cause not fixed · **Area:** `Scripts/regress.sh`, the machine · **Evidence:** below

Seven full runs in one session; only ONE came back clean. Each other run
failed on a DIFFERENT set of 3-8 heavy float tests, with `vvp exited non-zero`
or empty/truncated output. That matters beyond the annoyance: a run no longer
distinguishes a real regression from noise, and the whole-program optimiser of
item 13 is exactly the kind of change that can miscompile silently.

What is known, measured:
- the same `.vvp`, re-run in the same directory, alternates exit 0 / exit 1;
- nothing on stderr and no Icarus error, so the process is dying, not failing;
- a failed run leaves the VCD truncated (`cmm_tan`: 4.47 MB of the 5.36 MB a
  good run writes), i.e. it dies mid-simulation;
- it tracks machine memory: the one clean run was with the machine free (19
  min for 134 tests); the bad ones had 1-2 GB free of 15.5 GB and a single
  simulation took 70 s, about 8x slower.

**NOT the waveform writer, measured:** the same testbench with all 16
`$dumpvars` commented out runs in the same time (69.9/69.0 s against
72.0/68.2 s). Gating the dump behind `+WAVE` would save 215 MB of disk per run
and nothing else. The truncated VCD is a symptom of dying mid-run, not the
cause; do not repeat that inference.

**Related, and free:** the working tree carries 288 MB that is not versioned
and regenerates: `.smoke/` (284 MB, of which 215 MB is 118 VCD files nobody
reads), `Teste/`, and `bin/`. `bin/` is worse than bloat: it goes stale (an
`appcomp` from June against sources from September) and then silently
mis-assembles, which has already cost one wrong investigation.

**Latest (2026-09-24), and what the machine looked like:** another run, three
C± failures (`cmm_fround1`, `cmm_fround2`, `cmm_hyper`), all passing when
re-run. At that moment the whole machine used about 3 % of its 22 cores, but
had **1.0 GB free of 15.5 GB**: the slowness is paging, not CPU. What held the
memory: VS Code 4.6 GB (41 processes, five windows), Claude Code 2.6 GB (20
processes: seven sessions in VS Code plus the desktop app), `node` 2.2 GB,
Chrome 1.2 GB. Some of it was idle for hours -- the desktop app (619 MB) and
two old sessions, found by the last-write time of each session's transcript
under `~/.claude/projects/<folder>/<session-id>.jsonl`. That is the quickest
way to see which sessions are only holding memory.

**Mitigated (2026-09-25):** `regress.sh` now re-runs a simulation that DIED
-- vvp exiting non-zero, or in the C++ phase an empty or truncated output (a
strict prefix of the golden) -- up to twice, and lists every retried test in
the summary (`retried (simulation died, TODO.md item 15): ...`). A wrong but
complete output still fails at once. Tested with a fake vvp: a test that dies
once passes and is listed, one that always dies fails. Two deaths in a row
were seen once, hence two retries. The board is repeatable again; the cause
(memory pressure) is not fixed, so a long `retried` list still means: free
memory before trusting timings.

**Meanwhile:** re-run a failing heavy test on its own; if it passes, it is
this. Free memory on the machine before trusting a board.

**Done when:** a full run is repeatable, or the cause is found and named.

## 17. Filling and copying arrays faster (hardware options)

**Status:** open, design discussion (2026-09-28); nothing measured, every
number below is deduced from the `.asm` · **Area:** `SAPHO/core.v`
(`mem_ctrl`, `rel_addr`), `SAPHO/processor.v` (`mem_data`), `instr_dec.v`,
the ISA, cppcomp · **Evidence:** test46's profile (item 14)

**Why.** `float scratch[2400] = {0.0f}` is 24 % of test46's cycles (11 991 of
49 711). The zero-fill loop is 20 instructions for 4 words, 5 a word, because
`STI` writes `mem[base + stack top] <= acc` and POPS: every word reloads the
index, adds its offset, pushes it and loads the zero again. A copy
`b[i] = a[i]` is `LOD i; PSH; LDI a; STI b`, 4 a word.

**The memory's limit.** `mem_data` is a simple dual-port RAM: one registered
read port and one write port, separate addresses. One read and one write a
cycle, so a copy can never beat one word a cycle.

**Options, smallest hardware first** (instructions a word, fill / copy):

| option | hardware | fill | copy |
|---|---|---|---|
| today | -- | 5 | 4 |
| cppcomp: 8 words a turn | none | ~4.5 | 4 |
| assembler: `array+k` operands (LDI/STI take a name OR a number today) | none | ~3.75 | 4 |
| new store, address from the acc, data from the stack, NO pop | write-address mux (stack/acc) + write-data mux (acc/stack), generated only if used | ~1.6 | none (the stack grows) |
| the same with POP (a second decode row, same datapath) | as above | ~3 | 3 |
| index register `X` with post-increment (`SETX`, `LDX a`: acc <= mem[a+X], `STX b`: mem[b+X] <= acc, X++) | an MDATAW register, an incrementer, a third index source in both address adders | 1 (`LOD 0` once, then `STX`) | 2 |
| pipelined copy instruction (read now, write the read word next cycle) | a data pipeline register, the destination base from the stack or a second register, overlap and flush care | -- | 1 (the RAM's limit) |

**Rejected: redefining `STI` as address-from-acc, data-from-stack.** Plain
stores tie, but a read-modify-write at a COMPUTED address needs the address
in a temporary and reloaded: +1 instruction and +1 word each time, in loops.
test46's `out[i + j] += ai * b[j]` (convolution, 5.4 % of its cycles) is
exactly that. Keep `STI`; any new store is a separate opcode.

**Two full memory ports (true dual-port).** Do not speed a copy (a word is
still one read + one write, two accesses a cycle either way). They give two
writes a cycle (fill at 2 words a cycle) or two READS a cycle, the DSP dual
data bus: `acc += mem[a + X] * mem[b + Y]` for FIR / convolution / dot
product, which needs two index registers and a two-address instruction (the
operand field holds one base). Cost on Cyclone V: an M10K is 256x40 in simple
dual-port but 512x20 in true dual-port, so 32-bit memories up to 256 words
take 2 blocks instead of 1 (512 words: 2 either way). A read on the second
port cannot share a cycle with a write, and the mixed-port read-during-write
behaviour has to be defined.

**Compiler side.** Loops cppcomp writes itself (the `{0}` fill,
`copy_to_block`) can use any of these directly. User loops need the
compiler to see that `i` walks the array and replace it with `X` (the
induction-variable elimination item 14 found expensive).

**Recommendation (not decided, 2026-09-28):** if the goal is only fill and copy, the
store options; if SAPHO is to be stronger at DSP, the index register `X`
first (it covers fill, copy and sequential loops, and later a modulo mode
for circular FIR buffers), then `Y` with two full ports if the MAC loops of
real group programs are the bottleneck. Each step measured first: ALUTs and
Fmax in Quartus (a program that uses it and one that must pay nothing), and
test46's cycles.

## 21. Documentation that cannot fall behind a release

**Status:** Chrysthofer answered on 2026-10-13 with a proposal, "Diretrizes
e arquitetura dos projetos do nipscernlab" (PDF, 28 pages, attached to the
email thread); its phase 0 is Luciano and him agreeing on it. Nothing is
decided yet (opened 2026-10-09) · **Area:** this repo,
nipscernlab/docs_aurora, nipscernlab/.github

The published manual (nipscernlab/docs_aurora, nipscern.com/library/sapho)
stopped at AURORA 6.11.0.5 (2026-08-31, about yanc v5.3: it lists LDA/STA and
old directive defaults), and Aurora Intelligence quotes it. Luciano wants hard
guidelines for every public nipscernlab repo (each repo an independent
project, the guidelines global, in nipscernlab/.github): no release without
green CI, rigorous per-repo tests and coherent documentation; English the
source language, Portuguese generated and reviewed by a person. For yanc: the
yanc chapters of the manual living in this repo and published on the site,
the reference pages generated from `isa.tsv` and the other tables, a release
routine that refuses to tag without them. Every feature since v6.0 has its
docs/ note ending in "For the documentation (item 21)": `command-line.md`
(the v7.1 command line and output folder), `toma-and-cade-links.md` 11
(with three points kept from `toma-and-cade.md` 8.7), `myfifo.md`,
`asm-names.md`.

What the proposal asks of yanc (to weigh, not decided): the language file
(today AURORA's hand-generated `sapho_rules.json`) generated by yanc's CI
at each release and attached to it, the release failing without it (its
phase 4); the manual in English in docs_aurora, built and published by CI,
the SAPHO release locked while the manual is behind; and for the repo
itself `AGENTS.md`, `ROADMAP.md` (from issues and milestones instead of
TODO.md), ADRs, a ruleset on main without exceptions, lint, secret scanning,
a third-party list and `CITATION.cff`.

**Done when** the guidelines are agreed (the email "Diretrizes para os
repositórios do nipscernlab", 2026-10-09, is the basis) and yanc follows them.

## 24. Processors with different float formats on one link

**Status:** parked (Luciano, 2026-10-11: the check belongs to AURORA, which
sees every program of a project; a conversion maybe in the future) ·
**Evidence:** docs/design/toma-and-cade.md section 3,
docs/design/toma-and-cade-links.md section 9

`toma` sends a float's word unconverted and `fcade` reads it as a float:
exact when both processors have the same `#NBMANT`/`#NBEXPO`, garbled with
no warning when they differ (each program is compiled alone); ints with
different `#NUBITS` only get the tools' port-width warning.

## 28. Burst: a whole array through a port, one word per clock

**Status:** design written (Luciano's block, 2026-10-14), nothing
implemented · **Area:** `SAPHO/core.v`, `instr_dec.v`, `isa.tsv`, asmcomp,
then cmmcomp and cppcomp · **Evidence:** docs/design/burst.md

Sending an array to a FIFO costs 8 instructions a word in a loop, or 2 to 3
a word unrolled (code growing with the array). A `burst` block (size
register, pointer, an `out` that waits in place until the end) sends one
word a clock with fixed code, and stops while the FIFO is full through a new
`out_ok[p]` pin; the same block fills an array from an input FIFO. Not to
be implemented yet (Luciano, 2026-10-14): the idea is recorded. Open: the
accumulator after a burst, the one-word margin against a full FIFO (and an
empty one on input), the element type, the compiler passes that drop and
share arrays, names and opcodes (they fit at the end of the table), the
language forms (note sections 5 and 5a). Shares its pointer with item 17's
index register: design the two together.

## 30. SAPHO synthesized without showing its source (if a partnership comes)

**Status:** parked (Luciano, 2026-10-14): to look at only if UFJF partners
with a company that would invest in SAPHO · **Evidence:**
docs/design/ip-protection.md

Shipping `SAPHO/` so Quartus or Vivado synthesize it without anyone reading
it. IEEE 1735 keeps the per-program parameters working, but Quartus
decrypts it only in Pro (the lab's Cyclone V is Lite/Standard; Intel's
Ampcrypt through sales instead), and Vivado needs the AMD key and an
encryption license; a netlist service is the vendor-free option. SAPHO up
to v7.1 is public under MIT and stays so. If it comes: ask Intel and AMD,
test with a small module then `sapho_all`, decide the licence of future
versions with UFJF's technology transfer office first.

## 31. The Dirac output of an array: a wasted multiply, a missing form

**Status:** open, found 2026-10-14 while measuring item 28, not fixed ·
**Area:** cmmcomp (Luciano's code: the diff goes to him first) ·
**Evidence:** docs/design/burst.md section 1

`out(0, 1|a⟩);` writes each element as `LOD_V a k; MLT 1; OUT 0`: the
`MLT 1` costs a cycle a word and synthesizes the integer multiplier for a
multiplication by one (pay only for what you use). And `out(0, |a⟩);`,
without the scalar, is a syntax error. Fixed, the form is two instructions a
word (`LOD_V`, `OUT`), the least today's ISA allows, independent of the
burst. A fixture with the scalar 1 and with none, the golden checking that
no `MLT` and no multiplier remain.

## 32. Divider stage count computed from the widths

**Status:** recorded 2026-10-16, for later (Luciano: 5/4 stay fixed for
now, item 29, closed) · **Area:** `SAPHO/ula.v` (`div_array`), `asm_divseq`,
asmcomp, `hdl.c` · **Evidence:** docs/design/pipeln-and-division.md
section 4

The cut positions already follow the widths (`first_row()` spreads PRE + R
rows + POST evenly over `STG` cycles); the number of cuts does not. The
other direction: fix how much logic fits in a cycle, a budget B in rows,
and take `STG = ceil((PRE + R + POST) / B)`, R = `NUBITS` for `DIV`/`MOD`
and `NBMANT` + 0/1/3 for `F_DIV` at `#FROUND` 0/1/2, PRE/POST the ones in
the code (integer 1.5/2.8, float 0.8/3.6 rows). The measured 32-bit
minima (5 and 4) hold for B between 7.6 and 9.07 rows; with B = 8.5:
integer 32/24/16 bits -> 5/4/3, float `NBMANT` 23 at `#FROUND` 2 or 0 ->
4, `NBMANT` 15 at `#FROUND` 2 -> 3. The gain is in narrower words only (a
16-bit processor saves two NOPs and two cycles a division, and two cuts).
Compute it once, in C, and pass `STG` to the generated Verilog like the
opcode parameters, so the HDL and `asm_divseq` cannot disagree (`hdl.c`
not checked yet). Weak point, deduced and not measured: the rest of the
processor also shortens at narrower widths, so one B in rows may over- or
under-cut; a fit at 16 bits would settle it. Done: the formula in the
compilers, `STG` a parameter of both dividers, regress, one narrow fit.

Seen 2026-10-17 on the ZYBO (Vivado, one run, after item 29): the worst
path is the first part of `F_DIV` (data memory -> array up to its second
cut, 26.5 ns; 37.26 MHz), not the float add as on the DE10-Nano. A fifth
`F_DIV` stage, tried on a copy: 37.79 MHz (noise), the next path (the
result written back to memory) only 0.4 ns behind; Luciano kept 4. The
budget per cycle differs per FPGA family, so a computed `STG` would need a
per-target B, or the larger of the two counts.

## 33. A float as a condition: neither compiler gives C's answer

**Status:** STUDY first (Luciano, 2026-10-17): compare the approaches, pick
one, only then implement · **Area:** cmmcomp (`ast.c`
`emit_cond_int_load`, `oper.c` `oper_lin`/`oper_lanor`; Luciano's code,
diff to him first), cppcomp (`codegen.c` `gen_bool`, the float -> bool
cast) · **Evidence:** simulated 2026-10-17 (.smoke/s8/fcond, .smoke/s8/cpp)

In C a float condition is true when the value is not zero. Measured on
the simulated processor, `#FROUND 2` (and `0.6` also at `#FROUND 0`):

| condition | C | C+- today | C++ today |
|---|---|---|---|
| `if (0.5)` | 1 | **0** | 1 |
| `if (2.5)` | 1 | 1 | 1 |
| `if (-0.3)` | 1 | **0** | 1 |
| `if (0.0)` | 0 | 0 | **1** |
| `if (a - a)` | 0 | 0 (not run) | **1** |
| `while (a)`, a = 0.5, a -= 0.25 | 2 turns | **0 turns** | (not run) |

C+-: the condition goes through `F2I` (truncation; any float in (-1, 1) is
false), on purpose, with a warning that says "Gonna round it" although it
truncates (0.6 is false at both levels measured). `!`, `&&`, `||` on a
float do the same (`MSG_WARN_LOGIC_FLOAT`). No C+- golden has a float
condition. C++: `gen_bool` tests the float's bits, and SAPHO's zero float
is not all zero bits (its exponent is the minimum, not 0), so every float
condition is true: a correctness bug.

Approaches to compare (none chosen):
1. Test the mantissa: SAPHO keeps the leading one, so a float is zero
   exactly when its mantissa is zero (+0, -0, every `#FROUND`):
   `AND <2^NBMANT - 1>` before the `JIZ`. One fast integer instruction;
   the AND operator is built if the program had none. To check: denormals
   at `#FROUND 0`, values the encoder turns into zero.
2. Compare with zero: `EQU` against the zero constant, then invert (`LIN`)
   or swap the branch targets. Two zero encodings at `#FROUND 0` (the sign
   is kept), so it may need two compares there.
3. A hardware change: the zero flag of `JIZ` computed from the float's
   meaning for float results (an opcode or a flag from the compiler).
   Costs hardware and touches the path just shortened by item 29.
4. Keep C+- as it is (document the truncation, fix the warning's text to
   "truncate") and fix only C++.
5. (Luciano, 2026-10-17) The float zero with every bit at 0, in hardware,
   keeping the explicit leading one: a float is zero exactly when its
   mantissa is zero, so pick the all-zero code among them. Output: the
   normaliser emits all zeros for a zero (and the sign at 0, also at
   `#FROUND 0`); `F_NEG`/`F_ABS` keep a zero a zero. Input: whoever reads the
   exponent treats mantissa 0 as the minimum exponent (the add's alignment,
   `F_LES`/`F_GRE`, `F_SCL`/`XPO`); `F_MLT`, `F_DIV`, conversions already
   work. Then `JIZ` (the OR of all bits) is right for floats with no extra
   instruction, old data with the old zero code still reads as zero, and
   0 and 0.0 share a code. The input fix sits on the add, today's worst
   path: needs a fit. The encoder emits 0.0 as all zeros.

Test sequence (agreed 2026-10-17): 1) the reference test against host gcc
(below), run on today's compilers; 2) a prototype of each approach outside
main (1, 2, 4 in the compilers; 5 in a copy of SAPHO/ plus the compilers,
with `tb_alu` zero cases, the C+- programs and a fit), each run through the
same test and costed (words and cycles per condition; ALMs and Fmax for
5); 3 is left last, 5 covers its goal more simply; 3) a comparison table
for Luciano to choose; 4) only the chosen one on main, the test kept as a
regress fixture.

Step 1 DONE 2026-10-17: `Scripts/fcond/` (gen.py writes one C++ file, run
by the host g++ and by cppcomp, and the same cases in C+- at `#FROUND`
0/1/2; run.sh simulates, compare.py lines them up; BIN and HDL point at a
prototype). 16 values x 11 forms. Today: C++ 83 of 176 wrong (every zero
taken as true in `if`, `!`, loops, `&&`, `||`, `?:`; loops then run one
turn too many for any value; `bool b = x` is right, only `gen_bool` is
wrong). C+- 35 of 80 wrong, the same at the three levels: every nonzero
value in (-1, 1) taken as false (0.5, -0.3, +-0.999, 0.001, 1/3, 0.0005)
in `if`, `!`, `while`, `for`, `do`; zeros, -0 and |x| >= 1 right. C+-
refuses a float in `&&`/`||` with an error (those forms are C++ only in
the test). The same at the three formats of the fixtures (32/23/8,
16/10/5, 23/16/6; values below a format's smallest normal left out). The
fix must take the widths from the program (`#NBMANT`/`#NBEXPO`, `#pragma
yanc`), never a constant. cppcomp already tests a float for zero by its
mantissa (`AND 2^NBMANT - 1`) in its float -> bool cast, which is why
`bool b = x` is right; `gen_bool` does not use it.

Step 2 so far (prototypes in worktrees .smoke/proto1, .smoke/proto2, not on
main), the test now 16 values x 18 forms (&&/|| with a computed operand
on either side, both computed, with an int): approach 1 (AND the
mantissa) gives 0 differences from g++ in C++ (288 cases) and C+- (256,
also with && and || on floats, which Luciano asked to allow: the error
now only for a comp; the stack-on-both case goes through `__aux_var`), at
the three formats and levels; +1 word and cycle per float condition (+2
for C++ `!x`). Approach 2 (`EQU 0.0; LIN`): C++ right, C+- wrong for -0 at
`#FROUND 0` (5 cases a format), +2 words. Approach 4 = approach 1's C++
half with C+- as it is. Today's compilers on the extended test: C+- does
not build (&& on floats), C++ 118 of 288 wrong. No C++ regress test used
another float format before this one. Next: approach 5 (hardware), then
the comparison table.

How to choose: a differential test against a C compiler first, the same
source on host gcc (IEEE) and on the simulated SAPHO, many values (0, -0,
values in (-1, 1), integers, results of `a - a`, `a * 0`, divisions) and
every form (`if`, `while`, `for`, `do`, `!`, `&&`, `||`, `?:` and float ->
bool in C++), comparing the printed 0/1. It shows today's failures and
proves the chosen fix. Leave out values below SAPHO's smallest normal
(they become zero at `#FROUND 1/2`, a format difference, not an error).
Then compare the approaches on words per condition, cycles and hardware.
Done: the approach chosen by Luciano, both compilers equal to gcc on the
test, the test kept as a fixture, the manual (C+-, C++, messages) and
AURORA's text updated.

## 27. C++ gaps that real embedded code hits

**Status:** open, noted 2026-10-13 while writing docs/language-cpp.md (each
checked with a program) · **Area:** `Compilers/CPPComp/`

Accepted by C++ compilers, a syntax error in cppcomp: an `enum` value that
is not a plain literal (`= -1`, `= A + 1`) and an underlying type
(`enum class E : int`); operator overloading as a free function, and of
`%`, the shifts, the bitwise operators, `()`; multiple inheritance;
anonymous namespaces; variadic functions; lambdas inside functions or with
captures; template specialization and default template arguments. Also: no
`fin`/`fout`, and `out()` of a float writes its raw bits (C± converts);
`#pragma yanc nugain` is accepted and changes nothing. Each is a candidate
when a real program needs it (the missing-feature protocol: ask first).

## 26. Arrays in the waveform without one signal per element

**Status:** open, noted 2026-10-12 (Luciano) · **Area:** cmmcomp
`--wave-arrays`, asmcomp `hdl.c` (the `$dumpvars` of `sim_name_arr`),
`gen_gtkw`

**Why.** With `--wave-arrays` the testbench dumps every element of every
array as its own variable, so the `.vcd` and the GTKWave signal list grow
with the array sizes (a 2400-word buffer is 2400 signals); without it the
arrays do not show at all. The option exists only to keep that cost away.

**Done when** an array can be watched without a variable per position: for
example one signal for the last write (address and value), or a dump of
the array memory read back by the viewer, chosen after measuring the
`.vcd` size and the simulation time on a test with large arrays (test46).

## Workarounds at `#FROUND 0` (worth a line in the README)

- Keep long-running accumulators (phase, time base, counters) in **integer /
  fixed point**: `ADD` is exact, and wrap-around modulo 2^NUBITS gives a free
  "mod 2*pi" for binary angles.
- Accumulate **deviations** from a nominal value instead of absolute values.
- Raise `#NBMANT` for the processor that accumulates (up to 23 — item 5).
- A compensated (Kahan) sum does **not** help at level 0: it relies on
  `t - sum` being exact, which loses a bit there. It works from level 1 on.
