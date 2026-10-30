# Array unit: a memory for arrays, a controller and a MAC

Status: design, started 2026-10-24, **parked on 2026-10-26** (Luciano), to
be taken up later, whole, in part or changed; section 9 says where it
stopped and how to resume. Nothing in `main` but this note and the MAC
study's scripts (`Scripts/hw/mac_study/`). A prototype, to measure and not
to merge, is kept in the tag `archive/array-unit-proto` (steps 4a and 4b
of section 7); the implementation for `main` is written again, clean, once
the decisions are taken. **Rule (Luciano,
2026-10-24): no code in this repository until every decision below is
taken, each one backed by tests and measurements made outside the YANC
code** (prototypes in a scratch folder or a worktree, standalone
testbenches, models in Python). Tracked in issue #18. Marks: **fact** (read
in the code), **measured**, **deduced** (reasoned, to be checked),
**general knowledge** (not from this repository, to be checked before it
is used for a decision).

## 1. How this started

Item 31 of TODO.md (the Dirac output `out(p, c|a⟩)` multiplies by 1) led to
a survey of the whole Dirac library of C± on 2026-10-24.

**Fact:** every Dirac form is unrolled at compile time, one group of
instructions per element and no loop (`exec_*` in
`Compilers/CMMComp/Sources/stdlib.c`, from line 2625). **Deduced** from the
generator: a float `y # |M|v⟩` of 16 x 16 is about 768 words of program
(per row `LOD_V`, `F_MLT`, 15 x (`P_LOD_V`, `F_MLT_V`, `SF_ADD`), `SET_V`).
Program memory grows with the arrays, which rules the notation out for the
sizes where it would matter.

**Measured**, a defect: the outer product is stored transposed. `A #
|a⟩⟨b|` with a = (1, 2), b = (10, 30) outputs `A[0][0..1], A[1][0..1]` =
10 20 30 60; the right answer is 10 30 20 60 (`exec_vvt` writes `N*j+i`). A
control test, `y # |M|v⟩` and a read of `M[0][1]`, is right, so the layout
is row-major and only the outer product is wrong. `exec_Mmvvt` (`|A| -
|a⟩⟨b|`) uses the same index: deduced to have the same defect, not tested.
The fixtures miss it: in `proc_rls` the outer product is `K⟨Px|` with `K =
g Px`, a symmetric matrix; `dirac_assign` never outputs the matrix.
Luciano: not fixed now; it goes with the redesign.

The survey's list of limits and missing forms is in issue #18. In short:
no `comp`, no int/float mix, a useless `MLT` when the scalar is 1, square
outer products only, and missing the bilinear form `⟨a|M|b⟩`, the row
vector `⟨a|M|`, unweighted sums and copies, the general rank-one update,
the matrix product, the transpose and the sum of the elements.

**Decision (Luciano, 2026-10-24):** the Dirac library is not extended as
it is. SAPHO first gets a unit for arrays, separate from the ALU; the
Dirac notation is then redone on it. The burst ([burst.md](burst.md),
issue #18) is part of the same unit: the inner product needs the same
size, pointer and wait in place as a burst to a port.

**Changed (Luciano, 2026-10-25):** the Dirac notation is redone first, on
its own, as expressions with shapes computed in loops
([dirac-expressions.md](dirac-expressions.md)); step 5 showed why unrolled code
cannot stay (section 6, decision 13). The array unit enters it later, as
the hardware choice beside the loop in software.

## 2. The architecture (Luciano, 2026-10-24)

- **An option**, chosen by a parameter: with it off, the processor is
  today's, unchanged.
- **Every array, and every object reached through a pointer, lives in a
  new dual-port memory**, the array memory. The data memory keeps the
  scalars only and no longer needs indirect addressing.
- **A controller owns the array memory.** It reads the data and sends
  them to the ALU's input through a new multiplexer, beside the data
  memory's output.
- **The controller can run a sequence of operations on the array memory
  by itself.** Meanwhile the core waits on an instruction that jumps to its
  own address, as `TOM` and `CAD` do, until the controller finishes.
- **A MAC of its own** (decided after the alternative below was weighed):
  a multiply and an add every clock, separate from the ALU.

The alternative weighed for the arithmetic: the ALU itself, idle while the
core waits, fed by the controller through the new multiplexers. No new
arithmetic hardware, results identical to today's sequential code, but two
ALU operations per element of an inner product (multiply, then add), so 2
clocks per element. Luciano chose the separate MAC.

## 3. Facts in today's code the design builds on

- `mem_data` has one read port and one write port (`SAPHO/processor.v`
  54-80: `data_out <= mem[addr_rd]`, `if (wr) mem[addr_wr] <= data_in`).
  An inner product at one pair per clock needs two reads per clock: the
  reason for a dual-port array memory.
- Without `LDI`/`STI`/`ILI`/`ISI` in the program, the data memory's
  address is the operand field alone (`SAPHO/core.v` 1177). The adder `acc
  + offset` (`rel_addr`) and the FFT bit reversal are generated only with
  those instructions (`core.v` 470-479). In this design the data memory is
  always in the simple case; the indirect path moves to the controller.
  **Deduced:** the path from the ALU to the data memory's read address,
  which item 29 had to handle (the `NOP` asmcomp inserts before
  `LDI`/`ILI`), leaves the data memory.
- The wait in place exists: `pc_self` in `core.v` makes the fetch take
  the same address again (`TOM`/`CAD`). Checked in burst.md 5a: it repeats
  every clock with no bubble.
- C++ (`docs/language-cpp.md`): a recursive function gets a frame on a
  1024-word call stack; non-recursive functions keep their locals at fixed
  addresses; `new`/`malloc` share a 2048-word heap. **Deduced:** under the
  rule of section 2 the heap, the call stack and every object whose address
  is taken go to the array memory, so in a C++ program most of the data
  would.
- The float format (`docs/floating-point.md`): sign and magnitude, an
  exponent in two's complement with no bias, a mantissa of `NBMANT` bits
  with its top bit written out (`2^(M-1) <= m < 2^M`). The product of two
  mantissas lies in `[2^(2M-2), 2^(2M))`: at most one bit of normalisation.
  `F_MLT` already skips the leading-zero count (precision-and-width-review.md 2.7).

## 4. The MAC

### 4.1 What sets the clock

At one pair per clock, each sum depends on the previous one. The path
that sets the clock is the **feedback**: accumulator -> add ->
accumulator. The multiply, and any shift of the product, come before the
feedback and can sit in an earlier stage behind a register: the rate stays
one pair per clock and the whole vector pays one clock of latency
(deduced). "Multiply and add in the same cycle" is then a matter of rate,
not of putting both in one combinational path.

### 4.2 Without normalisation between the multiply and the add

The product enters the add whole (`2M` bits), not rounded or normalised:
one rounding fewer, a more exact result. That alone shortens little. The
gain is in taking the normalisation out of the feedback. Variants:

| variant | in the feedback | result |
|---|---|---|
| fused multiply-add per element | align, add, leading-zero count, normalise | as long as today's `F_ADD`, the path that limits the clock (item 29): no clock gained; kept only as the baseline |
| **A.** lazy accumulator | compare exponents, align, add; normalised once at the end | loses precision on a large cancellation unless the accumulator is wide |
| **B.** long fixed-point accumulator, windowed | an integer add (carry-save possible) | exact inside a window around an exponent fixed by a parameter; a flag (a pin, like `cheguei`) when a product falls outside |
| **C.** long fixed-point accumulator, full range (Kulisch) | an integer add | exact for any data, one rounding at the end, independent of the order of the terms |

In B and C each product is shifted by its exponent to its place in the
long register before the feedback (a stage of its own), so the feedback is
an integer add.

### 4.3 Widths (deduced, not measured)

The product is `m1 m2 . 2^(e1+e2)`. A long accumulator covering every
product needs about `2M + 2^(E+1) + log2(N)` bits (`M = NBMANT`, `E =
NBEXPO`, `N` the most terms summed):

| format | full range (C) |
|---|---|
| 32 bits (`NBMANT` 23, `NBEXPO` 8), N up to 1024 | about 570 bits |
| 16 bits (`NBMANT` 10, `NBEXPO` 5) | about 95 bits |

At 32 bits the register and its adder are less of a worry than the
shifter that places a 46-bit product anywhere in 570 positions: estimated
in the thousands of multiplexers, comparable with the whole `sapho_all`
(3759 ALMs measured 2026-10-02). Two ways to shrink it: the
window of B (100 to 150 bits, derived from the widths), or an accumulator
in blocks, where the high bits of the exponent choose the block and the
shifter covers only the low bits.

### 4.4 Rounding and results

- The final rounding follows `#FROUND` (truncation at 0 and 1, nearest
  with ties to even at 2).
- The result differs from today's sequential code, on purpose: it is more
  exact. The goldens of programs that use the Dirac forms (`proc_rls`)
  change, with the reason written in the pull request.
- **Testing is simple for B and C:** the exact sum is a rational number
  (Python `fractions.Fraction`), rounded once; the reference does not
  depend on the order of the terms.

### 4.5 Integers and complex numbers

- **int:** the multiply in one stage, the add in the next. The product has
  `2 NUBITS` bits; the accumulator's width and what happens on overflow are
  open (section 6). General knowledge: the DSP blocks of the Cyclone V and
  of the Zynq have a multiplier and an accumulator built in.
- **comp:** four real products and two accumulators (real and imaginary);
  with `comp`, the bra conjugates. Later.

## 5. The array memory

Accesses per element, by operation (deduced):

| operation | reads | writes | clocks per element, two ports |
|---|---|---|---|
| inner product `⟨a\|b⟩` | 2 | 0 | 1 |
| matrix-vector `\|M\|v⟩` | 2 | one per row | about 1 |
| `\|a⟩ + c\|b⟩`, rank-one update | 2 | 1 | about 1.5 |
| copy | 1 | 1 | 1 |
| fill | 0 | 1 | 1 (2 with both ports) |
| burst to a port | 1 | 0 | 1 |

General knowledge, to be checked by synthesis before any decision: the
Cyclone V M10K in true dual-port mode is narrower than in simple dual-port
mode (20 bits against 40), so a 32-bit true dual-port memory would take
twice the blocks; the Xilinx 36 Kb block RAM is 36 bits wide in true
dual-port mode. The alternative to true dual port: two banks (X and Y, as
in classic DSPs), the compiler placing the two operands of an operation in
different banks; it cannot serve `⟨a|a⟩` without a copy. Luciano left it
out (2026-10-24): the memory is true dual port.

## 6. Decisions to take

1. **The parameter**: its name, a directive or an option of `yanc`, its
   default. Off: today's processor, bit for bit.
2. **What goes to the array memory**: every array; every object whose
   address is taken (C++); the C++ heap and call stack.
3. **A single access** (`a[k]` with `k` variable): the same timing as
   today's `LDI`, one clock with no wait. A requirement, to be checked in
   simulation first.
4. **The instruction set of the controller**: one instruction per
   operation (`DOT`, `AXPY`, ...) or a small programmable sequencer.
   Designed once with the burst's `BSZ`/`BPT`/`BOT`/`BIN` and the fast fill
   and copy (issue #18). **The inner product, decided (Luciano, 2026-10-25):
   `LEA a; P_LEA b; DOT N`**, with no register for the size or the pointers:
   N is the operand of `DOT`, read as `LOD` reads it; pointer a is on the
   stack top, popped once when `DOT` ends; pointer b is in the acc. Fitted
   beside it, not decided (section 7.5): a second form, `LEA a; P_LOD N; DOB b`, with b an
   address fixed when compiling, one clock shorter. The burst's `BSZ` and
   `BPT` (burst.md) predate this and load registers; whether the burst
   takes `DOT`'s form (pointer on the stack, N as the operand, no
   registers) is open.
5. **The MAC variant** (A, B or C), and for B the window. **Decided
   (Luciano, 2026-10-24): A**, the lazy accumulator with W = 2M+8, on the
   results of section 7.1: the error of one rounding, as C, at the area
   of B, with no window for anyone to choose.
6. **The int accumulator**: width, overflow (wrap, saturate, flag). From
   the code: `MLT` keeps the low `NUBITS` bits of the product (`ula.v` 536:
   `assign out = in1 * in2` into an `NUBITS`-bit wire) and the integer add
   wraps. A sum modulo 2^`NUBITS` does not depend on the order, so an
   `NUBITS`-bit accumulator gives exactly today's result; a wider one
   matters only to saturate or to flag an overflow, a choice of meaning,
   not of accuracy.
7. **The scalar operand** of `c|a⟩`: from the accumulator into a
   register of the controller.
8. **The memory**: true dual port (two banks left out, Luciano,
   2026-10-24); the size of each memory (today one `MDATAS`).
9. **Interrupts** during a sequence (the same question as for the burst
   and `TOM`/`CAD`, toma-and-cade.md section 6).
10. **The compilers**: two address spaces, two memory images. **How an
    instruction names its memory, decided (Luciano, 2026-10-24): a bit of
    the operand.** With the option on, asmcomp numbers two memories from 0:
    scalars and constants in the normal one, everything declared by
    `#array`, `#arrays` or `#arrayd` in the array memory. The data operand
    gets one bit above the address bits, S: 0 the normal memory, 1 the
    array memory (`LOD k` = `0 00011`, `F_MLT a` = `1 00000`, `LOD_V a 2` =
    `1 00010`). S, registered, drives the new mux at ALU input in1 on a read
    and picks which memory takes `mem_wr` on a write. `LDI`/`STI`/`ILI`/`ISI`
    always use the array memory, whatever S, so a pointer (`LDI 0`, the
    address in the acc) is an address of the array memory; `LEA a` gives
    a's address there, without S. The normal memory loses the indirect
    adder: its address is the operand. Rejected: new opcodes for the direct
    forms. Element 0 of an array is reached by the plain instruction (`LOD
    w`, `F_MLT x`, `SET x` in `proc_rls`; `F_MLT a` in `dirac_assign`), so
    any of the 56 memory-operand mnemonics of `isa.tsv` can take an array,
    and 20 opcodes are free in 7 bits: twins of `LOD`/`P_LOD`/`SET` alone
    would cost a load before every other operation on an element, twins of
    all would need an 8-bit opcode. The cost of S: one bit more in the
    operand when the data addresses, not the code's, set its width.
    cmmcomp and cppcomp keep writing `LOD a`, `LOD_V a 2`; the change is in
    asmcomp (both of its passes). `asm_reach`
    and `asm_share` per space, and both must see a sequence's pointers as
    reads (and writes) of the whole array (burst.md 5a, hole 4).
11. **Simulation and AURORA**: the variables of the array memory visible
    by name in the waveform (`pc_sim_val`, `gen_gtkw`, the `YANC_SIM_VIS`
    block AURORA reads).
12. **Hand-written `.asm`**: what `LDI`/`STI` mean with the option on.
    Answered by decision 10: they always reach the array memory, so the
    address they use is one of the array memory (as in the prototype).
13. **The Dirac notation redone**: the forms of issue #18, `comp`, the
    conjugating bra, the outer product the right way round. **Taken out
    of this unit** (Luciano, 2026-10-25): unrolled, `proc_rls` grows as
    14N² + 21N + 27 words (232 091 at N = 128, more than the block RAM of
    the DE10-Nano), and each new combination of vectors and matrices needs
    a grammar rule and a generator of its own. The notation becomes
    expressions in the parser, computed in loops, independent of this unit
    ([dirac-expressions.md](dirac-expressions.md)); only
    then a choice between the loop in software and this unit.
14. **Operations beyond the inner product** (recorded, Luciano,
    2026-10-25; none designed, nothing measured). Each reuses what `DOT`
    built: the controller's pointers and counter, the wait in place, the
    two ports, and the result register inside the unit, which can feed the
    array memory's write port as well as ALU input in1. The least clocks a
    word the two ports allow (**deduced**: accesses a word over two ports):

    | operation | accesses a word | clocks a word | result |
    |---|---|---|---|
    | burst out, an array to a port (`BOT`) | 1 read | 1, while the port takes it | the port |
    | burst in, a port into an array (`BIN`) | 1 write | 1, while the port has a word | the array |
    | copy of an array into another | 1 read, 1 write | 1 | the array |
    | fill of an array with a value | 1 write | 1 (0.5 with both ports) | the array |
    | elementwise: `\|a⟩ + \|b⟩`, `c\|a⟩ + \|b⟩`, products, ... | 2 reads, 1 write | 1.5 | the array |

    Open for each: the instruction form (decision 4: `DOT`'s, the pointers
    on the stack and in the acc, or the burst's `BSZ`/`BPT` registers); the
    port's handshake for the bursts (burst.md 4 and 5a); interrupts
    (decision 9). For the elementwise operations, who computes each word:

    - **the MAC**, `a·1 + b·1` and the like: two products a word on its one
      multiplier, so 2 clocks a word; only what a multiply-add does, and a
      word rounded once, not as today's loop rounds it;
    - **the ALU, fed by the controller** (the alternative of section 2,
      left out there for the inner product, where it takes two operations
      a pair): one operation a word, any operation of the ALU, and words
      equal bit for bit to the loop in software, so the choice between the
      loop and the unit in the Dirac expressions (decision 13) would not
      change a program's output. The cost: a new input on the mux at ALU
      input in2, at the start of the worst path, and `ula_out` into the
      array memory's write port.

    **No elementwise operation with a scalar and a sum in one** (Luciano,
    2026-10-25): `c|a⟩ + |b⟩` is a scale (`α·a`) then an add, two
    operations. Two roundings, as today's loop rounds, so the words stay
    equal to the loop's (through the ALU); the cost (**deduced**): 1 + 1.5
    clocks a word and a temporary array, against 2 for a fused one through
    the ALU.

    **Open, asked when the session ended** (2026-10-25):

    - **The operands' form** (decision 4): A, `DOT`'s form for every
      operation (stack top, acc, operand; no state kept in the unit); B,
      registers as in burst.md (`BSZ`/`BPT`); C, `DOT`'s form with a
      register only where values are left over. Recommended: A.
    - **In place or a separate destination** for the scale and the add.
      In place (`a = α·a`, `a = a + b`) every operation of the unit takes
      three values and fits `DOT`'s form exactly; `c = a + b` keeping a is
      then a copy and an add, one pass more (1 clock a word). A separate
      destination takes four values: two levels of the stack, a clock more
      at the start (deduced). Recommended: in place.

## 7. Tests and measurements before deciding (no YANC code touched)

1. **MAC models in Python**, bit-exact, for A, B, C and the fused
   baseline: random vectors, with and without cancellation, every
   `#FROUND`, at 32/23/8 and 16/10/5, against the exact `Fraction` sum.
   Gives the error of each variant.
2. **Each MAC in Verilog, standalone** (the way the normalisation and add
   surveys built their variants), checked against the models, then area
   and Fmax. A MAC alone gives a bound, not the answer: the critical path
   is the whole processor's (`fmax.sh`), measured in step 4.
3. **The array memory**: a 32-bit true dual-port memory of the sizes of
   the fixtures, synthesized in Quartus and Vivado; blocks used, against
   today's simple dual port.
4. **A prototype of the whole** in a worktree copy of `SAPHO/`, with
   hand-written `.asm`: the single access with today's timing, an inner
   product in a sequence, the wait in place; then `fmax.sh` on `sapho_all`
   with and without the option, the area from the same fits (without it,
   nothing may change).
5. **What it buys**: words of program and clocks of the Dirac fixtures and
   of `proc_rls`, today against each variant.

### 7.1 Step 1, the accuracy of each MAC (2026-10-24)

`bash Scripts/hw/mac_study/run.sh` (about 5 minutes; the report in
`.smoke/hw/mac_study/<width>/report.txt`). Inner products of 4 to 1024
terms (256 at 16 bits), 30 vectors per size and set: `pos` (magnitudes
2^-2 to 2^2, positive), `mixed` (the same, random signs), `wide` (2^-20 to
2^20 at 32 bits, 2^-3 to 2^6 at 16), `cancel` (pairs that cancel exactly
and two small terms that survive), `sparse` (a third of zeros). "today" is
the Dirac loop run on `SAPHO/ula.v` (`tb.v`: `F_MLT` with in1 = b, in2 = a,
then `F_ADD` with in1 = the sum, in2 = the product, as `core.v` feeds the
ALU); the variants are bit-level models in `mac.py`. The error is in units
of the last place (ULP) of the exact result, mean |e| / max |e|.

Excerpt at 32/23/8, 1024 terms (the full tables are in the report):

| set | `#FROUND` | today | F (fused, rounded per term) | A, W = 2M+8 | B, 64 bits | C (exact) |
|---|---|---|---|---|---|---|
| pos | 0 | 495 / 707 | 249 / 349 | 0.50 / 0.91 | 0.50 / 0.91 | 0.50 / 0.91 |
| pos | 2 | 3.2 / 15.6 | 3.2 / 15.6 | 0.31 / 0.49 | 0.31 / 0.49 | 0.31 / 0.49 |
| mixed | 1 | 16 / 81 | 357 / 2420 | 0.65 / 0.98 | 0.65 / 0.98 | 0.65 / 0.98 |
| mixed | 2 | 9.7 / 79 | 8.6 / 49 | 0.25 / 0.46 | 0.25 / 0.46 | 0.25 / 0.46 |
| cancel | 2 | 2.5e6 / 3.2e7 | 2.9e6 / 3.9e7 | 0.26 / 0.47 | 0.26 / 0.47 | 0.26 / 0.47 |

What the numbers say (measured on these data):

- **Every variant that keeps the sum unrounded to the end gives the error
  of one rounding**, at most 0.5 ULP at `#FROUND 2` and below 1 ULP when it
  truncates, at every length. Today's loop grows with the length: 495 ULP
  on average for 1024 positive terms at `#FROUND 0`, 3.2 (max 15.6) at
  `#FROUND 2`; on the cancellation set, millions of ULP of the small result,
  as any sequential float sum.
- **F, the fused multiply-add rounded per term, buys little**: as today at
  `#FROUND 2`; at 0, half of today on positive terms and the same on mixed
  signs; worse than today at `#FROUND 1` on
  mixed signs (where today's two errors of level 1 cancel,
  fround1-subtraction.md). Together with section 4.2 (no clock gained), F
  is out.
- **A, the lazy accumulator**: with W = 2M+8 (54 bits at 32, 28 at 16) it
  matched C on every vector; with W = 2M+2 it lost up to 316 ULP on 1024
  cancelling terms at 32 bits (68 on 256 at 16). The extra low bits matter
  only where the sum cancels. The shifts are arithmetic, as in the
  hardware (toward minus infinity, a bias); the first version of the model
  cut toward zero and lost at most 6 ULP at W = 2M+2: the direction of the
  cut matters only when the width is short.
- **B, the window**: placed from the data's range (an oracle here; in
  hardware a parameter someone must choose), 2 x `NUBITS` bits matched C on
  the sets of magnitudes 2^-2 to 2^2; the `wide` set at 32 bits needed 96 bits
  (with 48 the error reached 3.4e5 ULP). Products that fall below the window
  are cut without notice; only an overflow at the top can be flagged.
- **C, the full range**: 567 bits at 32/23/8 for 1024 terms, 91 at 16/10/5
  for 256: exact by construction, the reference itself.
- At 16/10/5 the `cancel` set has 12 vectors whose exact sum is below the
  smallest float of the format; they are left out of every column.

### 7.2 Step 2, each MAC in Verilog (2026-10-24)

**Correctness:** the four variants in Verilog
(`Scripts/hw/mac_study/mac.v`: A with W = 2M+8, B with 2 and 3 x `NUBITS`,
C) give the word of their model (`mac.py expect`) on every vector of the
study, 750 at 32/23/8 and 588 at 16/10/5 (12 left out, out of range), at
`#FROUND 0` and 2 (`check.sh`, 16 runs, 0 different). A mutation (the
hardware at level 0 against the level-2 model) gives 287 different, the
first by 1 ULP: the check sees a one-unit error.

**Cost:** Quartus 24.1 on the DE10-Nano's Cyclone V
(5CSEBA6U23I7, the slow 1100 mV 100 C corner), each block alone in
`mac_fit` (inputs and output registered, the pins virtual), `#FROUND 2`,
`Scripts/hw/mac_study/fit.sh`; "synthesis" is `quartus_map`'s estimate
(`MAPONLY=1`), the rest a full fit with a 10 ns clock asked. The worst path
into each group of registers, data delay in ns:

| variant | ALMs | DSP | Fmax of the block | feedback (the loop) | rounding at the end | prepared product | product |
|---|---|---|---|---|---|---|---|
| A, W = 54, 32 bits | 1003 (fit) | 1 | 48.7 MHz | **15.7** | 20.3 | 5.1 | 3.1 |
| B, 64 bits, 32 bits | 964 (synthesis) | 1 | not fitted | | | | |
| C, 567 bits, 32 bits | 4654 (synthesis) | 1 | not fitted | | | | |
| C, 91 bits, 16 bits | 847 (fit; synthesis 807) | 1 | about 76 MHz | 5.4 | 12.8 | 4.6 | 4.1 |

- **The synthesis estimate is close to the fit** where both exist (807
  against 847 ALMs, C at 16 bits; 1048 against 1003, A).
- **The loops.** A's feedback (exponent compare, 55-bit alignment, add)
  takes 15.7 ns, as estimated in section 4.3 from the denormaliser and adder
  of `F_ADD` (11.5 + 3.0 ns, 2026-10-02). The integer add of C at 91 bits
  takes 5.4 ns; B's 64-bit add is deduced to be no longer (not fitted).
  `sapho_all` runs at 32.4 MHz on this board (item 29), a period of about
  31 ns: **every loop fits in the processor's clock**, A's with half of it
  to spare.
- **The rounding at the end** is the longest path of each block (12.8 to
  20.3 ns; for C at 567 bits, with a 567-bit leading-zero count, not fitted
  and expected longer). It runs once per vector, so it can take two clocks,
  or be cut by a register, at no cost in rate.
- **Area.** A and B cost about 1000 ALMs at 32 bits; C at 32 bits about
  4650, more than the whole `sapho_all` (3759 ALMs, 2026-10-02); C
  at 16 bits about 850. Every variant uses one DSP block for the 24 x 24
  product.
- Not run, judged not needed for the choice: B at 96 bits, the fits of B64
  and C567, and every Vivado fit (they follow, for the variant chosen). A
  fit of this small block takes about 30 minutes, most of it preparing the
  placement, whatever clock is asked (5 and 10 ns tried).

### 7.3 Step 3, the memory (2026-10-24)

What matters is that the tools build
a 32-bit true dual-port memory out of RAM blocks; the block count does not
change the decision (Luciano). `Scripts/hw/mac_study/mem.sh` (`mem.v`),
256 words: Quartus 24.1 on the DE10-Nano takes it as `altsyncram` (2 M10K,
against 1 for today's simple dual port), Vivado 2025.2 on the Z-7010 as one
block RAM tile (half a tile for simple dual port). The template matters:
written as each port reading the old word on a write (`q <= mem[a]` beside
the write), Quartus refused it (276009, unsupported read-during-write) and
built the array from 8256 registers, about 10 000 ALMs, in a fit that did
not end in 50 minutes. Written as each port returning the word it writes
(Intel's template, `mem_tdp` in `mem.v`), it is a RAM. The prototype of
step 4 uses that form, or checks the behaviour it needs on a write.

### 7.4 Step 4a, the array memory in the processor (2026-10-24)

A prototype in a worktree (tag `archive/array-unit-proto`; the
tests below are its `Scripts/hw/array_proto/` `arrall.sh`, `cppall.sh` and
`fit4a.sh`): asmcomp with `YANC_ARRMEM` set numbers the two memories as decision 10
says and writes `<name>_arr.mif`; `SAPHO/` gets the parameter `ARRMEM`,
the array memory (read on one port, written on the other, as the data
memory) and in `core.v` the choice by S or by `LDI`/`STI`, registered,
driving a mux before ALU input in1; the data memory loses the indirect
adder. cmmcomp and cppcomp unchanged, so the `.asm` is the golden one.
With the option on, the simulated outputs against `golden_sim`:

- C+-: 70 of the 71 fixtures that have a `golden_sim` and a testbench of
  their own name give today's output (`ResetCheck` has its own testbench,
  not run). `cmm_fft_rd`/`cmm_fft_wr` first failed to elaborate: their
  array memory is exactly 8 words, as wide as the FFT, and `rel_addr`
  built `offset[MDATAW-1:FFTSIZ]`, an empty range; fixed in the prototype.
  The data memory, holding the scalars too, is always wider, so today's
  code never meets it.
- C++: 81 of the 89 tests simulated by iverilog give `golden.txt`; the 6
  run by Verilator were not run. The other 8 stop in asmcomp, every one on
  `LEA` of a scalar (`main_p`, `main_x`, `main_i`, `g_total`,
  `sum_pointer_i`, `bit_cast_Tif_v`, static members `Widget__count`,
  `Counter__total`): a pointer is an address of the array memory, so a
  scalar whose address is taken has to live there. That is decision 2,
  now with its size: 8 of 95 C++ tests, no C+- fixture.
- Off, the same fixtures give today's output; the full regression with
  the option off has not been run on the prototype.
- The single access (decision 3) keeps today's timing by construction
  (the choice of memory is registered beside the read, no clock added):
  deduced, the clock counts of the fixtures were not compared.
- With the option on, the arrays leave the waveform: their mirror in the
  generated `<name>.v` reads the data `.mif` at the array's address
  (decision 11).

The clock (2026-10-24), `sapho_all` through `Scripts/hw/fmax.sh` on the
DE10-Nano (5CSEBA6U23I7, slow 1100 mV 100 C, 10 ns asked, seed 1), the
same program and the prototype's `SAPHO/`, the option off and on:

| `sapho_all` | Fmax | ALMs | RAM blocks | registers |
|---|---|---|---|---|
| option off (today) | 32.11 MHz | 3545 | 6 | 1193 |
| array memory on | 32.58 MHz | 3588 | 7 | 1185 |

- **No clock lost.** +1.5 %, inside the spread of one fit against
  another (several per cent): the same clock. Off, 32.11 MHz agrees with
  the 32.4 to 32.6 MHz of item 29 (2026-10-15).
- **The new mux is now on the worst path, and cheap.** Off, the worst path
  starts at the accumulator and enters the ALU at in2 (30.9 ns). On, it
  starts at the array memory, through the mux at in1 (`uic1`, 1.2 ns, one
  level of logic), the float adder's alignment (7.3), add (3.6) and
  normaliser (11.5 ns, 41 %), to the accumulator: 28.1 ns. The normaliser
  is still what sets the clock.
- **Area:** 43 ALMs and the array memory's RAM block.

### 7.5 Step 4b, the inner product (2026-10-25)

The same prototype with `DOT` (decision 4): opcode 108, decoded as `LOD`
(it reads N), waiting in place by `pc_self` as `TOM`/`CAD` do, and in
`core.v` a controller (`arr_dot`) that reads a[k] on the array memory's
port A and b[k] on port B, one pair per clock, into MAC A (W = 2M+8): first
`mac.v`'s three stages copied, then the same arithmetic in one stage
(`mac_a1`, below), checked through the processor, not by `check.sh`. With
`DOT` the array memory becomes true dual port, in Intel's template (step
3).

The tests are `dottest.sh` (with `dot_gen.py`), `dotclk.sh` and `fit4b.sh`
in `Scripts/hw/array_proto/` in the tag `archive/array-unit-proto`.

**Correctness:** a hand-written `.asm` (the compilers do not emit `DOT`)
with random floats of magnitude 2^-2 to 2^2, words expected from the model
of step 1 (`mac.py`, the lazy accumulator): the whole vectors, pointers
into the middle, N from a variable, N = 0 and N = 1, the stack balanced
after `DOT`, the result stored by a `SET` right after `DOT`, two `DOT` in a
row summed by `SF_ADD`. Every word equal, at `#FROUND` 0 and 2, three
seeds, in each version below.

**The clock,** `sapho_all` with a `DOT` added before `#TOAQUI`, as in
step 4a (`fmax.sh`, DE10-Nano, slow corner, seed 1). Clocks are those the
`DOT` stays decoded, for N >= 1 (N + 8 read from the RTL, the others
counted in simulation for N = 0, 1, 2, 3, 5 and 40); the whole sequence
takes two more (`LEA`, `P_LEA`), and the next instruction follows with no
clock lost.

| version | `DOT` clocks | Fmax | ALMs | RAM blocks | DSP |
|---|---|---|---|---|---|
| no `DOT` (step 4a) | | 32.58 MHz | 3588 | 7 | 3 |
| MAC in 3 stages, the rounding at the end combinational into ALU input in1 | N + 8 | 20.9 MHz | 4707 | 8 | 4 |
| the same, the rounded result in a register | N + 8 | 31.85 MHz | 4464 | 8 | 4 |
| MAC in 1 stage, a new controller | N + 3 | 31.34 MHz | 4463 | 8 | 4 |
| `DOT` and `DOB`, the result in place of the ALU's output (dropped) | N + 2, `DOB` N + 1 | 30.53 MHz | 4571 | 8 | 4 |
| `DOT` and `DOB`, the result through ALU input in1 | N + 3, `DOB` N + 2 | 29.81 MHz | 4484 | 8 | 4 |

- **The first version lost a third of the clock,** a defect of the
  prototype, not of the design: the rounding at the end (20.3 ns, step 2)
  went into ALU input in1, so the worst path ran rounding, mux, float adder,
  accumulator: 47.5 ns. Step 2 had said the rounding needs a clock of its
  own; with a register on the result the worst path is again the float
  adder's (memory, in1, alignment, add, normaliser), with no part of the
  MAC in it.
- **The MAC fits in one stage.** The product, its alignment and the add
  into the sum in the clock the pair leaves the memory, estimated at about
  26 ns from the step 2 fits (3.1 + 5.1 + 15.7 ns and the memory's
  output): no MAC path among the 3000 worst of the fit. The new controller
  reads pair 0 in the clock after `DOT` is decoded, from the stack top and
  the acc (both registers by then), while N comes out of the ALU; the
  three clocks of state the first version spent (taking the pointers,
  taking N, a drain longer than the MAC) are gone, and the two stages
  merged into the loop take two more: N + 8 to N + 3.
- **Below step 4a with no part of the MAC on the worst path:** 31.85 and
  31.34 MHz are 2.2 and 3.8 % under 32.58, their worst path the same
  float adder's: placement or chance, see the timing reports below.
- **`DOT` costs** (the MAC, the controller, the second port) about 875
  ALMs, 1 RAM block and 1 DSP block (the MAC's product, as in step 2;
  `sapho_all` has 3 already): `sapho_all` with `DOT` uses 24 % more ALMs
  than without.

**What is left, by clock** (`LEA a; P_LEA b; DOT N`, N + 5 in the 1-stage
version): the two instructions that bring a and b (each instruction
carries one operand); one clock for N to come from memory while b is
still leaving the ALU; N reads, one pair per clock on the two ports; one
clock for the memory to give the last pair, summed; one to round into a
register. The rounded word then passes the ALU into the acc in the next
clock, as a word read by `LOD` does, while the next instruction is
decoded. The last two rows try two cuts:

- **`DOB b`** (`LEA a; P_LOD N; DOB b`, opcode 109): b is written in the
  instruction, so in the clock `DOB` is decoded a is the stack top, b the
  operand and N leaves the ALU (`P_LOD N`): pair 0 is read at once. b is an
  address fixed when compiling (in the prototype an array's name; `h + 3`
  not yet). Fits an FIR, where the coefficients stay and the samples
  slide; when both pointers are computed, `DOT` stays.
- **The result in place of the ALU's output** (`ula_out`) in the clock
  `DOT` executes, rounded there (about 22 ns with the mux), instead of
  entering ALU input in1: the acc, the stack and the memories get it, and
  the rounding no longer takes a clock of the wait. It
  adds an input to a mux at the end of the worst path: the fit says what
  it costs. `ula_fast` does not see it, so asmcomp puts a `NOP` between a
  `DOT` and a `JIZ`/`LDI`/`ILI` right after it, as after a float
  operation. With both cuts the sequence takes N + 3: the two
  instructions, N reads, the memory's clock and the rounding. **Dropped,
  for the structure** (Luciano, 2026-10-25), after the seed sweep below
  found no difference in the clock. Kept in the prototype's history
  (65f7b96, 665ef7f).

**One fit per version could not tell these apart.** The first fit with
idea 2 (30.53 MHz against 31.34) looked like a clock lost on every
instruction, and idea 2 was dropped on it; the next fit, the result back
through in1, gave 29.81 MHz, below the version dropped. The timing reports
of the four fits with the array memory:

- **The worst path is the same logic in every one** (a memory, ALU input
  in1, the float adder's alignment, add and normaliser, the ALU's last
  mux, the acc), with no part of the MAC, of `DOB` or of the dropped mux
  in it. The dropped mux added no level of logic: Quartus folded it into
  the two cells of the ALU's last mux (`ula_out~50`, `~51`, 2.9 ns),
  which take 2.4 to 4.0 ns in the other fits.
- **A path no version touched got slower by itself.** The worst path
  from the data memory (the scalars, the same in every version), through
  the same float adder: slack -20.1 ns in step 4a, -21.9 with `DOT`,
  -22.6 with idea 2, -23.4 without it. The same logic, 3.3 ns apart:
  placement and routing of a design 25 % larger, or chance; one fit per
  version cannot tell them apart.
- **The true dual-port array memory is slower at its output** than the
  simple dual-port one (1.0 ns against 0.2, and 0.5 to 0.9 ns less of
  clock skew in its favour), but within one fit its paths are only 0.2
  ns worse than those from the data memory.

**The seed sweep** (`Scripts/hw/array_proto/sweep.sh`, 2026-10-25): the
three versions below fitted again with seeds 2 and 3, beside seed 1 above
(Fmax in MHz; ALMs of the whole design, the mean of seeds 2 and 3):

| version | seed 1 | seed 2 | seed 3 | mean | ALMs |
|---|---|---|---|---|---|
| array memory, no `DOT` | 32.58 | 30.97 | 31.16 | 31.57 MHz | 3555 |
| `DOT` and `DOB`, the result through in1 | 29.81 | 30.83 | 30.86 | 30.50 MHz | 4478 |
| the same, the result in place of `ula_out` (idea 2) | 30.53 | 30.02 | 30.71 | 30.42 MHz | 4575 |

- **The clock of `DOT` is below what three seeds resolve.** The means are
  1.07 MHz (3.4 %) apart, but seed by seed `DOT` costs 2.77, 0.14 and
  0.30 MHz: under 1 % in two seeds of three, the mean set by seed 1.
- **Idea 2 ties.** 0.08 MHz apart in the means, and the order turns over
  from seed to seed (+0.72, -0.81, -0.15 MHz).
- **The area is measured,** each version within 10 ALMs from seed to
  seed: `DOT` and `DOB` about 920 ALMs, 1 RAM block and 1 DSP block; idea
  2 about 95 ALMs more, in the mux at `ula_out` (`core`'s own logic 13 to
  60 ALMs) and in the rounding (`mac_round` 375 to 447: **deduced**,
  timing-driven synthesis rebuilds the rounding once it ends at the acc
  instead of a register).

**Why idea 2 was dropped** (Luciano, 2026-10-25), with the clock a tie and
one clock a `DOT` in its favour:

- **The result through in1 is how every outside word enters the ALU**:
  the memories through `mem_rd_sel` at in1, the input port and `cade`
  through the register `ior` at in2. Idea 2 would be the first block to
  write after the ALU.
- **Every later operation of the unit returns its result the same way**
  (section 6, decision 14): into the unit's result register, read by the
  core as a word from memory. Under idea 2 each would add an input to the
  mux at the end of the worst path and an entry to the `NOP` list.
- **The register isolates the timing.** The MAC, the rounding and the
  controller can change without reaching the core's worst path; under idea
  2 the rounding (about 22 ns, deduced from step 2) ends at the acc and
  would cap the clock near 45 MHz once the float adder's path gets faster.
- **No exception for anyone:** `ula_fast` sees the result, so `JIZ`,
  `LDI` and `ILI` may follow `DOT` with no `NOP`. Idea 2's `NOP` would
  not have been a new rule (asmcomp's `asm_normnop` already puts one after
  each float operation; `DOT` and `DOB` would join `asm_is_norm`), but the
  three points above stand.

**Not tried** (they would change the design): the first read in the clock
`DOT` is decoded, taking b from the ALU's output (that path into the
memory's address); the rounding inside the loop (36 ns, more than the
period); a `DOT` that does not wait, its result read later; two pairs
per clock (four ports, or the banks left out in section 5).

## 8. For the documentation (issue #40)

- The parameter and what it changes, in the directives and in the
  instruction reference (generated from `isa.tsv`).
- The array memory in "Simulation" (two memory images; the variables still
  visible by name).
- The Dirac chapter of the library rewritten for the new forms, with the
  exactness of the sums and the rounding at the end.

## 9. Where it stopped, and how to resume (2026-10-26)

Parked by Luciano on 2026-10-26, with `main` back to its own line of
releases. The code is in tags:

- `archive/array-unit-design`: this note's history (PR #19, closed
  unmerged) and the MAC study, whose scripts are also in `main`
  (`Scripts/hw/mac_study/`).
- `archive/array-unit-proto`: the prototype of steps 4a and 4b (`SAPHO/`
  with `ARRMEM`, `DOT` and `DOB`, asmcomp with the S bit, and
  `Scripts/hw/array_proto/` with the tests, the fits and the seed sweep).
  `git switch -c <branch> archive/array-unit-proto` gives it back as a
  branch; `main` has moved since, so it is merged with `main` first.

Where it stopped: section 6, two questions asked and not answered,
recorded under decision 14: the form of the operands (decision 4: A, the
form of `DOT` for every operation, recommended; B, registers as in
burst.md; C, both) and, for the scale and the add, in place or a separate
destination (in place recommended). The other open decisions: 1 to 3, 6,
7, 9 and 11.

Since then (2026-10-26) the Dirac loops with pointers and `RPT`
([dirac-expressions.md](dirac-expressions.md), section 6) were measured on
`proc_rls` whole: the same bits, faster than today's unrolled code from N
= 16, for about 100 ALMs. Its section 6.8 compares the two designs; the
gain of this unit is to be weighed against those loops, not against loops
in software (deduced: 2 to 3 times on the N² parts, not tenfold).
