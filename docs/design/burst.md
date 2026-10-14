# Burst: a whole array through a port, one word per clock

Status: design, 2026-10-14. Nothing implemented, and not to be yet (Luciano):
the idea is recorded. The design is Luciano's
(the `burst` block, a size register, a counter, an instruction that waits in
place); this note adds the refinements discussed on 2026-10-14 and the
timing measured before writing it. Marks: **fact** (read in the code),
**measured**, **deduced** (reasoned, to be checked in simulation).

## 1. The problem

A program that hands an array to a FIFO writes it word by word. Measured on
the v7.1 compilers, an 8-int array to port 0 (2026-10-14):

| form | instructions per word | code size |
|---|---|---|
| `for (k = 0; k < 8; k++) out(0, a[k]);` | 8 (`LOD k`, `LDI a`, `OUT`, `LOD k`, `ADD 1`, `SET k`, `LES 7`, `JIZ`) | fixed |
| Dirac `out(0, 1\|a⟩);` | 3 (`LOD_V a k`, `MLT 1`, `OUT 0`) | 3 words per element |
| the least the ISA allows (`LOD_V`, `OUT`) | 2 | 2 words per element |

**Fact:** `OUT p` writes the accumulator, so with today's instructions two
are needed per word. Unrolling (the Dirac form) costs program memory in
proportion to the array, which rules it out for large arrays (Luciano). The
Dirac form also has two defects of its own, independent of the burst: it
emits `MLT 1` when the scalar is 1 (a cycle per word, and the multiplier is
synthesized for nothing), and `out(p, |a⟩)` without a scalar is a syntax
error.

The goal: one word per clock, code of fixed size, and the transfer stops
by itself while the FIFO is full, without losing a word.

## 2. The design

A `burst` block in the core, generated only when the program uses its
instructions (pay only for what you use):

- a **size** register, written by an instruction of its own;
- a **pointer** (the counter), set to the array's first address by a second
  instruction, which also computes, once, the **end** address
  (first + size);
- a **burst out** instruction that waits in place, as `TOM`/`CAD` do
  (**fact:** `pc_self` in `core.v` makes the fetch take the same address
  again), sending one word per clock until the pointer reaches the end;
- symmetric, a **burst in** instruction that fills an array from an input
  port, with the same registers.

Assembly, names to be chosen (three letters, as the mnemonic convention):

```
LOD  8          ; acc = size (a constant or a computed value)
BSZ             ; burst: size <- acc
BPT  main_a     ; burst: pointer <- &a[0], end <- &a[0] + size
BOT  0          ; burst out: a[pointer] to port 0 while the port may take it; leaves at the end
```

Refinements over the first sketch, and why:

- **Size from the accumulator** (like `SET` writes memory from it): serves a
  constant and a size computed at run time. The operand field stays free.
- **End computed once, in `BPT`**: during the transfer the hardware only
  increments the pointer and compares it with the end. The adder is out of
  the per-word path.
- **The stop condition from a register**: "the next word is the last"
  registered one clock ahead, so the fetch path (`pc_self`) gets a flip-flop,
  not a comparator. (Deduced; the critical path is measured, item 6.)
- **The port is the operand of `BOT`**, as in `OUT p`; one-hot `out_en[p]`
  as today.

## 3. Timing of today's OUT (measured)

Probe on a program `LOD 5; SET a; OUT 0; LOD 7; OUT 0; LOD 9; OUT 1; LOD 11;
OUT 0`, values sampled at each rising edge (what the generated testbench
and a FIFO see), 2026-10-14:

| edge | PC | decoder `id_out_en` | pin `out_en` | pin `out` |
|---|---|---|---|---|
| 7 | 4 | 1 (the first `OUT 0` in decode) | 00 | 5 |
| 8 | 5 | 0 | 01 | 5 |
| 9 | 6 | 1 | 00 | 7 |
| 10 | 7 | 0 | 01 | 7 |
| 12 | 9 | 0 | 10 | 9 |
| 14 | 11 | 0 | 01 | 11 |

**Facts** behind it (`core.v`, `processor.v`): `out_en` is the decoder's
`id_out_en` through one register (`io_ctrl`: `en_out <= out_en`); the data
pin `out` is `io_out = mem_data_out`, the ALU output (`ula_out`), with no
register. **Measured:** the two are valid in the same clock, the one in
which the `OUT` is in the ALU stage; the consumer writes on the edge that
ends it. The generated testbench recorded 5, 7, 11 on port 0 and 9 on port
1, as written.

The burst must keep exactly this alignment. **Deduced:** it does if every
step of `BOT` does what `LOD` and `OUT` already do: the address `pointer`
goes to the data memory in the decode stage (as `LOD`'s operand does; the
memory is synchronous, the word comes back one clock later), the word
reaches the ALU stage and goes out through `ula_out`, and the decoder's
enable goes through the same `io_ctrl` register. Checked in simulation
before anything else (step 1 of section 7). The input side has its own
alignment (`req_in` is not registered, the processor captures `io_in` at
the edge that ends the `INN`'s decode cycle, `ula_in2_ctrl`); `BIN` has to
be checked against it the same way, never assumed from `BOT`.

## 4. The full FIFO

A new input pin per output port, `out_ok[p]` (name to be chosen),
generated only with `BOT p` in the program. The top level ties it to the
FIFO: `~full` of `myFIFO`, `~wrfull` of `myDCFIFO`. While it is 0, `BOT`
waits in place: no `out_en`, the pointer holds.

**Deduced, the trap:** the decision to send word i+1 is taken in decode
while word i is still on its way (it is written one clock later). With a
plain `~full`, a FIFO with one free place accepts the decision for two
words and the second is lost. Three ways out, to be chosen by measuring:

- the top level gives "at least two free places" (`usedw`/`wrusedw`
  against `LENGTH - 2`): no core logic, but each top level has to know it;
- the burst block counts the word in flight and stops one ahead: the rule
  lives in the core, the pin keeps the plain meaning "not full";
- `myFIFO`/`myDCFIFO` gain an `almost_full` output with that meaning.

`myDCFIFO`'s `wrfull` is computed from a pointer synchronized from the
other clock: it can say full longer than needed (safe), never less
(deduced from the Gray-code design, `docs/design/myfifo.md`).

## 5. Open decisions

1. **The accumulator.** **Fact:** the ALU writes `racc` every clock, so a
   `BOT` built from `LOD` steps leaves the last word in the accumulator. Keep
   it so (document it; the compiler knows) or hold the accumulator during
   the burst (a multiplexer on the accumulator path, in every processor
   that uses the burst).
2. **Which rule against the full FIFO** (section 4).
3. **Interrupts.** **Fact:** an interrupt during a `TOM`/`CAD` wait puts a
   pair out of step (`toma-and-cade.md` section 6). A `#PRACA` restart
   during a burst leaves part of the array sent; the same advice (do not
   interrupt during a burst) or a rule.
4. **Several bursts at once.** One size/pointer pair serves one transfer at
   a time; `BOT` ends before the next instruction runs, so one pair is
   enough unless a `BOT` on one port must interleave with another. Assumed
   not needed.
5. **Names and opcodes.** **Fact:** opcodes go up to 107 of 128 (7 bits).
   The new ones fit at the end of the table, so no opcode moves and an
   older `.asm` keeps its meaning; a new SAPHO is still needed for the new
   instructions (SAPHO and asmcomp from the same release, as since v5.5).
6. **The languages.** C±: `out(p, a)` with a whole array, or the Dirac form
   `out(p, |a⟩)` mapped to the burst (cmmcomp: Luciano's code, diff first).
   C++: an `out(p, array)` overload in cppcomp. The size is the declared
   one, or a part of the array (`out(p, a, n)`)?

7. **Item 17 of TODO.md** (filling and copying arrays faster) proposes an
   index register `X` with post-increment. The burst pointer is that
   register plus an end and a wait in place. If both are built, they should
   be one block: the same pointer would give a fill at one word per clock
   (a burst store of the accumulator: `mem[pointer] <= acc`, waiting in
   place to the end) and the sequential loads of item 17. Decide the
   instruction set with both in view, so that `BSZ`/`BPT` are not designed
   twice.

## 5a. Checked, and holes still open (2026-10-14)

**Checked:** one word per clock is possible. A wait in place repeats the
instruction every clock with no bubble: the jump is decided in the cycle
the instruction leaves the instruction memory (`toma-and-cade.md`
section 5), and in the probe of section 3 `@fim JMP fim` shows PC 12 at
every edge.

**Holes, to be closed before implementing:**

1. **The input side has the same one-word trap, reversed.** A FIFO's
   empty flag changes only after the read, so `BIN` can decide to read the
   next word from a FIFO that is already empty and take garbage. `myFIFO`
   already has an `almost_empty` output; the margin rule of section 4
   applies here too.
2. **The element type.** Today `out` of a float sends its raw bits (with a
   warning) and `fout` converts it to an int. A burst of a float array
   needs the same choice; the word goes through the ALU, so a conversion
   per word (`F2I`) is possible. A `comp` element is two words.
3. **Size 0 and size 1.** The registered "next is the last" flag must
   handle both: with size 0, `BOT` sends nothing and does not wait.
4. **The compiler passes that drop and share data.** `asm_reach` removes
   arrays nothing reads, and `asm_share` lets variables whose lives do not
   overlap share a word. Both must count `BPT` as a read of the array (and,
   for `BIN`, a write of all of it), or the array is dropped or overwritten
   by another variable, silently. A small test passes; a large program
   breaks.
5. **`OUT` and `BOT` on one port.** A plain `OUT` ignores `out_ok`, so a
   program mixing both on one port can still overflow the FIFO. Document
   it, or make `OUT` wait on `out_ok` too (a change to today's behaviour,
   with its own pin cost).
6. **Simulation and its consumers.** The generated testbench has to drive
   `out_ok`/`in_ok` (tied to 1, or better a model FIFO that fills, to test
   the stop), and so do the Verilator harness, the mirror block AURORA reads
   (`YANC_SIM_VIS`) and `gen_gtkw` (the new pins in the view).

## 6. Cost, to be measured

**Fact (method):** the critical path is measured on the whole processor,
`Scripts/hw/fmax.sh` on `sapho_all`, never on the block alone. Here:
`sapho_all` without the burst must not change at all (the generate is off),
and a variant with `BOT` gives the new path: the pointer multiplexer into
the data-memory read address, and `out_ok` into the fetch. Area with
`Scripts/hw/area.sh NOSHARE=1`.

## 7. Steps

1. HDL: the `burst` block and `BOT`, `BSZ`, `BPT` behind their generate;
   asmcomp knows the opcodes (`isa.tsv`, `check_isa.py`). A hand-written
   `.asm` test with an 8-word array into `myFIFO`: the timing of section 3
   checked on the waveform, then a FIFO smaller than the array, with a slow
   reader, so that `out_ok` stops the burst; no word lost or repeated.
2. The same into `myDCFIFO` on two clocks.
3. `BIN` and its test (FIFO into an array, waiting on empty).
4. `fmax.sh` and `area.sh` (section 6).
5. C± (the diff to Luciano first), then C++; regress fixtures in both.

## 8. For the documentation (item 21 in TODO.md)

- The new forms in the language chapters, and the pins `out_ok`/`in_ok` in
  "Several processors" (what to tie them to, the two-free-places rule if
  that is the one chosen).
- The instructions in the instruction reference (generated from
  `isa.tsv`).
- What the accumulator holds after a burst, and that an interrupt during a
  burst leaves the array half sent.
