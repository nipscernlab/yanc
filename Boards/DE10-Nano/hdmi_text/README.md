# DE10-Nano: the SAPHO's text screen on HDMI

A C++ SAPHO of 8 bits writes a test card to the HDMI output through
`SAPHO/myADV7513Text.v` (docs/peripherals.md, "Screen through an ADV7513"). Board:
Terasic DE10-Nano, Cyclone V SE 5CSEBA6U23I7, HDMI transmitter ADV7513.

| file | what it is |
|---|---|
| `hdmi_demo.cpp` | the program: the test card (title, the 95 printable characters, 80 `=` from edge to edge, a counter, a last line) |
| `de10nano_hdmi.v` | the top: PLL, reset, the SAPHO, `myADV7513Text`, the pins of the ADV7513 |
| `pll_pixel.v` | 50 MHz to 25.175 MHz (`altpll`, 141/280; Quartus reaches 25.1786 MHz) |
| `de10nano_hdmi.qsf`, `.qpf` | the Quartus project: device, files, pins |
| `de10nano_hdmi.sdc` | the clocks, and the outputs timed against the ADV7513's setup and hold |

## Build

In this folder, in the MSYS2 UCRT64 terminal (Windows) or a shell (Linux):

```sh
yanc hdmi_demo.cpp                     # writes hdmi_demo/: the processor and its .mif
quartus_sh --flow compile de10nano_hdmi
```

or open `de10nano_hdmi.qpf` in Quartus and compile. The `.qsf` reads the
SAPHO from `../../../SAPHO/` (this repository) and the `.mif` from
`hdmi_demo/` (`SEARCH_PATH`). Program `output_files/de10nano_hdmi.sof`
through the USB-Blaster II (the Mini-B port marked UB2), with the board's
MSEL switches at their factory setting.

## What to see

About a third of a second after the board is programmed, or after KEY[0] is
released, the monitor shows white text on black at 640 x 480: the title on
the first line, the characters `!` to `~` on lines 3 and 4, a line of `=`
that touches both edges of the picture, `contador: ` counting from 000, and
`linha 30 de 30` on the last line. KEY[0] clears the screen, starts the
program again and sets the ADV7513 up again.

## Not yet checked on this board

Everything here is simulated and synthesized, not run on the board: see
docs/design/adv7513-text.md, section 5.
