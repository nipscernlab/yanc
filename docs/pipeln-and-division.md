# The slow operators: what was tried for a faster clock, and dropped

Record of TODO item 19 (2026-10-03 to 2026-10-05). The goal was a higher
clock for programs that use the slow ALU operators. A paused pipeline was
designed, built for the three dividers and measured on two FPGA families;
Luciano then dropped it (2026-10-05) and `SAPHO/` went back to the processor
before it (commit 954c853). **Nothing of what is described here is in
`SAPHO/` today.** A co-processor for the dividers alone (section 3) was
discussed next and dropped too (Luciano, 2026-10-05): the processor stays
all combinational, and a program that divides runs at the divider's clock
(~10-12 MHz). What stays is the measurements below and the measurement
tools; section 3 keeps the co-processor options as they were discussed, in
case the question comes back.

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
`docs/precision-and-width-review.md` §2.6.

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

## 4. Tools kept from this work

- `Scripts/hw/fmax.sh`: `SEED=<n>` (compare a change across seeds: one fit
  is off by a few per cent);
- `Scripts/hw/path_blocks.py`: each block of the worst path split into
  interconnect and cell delay;
- `Scripts/hw/vfmax.sh`: retries Vivado's intermittent empty "[Synth 20-411]";
- `Scripts/hw/opmix/` (`pipeln_cost.py`): the operation mix of the regress
  programs and the extra cycles of a given latency table;
- `Scripts/hw/est/`: the Yosys per-family estimate.
