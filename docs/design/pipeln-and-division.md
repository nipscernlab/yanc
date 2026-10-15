# The slow dividers: what was tried for a faster clock, and what stayed

Record of TODO item 19 (2026-10-03 to 2026-10-05). The goal was a higher
clock for programs that use the slow ALU operators. A paused pipeline was
designed, built for the three dividers and measured on two FPGA families;
Luciano then dropped it (2026-10-05) and `SAPHO/` went back to the processor
before it (commit 954c853); **nothing of section 2 is in `SAPHO/` today.** A
co-processor for the dividers alone (section 3) was discussed next and
dropped too, before any code. **Section 4 is what stayed** (2026-10-06): two
register cuts inside each divider and a fixed three-word sequence written by
the compilers, with no pause and no new circuit outside the dividers; every
other operation is still combinational, one cycle. Released in v6.0
(2026-10-07): `sapho_all` went from 9.43 to 21.9 MHz on the DE10-Nano and
from 8.80 to 23.98 MHz on the ZYBO (section 4, **Measured**). Sections 1-3 keep the
measurements and the options as they were weighed, in case the question
comes back.

The code that was built stays in the history of main: step 1 is commit
e2ca31b, step 2 is commit 3fe1967 (`git show 3fe1967`), and the commit that
added this file returns `SAPHO/` to 954c853.

## 1. Why: the factor table

The processor's clock is set by the slowest operation the program uses. Each
`sapho_all` variant keeps the light operators plus one heavy group; the factor
is its worst register-to-register period over the base variant's, routing
included, measured with `Scripts/hw/fmax.sh` (Quartus) and
`Scripts/hw/vfmax.sh` (Vivado), memories in block RAM on both.

| variant | Cyclone V 5CSEMA5F31C6 | Zynq-7010 xc7z010clg400-1 |
|---|---|---|
| base | 53.8 MHz | 45.4 MHz |
| `F_ADD` | 33.8 | 32.4 |
| `DIV` | 12.0 | 9.5 |
| `MOD` | 11.9 | 9.3 |
| `F_DIV` | 10.6 | 9.7 |
| full `sapho_all` | 10.7 | 9.0 |

Factors, the larger of the two families: `SGN` 1.02, `F2I` 1.03, shifts 1.03,
int compares 1.04, `NRM` 1.05, `MLT` 1.09, float compares 1.09,
`F_ROT`/`F_SCL`/`XPO` 1.11, `I2F` 1.17, `F_MLT` 1.26, `F_ADD` 1.59, `DIV`
4.78, `MOD` 4.86, `F_DIV` 5.07. The two families agree within ~10 % on the
whole processor (5.05 vs 5.06). Yosys alone does not: routing is 70-75 % of a
LUT path's delay on the Zynq and ~40 % of a carry path's
(`Scripts/hw/est/`).

The base path itself, on the Cyclone V (18.3 ns): operand select 1.2, `NEG`
1.9, `I2F` 3.5, float normaliser mux 6.7, ALU output mux 1.0, jump decision +
PC 3.9 ns. The jump decision reads the ALU result in the same cycle
(`if_acc = |ula_out` feeds `JIZ`), so every path ends at the PC.

The dividers are the outliers: ~5x the base period. Everything else is
within 1.6x.

**Before the real fits: a Yosys estimate** (`Scripts/hw/est/`, cell delays
only, memories modelled as block RAM; worst path over the base variant's):

| group | iCE40 | Xilinx 7 | Xilinx 7 + routing 0.3-0.8 ns/net |
|---|---|---|---|
| base | 1.00 (17.3 ns) | 1.00 (9.4 ns) | 1.00 |
| `F_ADD` | 1.13 | 1.21 | 1.17-1.18 |
| `F_MLT` | 1.16 | 1.24 | 1.24 |
| `DIV`/`MOD` | 9.0 | 6.1 | 4.5-5.1 |
| `F_DIV` | 6.1 | 4.6 | 3.6-4.0 |
| the rest | 0.93-1.07 | 0.93-1.07 | ~1 |

It agrees with the real fits on the dividers and not on `F_ADD` (1.2 against
1.6 on the Cyclone V): routing, which Yosys does not see, weighs more on LUT
paths (the normaliser) than on carry chains. iCE40 (LUT4) is off the curve;
ECP5 and Gowin are out (no carry timing in Yosys 0.56). `report_timing
-through` one operator overstates the light ones: a path "through" `NEG_M`
is really `I2F_M`'s (shared logic).

**The cost in cycles** of a paused pipeline, from the ALU operation each
cycle executes in 151 regress programs (Icarus with a probe on `id_ula_op`,
stopped at `@fim`; `Scripts/hw/opmix/run_mix.sh`, then `pipeln_cost.py` or
`cost.py`):
- with the latencies of the factor table at a 10 % tolerance (1 cycle up to
  1.09, the clock at base / 1.09; 2 cycles for `F_ROT`/`F_SCL`/`XPO`, `I2F`,
  `F_MLT`, `F_ADD`; 5 for the dividers): C++ median +0 % (mean +4 %), C±
  median +2 % (mean +16 %), the real applications (test46/48/50, `proc_fft`)
  +7..11 %, worst +94 % (a float-library fixture). Nearly all of it is
  `F_ADD`/`F_MLT`, not the dividers;
- an earlier estimate, with an 18 ns stage budget from `report_timing
  -through` (`F_ADD`, `F_MLT`, `I2F` at 3 cycles, `MLT` and the compares at
  2): C++ median +7 %, C± median +18 %, the real applications +10..28 %. Net
  at +25 % cycles: ~3.7x faster for a processor with a divider (10.7 -> ~50
  MHz), ~1.9x for one without (~21.5 -> 50 MHz). Superseded by the measured
  table.

## 2. The paused pipeline (`#PIPELN`), built and dropped

**Design** (Luciano's decisions, 2026-10-04): a directive `#PIPELN 0|1|2`
(off, on, automatic). At 1 the core pauses for k-1 cycles while an operation
of latency k runs: a signal `run` holds every state register and masks every
effect (memory and stack writes, jumps, CAL/RET, I/O strobes, the interrupt,
the `#TOAQUI` pin). The slow operators get explicit register stages inside
them (option B), not multicycle constraints (option A, rejected: a fit's
Fmax must stay the global truth, with no constraint file a user can leave
out). Mode 2 would decide in asmcomp from a static estimate and say so:
clock from the factor table (stated as based on current Altera/Xilinx
families), cycles from the slow operations weighted by the loops they sit in
(10^n for n nested loops, a stated heuristic), mode 1 when it wins by more
than ~10 %, and a message telling the user to simulate both. The full design
proposal, with the list of every register held and every effect masked, is
`docs/pipeln-design.md` in commit 954c853 (`git show
954c853:docs/pipeln-design.md`); the list of state registers comes from
`docs/design/precision-and-width-review.md` §2.6.

How the core runs, which is what any pause has to respect: two stages
overlap. In one cycle the core decodes instruction i+1 (its data memory
address, its stack push/pop, its jump decision) while the ALU executes
instruction i, and the ALU result feeds, in that same cycle, the
accumulator, the data memory write of i+1 (`SET`), the data stack push, the
`LDI` address and the `JIZ` decision of i+1, and through it the PC.

**Step 1** (e2ca31b): the pause, and `F_DIV` in 5 stages. The divider rows
split into groups with a register between groups (the partial remainder and
the quotient bits so far, no enable: the operands are held by the pause),
cut points balanced against the logic outside the array (`PRE10`/`POST10`,
tenths of a row). Proof: regress at `PIPELN 0` 177/177; staged == combinational
over 20000 divisions (`tb_fdiv.v`); the `F_DIV` fixtures at `PIPELN 1` give
the regress outputs as a prefix (a fixed clock budget yields fewer lines).

**Step 2** (3fe1967): `DIV`/`MOD` staged too, with one restoring array
module (`div_array`) shared by the three dividers; at `STG 1` `ula_div`/
`ula_mod` kept `/` and `%`. Proof: `tb_alu.sh` staged 2/3/5 == `/`, `%` in
four formats (8 to 64 bits); 13 fixtures with `DIV`/`MOD` and 23 with `F_DIV`
correct at `PIPELN 1`.

**Results** (one heavy group per variant, `PIPELN 1`):

| | Zynq-7010, before -> after | Cyclone V, before -> after |
|---|---|---|
| `F_DIV`, 5 stages | 9.7 -> 46.8 MHz | 10.6 -> 41.6 MHz |
| `DIV`, 6 stages | 9.5 -> 45.7 MHz | 12.0 -> 42.1..43.3 MHz |
| `MOD`, 6 stages | 9.3 -> 47.0 MHz | not run |

Lessons from building it:
- **Register the divisor once.** Every row reads the divisor, so without a
  register each group also carried the memory read and `|in2|`: `DIV` at 5
  stages went 32.1 -> 38.6 MHz on the Zynq when the divisor and the dividend
  bits were registered at the first cut. It also took `F_DIV` from 43.1 to
  46.8 MHz.
- **The cost of a cut is not free.** `pipeln_cost.py` gave k = 5 for
  `DIV`/`MOD` (factor / clock, rounded up); at 5 stages Vivado measured even
  stages of 24-26 ns, each above the base (22 ns): 32 rows of 33 bits at
  ~3.1 ns a row do not fit in 5 cycles once each cut adds its register and
  routing. 6 stages did.
- **The latency was a 32-bit constant.** `LAT_DIV = 6`, `LAT_FDIV = 5` were
  fixed in `core.v`; the rows follow `NUBITS` (int) and `NBMANT`/`FROUND`
  (float), so a 16-bit processor would pause for nothing and a wider one
  would not pause enough. Any divider with a latency has to derive it from
  the widths, with the rows per cycle calibrated at more than one width
  (a narrower row is faster).
- **The pause logic itself is cheap.** The base at `PIPELN 1` (no staged
  operator, the pause logic present): 51.4 / 52.5 MHz against 51.7-54.2 at
  `PIPELN 0`.
- `lat()` compared the operation code with every slow operator, present or
  not; the comparisons stayed in `run`'s logic. Should be gated on the
  operator being instantiated.

**The Cyclone V does not reach the base** with a staged divider in, and it is
not the divider: its own stages are 18.7-20.1 ns (queried from the fit). The
base path (operand -> `I2F` -> normaliser -> ALU mux -> jump -> PC) grows from
18.2-18.7 ns to 22.5-23.1 ns, ~2.8 ns of it routing and ~1.9 ns cells, spread
over the whole path. Measured, to rule causes out:
- fit noise: 3 seeds of each, base 51.7-54.2, `divp1` 42.1-43.3 MHz;
- the pause logic: see above, ~1 MHz;
- the ALU mux depth (it grew from 1-2 to 3 cells): selecting the normaliser
  result last, in a 2:1 of its own, removed a cell and the time came back as
  routing; no gain, reverted;
- physical synthesis: already on in every fit (the `HIGH PERFORMANCE EFFORT`
  mode turns it on, retiming included, 0 ps on the dividers).

What remains is placement: the staged divider (~1200 ALMs on a ~750-ALM
processor) spreads the rest apart. The Zynq does not show it: its base path
is slow already (22 ns) and did not grow. Deduced from the per-block split,
not proven.

**Why it was dropped** (Luciano, 2026-10-05):
- the core got confusing: `run` reaches ~15 places;
- staging the dividers costs few cycles (divisions are rare), but the next
  operators (`F_ADD`, `F_MLT`, k 2) run all the time: their extra cycles are
  most of the +7..11 % the real applications would pay;
- cutting inside the shared normaliser, which is what a globally optimal
  clock would need (it is 37 % of the base path), adds a cycle to every float
  operation: rejected.

## 3. Options for the dividers alone (discussed, dropped)

Dropped by Luciano on 2026-10-05, before any code: none of these is planned.
Kept as they were weighed.

The idea that replaced the pipeline: take the three dividers out of the ALU
into one sequential block, a co-processor with a single iterative core
(quotient, remainder and the float mantissa quotient come out of the same
restoring rows), with no directive: a program that divides gets it. The ALU
stays combinational; it keeps the operation codes 6/7/8 and takes their
results from outside (`div`/`mod` into the output mux, `fdiv` through the
normaliser, so the float rounding stays where it is). The co-processor latches
its operands when the division starts (the accumulator, the memory operand
or the stack top `S_DIV` popped). This relaxes the 2026-09-14 rule "the ALU
is combinational" for the dividers only.

Iterative instead of staged: P rows reused every cycle instead of R rows with
registers between groups. Estimate at 32 bits, not built: P ~5 rows fit in
the base period (~3.1 ns a row), so 5 subtractors instead of 32 (~6x less
logic), `DIV`/`MOD` in ceil(32/5) = 7 cycles, `F_DIV` in 5-6. A smaller block
should also remove the Cyclone V placement penalty above (to be measured).
Latency k = ceil(R / P), R from the widths.

How the program waits for the result, three ways:

**A. Co-processor + a stall in hardware.** The ISA does not change. While
the block works: the PC, the fetched instruction and `id_ula_op` hold (three
clock enables); the opcode entering decode is forced to `NOP` (an AND on
~7 bits), which masks at once every effect the next instruction would have
(memory write, stacks, jumps, I/O); the interrupt and the `#TOAQUI` pin wait
(two gates). The accumulator needs no enable: it loads the ALU output every
cycle, garbage while the block works and the result on the last cycle (its old
value is not needed: the block latched the operands). `popr`/`stkr`,
`req_inr`/`ior` and the data memory read capture what the `NOP` asks during
the stall and the real instruction's on the last cycle. The counter is the
co-processor's own; the core only receives `stall`, from a register. About
10 LUTs in the core, against ~15 masks for `run` in the pipeline. Deduced from
the 954c853 code, not simulated. To measure: the AND on the opcode sits on
the instruction -> jump decision -> next fetch path.
- pro: ISA, compilers and code size unchanged (one word per division);
- con: still a pause in the core (small).

**B. Co-processor + NOPs inserted by asmcomp.** `DIV x` only starts the block
(the accumulator is left as is), asmcomp follows it with k-2 `NOP`s and a new
instruction `GDV` that loads the result into the accumulator (through the
ALU's division inputs). The block holds the result until `GDV`, so the NOP
count is a minimum, not an exact time. An interrupt between `DIV` and `GDV`
is harmless unless its routine divides; one flip-flop "busy" (set at the
start, cleared by `GDV`) holding the interrupt off covers it. asmcomp expands
every division (the hand-written `Includes/*.asm` too); cmmcomp and cppcomp
do not change.
- pro: the core changes least (the start decode, `GDV`, one flip-flop, one
  gate); later, asmcomp could fill the NOPs with independent instructions;
- con: one new opcode and a new meaning for the divisions (`isa.tsv`,
  `check_isa.py`, the docs, what Aurora says about the ISA); ~k words of
  program per division (~7 at 32 bits).

**C. Division in software** (an assembly routine, as `Includes/float_sqrt.asm`
is for `sqrt`). Counted by hand, not run:
- `DIV`/`MOD` bit by bit with integer instructions: ~18-20 cycles a bit
  (shift the remainder in, shift the dividend, add the negated divisor, test
  the sign, update), ~600 cycles unrolled (~600 words) or ~700 in a loop
  (~25 words) at 32 bits, plus ~15 for the signs; `LES` is signed, so
  divisors above 2^30 need a few more. Exact.
- `F_DIV` by Newton with the float multiplier (1/d from a power-of-two
  estimate, x <- x(2 - dx), ~5 iterations for 24 bits): ~30-40 cycles. Not
  exact: off in the last bit, and `#FROUND 2` loses its correct rounding;
  needs `F_MLT` and `F_ADD`. `float_sqrt.asm` does 4 `F_DIV`s.
- pro: no hardware, the core untouched, no Fmax cost;
- con: ~100x the cycles of A/B for `DIV`/`MOD`, the float result changes.

| | `DIV`/`MOD` 32 bits | `F_DIV` | hardware |
|---|---|---|---|
| combinational (today) | 1 cycle, clock ~12 MHz | 1 cycle, ~10 MHz | large arrays in the ALU |
| A or B (co-processor) | ~7 cycles | ~5-6 | small block, near-base clock |
| C (software) | ~600-700 cycles, exact | ~35, not exact | none |

Any of the three changes the cycle count of every program that divides, so
the goldens of the fixtures with a fixed clock budget, the C++ `.clocks`
sidecars and Aurora's cycle counts move once.

## 4. The dividers: two cuts and a three-word sequence (implemented)

Worked out with Luciano on 2026-10-05, after everything above was dropped,
and implemented on 2026-10-06 (`QUO`/`REM`/`F_QUO` in 3637b60, the rest in
the commit after it). The delays below are estimates from the step 1
measurements (Cyclone V: ~2.6 ns before the array, ~3.1 ns a row, ~11.3 ns
after it); the clock actually gained was measured afterwards: see
**Measured** at the end of this section.

**Where to cut.** The `F_DIV` path of `sapho_all` is 91.8 ns (measured; ~94
by the sum above): data memory -> operand select -> 26-row restoring array
(`#FROUND 2`, 32 bits) -> slice -> normaliser -> ALU mux -> jump decision ->
PC. The next path, `F_ADD`, is 44.4 ns. One cut cannot bring both halves
under it (two halves of at most 44.4 ns add up to 88.8 < 94), so **two cuts,
both inside `ula_fdiv`**, three parts of ~31-33 ns:

| part | contents | estimate |
|---|---|---|
| 1 | memory + operand select + rows 0-8 | ~30.5 ns |
| cut 1, before row 9 | | |
| 2 | rows 9-18 | ~31 ns |
| cut 2, before row 19 | | |
| 3 | rows 19-25 + slice + normaliser + ALU mux + jump + PC | ~33 ns |

The slack under 44.4 ns covers each cut's own cost (register and routing,
not negligible: section 2). The cut rows follow the width: rows R = 23 / 24 /
26 at `#FROUND` 0 / 1 / 2 (32 bits), cut s at about
(s x (PRE + R x row + POST) / 3 - PRE) / row, as `first_row()` computed in
3fe1967; the number of cuts stays two.

**What each cut registers.** Without a pause the core does not hold the
operands: in the two cycles after `F_DIV`, `in1` is what the data memory gives
the next instructions and `in2` is the accumulator, already overwritten. So
everything the later parts use travels with the data, a plain pipeline: the
partial remainder (24 bits), the quotient bits found so far (up to 26), the
divisor (23), and the result's sign (1), exponent (10) and divide-by-zero
flag (1), the last three computed once in part 1. ~85 flip-flops a cut.
On an FPGA they take the flip-flops next to the LUTs, mostly idle: in step 1
four cuts in `F_DIV` left the area unchanged (1552 -> 1524 ALMs).

**The sequence.** Every `F_DIV x` becomes

```
F_DIV x     ; cycle t:   part 1 with the right operands
NOP         ; cycle t+1: part 2
F_QUO       ; cycle t+2: part 3; the result reaches the accumulator
```

`F_QUO` is an **assembler alias of `F_DIV`** (same opcode, as `LOD`, `LEA`
and `LOD_V` share opcode 1) with no operand (operand field 0), named for what
it delivers; `QUO` and `REM` are the aliases of `DIV` and `MOD`. It works
because the ALU, executing an `F_DIV` at t+2, shows part 3's output, which is
the first division's result; the division `F_QUO` itself starts (from the
overwritten accumulator and data word 0) is never read. The
accumulator receives garbage at t and t+1, which does no harm: part 1
consumed the operands at t. The instruction after `F_QUO` sees the result
through `ula_out` in the same cycle, as today (`SET`, `JIZ` right after
work). A following division reads the result from the accumulator at t+3.
The stack form cannot be repeated (it would pop again): `SF_DIV; NOP;
F_QUO`, `F_QUO` decoding as the memory form; a program that only had
`SF_DIV` then also gets the `F_DIV` decode row (small; the divider is shared
by both forms).

- circuit outside the dividers: none (no decoder row, no flip-flop, no
  mux); the ALU gets a clock for their cuts;
- who writes the sequence: a pass shared by both compilers, `asm_divseq`
  (`Compilers/common/asm_share.c`), right after `asm_reach`. It has to run
  before appcomp, which counts the words that become every label's address,
  and next to the pc map the compilers write (the waveform's line view
  repeats the division's line). It covers the macros (`Includes/*.asm`,
  `float_sqrt.asm` has 4 `F_DIV`s) and C++ inline assembly, leaves a
  division already followed by its sequence alone, and asmcomp rejects a
  division that is not (a hand-written `.asm` must write it out);
- ISA: unchanged but for the three aliases;
- cost: 3 cycles and 3 program words per `F_DIV`; the cycle counts of the
  fixtures with `F_DIV`, the C++ `.clocks` sidecars and Aurora's numbers move
  once.

Two other ways to deliver the result were weighed and are worse: two
flip-flops remembering "an `F_DIV` two cycles ago" that force the ALU mux to
the divider at t+2 (plain `NOP`s, but new logic in the core), or a new opcode
(a decoder row and an ISA change).

**The interrupt.** Today's interrupt is a level-sensitive restart to
`ITRADD` with no return (`docs/design/hdl-architecture-audit.md` 1.1): an
interrupted division is never resumed, so it does not matter that it is
lost, and with the alias nothing writes the accumulator behind the handler's
back. Nothing to do. Holding the interrupt off for the two cycles would be
wrong today (a short level would be lost: nothing latches it). If the
interrupt becomes a real one (edge latched, PC saved, return), the
sequence must not be split: hold it off from the division to its read then.

**`DIV` and `MOD`: one shared circuit, the same treatment.** Every operation
above the `F_ADD` path (44.4 ns in `sapho_all`) is a divider: with the
dividers taken out of the timing analysis, `F_ADD` is the worst path left,
and in the per-operator table nothing else is above its factor 1.59
(`F_MLT` 1.26 is the next). `F_ADD` itself runs too often to cut. So the
idea covers exactly `F_DIV`, `DIV` and `MOD`.

`DIV` and `MOD` share one restoring array on the operands' magnitudes: the
quotient comes out of the rows, the remainder is what the last row leaves.
Two cuts (32 rows of 33 bits, 87 ns, no normaliser after it: parts of
~29-31 ns): part 1 takes the magnitudes and the first rows, part 3 the last
rows and the sign fix (the quotient negated when the signs differ, the
remainder with the dividend's sign). Each cut carries the partial
remainder, the quotient bits so far, the divisor magnitude and both signs.
Part 3 drives the ALU's existing `div` and `mod` inputs; `INT_MIN / -1`
wraps by itself (as in 3fe1967). Today they are Verilog `/` and `%`, which
cannot be cut: the explicit array is needed.

The **third instruction chooses** quotient or remainder, by its ALU code (6
or 8), so the first one only starts the shared circuit:

```
DIV x ; NOP ; <alias of DIV>   -> quotient
MOD x ; NOP ; <alias of MOD>   -> remainder
```

The quotient and the remainder of the same division cannot both be read
from one sequence (the cut registers load something new every cycle); a
program that needs both runs it twice.

**Aliases: one per operation, three in all** (an alias of `F_DIV`, of `DIV`
and of `MOD`), not one per instruction: the read always uses the memory
form, since repeating a stack form (`SF_DIV`, `S_DIV`, `S_MOD`) would pop
again. `F_DIV` keeps its own array: through the 32-row integer one it would
get longer (23-26 rows of 24 bits today) and need an input mux.

**Left out.** At `#FROUND 0` the `F_ADD` path is shorter and whether
`F_MLT` then passes it was not measured; at narrower widths the dividers
shrink faster than the adder (fewer and shorter rows), expected to stay
above it (deduced). To verify before trusting it: a fit of `sapho_all` with
the cuts (placement moves every number), a cycle-by-cycle simulation of the
sequences, back-to-back divisions, the stack forms, and every `#FROUND`
level against `tb_fdiv.v` (and `tb_alu.v` for `DIV`/`MOD`).

**Measured (2026-10-07, released as v6.0).** One fit of `sapho_all` (the
unified version of 59cc0c2) per board, on the two boards of the lab's
student tests, before (v5.7, every divider one combinational cycle) and
after (a862a9e). Out of context: no pins, no board top.

| board (part, tool) | v5.7 | v6.0 | worst path after |
|---|---|---|---|
| DE10-Nano (`5CSEBA6U23I7`, Quartus 24.1, slow 100C model) | 9.43 MHz | **21.9 MHz** | data memory -> `NEG_M` -> integer array to `cut[1]`: 43.1 ns |
| ZYBO (`xc7z010clg400-1`, Vivado 2025.2) | 8.80 MHz | **23.98 MHz** | data memory -> integer array to `cut[1]`: 41.7 ns (96 `CARRY4`) |

Before, the worst path went through all of `F_DIV` on both boards (87.5 ns
of 103.9 on the Cyclone V). The students' own v5.7 builds, with their pins
and top, reported 9.53 MHz (Quartus) and about 8.22 MHz (Vivado), within 1
% and 7 % of these. After, the limit is still a divider, but only its first
part, and only the **integer** one: part 1 of `DIV`/`MOD` (memory, the
operands' magnitudes, the first rows) came out at ~41-43 ns, above the
~29-31 ns estimated, so its first cut sits too late. The ten worst paths
reported all end at that cut, so how close `F_DIV`'s parts or `F_ADD` come
was not seen. Not done: other seeds (one fit each, a few per cent of
noise), moving the integer cut earlier, and finding what limits once part 1
is shorter (`F_ADD`, 44.4 ns on the Cyclone V part of the earlier
measurements, is the estimate). The real limit on the boards
comes from the students' frequency sweep.

### 4.1 What limits behind the integer divider (measured 2026-10-14)

On the v7.1 `sapho_all` fit of 2026-10-11 (DE10-Nano, 21.82 MHz; the
hardware is unchanged since, only comments moved), the timing analysis was
run again with the worst endpoints declared false paths one group at a
time, to see the next one (`Scripts/hw/path_groups.py` groups the 5000
worst paths by destination). No new fit: `quartus_sta -t` on the fitted
project, with `clk.sdc` plus lines like these (in Quartus, `*` does not
cross a `|` unless `-compatibility_mode` is given):

```
set_false_path -to   [get_registers {*my_idiv|div_array:arr|cut[*}]
set_false_path -from [get_registers {*my_idiv|div_array:arr|cut[*}]
set_false_path -to   [get_registers {*my_fdiv|div_array:arr|cut[*}]
set_false_path -from [get_registers {*my_fdiv|div_array:arr|cut[*}]
set_false_path -through [get_cells -compatibility_mode {*op_fadd.my_fadd|*}]
set_false_path -through [get_cells -compatibility_mode {*op_denorm.denorm|*}]
```

and `report_timing -setup -npaths 5000 -detail summary` (and
`-to [get_registers {*core:core|racc*}]` for the paths to the
accumulator). The placement is the same, so these are the
paths of this fit, not of a design with the dividers changed:

| excluded | worst path left | data delay |
|---|---|---|
| nothing | integer divider part 1: data memory -> `NEG_M` -> rows to `cut[1]` | 43.3 ns (slack -35.8) |
| `cut[1]` of `DIV`/`MOD` | integer divider part 2: `cut[1]` -> `cut[2]` | 43.6 ns (slack -34.3) |
| every cut of `DIV`/`MOD` | `F_DIV` part 3: `cut[2]` -> rows (23.5) -> normaliser (9.8) -> ALU mux -> `JIZ` -> `pc` | 42.4 ns (slack -33.0) |
| every cut of both dividers | `F_ADD`: data memory -> `denorm` (9.0) -> add (2.2) -> normaliser `norm_mux` (17.9) -> ALU mux -> `JIZ` -> `pc` | 40.2 ns (slack -32.4) |

Conclusions:

- The integer divider is not cut in the wrong place: its two first parts
  are equal (43.3 and 43.6 ns). Moving a cut gains nothing; a third cut
  (four stages, `DIV x; NOP; NOP; QUO`) is the change that shortens it.
  Luciano (2026-10-14): more registers in the integer divider are fine, a
  division is rare. `F_DIV`'s last part is as long (42.4 ns) and needs the
  same.
- The clock follows the slack, not the data delay (period = 10 ns - slack:
  skew and setup count): 21.8 MHz today (slack -35.8); a third cut in the
  integer divider leaves `F_DIV` part 3 (slack -33.0, ~23.2 MHz, +6.5 %);
  a third cut in both leaves `F_ADD` (slack -32.4, ~23.6 MHz, +8 %).
  Estimates on this placement, not a fit. The
  `F_ADD` path ends at the program counter because `JIZ` decides on the
  ALU output in the same clock (`if_acc = |ula_out`); the normaliser is
  44 % of it. `F_ADD` runs too often to take a cycle (ula-combinational).
- Not measured: the Zynq (Vivado) side of the same exclusion, and a real
  fit with a third cut (placement changes).

### 4.2 Behind the dividers, without registers (2026-10-14)

Recorded, not to be implemented yet (Luciano). Same fit, same method, the
dividers excluded; each float path ends at two places, the program counter
(through the `JIZ` decision, which reads the ALU output in the same clock,
`if_acc = |ula_out`) and the accumulator:

| path (data delay of each block, ns) | to the PC | to the accumulator |
|---|---|---|
| `F_ADD`: memory -> `denorm` 9.0 -> add 2.2 -> normaliser 17.9 -> ALU mux | slack -32.4 (~23.6 MHz) | -28.7 (~25.8 MHz) |
| with `F_ADD` excluded too, `I2F_M`: memory -> `NEG_M` 3.2 -> convert 8.4 -> normaliser 16.9 -> ALU mux | -30.4 (~24.8 MHz) | -26.7 (~27.3 MHz) |

Two facts follow. The `JIZ` decision adds about 4 ns to every path that
ends at the PC. The shared normaliser (`op_smx.norm_mux`) is 44-48 % of
each float path, `F_ADD` and `I2F_M` alike. 55 % of the delay is
interconnect: seeds give a few per cent, not more.

Ideas without a new register, and how they combine (all estimates on this
placement):

1. **The `JIZ` zero test early**: "the result is not zero" computed beside
   the ALU instead of from its final output (for `F_ADD`, the raw sum is
   zero before normalising). Every path loses its PC tail, the limit
   becomes the accumulator paths (~25.8 MHz with the dividers fixed). It
   must agree exactly with the stored result, including a tiny result the
   normaliser flushes to zero (`#FROUND`), or `JIZ` jumps on a value that
   is not the one kept. Small in area; touches every processor's core.
2. **A two-path float adder** (far path: big alignment, at most one bit of
   normalising; near path: no big alignment, full normalising): one large
   shifter per path instead of two in series. Shortens only `F_ADD`, and
   `I2F_M` through the same normaliser is right behind (~27.3 MHz). Costs
   area; must stay bit-exact at every `#FROUND` level or every golden moves.
3. **Leading-zero anticipation** in the normaliser: count the leading zeros
   while the value is computed, not after.
4. **The normaliser's input mux out of the path**: a normaliser per float
   operation instead of one shared one.

Combined: 1 with a faster normaliser (3, 4) shortens every float path at
once; 2 only if `F_ADD` is still ahead after that. If the normaliser lost
half of its ~17 ns, the accumulator paths would be ~26-28 ns, near 30 MHz;
speculation, since the path behind those was not looked at.

### 4.3 The experiments, timed (2026-10-15)

Luciano asked to try the combination of 4.1 and 4.2 and see whether
`sapho_all` reaches 30 MHz. Four variants, each containing the one before,
built as copies of `SAPHO/` by `Scripts/hw/clock_exp/make_variants.py`
(nothing in `SAPHO/` changed), one Quartus fit each on the DE10-Nano part
(5CSEBA6U23I7, slow 100C model, seed 1):

| variant | change | Fmax | worst path after |
|---|---|---|---|
| v7.1 (2026-10-11) | -- | 21.82 MHz | integer divider part 1 |
| `stg4` | both dividers in 4 stages | 26.39 MHz | `F_ADD` -> normaliser -> `JIZ` -> PC, 35.8 ns |
| `jz` | + `JIZ` reads an ALU output without the normaliser | 28.49 MHz | integer divider part 1, 33.0 ns; second, `F_ADD` -> normaliser -> indirect read index -> data memory, 34.7 ns delay |
| `lzc` | + the normaliser's leading-zero count as a tree | 29.03 MHz | integer divider part 1, 31.9 ns (all 3000 worst paths) |
| `addr` | + the indirect read index from the same fast output, integer divider in 5 stages | **32.07 MHz** (+47 %) | `F_ADD` -> normaliser -> accumulator, 30.9 ns (denorm 7.5, add 2.6, normaliser 14.8) |

Facts behind them:

- The placement changes with every fit: `stg4` came out at 26.4 MHz where
  the estimate on the old placement (4.1) said 23.6. The estimates of 4.1
  and 4.2 were conservative.
- The tree leading-zero count (`ula_lzc`) gives the same normaliser output
  as the chain: 200000 random inputs in each of nine formats and rounding
  levels (4 to 52 mantissa bits, `#FROUND` 0, 1, 2), 1.8 million outputs,
  all equal (`Scripts/hw/clock_exp/eq_norm.sh`). It is the only change of
  the four that keeps every program's behaviour.
- `jz` and `addr` rely on a rule: no `JIZ`, `LDI` or `ILI` right after an
  operation that goes through the normaliser (`F_ADD`, `F_SU1`, `F_SU2`,
  `F_MLT`, `F_DIV`, `I2F`, `I2F_M`, `F_ROT` and their stack forms). Counted
  in the 87 C± goldens: 450 `JIZ`, 141 `LDI`, 42 `ILI`, none after such an
  operation. The C++ side was not counted.

**The ALU output split instead of copied** (Luciano's proposal, 2026-10-15).
`jz` and `addr` copy the whole `ula_mux` with the normaliser input tied to
0. The cleaner structure: one mux of the operations that do not go through
the normaliser (`out_fast`, read by `JIZ` and the indirect index, the
normalised operations at `x` in it) and, after it, a 2-input mux choosing
the normaliser's output for the eight codes of the `norm_mux` list (3, 5,
7, 25, 26, 46, 47, 48; `out`, to the accumulator). Variant `split` (`addr`
with that change), one fit:

| variant | Fmax | logic |
|---|---|---|
| `addr` (copied mux) | 32.07 MHz | 3,816 ALMs |
| `split` | 31.52 MHz | 3,622 ALMs |

194 ALMs fewer, back near `stg4`'s 3,602, and the clock within one fit's
noise (the worst path's data delay is shorter, 29.1 against 30.9 ns; the
slack is worse by the clock skew of this placement). The 2-input mux is one
cell on the worst path (2.6 ns, most of it wire), as expected. `split` was
also simulated against stock `SAPHO/` on three float programs without a
division (`cmm_cexp`, `sw_test`, `proc_fft`, at the regress's `-f 100 -c
100000`): the outputs are equal to stock and to the goldens.

**Does an `x` mux input cost logic?** Measured in Yosys on `ula_mux` alone
(`synth -flatten -noshare`, `abc -lut 4`), unused inputs at `x` against a mux
written with only the used arms (`Scripts/hw/clock_exp/mux_x_cost.py`): integer set (8 of 52 arms) 235 against
229 LUT4, noise; float set (18 arms) 306 against 253, +53 LUT4 (+21 %),
above `abc`'s ~10 % noise. Every input real: 1,068. So `x` removes most of
an unused arm but not all of it in a float program; on the whole processor
that is about 1 %. Generating the arms from the opcode parameters would take
it out (the rule of pay-only-for-what-you-use: `x` is a fallback). One run,
Yosys, not Quartus.

**The muxes generated from the opcode parameters** (Luciano, 2026-10-15;
in progress). Variants built from `split`: `param` (fast mux as an OR of one
term per operation the program has, each term in a `generate` on its
parameter; the 2-input mux and `is_norm` only with a normalised operation),
`pcase` (the fast mux in the instruction decoder's style: a case with one
row per operation guarded by its parameter, default 0), `pnorm` (`param`
plus the normaliser's input mux `norm_mux` guarded the same way, default 0
instead of `x`). All three simulated equal to stock on `cmm_cexp`,
`sw_test`, `proc_fft`. The instruction decoder (`instr_dec.v`) already has
that form: each row guarded by its parameter, default 0 (76 -> 31 ALUTs
when it got it). The other core muxes are already chosen by `generate`
(ALU input 2, jump target, return, interrupt, FFT index) or driven by a
decoder constant (ALU input 1, deduced to be removed by constant
propagation, not measured).

`sapho_all` fit: `param` **32.26 MHz, 3,539 ALMs** (below v7.1's 3,558).
Synthesis-only area estimates (Quartus map, `fmax.sh MAPONLY=1`), ALMs:

| program (ops) | stock | split | param | pcase |
|---|---|---|---|---|
| `cmm_tiny0` (4) | 29 | 29 | 29 | 29 |
| `sw_test` (8) | 66 | 66 | 73 | 75 |
| `cmm_cexp` (18) | 585 | 570 | 543 | 581 |
| `ProcDTW` (21) | 439 | 439 | 431 | 379 |

Mixed: estimates of synthesis, not fits, and small designs move by a few
ALMs either way. Still to run: the `pcase` fit and the `pnorm` battery.

What each would need to become the processor, none done:

1. More division stages: `asm_divseq` in both compilers and asmcomp's check
   write `<div> x; NOP; NOP; QUO` (four words; five for `DIV`/`MOD` in
   `addr`), derived from the stage parameter, not a constant (derive from
   widths); every golden with a division moves; hand-written `.asm` with a
   division changes.
2. The `JIZ` and the indirect index without the normaliser: asmcomp refuses
   (or pads with a `NOP`) a `JIZ`/`LDI`/`ILI` right after a normalised
   operation; the C++ goldens counted; the split structure of `split`
   rather than a copy of the mux, built only where the program has a
   normalised operation (pay only for what you use).
3. The tree count: `ula_lzc` in `SAPHO/ula.v` and `eq_norm.sh` kept as a
   test; `elab.sh`, `elab_single.sh` and `tb_alu.sh` on it.
4. All: the regress, area (`area.sh`; the fits show 3,558 -> 3,816 ALMs
   from v7.1 to `addr`), the Vivado side (not timed), more seeds.

Behind 32 MHz the float add itself limits (30.9 ns to the accumulator): the
shift and the rounding inside the normaliser, or the two-path adder of 4.2.

## 5. Tools kept from this work

- `Scripts/hw/fmax.sh`: `SEED=<n>` (compare a change across seeds: one fit
  is off by a few per cent);
- `Scripts/hw/path_blocks.py`: each block of the worst path split into
  interconnect and cell delay;
- `Scripts/hw/path_groups.py`: the worst path of each destination block in
  a long report (fmax.sh and vfmax.sh now write `many_paths.txt`), so the
  second block that limits is not hidden behind the first;
- `Scripts/hw/vfmax.sh`: retries Vivado's intermittent empty "[Synth 20-411]";
- `Scripts/hw/opmix/` (`pipeln_cost.py`): the operation mix of the regress
  programs and the extra cycles of a given latency table;
- `Scripts/hw/est/`: the Yosys per-family estimate.
