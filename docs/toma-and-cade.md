# `toma` and `cade`: two SAPHOs passing a value

Design record of TODO item 20 (2026-10-07, target v7.0). Worked out with
Luciano in conversation; nothing below is implemented yet. Sections 1-4 are
the design as agreed, section 5 what was checked in the code and how, section
6 what is still open, section 7 the alternatives that were weighed and why
they lost.

## 1. What the user writes

One SAPHO hands a value to another; each side waits for the other, nothing
is lost and nothing is read twice:

```c
// writer                         // reader
toma(x);                          y = cade();
```

`toma(x)` does not return until the partner has taken `x`; `cade()` does not
return until the partner has offered a value. Whoever arrives first waits
(a synchronous channel, the rendezvous of Go's unbuffered channels and of
Occam). There is no port number and no status port: each processor has **one**
such link, to one partner, usable in both directions. The ordinary I/O ports
(`in`, `out`, `req_in`, `out_en`) are untouched and never wait.

C+- and C++ both get the two builtins. For now `cade()` is accepted only as
the whole right-hand side of an assignment (`y = cade();`), in both
compilers; inside an expression (`z + cade()`) it is an error until a
push form is designed (section 6).

## 2. The protocol: one bit per side

Each side owns one bit that only it flips, and reads the partner's:

- the writer owns **T** ("toma"): it flips T when it offers a value;
- the reader owns **C** ("cade"): it flips C when it takes the value;
- a value is waiting while **T != C**; it has been taken when they are
  equal again.

The writer also needs a flag **W** ("waiting") so that T flips once per
`toma`, not on every turn of its waiting loop.

| | writer, `TOM` | reader, `CAD` |
|---|---|---|
| goes on when | W is set and the partner's C == T | the partner's T != C |
| otherwise | if W is clear: flip T, set W; jump to itself | jump to itself |
| when it goes on | clear W | take the input, flip C |

The two orders:

- **`toma` first.** The writer flips T and loops. The reader's `CAD` sees
  T != C, takes the value, flips C and goes on. The writer's next turn sees
  C == T and goes on.
- **`cade` first.** The reader sees T == C and loops. The writer arrives,
  flips T and loops. The reader's next turn sees T != C, takes the value,
  flips C and goes on. The writer then sees C == T and goes on.

In both orders the writer leaves only after the value was taken, so **no
buffer is needed**: the value stays on the writer's output bus while it
loops (section 5). The bits are levels held in flip-flops, never one-cycle
pulses: two processors spinning in loops see each other's instruction only
one cycle in every turn, and with equal loop lengths out of phase they
would never meet.

Reset clears T, C and W on both sides.

## 3. Hardware

**Two instructions, `TOM` and `CAD`**, each a conditional jump to its own
address, decided where `JMP`/`JIZ` are decided (the prefetch, section 5).
The operand is the instruction's own address, written as a label, as in
`@fim JMP fim` (`@L TOM L`); neither needs a data operand. Neither has a
row in `instr_dec.v` (like `JMP`/`JIZ`): an opcode without one decodes to
"pass the accumulator through the ALU", which is what a looping `TOM` needs.
`CAD` loads the accumulator through `INN`'s path (`ula_in2_ctrl`) with
`cade_dado` in place of `io_in`, raising no `req_in` outside.

**Opcodes** (agreed 2026-10-07). Flow control is kept below opcode 32
(`check_isa.py`), and 0-31 are all taken: `TOM` = 20 and `CAD` = 21, right
after `RET`, every opcode from `ADD` up moving by 2 (the highest becomes
107, still 7 bits). An encoding break, in line with v7.0.

**Generated only when used** (pay for what you use): a program with no
`TOM` gets no T, no W and no `TOM` pins; with no `CAD`, no C and no `CAD`
pins.

**Pins of the generated processor:**

| pin | direction | width | with | what |
|---|---|---|---|---|
| `toma` | out | 1 | `TOM` | my T |
| `valeu` | in | 1 | `TOM` | the partner's C (has it been taken?) |
| `cade` | out | 1 | `CAD` | my C |
| `taqui` | in | 1 | `CAD` | the partner's T (is a value offered?) |
| `cade_dado` | in | NUBITS | `CAD` | the partner's `out` bus |

**Wiring two processors** (A sends to B; the other direction is the same
with A and B swapped), wires only, no user logic:

```
A.toma -> B.taqui      B.cade -> A.valeu      A.out -> B.cade_dado
```

`cade_dado` is a pin of its own: inside the reader a mux picks `in` or
`cade_dado` by the instruction (`INN` or `CAD`), hidden from the outside.
Sharing the `in` pin would need a mux in the user's top, which cannot know
when to switch (`CAD` raises no `req_in`).

The only signals crossing between the processors are T and C, each straight
out of a flip-flop and into the other's jump decision: no combinational path
runs from one processor into the other. No Fmax loss is expected from the
link (deduced from the structure; to be measured, section 6).

## 4. Compilers and assembler

- `toma(e)` -> the value of `e` in the accumulator, then `@L TOM L`.
- `y = cade();` -> `@L CAD L`, then `SET y`.
- asmcomp and appcomp: the two mnemonics (operand class `code`, the own
  label); `isa.tsv`, `asm_share.c`, `core.v`'s flow localparams and
  `check_isa.py` in step. appcomp's lexer (`app.l`) is a sixth copy of the
  mnemonic list: it counts the instructions that size the instruction memory,
  and without `TOM`/`CAD` there `MINSTS` came out one short per use (the
  last instruction fell off the memory). `check_isa.py` now holds `app.l`
  to `ASMComp.l` too.
- hdl.c: the pins above in the generated top, only with the instruction
  present; the standalone testbench ties the inputs to 0 (a lone `TOM` or
  `CAD` waits forever, as on a board).
- cmmcomp (diff shown to Luciano first): `toma` and `cade` are reserved
  words. `toma(e);` is a statement (`STMT_TOMA`, `exec_toma`); `cade()` is a
  stdlib expression (`OP_STD_CADE`) that only `STMT_ASSIGN` accepts, when it
  is the whole right-hand side (`exec_cade`): the word lands in the variable
  with the variable's type, no int/float conversion. Anywhere else the walker
  stops with "cade() may only stand alone"; a comp destination or argument is
  rejected (one word). No new grammar conflict.
- cppcomp: the same two builtins. `cade()` is allowed only where a flag set
  by the scalar-variable store and the local initializers says so
  (`x = cade();`, `T x = cade();`); `gen_expr_to` consumes the flag at once,
  so `x = f(cade())` is rejected too. Any one-word type goes through as it
  is; a wider one is rejected.
- C++ inline assembly may write `TOM`/`CAD` directly.
- Tests: `Compilers/CMMComp/Tests/link_*` (the four programs) and the
  `LINK` project pass in `Scripts/regress.sh` (two pairs of processors,
  checked against the sim golden and against the sequences n*7+3 and n+1
  computed by the script); `NegTests/cade_*` for the rejected forms. `TOM`
  and `CAD` are in `sapho_all`'s `blocks_except.txt` (alone they would
  wait forever). The C++ side was checked by hand (the same two scenarios
  and the rejected forms); there is no C++ two-processor pass in the regress.

## 5. Checked in the code, then in simulation (2026-10-07)

Read in `SAPHO/core.v` first, then confirmed by three simulations of two
processors from hand-written assembly (Icarus, outside the regress):

| test | result |
|---|---|
| writer offers `i*7+3`, i = 0..15, with a waiting loop that grows with i; reader's shrinks (each side first in turn) | 16 values, in order, none wrong; 1709 cycles, `TOM` repeated 577 cycles, `CAD` 387 |
| no waiting loops on either side | 16 values right, 260 cycles (~16 a value) |
| one link both ways: ping offers i and takes back i+1, pong takes, adds 1, offers back | 16 answers right |

What the reading said:

1. **The writer's output bus holds the value while it loops.** `out` is the
   ALU output (`mem_data_wr = ula_out`), not the accumulator, which is why it
   changes on every instruction. Instructions that leave the accumulator alone
   (`OUT`, `JMP`, `NOP`) pass it through the ALU, so a repeating `TOM`
   decoded the same way keeps `x` on the bus.
2. **When things happen.** A jump is decided in the cycle the instruction
   leaves instruction memory (`prefetch`); in that cycle the ALU executes the
   instruction before it, which is why `JIZ` tests `ula_out` and not the
   accumulator (`if_acc = |ula_out`). For `LOD x; TOM`: in the cycle `TOM` is
   decoded the ALU executes `LOD x`, so `x` is already on the bus; T flips at
   the end of that cycle and `x` stays. `CAD` samples its input at the end of
   the cycle it goes on (`ior <= io_in`, as for `INN`), the cycle C flips.
3. **Nothing after a looping instruction runs.** The next fetch address is
   computed in the same cycle as the decision; while `TOM`/`CAD` loops the
   address fetched is its own, as with `@fim JMP fim`.

## 6. Open

- **The interrupt** (left for later, Luciano 2026-10-07). Today's interrupt
  restarts at `ITRADD` without clearing T, C or W; a restart in the middle
  of a `toma` leaves T flipped with nobody holding the value.
- **`cade()` inside an expression**: needs a push form (as `P_INN`); later.
- **Deadlock**: both sides in `toma` (or both in `cade`) wait forever, as in
  Go; the program's responsibility. A simulation could flag it.
- **Two clocks**: out of scope (same clock assumed). Different clocks need a
  synchronizer on T and C.
- **Fmax**: one fit of a two-processor design on each board.

Test plan: two SAPHOs in Icarus; the writer sends 0, 1, 2, ... N; the reader
checks the sequence. Writer faster, reader faster, alternating speeds
(waiting loops of different lengths); both directions on one link. Passes
if every value arrives once and in order. Then a fixture in the regress
(a project pass, as DTW) and a small example for the boards.

## 7. Weighed and dropped

- **A FIFO between the two with a status port read by `in()`** (the first
  proposal): works without touching the core, but every transfer needs a
  waiting loop in the program and two ports per side. Dropped for
  transparency. `SAPHO/myFIFO.v` would not have served as is: it writes
  `data` one cycle after `wrreq`, when a SAPHO's `out` bus has already
  changed (by reading; not simulated).
- **Freezing the core while waiting** (a `wait` pin): as transparent, but it
  is the pause logic dropped from `#PIPELN` (docs/pipeln-and-division.md
  section 2) and puts a combinational path from one processor's decoder
  into the other's stall.
- **One-cycle pulses instead of the two bits**: two looping processors can
  miss each other forever (section 2).
