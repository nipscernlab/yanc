# Peripherals

Blocks in the `SAPHO/` folder that sit beside a processor, in the Verilog
top level you write: they are not generated from a program, you instantiate
them and wire them to the processor's pins. Each one has a testbench under
`Scripts/hw/` that the regress runs.

| block | what it is | the program reads or writes it with |
|---|---|---|
| `myFIFO` | a buffer of words, one clock | `out()` on one side, `in()` on the other |
| `myDCFIFO` | a buffer of words between two clocks | `out()` on one side, `in()` on the other |
| `myKeyPS2` | a PS/2 keyboard, characters ready | `cade_sync(k)` |

A VGA text screen fed by `toma` is planned.

## FIFOs: `myFIFO` and `myDCFIFO`

When a program produces a buffer of words for another (or for other logic),
a FIFO between an output port and an input port lets the writer go on
without waiting for each word to be taken.

- `myFIFO`: one clock. Ports `clk`, `sclr`, `data`, `wrreq`, `full`, `q`,
  `rdreq`, `empty`, `almost_empty`, `usedw`; parameters `WORD`, `LENGTH`,
  `ALMOST`. It holds `LENGTH - 1` words.
- `myDCFIFO`: two clocks, ports in the style of the Quartus `dcfifo`:
  `aclr`; `wrclk`, `data`, `wrreq`, `wrfull`, `wrusedw` on the writer's
  side; `rdclk`, `rdreq`, `q`, `rdempty`, `rdalmost_empty`, `rdusedw` on the
  reader's. It holds `LENGTH` words (a power of two); a word shows up on the
  reader's side 2 to 3 reader cycles after it is written. Hold `aclr` for a
  few cycles of the slower clock.

In both, the word on `data` is stored in the cycle `wrreq` is 1, `q` shows
the oldest word, and `rdreq` moves to the next. Wired to two SAPHOs (as the
regress does in `Compilers/CMMComp/Tests/FIFO`):

```verilog
// writer: out() on port 0
fifo_w W (.clk(clk_a), .rst(rst_a), .out(w_out), .out_en(w_en));
myDCFIFO #(.WORD(16), .LENGTH(16)) D (.aclr(rst),
    .wrclk(clk_a), .data(w_out), .wrreq(w_en[0]), .wrfull(full),
    .rdclk(clk_b), .rdreq(r_req[0]), .rdempty(empty), .q(q));
// reader: in(1) is the empty flag, in(0) takes the oldest word
assign r_in = r_req[1] ? {15'd0, empty} : q;
fifo_r R (.clk(clk_b), .rst(rst_b), .in(r_in), .req_in(r_req), .out(r_out), .out_en(r_en));
```

The reader waits with `while (in(1)) { }` and takes a word with
`x = in(0);`. A writer that may outrun the reader reads the full flag the
same way before each `out()`.

Checked by `Scripts/hw/tb_fifo.v` and `Scripts/hw/tb_dcfifo.v`.

## Keyboard: `myKeyPS2`

A PS/2 keyboard read with `cade()`: the block takes the keyboard's two
wires and hands the program one character per key press, ready to use.

```c
int c = cade_sync(0);     // waits for a key; c is its character
```

**What the program gets.** Only presses (holding a key repeats it, the
keyboard's own repeat). Every word is a character: the printable ASCII of
the key's position on the chosen layout, Shift included, or Enter = 10,
Backspace = 8, Tab = 9, Esc = 27. Keys without an ASCII character give
nothing: the arrows, F1 to F12, the numeric keypad, Caps Lock (only Shift
makes capitals), and on ABNT2 the `ç`, the acute `´` and the diaeresis
`¨`. ABNT2's accent keys give the accent itself (`~`, `^`, `` ` ``), with
no composition.

**Parameters.**

| parameter | default | meaning |
|---|---|---|
| `LAYOUT` | `"ABNT2"` | `"ABNT2"` or `"US"`: which symbol each key position gives |
| `NUBITS` | 32 | width of the word `cade()` returns (the program's `#NUBITS`, at least 8) |
| `DEPTH` | 4 | characters the block holds while the program does not read; one more is dropped |
| `CLK_KHZ` | 25000 | the clock in kHz, only to count a 1 ms timeout (a wrong value just changes it) |

**Pins.**

| pin | direction | connect to |
|---|---|---|
| `clk`, `rst` | in | the processor's clock and reset |
| `ps2_clk`, `ps2_dat` | in | the keyboard (inputs only: nothing is sent to the keyboard) |
| `taqui` | out | the processor's `taqui_k` |
| `dado` | out | the processor's `cade_dado_k` |
| `cade` | in | the processor's `cade_k` |

**Wiring**, link 0, on the processor's clock:

```verilog
nucleo sapho (.clk(clk), .rst(rst), .in(in), .out(out), .req_in(req_in), .out_en(out_en),
              .cade_0(cade_0), .taqui_0(taqui_0), .cade_dado_0(cade_dado_0));

myKeyPS2 #(.LAYOUT("ABNT2"), .CLK_KHZ(25000)) kbd (
    .clk(clk), .rst(rst), .ps2_clk(PS2_CLK), .ps2_dat(PS2_DAT),
    .taqui(taqui_0), .dado(cade_dado_0), .cade(cade_0));
```

The same in C± and in C++: `c = cade_sync(0);`. With the block on another
clock than the processor, the program uses the plain `cade(0)` instead,
whose link synchronizes the two clocks ([Several processors](multiprocessor.md)).

**Clock.** The link is a handshake, not a timing, so the processor's clock
does not have to be any particular one: the testbench runs the block at 2
and at 25 MHz. The block samples the keyboard's 10 to 17 kHz clock, so keep
the processor well above that (a few MHz). `CLK_KHZ` only sizes the 1 ms
after which a frame cut in the middle (noise, a keyboard plugged in with the
board on) is forgotten.

**Size**, measured with Quartus 24.1 on a Cyclone IV E (DE2-115), `DEPTH` 4:
233 logic elements and 92 registers, no memory bits. The queue stays in
flip-flops: the read is registered, so a synthesizer may use a memory
block, but Quartus keeps memories this small in logic.

**An 8-bit processor is enough**: a character fits in 8 bits. The regress
runs the block on two SAPHOs with `#NUBITS 8` (`Compilers/CMMComp/Tests/KEYPS2`):
a C± one on a US keyboard and a C++ one on an ABNT2 keyboard, both echoing
`cade_sync(0)` to a port. In C++ the word width is `#pragma yanc nubits 8`.

Checked by `Scripts/hw/tb_keyps2.v`: every key of both layouts with and
without Shift, releases, extended keys, a bad parity, a cut frame, a full
queue. Design and decisions: [design/ps2-keyboard.md](design/ps2-keyboard.md).
