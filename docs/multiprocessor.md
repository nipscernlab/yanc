# Several processors

A design can hold several SAPHOs, each running its own program, joined by
wires in a Verilog top level written by you. They pass values in two ways:
**links** (`toma`/`cade`), one word at a time, the sender waiting until the
receiver takes it; and **FIFOs** (`myFIFO`, `myDCFIFO`), a buffer of words
between an output port and an input port.

## The processor's pins

`yanc prog.cmm` writes the module `prog` with only the pins its program
uses:

| pin | direction | what |
|---|---|---|
| `clk`, `rst` | in | the clock and the reset (active high) |
| `in` | in, `#NUBITS` | the word of the input port being read |
| `req_in[p]` | out, one bit per input port | 1 in the clock cycle the program reads port `p`, when `in` must hold its word |
| `out` | out, `#NUBITS` | the word being written |
| `out_en[p]` | out, one bit per output port | 1 in the clock cycle the program writes port `p` |
| `itr` | in | the `#PRACA` restart (see [Directives](directives.md)) |
| `cheguei` | out | the `#TOAQUI` marker |
| link pins | | below |

All input ports share `in`: the top level puts on it the word of the port
whose `req_in` bit is 1. All output ports share `out`: a port's word is
valid when its `out_en` bit is 1.

The word on `out` (and on `toma_dado`, below) comes straight from the
processor's ALU: a block that only stores it when `out_en` is 1 adds
nothing to the clock, but a block that decodes or computes on it before a
register makes the processor's longest path longer. Register it first
(GitHub issue #7 weighs a register inside the processor).

### The warnings Quartus and Vivado give

Every opcode is a parameter and only what the program uses is built, so
the synthesizers report, for any program, things that are not faults:

| tool | warning | why, and what to do |
|---|---|---|
| Quartus | `Critical Warning (127005): Memory depth (16) in the design file differs from memory depth (11) in the Memory Initialization File` | a small memory (a few words, built in logic) is as deep as its address reaches, the `.mif` holds the program's words only; the rest is set to 0 and never addressed. Nothing to do: padding the `.mif` would cost a block RAM on bigger programs |
| Quartus | `Warning (10036): ... assigned a value but never read`, `(10030)` on the font of `myVGAText` | a signal whose reader belongs to an operation the program does not use; the font is a read-only memory |
| Vivado | `[Synth 8-7129] Port ... is either unconnected or has no load`, `[Synth 8-3936] ... trimmed`, `[Synth 8-6014]` | the same: inputs of operations not built, divider stage registers with constant bits |

Measured with Quartus 24.1 (Cyclone IV E and V) and Vivado 2025.2 (Zynq-7010)
on 2026-10-21. A warning not in this table may be a real problem: say so in
an issue.

## Links: `toma` and `cade`

```c
toma(k, x);        // hand x over link k; wait until the partner takes it
y = cade(k);       // wait until link k offers a word; take it (int)
f = fcade(k);      // the same, as a float
```

`k` is the link number, an integer literal from 0. Whoever arrives first
waits, so no word is lost or read twice. A processor has as many links as
its program uses, each joining it to one partner, and a link can carry words
both ways. With them a program writes chains, fan-out (one processor feeding
several) and fan-in (several feeding one) directly. The same calls exist in
C++.

Per link, the processor gets these pins:

| program uses | pins |
|---|---|
| `toma(k, ...)` | out `toma_k`, in `valeu_k` |
| `cade(k)` / `fcade(k)` | out `cade_k`, in `taqui_k`, in `cade_dado_k` (`#NUBITS`) |
| any `toma` | out `toma_dado` (`#NUBITS`), the word offered, shared by all links |

Wiring sender A's link `i` to receiver B's link `j` takes three
connections: `A.toma_i -> B.taqui_j`, `B.cade_j -> A.valeu_i`,
`A.toma_dado -> B.cade_dado_j`. A chain of three processors, A and B on one
clock and C on another, as in the regress (`Compilers/CMMComp/Tests/LINK3`):

```c
// lk_chain_a.cmm          // lk_chain_b.cmm                // lk_chain_c.cmm
toma_sync(0, n * 3 + 1);   x = cade_sync(0);                out(0, cade(0));
                           toma(1, x * 2);
```

```verilog
lk_chain_a A (.clk(clk_f), .rst(rst_f), .out(a_out), .toma_0(a_t0), .valeu_0(b_c0), .toma_dado(a_dado));
lk_chain_b B (.clk(clk_f), .rst(rst_f), .out(b_out),
              .cade_0(b_c0), .taqui_0(a_t0), .cade_dado_0(a_dado),
              .toma_1(b_t1), .valeu_1(c_c0), .toma_dado(b_dado));
lk_chain_c C (.clk(clk_s), .rst(rst_s), .out(c_out), .out_en(c_en),
              .cade_0(c_c0), .taqui_0(b_t1), .cade_dado_0(b_dado));
```

### One clock or two

A link works between processors on different clocks: `toma`, `cade` and
`fcade` synchronize what crosses (two flip-flops on each incoming bit, the
offered word held in a register). When both processors are on the **same
clock**, the forms `toma_sync`, `cade_sync` and `fcade_sync` drop the
synchronizer and save 4 clock cycles per word (measured: 8 against 12 back
to back). A link is used with one form or the other on both ends; the
compilers stop when a program uses both forms on one link.

Two clocks need, in the board's constraints file, the two clocks declared
as unrelated:

```
create_clock -period <ns> -name clk_a [get_ports clk_a]
create_clock -period <ns> -name clk_b [get_ports clk_b]
derive_clock_uncertainty
set_clock_groups -asynchronous -group [get_clocks clk_a] -group [get_clocks clk_b]
```

and the two processors leave reset together, each with the release
synchronized to its own clock (two flip-flops per clock from one reset
source, as `rst_f`/`rst_s` in the regress top level). Measured on both
boards of the group, a pair of processors joined by a link runs at the
clock of one processor alone: the link costs no frequency.

### What the program has to respect

- **The same word format at both ends.** The word crosses as it is, with no
  conversion: both processors need the same `#NUBITS`, and the same
  `#NBMANT`/`#NBEXPO` to pass floats. Nothing warns when they differ (each
  program is compiled alone), and the value arrives wrong.
- **No deadlock check.** Two processors each waiting for the other wait
  forever; the order of `toma` and `cade` is the program's responsibility.
- **No `itr` during a wait.** The `#PRACA` restart while a processor waits in
  `toma` or `cade` leaves the pair out of step.

### Simulating one processor alone

The testbench `yanc` writes plays the partner of every link: `cade(k)` takes
the words of the file `cade_k.txt`, beside the source, one per line (when
the file ends, the next `cade(k)` waits forever), and every `toma(k, x)`
writes `x` to `toma_k.txt` in the output folder.

## FIFOs: `myFIFO` and `myDCFIFO`

A FIFO between an output port and an input port lets a writer go on without
waiting for each word to be taken. The two FIFO blocks, their ports and how
to wire them are in [Peripherals](peripherals.md).

