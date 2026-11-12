# INC: k = k + 1 in one word (#203)

A loop counter ends every turn with `LOD k; ADD 1; SET k`, three words and
three cycles. `INC k` (opcode 109, `P_INC k` 110 pushes the accumulator
first) does it in one: k + 1 in the accumulator, and in k one cycle later.
This note says how the hardware does it without an adder of its own, the rule
the code around it follows, and what was measured (2026-11-12).

## Where the cycles were

Measured with a probe on the address of each executed instruction, on the
five programs of the study of #91 compiled by the main of 2026-11-12:

| program | cycles | `LOD k; ADD 1; SET k` run | saved by INC |
|---|---|---|---|
| ArcTan | 83 015 | 2 000 | 4.8 % |
| Seno | 75 531 | 2 000 | 5.3 % |
| Sqrt | 106 972 | 2 000 | 3.7 % |
| proc_fft | 997 | 19 | 3.8 % |
| proc_rls | 15 983 | 1 440 | 18.0 % |
| all | 282 498 | | 5.3 % |

The same probe, run on the programs with the INCs put in, gave those cycles
exactly, the same outputs, and the same final data memory but for the
constant 1 the ADD no longer reads.

## The hardware (core.v, instr_dec.v)

- **The adder is the ADD's.** INC decodes as an ADD of its data word and the
  accumulator (`ula_op` 2, `.ADD(... | INC | P_INC)`). The accumulator's old
  value is dead, since INC overwrites it, so the cycle INC is decoded `racc`
  loads 1 instead of the ALU's output: a constant on the register, its
  synchronous clear and set, not a mux in front of the ALU. That load is
  generated only with INC: written for every program, on an `id_inc` held at
  0, it still cost Yosys one LUT in sapho_min (20 against cells.txt's 19). `P_INC` pushes
  the old value in decode, from `ula_out`, as every push does.
- **The write is a cycle late.** k + 1 is on `ula_out` the cycle after the
  decode, when the operand in decode is already the next instruction's. So
  the operand is kept a cycle (`inc_a`, MDATAW bits, and `inc_w`), and that
  cycle it takes the write address; the data is `ula_out`, as for SET. That
  register and the address mux are all INC adds, and only in a program that
  uses it.
- **The instruction right after INC must not write a data word** (the memory
  has one write port) **nor read k** (its read leaves on the edge that writes
  k, and would see the old word). asmcomp refuses a program that breaks it
  (MSG_ERR_INC_WRITE, MSG_ERR_INC_READ; NegTests/inc_write.asm and
  inc_read.asm), so the port needs no arbiter and the read no bypass.
- **A `#PRACA` pulse while INC is in decode** sends the fetch to the restart
  point, whose instruction becomes the one after INC and is held to nothing:
  the pulse drops the INC's write, as it drops the push and pop of the
  instruction in decode (#188). One gate, only with `#ITRAD`.

## The compilers (asm_inc in Compilers/common/asm_share.c)

Both compilers run `asm_inc` after `asm_jnz`: three lines in a row, no label
on the ADD or the SET, k a name, and the next instruction neither writing a
data word nor naming k. A `LOD k` there would only reload what the
accumulator holds, and could go, but neither compiler writes one: cmmcomp
uses the accumulator it knows, and the only `LOD k` right after an increment
in the C± and C++ tests has a label (a jump reaches it too), so the pass does
not try. The instruction that runs right after an INC never becomes an INC
itself (`a++; b++` gives one INC and
one sequence of three). On 2026-11-12: 54 of the 125 C± goldens changed, by
three lines to one per INC and nothing else; the 116 C++ tests have 386 INC
in 56 programs; asmcomp accepts every one.

## The tests it moved

- 54 `golden.asm` (three lines to one per INC) and 67 `golden_map.txt` (the
  same, and 13 where only asmcomp's share of the instruction set moves, 106
  to 108 opcodes); `size_baseline.txt`, 54 smaller.
- `Scripts/wave_map.py --vcd` held a variable's change to a SET of it; INC's
  change shows while the word after it runs, at the address after the INC,
  and is taken there.
- dirac_assign is an endless loop that writes 1 each turn: in the same
  simulated time it now writes 263 lines instead of 209, the first 209 the
  same; its `golden_sim/output_0.txt` follows.
- `P_INC` is reached only in a Dirac loop (the acc keeps the sum while the
  counter steps), and sapho_all has none: it is in sapho_all's
  `blocks_except.txt`. proc_rls, dirac_assign, dirac_shapes and cmm_cov_misc
  run it.

## Measured cost

| | without INC | with INC |
|---|---|---|
| proc_rls, Yosys LUT4 (mean of three mappings) / FF | 4 308 / 427 | 4 342 / 434 |
| ArcTan, Yosys LUT4 / FF | 4 283 / 324 | 4 305 / 337 |
| proc_rls, Cyclone V (5CSEMA5F31C6) | 47.66 MHz, 1 330 ALMs | 47.53 MHz, 1 334 ALMs |
| proc_rls, Cyclone IV (EP4CE115F29C7) | 32.86 MHz, 2 589 LEs | 32.79 MHz, 2 599 LEs |

One fit per row and seed 1, so the Fmax differences (−0.3 % and −0.2 %) are
within what one fit varies. The worst path of all four is the float divider
(`ula_fdiv`); neither `inc_a` nor the address mux is among the ten worst.
The flip-flops are the MDATAW bits of `inc_a` plus `inc_w` (6 + 1 in
proc_rls, 12 + 1 in ArcTan).

## The name

`INC`, not `INC_M`: every `_M` of the instruction set reads the data word and
writes only the accumulator (`NEG_M x` leaves x as it was), and INC also
writes the word. It is listed as irregular in `Scripts/check_isa.py`, with
the operand effect `rw` the table gained for it.

## Open

The same write a cycle late, at a kept address, would serve other forms:
`a[i]++` (#204), a delay line (#205), `k = k - 1` (#206), a float `x += 1.0`
(#207), and the C± operators that would use them (#208). The forms looked at
and left out (a unary or binary in place, one opcode each) are in a comment
of #203.

## For the documentation

- docs/reference/instructions.md (from isa.tsv): INC and P_INC, and the rule
  about the instruction after them.
- docs/messages.md: the compilers' "N increments made one INC each".
