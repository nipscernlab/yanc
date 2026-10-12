# `toma`/`cade` with several links: chains, fan-out, fan-in

Design of TODO item 23 (target v7.1), opened and **implemented on
2026-10-11** (section 10 has what was built and measured). Marked: **fact**
(read in the code), **deduced** (from the code, not run), **to measure**. The
single-link design it extends is [`toma-and-cade.md`](toma-and-cade.md),
whose source forms (`toma(x)`, `cade()`), pins (`toma`, `cade_dado`) and
`LINKAS` parameter this replaces.

## 1. What it is for

Today a processor has one link, to one partner, usable both ways (fact,
`toma-and-cade.md` section 1): two processors, or a ring of two. With
several links per processor a SAPHO can hand values to several others and
take values from several others: a chain A -> B -> C, one writer feeding
two readers, one reader merging two writers, and any mix. Each link keeps
the rendezvous of today: whoever arrives first waits, nothing is lost or
read twice, and a deadlock is the program's responsibility.

Decided by Luciano, 2026-10-11: the operand of `TOM`/`CAD` becomes the link
number (option A), rather than a second pair of opcodes limited to two links
(option B). Versatility wins; the extra multiplexing is paid only by a
program that uses more than one link (pay for what you use).

**Design priority (Luciano, 2026-10-11): hardware only where the program
needs it, and nothing for the user to build outside the processor.** The
user only connects wires and buses in the top level. Every multiplexer,
bit and pin below sits inside the processor and behind a `generate` driven
by what the program uses: no link used, nothing generated; one link, no
multiplexer at all (today's cells); a `CAD` multiplexer only with `CAD` on
two or more links, a `tom_go` multiplexer only with `TOM` on two or more,
a synchronizer only on a link the program uses in the plain form (section 7). The `generate`
conditions are part of the design and get a Yosys cell count per case
(section 6).

## 2. What the user writes

```c
toma(1, x);        // hand x over link 1
y = cade(0);       // take a word from link 0
f = fcade(2);      // the same, as a float
```

The link number comes first and is a constant, as the port of `in(0)` /
`out(0, x)`. A chain:

```c
// A                 // B                          // C
toma(0, x);          y = cade(0);                  z = cade(0);
                     toma(1, y * 2);
```

with A's link 0 wired to B's link 0, and B's link 1 to C's link 0. Link
numbers are local to each processor: what joins two links is the wiring in
the top level.

## 3. Instructions

`TOM k` and `CAD k` (opcodes 20 and 21, unchanged): the operand is the link
number. Each is still a jump to itself until link k lets it through.

**Fact:** today the operand is the instruction's own address and the
prefetch uses it as the jump target (`core.v`, `instr_fetch`: `instr_addr =
... (pc_load) ? operand : pc_instr`, with `wJMP = wJMPZ | wTOM | wCAD`).

**Change:** the prefetch must re-fetch the current instruction without the
operand. The instruction memory reads synchronously, so the address
presented in the previous cycle is the address of the instruction now
decoded; keeping it in a register (MINSTW flip-flops) and selecting it when
`TOM`/`CAD` must wait gives the self-jump (deduced). Cost: one register and
one more input on the `instr_addr` multiplexer, the fetch path; generated
only when the program has `TOM` or `CAD`. **To measure:** Fmax of
`sapho_all` pairs before and after, since that multiplexer is on the path
that decides the next address.

The assembly changes from `@L TOM L` to `TOM 1`: an encoding break, free
while `TOM`/`CAD` are unreleased. asmcomp checks the operand is a link
number in range instead of its own label.

## 4. Hardware per link

Per link k, generated only for the links the program uses:

- with `TOM k`: the bits T(k) and W(k), pins `toma_k` (out) and `valeu_k`
  (in);
- with `CAD k`: the bit C(k), pins `cade_k` (out), `taqui_k` (in) and
  `cade_dado_k` (in, NUBITS);
- a synchronizer per link, chosen by the source (section 7): the two
  flip-flops on `valeu_k`/`taqui_k` unless the program uses the `_sync`
  form on link k. Each link is synchronous or not on its own: one
  processor can talk to a partner on its clock over link 0 and to another
  on a different clock over link 1. The shared `toma_dado` is a register
  as soon as one `TOM` link is synchronized.

**One `toma_dado` for all links** (deduced): a writer is inside one `TOM` at
a time and leaves it only after the reader took the value, so a single
offered-value register (synchronized links) or the `out` bus (only `_sync` links) serves
every link. One NUBITS pin instead of one per link.

**What the operand selects** (the multiplexers the decision accepts):

- the jump decision: `tom_go(k)` / `cad_go(k)` chosen by the operand, an
  N-to-1 bit multiplexer feeding `pc_load` (fetch path);
- the data of `CAD`: `cade_dado_k` chosen by the operand, an N-to-1 NUBITS
  multiplexer in front of `INN`'s path into the accumulator;
- which T/W or C flips: an operand decode, k compared once per link.

With a single link used, none of these multiplexers exists and the link is
today's hardware (to verify cell for cell with Yosys).

## 5. Compilers, assembler, generated top

- cmmcomp (diff shown to Luciano first) and cppcomp: `toma(k, e)`,
  `cade(k)`, `fcade(k)` and the `_sync` forms, k a constant, as `in`/`out`
  take their port.
- asmcomp/hdl.c: collect the link numbers used by `TOM` and by `CAD` and
  pass them as masks (`TOMS`, `CADS`, and which are `_sync`); emit the pins
  above; no `LINKAS` parameter left.
- Standalone simulation: the testbench plays every partner, one file per
  link: `Simulation/cade_k.txt` and `Simulation/toma_k.txt`.
- `check_isa.py`, `isa.tsv`, `asm_share.c`: the operand class of `TOM`/`CAD`
  changes from "own label" to "link number".

## 6. Tests

- The existing `LINK`, `LINK2` and `LINKALL` passes, rewritten to the new
  syntax and pin names, must give the same sequences.
- A chain of three on two clocks: A -> B on one clock, B -> C on another
  (B uses `cade_sync(0)` from A and plain `toma(1, y)` to C), the case asked for on 2026-10-11.
- Fan-out (one writer, two readers) and fan-in (one reader alternating two
  writers), checked against computed sequences.
- Negative fixtures: a link number that is not a constant, a link number out
  of range.
- Yosys: a program with one link gives today's cells; Fmax of a
  three-processor `sapho_all` chain on each board (heavy, mains power).

## 7. Synchronous or not: decided in the source

**Fact (main today, single link):** nothing is detected. The program is
the same either way; hdl.c gives the generated module a parameter
`LINKAS` (default 0) only when the program has `TOM` or `CAD`, and whoever
writes the top level sets `#(.LINKAS(1))` on both processors when their
clocks differ. A forgotten `LINKAS 1` passes in simulation (Icarus has no
metastability) and fails on the board now and then.

**Measured 2026-10-11** (Icarus, both processors on one clock, scratch
programs, not in the regress): the synchronizer adds 4 cycles a value.

| case | no synchronizer | synchronizer |
|---|---|---|
| back-to-back `toma` -> `cade`, nothing between | 8 cycles a value | 12 |
| `link_ping` <-> `link_pong` round trip | 15 cycles | 19 |
| cells, `link_ping` (Yosys, 2026-10-09) | 1127 | 1163 |

Negligible beside real work (a `sapho_all` turn is ~1000 cycles), 50 % in a
processor that only forwards data.

**Decided (Luciano, 2026-10-11):** the choice moves into the program and
`LINKAS` goes away.
- `toma(k, x)`, `cade(k)`, `fcade(k)`: link k is synchronized (two
  flip-flops on the incoming bit; the shared `toma_dado` register). The
  default and the safe one.
- `toma_sync(k, x)`, `cade_sync(k)`, `fcade_sync(k)`: "my partner on link k
  runs on my clock": no synchronizer on that link, 4 cycles a value saved.
  "sync" means synchronous (same clock), not "with a synchronizer"; the
  manual must say so plainly.
- hdl.c derives from the program which links get a synchronizer: no
  parameter left in the top level.
- Each side decides alone (deduced, to test): each processor's synchronizer
  sits on the bit coming from its partner, so a side's choice does not
  depend on the other's, and the compiler, which sees one program at a
  time, has nothing to check across processors. On one clock any mix
  works; across clocks both sides must use the plain form.
- A link used with both forms in one program (`toma` and `toma_sync` on
  link 1) is a compile error: a link has one hardware.
- The price: a `_sync` program states a fact of the board. Moved to a design
  where the partner's clock differs, it must go back to the plain form.

## 8. Answered by Luciano (2026-10-11)

1. The link number is always written (`toma(k, x)`, `cade(k)`), as `in`/
   `out` take their port; no short forms.
2. Pins carry the link number for link 0 too (`toma_0`, `cade_dado_0`, ...);
   the current tests are rewired.
3. No fixed limit on the number of links beyond the operand width.

## 9. Parked: processors with different formats

`toma(x)` already sends a float's word unconverted and `fcade()` reads it as
a float (fact: `exec_toma` emits no `F2I`; checked by hand 2026-10-07, C+-
to C++, 16 of 16). When both processors have the same `#NBMANT`/`#NBEXPO`
that is exact. When they differ, the word arrives garbled with no warning:
each program is compiled alone. Parked by Luciano (2026-10-11): the check
belongs to Aurora, which sees every program of a project; converting
between formats maybe in the future. v7.1 stops at the links.

## 10. Implemented (2026-10-11)

- `SAPHO/core.v`: the self-jump keeps the address of the instruction in
  decode (`r_self`) and re-fetches it; per link a `generate` slot with
  T/W (TOMS[k]), C (CADS[k]) and a synchronizer unless LSYNC[k]; the
  operand compare, `tom_go`/`cad_go` and the CAD word multiplexers only with
  two or more links of a kind; `toma_dado` a register as soon as one TOM link
  is synchronized. `SAPHO/processor.v`: `NLINK`, `TOMS`, `CADS`, `LSYNC`
  among the fixed parameters (before `P_LOD`, so `check_blocks.py` does not
  take them for opcode blocks).
- asmcomp: `TOM k` / `CAD k` (operand class `link`, checked against the
  operand width), the directive `#LKSYNC k` (an error on a link the program
  does not use), the pins and parameters in the generated top, one partner
  per link in the standalone testbench. appcomp counts `TOM`/`CAD` with
  their number and skips `#LKSYNC`.
- cmmcomp and cppcomp: `toma(k, e)`, `cade(k)`, `fcade(k)` and the `_sync`
  forms, k an integer literal; a link used with both forms is an error;
  `#LKSYNC k` written after `@fim` for each `_sync` link.
- Checks: `LINK`, `LINK2`, `LINKALL` give the same sequences as before;
  `LINK3` (Tests/LINK3/TopLevel/link3_tb.v, 20 / 27.3 ns): chain
  lk_chain_a -> lk_chain_b -> lk_chain_c ((3n+1)*2, also with the C++
  middle lk_cpp_chain_b), fan-out lk_fan_w (n and n+100), fan-in lk_merge
  (1000n + 7n), float lk_flt_w -> lk_flt_r (3n), all 16 of 16 in order.
  Negative fixtures: `link_mix`, `link_range` (C+-), `link_mix`, `link_lit`
  (C++).
- Yosys (`synth -flatten`, the same program with one link and with two):
  fan-out 1090 -> 1108 cells, fan-in 4199 -> 4270; `cade_sync` on lk_chain_b's
  link 0 takes 276 flip-flops to 272. The self-jump alone: +12 cells on
  link_ping, +35 on sapho_all_b, `sapho_all` (no link) unchanged.
- Fmax of a linked design (2026-10-11, the LINKALL pair: `sapho_all_a` on
  clk_a hands its result to `sapho_all_b` on clk_b over a synchronized link;
  top `Scripts/hw/linkall_top.v`, scripts `fmax_link.sh` and
  `vfmax_link.sh`), against `sapho_all` alone with the same HDL:

  | | `sapho_all` alone | pair, clk_a | pair, clk_b |
  |---|---|---|---|
  | DE10-Nano, Quartus 24.1, slow 100C | 21.82 MHz, 3,558 ALMs | 20.76 MHz | 22.36 MHz (7,127 ALMs both) |
  | ZYBO, Vivado 2025.2 | 23.54 MHz | 23.99 MHz | 23.89 MHz |

  Within the seed noise (~2.5 MHz, quartus-fmax notes): the link costs no
  clock. In every case the worst path is the integer divider (data memory ->
  `cut[1]` of `ula_idiv`), as in `sapho_all` alone. Quartus found 4
  synchronizer chains of 2 registers (the two link bits and the two reset
  releases), 9.4 ns of settling time; no MTBF, since the chains are not
  declared as synchronizers in the project.
- The board constraint, checked by both tools: two clocks and
  `set_clock_groups -asynchronous` between them. Every crossing is then cut:
  Vivado lists 33 paths clk_a -> clk_b (the toma bit and the 32 bits of
  `toma_dado`) and 1 clk_b -> clk_a (the cade bit), all "Asynchronous Groups".
  Cutting the word is safe because the protocol holds it still at least two
  receiver cycles before it is read (section 4); a board project writes:

  ```
  create_clock -period <ns> -name clk_a [get_ports clk_a]
  create_clock -period <ns> -name clk_b [get_ports clk_b]
  derive_clock_uncertainty
  set_clock_groups -asynchronous -group [get_clocks clk_a] -group [get_clocks clk_b]
  ```
- Still open: the board itself (Icarus has no metastability; only the board
  shows the link holding up for hours). The students' task: the LINKALL pair
  on each board with this `.sdc`, A on the switches, B on the LEDs.

## 11. For the documentation (item 21 in TODO.md)

The user-facing text says: a processor can have several links, numbered
from 0; `toma(k, x)` hands `x` over link k and `cade(k)` / `fcade(k)` take a
word from it; each link joins two processors and works both ways; how to
wire link k of one processor to link j of another (the pins of section 4,
one shared `toma_dado`); the `.sdc` of section 10 when the clocks differ; the plain forms work across clocks, the `_sync`
forms only when the partner runs on the same clock (and save 4 cycles a
value); a deadlock, and an interrupt during a wait, are the
program's responsibility.
