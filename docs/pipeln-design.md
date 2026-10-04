# `#PIPELN`: a processor that pauses for its slow operations

Design proposal for TODO item 19, for review before any HDL is written.
Measurements behind it: TODO item 19 (Cyclone V and Zynq-7010, `sapho_all`
variants), `Scripts/hw/opmix/` (operation mix of the regress programs).

## The directive

`#PIPELN 0 | 1 | 2` in C±, `#pragma yanc pipeln 0 | 1 | 2` in C++, passed by
asmcomp to the processor as a parameter `PIPELN`.

- `0` (default when absent): today's processor, bit-identical. Every
  instruction takes one cycle; the clock is set by the slowest operation the
  program uses.
- `1`: the core pauses while a slow operation runs. Every operation keeps a
  fixed number of cycles, so the cycle count of a program is still known at
  compile time; the clock is about the base's (the light operations).
- `2`: automatic. yanc estimates both and picks; it always says what it
  picked and why (below).

## How the core runs today (what the pause must respect)

Two stages overlap. In one cycle the core decodes instruction i+1 (its data
memory address, its stack push/pop, its jump decision) while the ALU executes
instruction i. The ALU result `ula_out` feeds, in that same cycle, the
accumulator, the data memory write of i+1 (`SET`), the data stack push
(`PSH`, `P_*`), the `LDI` address and the `JIZ` decision of i+1 (`if_acc =
|ula_out`), and through the jump decision the PC and the instruction memory
address. That is the path measured: data memory, operand select, ALU, output
mux, jump decision, PC.

## The pause

A small counter in the core. When an operation with latency k > 1 enters the
execute stage (`id_ula_op` is loaded with it), the counter loads k - 1 and
`run` goes low for k - 1 cycles. While `run` is low:

- held: the PC, the instruction memory's read register, `id_ula_op`, the data
  memory's read register (its read enable), `popr`/`stkr`, `req_inr`/`ior`,
  the accumulator, both stack pointers;
- masked (no effect until `run` is back): data memory write, data stack
  push/pop, instruction stack push/pop (`CAL`/`RET`), `pc_load`, the input
  request `en_in` (an external FIFO must not be read k times), the output
  strobe `en_out`, the interrupt `itr` (taken after the pause), the `#TOAQUI`
  pin.

On the last cycle `run` is high and everything happens as today, once. The
operands of the slow operation (data memory output, stack top, accumulator,
input register) stay unchanged for the k cycles.

## Two ways to give the slow operator its k cycles

**A. Multicycle paths (recommended).** The ALU stays combinational, as it is
(and as decided on 2026-09-14). Its inputs are stable for k cycles, so the
operator only has to settle within k clock periods. No register is added to
any operator; the new hardware is the counter and the enables. The synthesis
tools must be told: one `set_multicycle_path -setup k -through` the
operator's instance (and `-hold k-1`), in an `.sdc` (Quartus) and an `.xdc`
(Vivado) that asmcomp writes next to the `.v`, only for the operators the
program instantiates. A user who leaves the file out gets a timing report
that fails on those paths, though the hardware works if the real delay fits
in k periods.

**B. Pipeline registers inside the operators.** Each slow operator gets k - 1
register stages at measured cut points (inside the divider arrays, after the
float denormaliser, between the leading-zero count and the shift / round).
No constraint file; the tools see plain one-cycle paths. Costs registers in
every slow operator and an operator-by-operator design of the cut points, and
it makes the ALU sequential (the 2026-09-14 rule relaxed behind the
directive).

A is a fraction of B's work and of its hardware, and keeps the ALU as it is;
B is the conventional, constraint-free form. Decision: Luciano.

## Latencies

With the clock at about the base's period, an operation's latency is
k = ceil(its path / the base path), from the table measured on the current
families (period over the base's, Cyclone V / Zynq-7010):

| operation | factor | k |
|---|---|---|
| light (`LOD`, `ADD`, logic, ...) | 1 | 1 |
| `F_ADD`, `F_SU*` | 1.59 / 1.40 | 2 |
| `DIV`, `MOD` | 4.47 / 4.78 | 5 |
| `F_DIV` | 5.07 / 4.67 | 5 or 6 (6 for a margin on the Cyclone V) |
| `F_MLT`, conversions, compares, shifts, `NRM`, `SGN`, `F_ROT` | measured overnight 2026-10-03/04 | 1 or 2 |

asmcomp writes the latency table into the generated top as a parameter (one
k per ALU operation code); the counter reads it. A k of 1 for every operation
makes `PIPELN 1` behave as `PIPELN 0`.

## `#PIPELN 2`: the automatic choice

Time = cycles / clock, for both modes.

- **Clock**, estimated, based on the current Altera and Xilinx families
  (Cyclone V, 7-series), and said so in the message: the period of mode 0 is
  the base period times the factor of the slowest operation the program uses;
  the period of mode 1 is the base period. The ratio is the factor itself
  (1.5 for a float add, ~4.6-4.9 for a divider), independent of the chip to
  within the ~10 % the two families differ.
- **Cycles**, exact for a given run: cycles(1) = cycles(0) + sum over
  operations of count x (k - 1). The counts come from a simulation of mode 0;
  the generated testbench can write the histogram (`Scripts/hw/opmix/probe.v`
  does it today). Without a simulation, asmcomp can give the exact cost of
  each loop body and a range for the program.
- **Choice**: mode 1 if cycles(1) x period(1) < cycles(0) x period(0) by a
  margin (say 10 %); within the margin, mode 0 and a message that it is close.

Where it runs: asmcomp builds the top before the simulation exists. Two
options: (i) asmcomp decides from the static estimate (operation set, loop
bodies) and prints it; (ii) the decision is a step after the simulation
(Aurora, `single_proc*` scripts) that rewrites `PIPELN` in the top, which
changes no instruction and no memory image, only a parameter. Decision:
Luciano.

## What changes where

- `SAPHO/core.v`: the counter, `run`, the enables and masks above, under
  `generate` on `PIPELN` (absent at 0).
- `SAPHO/processor.v`: the two memories get a read enable (used only at 1).
- `SAPHO/ula.v`: nothing in option A.
- asmcomp: the directive, the parameter, the latency table, the `.sdc`/`.xdc`
  (option A), the messages of mode 2.
- cmmcomp / cppcomp: pass the directive / pragma through (as `#FROUND`).
- Simulation: cycle counts change at 1; the waveform shows `run`.
- Tests: every regress program at `PIPELN 1` must give the same outputs as at
  0 (a regress pass), plus a directed test with input/output during a pause
  (no double read, no double write) and an interrupt during a pause.
