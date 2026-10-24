# Array unit: a memory for arrays, a controller and a MAC

Status: design, started 2026-10-24. Nothing implemented. **Rule (Luciano,
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
Dirac notation is then redone on it. The burst of item 28
([burst.md](burst.md)) is part of the same unit: the inner product needs
the same size, pointer and wait in place as a burst to a port.

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
  `F_MLT` already skips the leading-zero count (TODO item 8, step 2).

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
(3759 ALMs measured 2026-10-02, TODO item 8). Two ways to shrink it: the
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
different banks; it cannot serve `⟨a|a⟩` without a copy.

## 6. Decisions to take

1. **The parameter**: its name, a directive or an option of `yanc`, its
   default. Off: today's processor, bit for bit.
2. **What goes to the array memory**: every array; every object whose
   address is taken (C++); the C++ heap and call stack.
3. **A single access** (`a[k]` with `k` variable): the same timing as
   today's `LDI`, one clock with no wait. A requirement, to be checked in
   simulation first.
4. **The instruction set of the controller**: registers for the size,
   the pointers and a stride; one instruction per operation (`DOT`,
   `AXPY`, ...) or a small programmable sequencer. Designed once with the
   burst's `BSZ`/`BPT`/`BOT`/`BIN` and the fast fill and copy (item 28).
5. **The MAC variant** (A, B or C), and for B the window.
6. **The int accumulator**: width, overflow (wrap, saturate, flag).
7. **The scalar operand** of `c|a⟩`: from the accumulator into a
   register of the controller.
8. **The memory**: true dual port or two banks; the size of each memory
   (today one `MDATAS`).
9. **Interrupts** during a sequence (the same question as for the burst
   and `TOM`/`CAD`, toma-and-cade.md section 6).
10. **The compilers**: two address spaces, two memory images; `asm_reach`
    and `asm_share` per space, and both must see a sequence's pointers as
    reads (and writes) of the whole array (burst.md 5a, hole 4).
11. **Simulation and AURORA**: the variables of the array memory visible
    by name in the waveform (`pc_sim_val`, `gen_gtkw`, the `YANC_SIM_VIS`
    block AURORA reads).
12. **Hand-written `.asm`**: what `LDI`/`STI` mean with the option on.
13. **The Dirac notation redone**: the forms of issue #18, `comp`, the
    conjugating bra, the outer product the right way round.

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
   the fixtures, synthesized in Quartus and Vivado; blocks used and Fmax;
   against two banks.
4. **A prototype of the whole** in a worktree copy of `SAPHO/`, with
   hand-written `.asm`: the single access with today's timing, an inner
   product in a sequence, the wait in place; then `fmax.sh` and `area.sh`
   on `sapho_all` with and without the option (without it, nothing may
   change).
5. **What it buys**: words of program and clocks of the Dirac fixtures and
   of `proc_rls`, today against each variant.

### 7.1 Step 1, measured (2026-10-24)

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

**Step 2, correctness (2026-10-24):** the four variants in Verilog
(`Scripts/hw/mac_study/mac.v`: A with W = 2M+8, B with 2 and 3 x `NUBITS`,
C) give the word of their model (`mac.py expect`) on every vector of the
study, 750 at 32/23/8 and 588 at 16/10/5 (12 left out, out of range), at
`#FROUND 0` and 2 (`check.sh`, 16 runs, 0 different). A mutation (the
hardware at level 0 against the level-2 model) gives 287 different, the
first by 1 ULP: the check sees a one-unit error.

**Step 2, cost (2026-10-24),** Quartus 24.1 on the DE10-Nano's Cyclone V
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
  4650, more than the whole `sapho_all` (3759 ALMs, item 8, 2026-10-02); C
  at 16 bits about 850. Every variant uses one DSP block for the 24 x 24
  product.
- Not run, judged not needed for the choice: B at 96 bits, the fits of B64
  and C567, and every Vivado fit (they follow, for the variant chosen). A
  fit of this small block takes about 30 minutes, most of it preparing the
  placement, whatever clock is asked (5 and 10 ns tried).

**Integer MAC (decision 6), from the code:** `MLT` keeps the low `NUBITS`
bits of the product (`ula.v` 536: `assign out = in1 * in2` into an
`NUBITS`-bit wire) and the integer add wraps. A sum modulo 2^`NUBITS` does
not depend on the order, so an `NUBITS`-bit accumulator gives exactly
today's result; a wider one matters only to saturate or to flag an
overflow, a choice of meaning, not of accuracy.

## 8. For the documentation (item 21 in TODO.md)

- The parameter and what it changes, in the directives and in the
  instruction reference (generated from `isa.tsv`).
- The array memory in "Simulation" (two memory images; the variables still
  visible by name).
- The Dirac chapter of the library rewritten for the new forms, with the
  exactness of the sums and the rounding at the end.
