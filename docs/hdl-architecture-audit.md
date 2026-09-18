# HDL architecture audit

Date: 2026-09-15 · Scope: `HDL/processor.v`, `core.v`, `instr_dec.v`, `ula.v`,
`myFIFO.v`, `addr_dec.v`, the generated `<proc>.v`/`<proc>_tb.v` (`hdl.c`),
and the two programs that use the interrupt (`ZeroCross`, `ProcDTW`).
Companion to [`precision-and-width-review.md`](precision-and-width-review.md)
(which covers the float datapath); the work items are TODO item 10.

The question asked: *where is this processor weak, and what would make it
professional without losing its defining property — the hardware adapts to
the assembly program, building only what the opcodes use?*

---

## 0. What the architecture is, as built

A **Harvard, single-cycle accumulator machine** with a memory operand, two
hardware stacks (data, return) and a two-stage overlap:

```
cycle N   : instruction N leaves mem_instr (registered read)
            instr_dec decodes it combinationally -> push/pop, mem_wr, req_in, out_en, ldi/sti/lda/sta
            the data read for its operand is issued (mem_data, registered read)
            ula_op for it is registered
cycle N+1 : ula_op and mem_data_rd arrive together, the ALU computes, racc <= ula_out
```

While instruction N executes in N+1, instruction N+1 is being decoded — and
four of its decisions consume `ula_out` **combinationally, the cycle it is
produced**: the `JIZ` branch (`if_acc = |ula_out` → `pc_load` → instruction
address), the `LDI`/`LDA` read address (`ula_out[MDATAW-1:0]`), the `SET`
data (`mem_data_wr = ula_out`) and the `PSH` data (`sp_in = ula_out`). That is
a full bypass network: **no data hazard, no control hazard, no branch
penalty, every instruction exactly one cycle**. The price is that the ALU's
depth sits on the branch and address paths, so it sets the clock directly.

Things this design does right that many "textbook" cores do not:

- **Opcode-driven resource allocation.** Every operator, decoder compare and
  ALU path exists only if the program uses it (`generate` on ~110 opcode
  parameters); memories are sized from the program (`MDATAS`, `MINSTS`,
  `NBOPER`). A 20-instruction integer program is a few hundred ALMs.
- **Deterministic timing.** One instruction per cycle, no cache, no stall, no
  speculation: the cycle count of a loop is known at compile time — the right
  property for DSP and control.
- **Single clock, synchronous reset** on every state element (v5.3),
  FPGA-friendly memories (registered reads, `$readmemb` init), no CDC.
- **Bypass by construction** rather than by forwarding logic.

Keep all four. Everything below fits inside them.

---

## 1. Weak points, by severity

### 1.1 The interrupt is a level-sensitive PC override

`prefetch` (`core.v`): `pc_l = itr | pc_load; instr_addr = itr ? ITRADD : …`.
As long as `itr` is high the fetch address is forced to `ITRADD` **every
cycle**: the instruction at the vector is fetched and executed once per cycle
the level is held, the interrupted PC is not saved, nothing masks the input,
and `RET` cannot resume because nothing was pushed. It works in DTW because
the source is a FIFO-not-empty flag whose first consumer (`fin`) clears it —
but `ProcDTW`'s vector instruction is `out(2, -123)`: if the level ever lasts
two cycles, the port pulses twice. This is an *event restart*, not an
interrupt, and its contract is implicit.

**Professional version, behind `ITRADD != 0` only:** edge-detect the input
(two flops, one-shot), optionally push the PC on the return stack so the
handler can `RET` (one extra `isp_push` condition), and a mask bit settable by
software. Cost: ~5 flops + a mux; no cost when the program has no `#PRACA`.
The compilers already emit the vector; they would gain a "return from
interrupt" form.

### 1.2 I/O has no flow control

`INN`/`F_INN` pulse `req_in`; the generated testbench answers on the same
cycle's negative edge, and `ula_in2_ctrl` samples `io_in` one cycle later.
`OUT` pulses `out_en` with the data. There is no *ready*: an empty input FIFO
returns stale data, a full output FIFO drops a word, and the core never knows.
The DTW project copes with `almost_empty`/interrupt plumbing outside the core.

Options, in order of cost:
1. **Document the contract** (respond in the cycle after `req_in`; never
   `OUT` into a full sink) — zero cost, overdue.
2. **Status-port convention**: reserve an input port index for status bits
   (`empty`, `full`) the program polls with `in()`; `hdl.c` wires it. No core
   change, no timing change.
3. **valid/ready with a global stall** — the ~10 register enables listed in
   `precision-and-width-review.md` §2.6. Changes cycle counts; only worth it
   if streaming peripherals become a design goal. Keep as an option behind a
   parameter, off by default.

### 1.3 Stack overflow and underflow are silent

Both `stack` instances wrap the pointer modulo `DEPTH`; the only detection is
the simulation-only `fl_full` flag. On hardware a deep call chain or a long
expression silently corrupts the stack.

Two fixes that fit the philosophy: (a) **let the compilers size the stacks**
— for non-recursive programs `cmmcomp`/`cppcomp` know the maximum call depth
and the maximum expression depth statically; `#NDSTAC`/`#SDEPTH` become an
optional override, and `asmcomp` refuses a value smaller than the program
needs; (b) a **sticky overflow flag** brought out as a pin, exactly like
`cheguei` (two flops), so the fault is visible on a board.

### 1.4 The ALU sits on the fetch and address paths

Consequence of the bypass network (§0). Measured on Cyclone V C6: a
division-free float program runs at ~51 MHz (level 0), ~40 MHz (level 2); a
program that divides at 14 MHz (was 8 before the explicit divider). Nothing
short of an ISA change (a registered branch: `JIZ` tests `racc`, one cycle
late, and the compiler schedules a NOP or an independent instruction) moves
the branch decision off the ALU path — and even then `LDI`/`LDA` addresses
would need the same treatment. Recommendation: **document, do not change**.
Revisit only if Fmax becomes the limiting resource for a real project; it
would be a parameter (`REGISTERED_BRANCH`) with compiler support, never the
default.

### 1.5 The ISA lives in four places with no consistency check

Opcode numbers are hard-coded in `ASMComp.l` (the `eval_opcode` order),
`instr_dec.v` (`opcode == 7'dNN`, 108 of them), the hand-minimised 6-bit
`ula_op` table (`instr_dec.v` `b5..b0`) and `core.v`'s
`{{NBOPCO-5{1'b0}}, 5'd15}` literals for `JMP`/`JIZ`/`CAL`/`RET` (which
silently assume those four opcodes stay below 32 and `NBOPCO >= 5`). Adding an
opcode means editing all four by hand; 108 of 128 codes are used.

**Professional version:** one ISA table in the repo (`ISA.csv`/`.json`:
mnemonic, opcode, ALU op, operand class, stack effect, I/O effect) from which
a generator emits the `.l` rules, the `instr_dec.v` compares and `ula_op`
table, the `opcodes.c` names and the ISA reference — or, as a first step, a
regress check that the four agree. Also lets `NBOPCO` grow to 8 when the
table fills.

### 1.6 Undefined behaviours

| case | today | proposal |
|---|---|---|
| integer `DIV`/`MOD` by zero | `x` in simulation, whatever the array gives in hardware | define (quotient 0 / remainder = dividend, or saturate) — item 8 step 4b, when `DIV`+`MOD` share one array |
| `LDI`/`LDA`/`STI`/`STA` address ≥ `MDATAS` | truncated to `MDATAW` bits, wraps | document; optional overflow flag pin |
| `JMP`/`JIZ`/`CAL` target ≥ `MINSTS` | truncated | document (the assembler can already reject it) |
| shift by ≥ `NUBITS` | Verilog semantics (0 / sign) | document |
| stack over/underflow | silent wrap | 1.3 |
| float division by zero | saturates at levels ≥ 1, all-ones quotient at 0 | documented (v5.4) |

### 1.7 Parameter hygiene and lint

- ~~Default parameter sets disagree~~ — **done (2026-09-18):** one set
  everywhere, and it is `cppcomp`'s, because that is the one actually
  exercised (all 81 C++ tests omit every `#pragma` and run on it, while every
  C± fixture writes its directives): `NUBITS 32 = NBMANT 23 + NBEXPO 8 + 1`,
  `NUGAIN 128`, `SDEPTH`/`DDEPTH` 128, `FFTSIZ 3`, `FROUND 0`. It is named in
  one comment block at the top of `processor.v`; `asmcomp` dropped its 23/16/6
  (a second float format nobody reached) and `cmmcomp` its 16/6.
- ~~`NUGAIN` is an untyped 32-bit parameter in `processor.v`/`core.v`~~ —
  **done**, typed `signed [NUBITS-1:0]` in all three (with step 6 of item 8).
  ~~`NBOPCO` is passed to `ula` and unused~~ — **done**, the parameter and the
  connection are gone.
- No invariant guard (`NUBITS == NBMANT+NBEXPO+1`, `NBMANT < 2^(NBEXPO-1)`,
  `NUGAIN` a power of two) — the assembler checks the first; the HDL checks
  nothing.
- Verilator runs with `-Wno-WIDTH -Wno-UNOPTFLAT -Wno-CASEINCOMPLETE …`. A
  professional flow is lint-clean at `-Wall`; the remaining WIDTH warnings are
  few and mechanical.
- `mem_instr` carries a fake write (`if (wr) mem[addr] <= 0` with `wr = 0`) to
  coax RAM inference — needs a comment or removal.
- `myFIFO` reads combinationally (`always @(*) q = mem[addr_r]`), which keeps
  it out of block RAM (M10K is synchronous-read only): fine at 128 × 16, wasteful
  larger. `full` depends combinationally on `wrreq`.

### 1.8 Verification is golden-only

The regress compares 125 self-blessed outputs; the only external reference is
`test50` (host IEEE, quantised to 1e-6). There is no per-module testbench, no
directed test of the control paths (branches, call/return depth, `LDI`/`STI`
addressing, I/O handshake, interrupt), no coverage of `#FROUND 1/2` beyond two
fixtures, and no assertions. `simulacao.c` is a trace writer, not a simulator.

**The standard answer for a small CPU is cheap here:** an instruction-set
simulator (ISS) in C — the ISA is ~110 opcodes with one-cycle semantics — and
a random-program generator; run thousands of random programs through the ISS
and through Icarus/Verilator and compare traces. That catches control bugs
goldens never will, gives users a fast software simulator, and is the natural
home for the ULA unit testbench already planned with the 64-bit work.

### 1.9 There is no ISA reference

Encoding, per-instruction semantics and timing, the I/O and interrupt
contracts, reset behaviour and the parameter list live in code comments, the
README and Claude's memory notes. A `docs/isa.md` (generated from the table of
1.5, ideally) is what a new user or a reviewer needs first.

---

## 2. What "professional" adds, in order, all opcode-tailored

| # | item | changes ISA/timing? | cost | why first |
|---|---|---|---|---|
| 1 | ISS + random differential test (1.8) | no | tooling, ~1 week | every later change gets verified for free |
| 2 | single-source ISA table + consistency check (1.5, 1.9) | no | tooling | removes the four-place edit; produces the reference doc |
| 3 | interrupt: one-shot + optional PC save/`RET` + mask (1.1) | adds a return form | ~5 flops, behind `ITRADD` | turns an implicit contract into a real interrupt |
| 4 | stacks sized by the compiler + overflow pin (1.3) | no | compilers + 2 flops | removes the user's guess; faults become visible |
| 5 | defined behaviour for every undefined case (1.6) | no | tiny | correctness on hardware |
| 6 | I/O contract documented + status-port convention (1.2) | no | doc + `hdl.c` | streaming without guessing |
| 7 | parameter/lint hygiene, invariant guard (1.7) | no | small | one afternoon |
| 8 | valid/ready I/O behind a parameter (1.2 option 3) | cycle counts | ~10 enables | only if streaming peripherals become a goal |
| 9 | registered-branch option (1.4) | yes | HDL + compilers | only if Fmax ever matters more than 1-cycle branches |

Not on the list, on purpose: pipelining the ALU, caches, multi-cycle
operators, a register file. Each would buy Fmax or generality by giving up
the one-instruction-one-cycle determinism and the "pay only for what you use"
allocation — the two properties that make this core what it is.
