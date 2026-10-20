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
| `myVGAText` | a VGA text screen, 80 x 30 characters | `toma_sync(k, w)` |

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

## Character codes: keyboard and screen

The keyboard and the screen share one table of codes, so a program can
pass what `cade_sync()` returns straight to `toma_sync()`:

| codes | what they are | keyboard | screen |
|---|---|---|---|
| 0 to 127 | ASCII | gives them | draws 32 to 126, obeys 8, 9, 10, ignores the rest |
| 128 to 159 | screen commands | never gives them | 128 clear, 129 cursor; 130 to 159 kept for later commands, ignored now |
| 160 to 255 | the keyboard's special keys | none yet (arrows, F-keys... later) | always ignored |

A code the screen ignores costs the program one `toma` and nothing else,
so an echo loop `toma_sync(1, cade_sync(0));` stays correct when the
keyboard learns new keys.

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

**Ctrl.** With either Ctrl held, a letter gives its control code, as on a
terminal: Ctrl+A = 1 up to Ctrl+Z = 26, with or without Shift (so Ctrl+H,
Ctrl+I and Ctrl+J are 8, 9 and 10, the same as Backspace, Tab and Enter).
Ctrl with any other key gives nothing.

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
| `tem_tecla` | out | optional: 1 while a character waits; to an input port |

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

**Is there a key?** `cade_sync(0)` waits until a key comes. A program that
has other work to do wires `tem_tecla` to an input port and reads it first:

```verilog
assign in = tem;                     // port 0: 1 while a character waits
```

```c
if (in(0)) c = cade_sync(0);         // takes a key only when there is one
```

**Reset.** On reset the block's T bit takes the program's C bit, so the
block can be reset alone, with the processor running, without handing the
program a character nobody typed. The queue is emptied.

**Clock.** The link is a handshake, not a timing, so the processor's clock
does not have to be any particular one: the testbench runs the block at 2
and at 25 MHz. The block samples the keyboard's 10 to 17 kHz clock, so keep
the processor well above that (a few MHz). `ps2_clk` is filtered: a new
level counts only after it holds for 8 cycles of `clk` (320 ns at 25 MHz),
so a slow or ringing edge on the open-drain line does not count twice.
`CLK_KHZ` only sizes the 1 ms after which a frame cut in the middle (noise,
a keyboard plugged in with the board on) is forgotten.

**Size**, measured with Quartus 24.1 on a Cyclone IV E (DE2-115), `DEPTH` 4:
294 logic elements and 99 registers, no memory bits. The queue stays in
flip-flops: the read is registered, so a synthesizer may use a memory
block, but Quartus keeps memories this small in logic.

**An 8-bit processor is enough**: a character fits in 8 bits. The regress
runs the block on two SAPHOs with `#NUBITS 8` (`Compilers/CMMComp/Tests/KEYPS2`):
a C± one on a US keyboard and a C++ one on an ABNT2 keyboard, both echoing
`cade_sync(0)` to a port. In C++ the word width is `#pragma yanc nubits 8`.
With the screen block, a whole keyboard-and-screen system fits in an 8-bit
SAPHO too (regress pass `VGATEXT`).

Checked by `Scripts/hw/tb_keyps2.v`: every key of both layouts with and
without Shift, releases, extended keys, a bad parity, a cut frame, a full
queue, frames whose `ps2_clk` rings at every edge, Ctrl with both Ctrl keys,
`tem_tecla`, a reset of the block alone. Design and decisions: [design/ps2-keyboard.md](design/ps2-keyboard.md).

## Screen: `myVGAText`

A VGA text screen written with `toma()`: 640 x 480 at 60 Hz, 80 columns by
30 lines of 8 x 16 characters, white on black, with a blinking underline
cursor. The program sends one word at a time, and the block shows it.

```c
toma_sync(1, 'A');        // writes A at the cursor
toma_sync(1, 128);        // clears the screen (the program waits until it is done)
```

**The words.**

| words | effect |
|---|---|
| 32 to 126 | the character, at the cursor; the cursor advances (past column 79, the next line) |
| 10 | column 0 of the next line |
| 8 | the cursor back one column, a space written there; nothing at column 0 |
| 9 | the next column that is a multiple of 8 |
| 128 | clear: the whole screen blank, the cursor at line 0, column 0 |
| 129, line, column | three words: the cursor at that line and column (out of range gives 0) |
| anything else | ignored: 130 to 159 are kept for later commands, 160 to 255 are the keyboard's special keys ([codes](#character-codes-keyboard-and-screen)) |

A new line from the last line **scrolls** the screen up: the text moves one
line up and the last line comes out blank. The block takes a word only when
it can carry it out, and answers the `toma` only when it is done: one cycle
for a character, 80 for a scroll, 2,400 for a clear, and one more for the
register the word comes in through. The program never waits
on a port of its own, and no character is lost while the screen clears.

A helper in C++, as the regress program `Tests/VGATEXT/vga_cpp.cpp` does:

```cpp
void escreve(const char *s) { while (*s) toma_sync(1, *s++); }
void cursor(int linha, int coluna) { toma_sync(1, 129); toma_sync(1, linha); toma_sync(1, coluna); }
```

**Parameter.** `NUBITS` (32): the width of the word `toma()` sends, the
program's `#NUBITS`; at least 8 (the commands fit in 8 bits).

**Pins.**

| pin | direction | connect to |
|---|---|---|
| `clk`, `rst` | in | the 25 MHz pixel clock, reset (active high; reset also clears the screen; the syncs stay at 1 and the screen dark while it is held) |
| `toma` | in | the processor's `toma_k` |
| `dado` | in | the processor's `toma_dado` |
| `valeu` | out | the processor's `valeu_k` |
| `hs`, `vs` | out | the sync pulses, active low |
| `ativo` | out | 1 in the visible 640 x 480 window |
| `pixel` | out | 1 = lit |

**The board.** The outputs are logic, the same on every board; the top of
each board maps them to its converter. The DE2-115 (8 bits a colour, an
ADV7123 converter):

```verilog
assign VGA_R = {8{pixel & ativo}};
assign VGA_G = {8{pixel & ativo}};
assign VGA_B = {8{pixel & ativo}};
assign VGA_BLANK_N = ativo;
assign VGA_SYNC_N  = 1'b0;
assign VGA_CLK     = ~clk;           // the 25 MHz pixel clock, inverted
assign VGA_HS = hs;
assign VGA_VS = vs;
```

The converter latches the colour on the rising edge of `VGA_CLK`. The
block's outputs change on the rising edge of `clk`, so with `VGA_CLK = clk`
the converter samples them as they change, with only the routing delays as
margin; inverted, it samples them half a period (20 ns) later, in the middle
of the pixel. **Not checked on the board:** Terasic's own demo was not read,
and Dênis's project, with the clock not inverted, shows a picture on his
DE2-115.

A board with a resistor converter (4 bits a colour, say) takes `{4{pixel &
ativo}}` on each colour and only the sync pins.

**Clock.** The block's clock is the 25 MHz pixel clock, from a PLL on the
board. A processor on that same clock writes with `toma_sync(k, w)`; a
processor on another clock writes with the plain `toma(k, w)`, whose link
synchronizes the two clocks.

**The font** is "Modern DOS" 8x16 by Jayvee Enaguas (CC0 1.0), the ASCII
characters only, inside `myVGAText.v` itself: the block is one file, with no
data file to point at. `Scripts/gen_vga_font.py` writes it from the original
font file and says where that comes from.

**Size**, measured with Quartus 24.1 on a Cyclone IV E (DE2-115), with
`NUBITS` 8: 337 logic elements, 127 registers and 33,184 memory bits (the screen, 2,400 x 7 bits,
and the font, 2,048 x 8, both in memory blocks). A whole system with an
8-bit C± SAPHO, `myKeyPS2` and `myVGAText` takes 689 logic elements, with
the clock at up to 88 to 100 MHz (three fitter seeds; the keyboard's table
is the limit) for the 25 MHz asked. A 32-bit C++ SAPHO with `<iostream>`,
the two blocks and a PLL takes 3,174 logic elements and runs at up to 27.7
to 28.0 MHz, the limit being the SAPHO's own float ALU.

**Why the word goes through a register.** The SAPHO puts the `toma` word
straight out of its ALU. Decoded in the same cycle, the SAPHO's longest path
(data memory, float adder, normaliser) and the block's decode were one
path: that 32-bit system ran at 26.2 to 26.6 MHz, 1.8 ns of margin at 25
MHz. The register cuts it there, at one cycle more per word. The word on
`out` also comes straight from the ALU: a block of your own that decodes a
`toma` or an `out` word, rather than only storing it, should register it
first (GitHub issue #7 weighs a register inside the SAPHO).

**Reset** clears the screen (2,400 cycles). A word the program offers
meanwhile waits and is carried out after it. On reset the block's C bit
takes the program's T bit, so the block can be reset alone without taking
an old word for a new one.

Checked by `Scripts/hw/tb_vgatext.v`: the picture rebuilt from the output
pins as a monitor would (the timing, a glyph against the font, the cursor
blinking), every word and control code, the codes 130 to 255 ignored, the
cursor command, a scroll, a clear and how long each holds the program, the
outputs during and after reset, a word offered during the reset's clear, a
reset of the block alone, the screen memory never read past its end. The regress pass `VGATEXT` runs
a C± and a C++ SAPHO of 8 bits, each with a keyboard and a screen, echoing
what a simulated keyboard types, and a 32-bit C++ SAPHO that uses
`<iostream>`. Design and decisions: [design/vga-text.md](design/vga-text.md).

## Text from C++: `<iostream>` and `<cstdio>`

With the keyboard on link 0 and the screen on link 1, both on the
processor's clock (the wiring above), a C++ program needs no `cade_sync` or
`toma_sync` of its own:

```cpp
#include <iostream>
using namespace std;

int main()
{
    int n;
    cout << "n? ";
    cin >> n;                        // typed with echo and backspace, ended by Enter
    cout << "2n = " << n + n << endl;
    return 0;
}
```

`<cstdio>` gives `putchar`, `puts`, `getchar` and `fgets`. Other links, or
blocks on another clock, are set with `#define` before the `#include`. The
whole list, what `cin` and `cout` take and what they cost:
[The C++ language](language-cpp.md#text-on-the-keyboard-and-the-screen).
Design: [design/cpp-iostream.md](design/cpp-iostream.md).
