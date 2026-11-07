# A text screen through an ADV7513 HDMI transmitter: `myADV7513Text`

Status: implemented 2026-11-05 (`SAPHO/myADV7513Text.v`, `Scripts/hw/tb_adv7513text.v`,
regress pass `ADV7513TEXT`, the DE10-Nano example in `Boards/DE10-Nano/hdmi_text/`);
simulated, synthesized for the DE10-Nano, **not yet run on the board**. Marks:
**fact** (read in code or a datasheet, or measured), **deduced** (to be
checked), **decided** (Luciano).

## 1. Where it comes from

Dênis (IC student) put the VGA text screen on the HDMI output of two boards
with the same transmitter, the ADV7513: the Cyclone V GX Starter Kit
(e-mail of 2026-11-03) and the DE10-Nano (project and e-mail of 2026-11-05).
Facts from his DE10-Nano project:

- `myVGAText` is used unchanged. The ADV7513 takes `{24{pixel & ativo}}` on
  D, `ativo` on DE, HS and VS directly, and `HDMI_TX_CLK = ~clk`.
- `i2c_com.v`, a write-only I2C master on the board's 50 MHz, sets the
  ADV7513 after a 200 ms wait: 15 writes, a state machine of seven states,
  a 32-bit wait counter, a 16-bit divider (100 kHz), the ACK read and the
  whole table written again after a NACK.
- With the chip in HDMI mode (0xAF = 0x06) and 16:9 (0x17 = 0x02) the HDMI
  monitor showed nothing, while an HDMI to VGA converter showed the
  picture. In DVI mode (0xAF = 0x04) and 4:3 (0x17 = 0x00) the monitor
  shows it, with visual defects he puts down to the cable or the socket,
  not tested.
- His PLL makes 25.178511 MHz (fit report: fractional PLL, VCO 881.25 MHz,
  C = 35), not the 25.125 MHz of his e-mail: 0.014% above the 25.175 MHz of
  640x480 at 60 Hz.
- No `.sdc`: the timing from the FPGA to the ADV7513 was never checked.
- His system (32-bit C++ SAPHO with `<iostream>`, `myKeyPS2`, `myVGAText`,
  `i2c_com`, PLL), Quartus 25.1: 1,341 ALMs, 671 registers, 14 RAM blocks,
  3 DSPs, Fmax 30.2 MHz (the worst slow model, -40 °C).

## 2. Decided

1. **The same interface as `myVGAText`** (Luciano, 2026-11-05): the same
   toma link and words, the same `hs`, `vs`, `ativo`, `pixel`, so a program
   that writes to the VGA screen writes to this one unchanged and the top
   swaps the module. Two pins more, the I2C.
2. **The leanest block that shows the screen, at the highest clock it
   allows** (Luciano): cut everything the picture does not need.

The name says the chip, not HDMI (Luciano, 2026-11-06): `myVGAText` serves
any board with a VGA output, while this block serves only an ADV7513, since
its writes are that chip's registers, and what goes out is DVI. It was
`myHDMIText` until then.

What follows was decided in the implementation, within those two:

3. **`myVGAText` is instantiated, not copied.** One font, one scan, one set
   of words for both screens; a fix to the text screen reaches both. The
   area and the Fmax of the screen are `myVGAText`'s; what this block
   adds is the I2C.
4. **One clock, the pixel clock, and the I2C timed by the scan's lines.**
   Weighed: keeping the board's 50 MHz for the I2C (a second clock in the
   block, the reset crossing it), and a divider on the pixel clock for 100
   kHz (6 bits more of counter). The line rate is already there: the falling
   edge of `hs` steps one counter, one quarter of an I2C bit a line, so SCL
   runs at 7.87 kHz; I2C has no lower limit (the ADV7513 Hardware User's
   Guide gives only the 400 kHz upper one). The same counter makes the wait
   after reset (2^13 lines, 260 ms; the chip needs 200 ms after its supply)
   and counts the slots of each write: no state machine, no divider.
5. **Eleven writes instead of fifteen, `0xD6` first.** From the ADV7513
   Programming Guide (Rev. B), section 3, Table 14 and the register map:

   | write | why |
   |---|---|
   | 0xD6 = 0xC0 | HPD taken as always high (section 4.2.1): the chip can be powered up with the monitor off or unplugged, and an unplug does not power it down and reset its registers (Table 74). First, so that the power-up below takes |
   | 0x41 = 0x10 | power up; bit 4 stays at its default 1 |
   | 0x98 0x9A 0x9C 0x9D 0xA2 0xA3 0xE0 0xF9 | the fixed registers that "must be set" after power-up (Table 14); every default differs from the required value |
   | 0x16 = 0x30 | 8 bits a colour: the default colour depth, 00, is "invalid" |

   Left out, being the chip's default: 0x15 = 0x00 (input ID 0, RGB 4:4:4
   with separate syncs), 0x17 = 0x00 (syncs passed through, 4:3, DE
   generator off), 0x18 = 0x46 (colour converter off). 0xAF defaults to
   0x14: DVI, HDCP off; Dênis wrote 0x04, which clears bit 4, HDCP frame
   encryption, a bit that does nothing with HDCP off (**deduced** from the
   register description, not tested). DVI is what made his monitor work,
   and DVI sends no InfoFrames by definition, so "DVI without an AVI
   InfoFrame" is not a cause of the defects: there is no InfoFrame to miss.
6. **The ACK is not read.** Nothing could be done about a missing one but
   write again, and whether the chip took the writes shows on the monitor.
   Weighed: a sticky NACK flag for an LED (a few registers, not measured),
   which helps a board test and is not needed for the picture.
7. **`HDMI_TX_CLK = ~clk` stays, now with an `.sdc`.** The chip latches on
   the rising edge of its clock and the block's outputs change on the rising
   edge of `clk`, so the inverted clock puts the chip's sample in the middle
   of the pixel. `de10nano_hdmi.sdc` describes that forwarded clock and the
   chip's setup and hold, so TimeQuest checks the margin (section 4).
8. **The PLL: `altpll` with 141/280**, Dênis's, which Quartus maps to
   50 x 141 / 8 / 35 = 25.1786 MHz, 0.014% above the nominal 25.175 MHz
   (an `altera_pll` written by hand did not work, section 4).
9. **The example has no keyboard.** The DE10-Nano has no PS/2 port; Dênis's
   top still instantiated `myKeyPS2` with `PS2_CLK` and `PS2_DAT` undeclared
   (implicit one-bit wires: it compiled with the keyboard unconnected, and
   his program stopped at its first `cin`). The example program writes a
   test card instead (section 3).

## 3. Specification (as implemented)

**Parameters.** `NUBITS` (32) and `LINK_SYNC` (0), as `myVGAText`.
`WAIT_BITS` (13): the I2C writes start 2^WAIT_BITS lines after reset; only a
testbench lowers it (at least 11).

**Pins.** Those of `myVGAText` (`clk`, `rst`, `toma`, `dado`, `valeu`, `hs`,
`vs`, `ativo`, `pixel`), plus `scl` and `sda`, open drain (the block pulls
low or lets go; the board pulls up).

**I2C.** After the wait, write n (0 to 10) takes 32 slots of four lines:
slot 0 the START (SDA falls with SCL high), slots 1 to 27 the bits of
{0x72, ACK, register, ACK, value, ACK} (SDA set while SCL is low, SCL high
in the middle two quarters, SDA released for the ACKs), slot 28 the STOP,
slots 29 to 31 the bus free. Each write takes 4.07 ms, the eleven 44.7 ms;
the counter then stops until the next reset. Every edge is one line, 31.8
us, from the next: far above every fast-mode limit (1.3 us at most).

**The board's top** (DE10-Nano):

```verilog
assign HDMI_TX_D   = {24{pixel}};   // pixel is 0 outside the visible window
assign HDMI_TX_DE  = ativo;
assign HDMI_TX_HS  = hs;
assign HDMI_TX_VS  = vs;
assign HDMI_TX_CLK = ~clk;
```

**The example** (`Boards/DE10-Nano/hdmi_text/`): `hdmi_demo.cpp`, a C++
SAPHO of 8 bits on the pixel clock (`toma_sync`, `LINK_SYNC` 1), writes a
test card: the title on line 0, the 95 printable characters on lines 2 and
3, 80 `=` on line 5 (they must touch both edges of the picture), a counter
on line 7 that changes every 60 ms (simulated at 25 MHz), and a line on
the last screen line. The top adds the PLL and a reset synchronizer.

**Tests.** `Scripts/hw/tb_adv7513text.v`: the block against a `myVGAText` of
reference fed the same words (every output equal in every cycle, the same
screen memory, the text written); an ADV7513 played on the bus that
decodes each transaction and checks the eleven writes in order, the
master's SCL and SDA edges against the fast-mode limits, SDA released in
the ACK slots, the first START no sooner than 2^WAIT_BITS lines after
reset (262 ms at 13), nothing on the bus after the eleventh write, and all
of it again after a second reset. `tb_adv7513text.sh` runs 8 bits with
`LINK_SYNC` 1 and the real wait, and 32 bits with `LINK_SYNC` 0 and
`WAIT_BITS` 11; the regress (step 4a0) runs 8 bits, `LINK_SYNC` 1 and
`WAIT_BITS` 11. The
regress pass `ADV7513TEXT` builds `hdmi_demo.cpp` and checks its test card on
the screen memory of a whole system.

## 4. What was measured

Quartus Prime Lite 23.1std.1, 5CSEBA6U23I7, one seed (Dênis's numbers are
from Quartus 25.1). Fmax is the worst of the slow models (100 °C and -40
°C).

**The block alone** (virtual pins, `NUBITS` 32, clock asked at 500 MHz,
high-performance effort), against Dênis's pair (`myVGAText` and `i2c_com`
on its own clock) fitted the same way. Both with the repository's
`myVGAText`: his project carries an older one (v9.1, codes 128 and 129),
so the difference below is the I2C alone.

| | ALMs | registers | memory blocks | Fmax |
|---|---|---|---|---|
| `myADV7513Text` | 213 | 170 | 6 M10K | 149.0 MHz |
| of which the I2C | 20.8 | 17 | 0 | (same clock) |
| `myVGAText` + `i2c_com` | 304 | 292 | 6 M10K | 146.6 MHz (pixel), 233.1 MHz (50 MHz) |
| of which `i2c_com` | 110.5 | 139 | 0 | |

The I2C goes from 110.5 ALMs and 139 registers to 20.8 and 17 (-81% and
-88%), and the block from 304 to 213 ALMs (-30%). The Fmax is
`myVGAText`'s in both: the worst paths end at its screen memory's address
(`crow`/`top` to `waddr`, `vc` to the read address). The 2.4 MHz between
the two fits is one seed each, not a difference of the blocks (**deduced**:
the paths are the same logic; no other seeds were run).

**The DE10-Nano example** (`Boards/DE10-Nano/hdmi_text/`, 8-bit C++ SAPHO,
`myADV7513Text`, PLL, reset synchronizer): 272 ALMs, 262 registers, 8 M10K, no
DSP; the SAPHO 87.8 ALMs, `myADV7513Text` 182.8 (its I2C 21.0, 23 registers).
Fmax 88.7 MHz for the 25.18 MHz asked; the limit is the SAPHO's data memory
to its core, not the screen. Dênis's system, with the 32-bit C++ SAPHO of
his keyboard demonstration, took 1,341 ALMs and 30.2 MHz; of those,
`myVGAText` 193.2 ALMs and `i2c_com` 97.0 (his fit report). The systems
differ by their processors; the comparison of the blocks is the table
above.

**The outputs to the ADV7513.** With `de10nano_hdmi.sdc` (the forwarded
clock inverted, setup 1.0 ns, hold 0.7 ns), over every corner: setup slack
14.09 ns and hold slack 17.84 ns, of the 19.86 ns of half a pixel. No path
is left unconstrained. So the phase of `HDMI_TX_CLK` has margin inside the
FPGA; the board's traces are not in the model (**deduced** to be matched,
not checked on the schematic).

**The PLL.** `altpll` with 141/280 gives 25.178511 MHz (VCO 881.25 MHz,
N 8, C 35), as in Dênis's project. An `altera_pll` written by hand with
`output_clock_frequency0 = "25.175000 MHz"` was tried first: Quartus left
its counters unset and the clock came out at 0.877 MHz, so the example
uses `altpll`.

**Simulation.** `tb_adv7513text.sh`: 8 bits with `LINK_SYNC` 1 and the real
wait (first START 262.2 ms after reset, the writes over at 307.2 ms), and
32 bits with `LINK_SYNC` 0 and `WAIT_BITS` 11, both pass. In the whole
system the test card is on the screen 0.34 ms after reset and the counter
reaches 001 at 60.0 ms (one run of 60 ms simulated, not in the regress).

**Mutations** (each planted in a copy of the block, `WAIT_BITS` 11): a
wrong value (0x9D = 0x60), one write fewer, the ACK slot of the address
driven low, a STOP whose SDA rises with SCL low, the wait halved. All five
are caught by `tb_adv7513text.v`. A START that keeps SCL high one quarter
longer passed, rightly: it is legal I2C.

## 5. Not tested on the board

- That the ADV7513 takes the eleven writes in this order and shows the
  picture: the order, the writes left out (0x15, 0x17, 0x18, 0xAF) and the
  slow I2C are new against Dênis's tested sequence.
- The visual defects Dênis saw: their cause is not known. Of the three
  suspects, two are out by measurement or definition (the pixel clock is
  25.1785 MHz; DVI carries no InfoFrame), and the phase of `HDMI_TX_CLK` has
  margin in TimeQuest (section 4) with the board's traces taken as matched.
  What is left to try on the board: another cable and monitor, and the
  ADV7513's input clock delay (0xBA[7:5], default 000 = -1.2 ns, per the
  register map).
- An unplug and replug of the monitor after the set-up (0xD6 should keep
  the registers).
- The reset key: the screen clears and the writes are made again.
