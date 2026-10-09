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

C+- and C++ both get `toma(x)`, `cade()` (the word as an int) and `fcade()`
(the word as a float), as `in()`/`fin()`. As the whole right-hand side of an
assignment (`y = cade();`) the word goes into `y` as it is, with `y`'s type;
inside an expression, an index or an argument (`z + cade()`, `out(0,
fcade())`) it is an int or a float operand. `CAD` has no push form: when the
accumulator is in use, the compilers write `PSH` before it (one word, one
cycle; the waiting loop jumps back to `CAD`, not to the `PSH`). Decided
with Luciano 2026-10-08, rather than a `P_CAD` opcode, which would need
another renumbering below 32.

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
| `cade_dado` | in | NUBITS | `CAD` | the partner's offered value |
| `toma_dado` | out | NUBITS | `TOM` | my offered value (`LINKAS` 0: equal to `out`; 1: a register, section 8) |

**Wiring two processors** (A sends to B; the other direction is the same
with A and B swapped), wires only, no user logic:

```
A.toma -> B.taqui      B.cade -> A.valeu      A.toma_dado -> B.cade_dado
```

On one clock `A.out -> B.cade_dado` is the same word (the wiring used
before `toma_dado` existed). On two clocks see section 8: `toma_dado` and
`#(.LINKAS(1))` on both.

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
  present. The standalone testbench plays the partner, so each side can be
  simulated alone, as `in()`/`out()` already are with `input_N.txt` /
  `output_N.txt`: `cade()` takes the words of `Simulation/cade.txt` one by one
  (when the file ends it says so and the next `cade()` waits forever), and
  every `toma()` value is written to `Simulation/toma.txt`. It keeps the same
  protocol (it owns T for `cade`, C for `toma`).
- asmcomp checks that a `TOM`/`CAD` operand is a label of its own line
  (`@L TOM L`); any other target is rejected ("waits by jumping to itself").
- cmmcomp (diff shown to Luciano first): `toma` and `cade` are reserved
  words, and `fcade`. `toma(e);` is a statement (`STMT_TOMA`, `exec_toma`);
  `cade()`/`fcade()` are stdlib expressions (`OP_STD_CADE`/`OP_STD_FCADE`).
  As the whole right-hand side of an assignment `STMT_ASSIGN` takes them
  (`exec_cade`: the word lands in the variable with its type); anywhere else
  `exec_cade_expr` gives an int / float operand, with `PSH` first when acc is
  live (the same choice as `INN` / `P_INN`). A comp destination or a comp
  `toma` is rejected (one word). No new grammar conflict.
- cppcomp: the same builtins. The scalar store and the local initializers set
  a flag that `gen_expr_to` consumes at once: there `x = cade();` puts the
  word in as it is; anywhere else `cade()`/`fcade()` is an ordinary int/float
  call (`@L CAD L`), and the expression code pushes a live acc by itself.
  `toma` of a wider value is rejected.
- C++ inline assembly may write `TOM`/`CAD` directly.
- Tests: `Compilers/CMMComp/Tests/link_*` (the four programs) and the
  `LINK` project pass in `Scripts/regress.sh` (two pairs of processors,
  checked against the sim golden and against the sequences n*7+3 and n+1
  computed by the script); `NegTests/cade_comp` for a comp destination.
  `link_pong` writes `toma(um * um + cade())` (the `PSH` path) and
  `link_cpp_ping` `out(0, cade())`; `fcade()` was checked by hand with real
  floats across the compilers (C+- hands n*1.5, C++ takes `0.5f + fcade()`,
  16 of 16 right). `TOM`
  and `CAD` are in `sapho_all`'s `blocks_except.txt` (alone they would
  wait forever). The same pass also runs both scenarios across the
  compilers: the C+- `link_envia` hands to the C++ `link_cpp_recebe`, the
  C++ `link_cpp_ping` talks to the C+- `link_pong` (sources in
  `Tests/LINK/cpp/`, built inside the pass with the CPP phase's commands).
  The C++ rejected forms were checked by hand.

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
- **Deadlock**: both sides in `toma` (or both in `cade`) wait forever, as in
  Go; the program's responsibility. A simulation could flag it.
- **Two clocks**: implemented in simulation (`LINKAS`, section 8); the
  board check and the Quartus constraints are open.
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
  changed (by reading at the time; confirmed in simulation on 2026-10-09,
  with two more defects: docs/myfifo.md).
- **Freezing the core while waiting** (a `wait` pin): as transparent, but it
  is the pause logic dropped from `#PIPELN` (docs/pipeln-and-division.md
  section 2) and puts a combinational path from one processor's decoder
  into the other's stall.
- **One-cycle pulses instead of the two bits**: two looping processors can
  miss each other forever (section 2).

## 8. Two clocks (`LINKAS`, implemented 2026-10-09, not released)

Asked by Luciano on 2026-10-09: can the link join two processors that run
on different clocks? Yes, and the protocol (section 2) does not change.
Proposed and approved the same day ("segue com suas sugestoes"): the data
register (8.2, a) and a module parameter (8.3, option 1). What is a fact of
the code, what was measured and what is deduced is marked.

### 8.1 Why the protocol already fits

- **Fact (code):** T and C are levels in flip-flops. Each flips once per
  value and then holds; there is no one-cycle pulse to miss. This is the
  two-phase ("toggle") handshake used to cross clock domains.
- **Fact (code):** the writer leaves `TOM` only after it sees C == T, so its
  value stays offered for the whole exchange. No buffer is needed.
- **Deduced:** across clocks a bit can only arrive late, never be lost. A
  late T makes the reader wait longer; a late C makes the writer wait
  longer. Order and "once and only once" hold as in one clock.

### 8.2 What changed in the hardware (`SAPHO/core.v`)

1. **A two flip-flop synchronizer on each incoming bit.** With `LINKAS = 1`,
   `taqui` enters the reader through two flip-flops on the reader's clock
   before `cad_go` and C use it; `valeu` enters the writer through two
   flip-flops on the writer's clock before `tom_go` and W use it.
2. **The writer's value in a register, `toma_dado`.** Without it the
   partner would read the `out` bus, which is the ALU output
   (`ula_out`), combinational (section 5, item 1). Deduced, not measured:
   in the cycle T flips the ALU goes from the instruction before `TOM` to
   `TOM` itself, and the opcode change may glitch the bus for a few
   nanoseconds after the edge. With `LINKAS = 1` a NUBITS-bit register is
   loaded with `ula_out` in the cycle T flips (`tom_offer`, the same edge),
   so the word the reader takes is a flip-flop output that was already
   still when T flipped. Dropped alternative: keep `out` and ask the user
   for a timing constraint on it.
3. **`toma_dado` is a pin of every processor that has `TOM`.** With
   `LINKAS = 0` it is a wire equal to `out`, so a synchronous link may be
   wired either way; the older wiring `A.out -> B.cade_dado` stays valid.
4. **Generated only when used.** `LINKAS = 0` keeps today's hardware. Yosys
   on `link_ping` (`TOM` and `CAD`), `synth -flatten`, 2026-10-09:
   - previous HDL: 1127 cells;
   - new HDL, `LINKAS = 0`: 1127 cells, identical type by type;
   - new HDL, `LINKAS = 1`: 1163 cells, of which +32 `$_DFFE_PP_` (the
     `toma_dado` register) and +4 `$_DFF_P_` (the two synchronizers).

### 8.3 How a link is declared asynchronous

Which clocks exist is known in the user's top-level, not in the program, so
the choice is made there. The generated module of a program with `TOM` or
`CAD` (hdl.c) now opens with `#(parameter LINKAS = 0)` and passes it to
`processor` and `core`. Nothing changes in C+-, C++, cmmcomp, cppcomp or
appcomp. A program without `TOM`/`CAD` gets no parameter.

**Rule:** set `LINKAS = 1` on BOTH processors of a link whose clocks differ
(the reader synchronizes T, the writer synchronizes C and holds the data),
and wire the data from `toma_dado`:

```verilog
link_envia  #(.LINKAS(1)) A (.clk(clk_a), .rst(rst_a), .toma(a_toma),
                             .valeu(b_cade), .toma_dado(a_dado), ...);
link_recebe #(.LINKAS(1)) B (.clk(clk_b), .rst(rst_b), .cade(b_cade),
                             .taqui(a_toma), .cade_dado(a_dado), ...);
```

**Reset:** both processors leave reset together, each with the release
synchronized to its own clock (two flip-flops per domain from one reset
source, as `link2_tb.v` does). A reset of one side alone leaves T and C out
of step, the same open problem as the interrupt (section 6).

### 8.4 Checks

- **Simulation (Icarus), in the regress:** `Compilers/CMMComp/Tests/LINK/
  TopLevel/link2_tb.v`, run inside the `LINK` pass as `LINK2`. The same
  programs on clocks of 20 ns and 27.3 ns, `LINKAS = 1`, three pairs:
  envia fast -> recebe slow, envia slow -> recebe fast, ping slow <-> pong
  fast. Checked against the sequences regress computes (n*7+3, n+1).
  Measured 2026-10-09: 16 of 16 in order on each of the three.
- **What the simulation does not prove.** The same testbench with
  `LINKAS = 0` also passes (measured): Icarus has no metastability and no
  glitches, so it shows the synchronizers do not break the protocol, not
  that the crossing is safe. That part rests on the synchronizer and on the
  board.
- **Unchanged:** `link_tb.v` (one clock, `LINKAS = 0`) gives the same four
  outputs as its golden; the standalone testbench of a processor with `TOM`
  connects `toma_dado` and still writes the right `toma.txt`.

### 8.5 Still open

- **Board:** a two-clock design on the DE10-Nano / ZYBO, with Fmax. The
  Quartus constraints to write and check there: a false path (or a
  `set_max_delay`) into the first synchronizer stage of `taqui` and `valeu`
  (`*tom_link.sync.s1`, `*cad_link.sync.s1`), and a `set_max_delay` of one
  reader period on `toma_dado -> cade_dado`. The exact `.sdc` lines are not
  written yet.
- **Latency:** each value pays about 2 synchronizer cycles on each side on
  top of today's ~16 cycles a value (estimated, not measured).

### 8.6 For the documentation (item 21 in TODO.md)

When the global documentation protocol is decided, the user-facing text is
short and must say, without the reasoning above:

- two processors on different clocks can use `toma`/`cade`;
- write `#(.LINKAS(1))` on both processors and wire `toma_dado` to the
  partner's `cade_dado` (the example in 8.3);
- the reset rule (8.3);
- the Quartus constraints (8.5);
- the cost: 4 flip-flops plus NUBITS on the writer, and a few cycles of
  latency per value.

