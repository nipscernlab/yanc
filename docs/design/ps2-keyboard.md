# A PS/2 keyboard block for the SAPHO, read with `cade()`

Status: implemented 2026-10-19 (`SAPHO/myKeyPS2.v`, `Scripts/hw/tb_keyps2.v`,
regress pass `KEYPS2`); the board check on the DE2-115 with Dênis is left.
Marks: **fact** (read in code or measured), **deduced** (to be checked on a
board), **decided** (Luciano).

## 1. Where it comes from

Dênis (IC student) has a SAPHO on a DE2-115 reading a PS/2 keyboard and
writing on a VGA text screen (project received 2026-10-19). Facts from his
project:

- The keyboard path is in Verilog: two-flop synchronizer, 11-bit frame with
  start/parity/stop check, a 16-byte FIFO; the SAPHO polls `in(1)` (queue not
  empty) and reads `in(2)` (oldest scancode, popped by `req_in[2]`).
- The program (C++) turns scancodes into characters: the `F0` (release) and
  `E0` (extended) prefixes, the Shift state, Caps Lock as a toggle, two
  132-entry tables (lower/upper case).
- **Fact (code reading, not tried on the board):** his `E0` handling skips
  the byte after `E0`, so a release `E0 F0 5A` (keypad Enter) loses the `F0`
  and `5A` arrives as a press: releasing keypad Enter prints a newline.
- **Fact (measured with yanc 8.0):** the two tables cost 819 of the 1,012
  instructions (cppcomp fills initialized arrays with code, three instructions
  per element, zeros included) and 264 data words: about 3/4 of the
  processor's memory.

## 2. Decided

1. **A block in `SAPHO/`, like `myFIFO.v`**: on one side the PS/2 pins, on
   the other the `cade` link pins (`taqui`, `cade_dado`, from the SAPHO
   `cade`). The program reads a key with `c = cade(k);` (Luciano).
2. **The block delivers the character ready**: Shift, `E0`/`F0`
   and the table live in the hardware, "a circuit takes care of whatever it
   can" (Luciano). The program does not parse scancodes.
3. **The layout is a parameter, `LAYOUT` = `"ABNT2"` (the default, Luciano) or `"US"`** (Luciano,
   2026-10-19; replaces "only the keys common to both", decided earlier the
   same day: without `; : / ? [ ] { } ' " | \ ~ ^` and the backquote, nobody
   can type a C program). Each key gives the symbol of its position on the chosen
   layout. ABNT2's accent keys give the symbol itself, no composition
   (`~`, `^`, `` ` ``). Characters outside ASCII (`ç`, `´`, `¨`) give
   nothing, by decision 6. **Deduced:** the ABNT2 positions below are from
   memory of the layout, to be checked on a real ABNT2 keyboard.

   | scancode | US | US + Shift | ABNT2 | ABNT2 + Shift |
   |---|---|---|---|---|
   | 0E | `` ` `` | `~` | `'` | `"` |
   | 36 (6) | `6` | `^` | `6` | none (`¨`) |
   | 54 | `[` | `{` | none (`´`) | `` ` `` |
   | 5B | `]` | `}` | `[` | `{` |
   | 4C | `;` | `:` | none (`ç`) | none (`Ç`) |
   | 52 | `'` | `"` | `~` | `^` |
   | 5D | `\` | `\|` | `]` | `}` |
   | 4A | `/` | `?` | `;` | `:` |
   | 61 (ISO, beside Z) | `\` | `\|` | `\` | `\|` |
   | 51 (ABNT2 only) | none | none | `/` | `?` |

   Every other key of the list (letters, digits, `- _ = + , < . >`, space,
   Enter, Backspace, Tab, Esc, the other shifted digits) is the same on both.
4. **Burst is not needed** here (TODO 28): a few tens of scancodes per
   second; the handshake matters, not the rate.
5. **Presses only**: a release never reaches the program, and there is no
   parameter to ask for releases (Luciano). Holding a key repeats it, by the
   keyboard's own typematic repeat.
6. **Keys without a character are dropped, for now** (Luciano): arrows,
   F1-F12, Home/End/Delete and the layout-dependent keys of decision 3 never
   reach the program. Every word `cade()` returns is a character: printable
   ASCII, or Enter = 10, Backspace = 8, Tab = 9, Esc = 27. Delivering them
   later (a marker bit with a key code was proposed) does not change the
   words a program gets today, only adds new ones.
7. **No Caps Lock, for now** (Luciano): the key is ignored, only Shift
   gives capitals. No hidden state, nothing sent back to the keyboard: the
   PS/2 pins stay inputs and the block has no transmitter. (Caps Lock with
   its LED needs the host-to-device command `ED`, open-drain `inout` pins and
   a transmitter about the size of the receiver; Dênis has one, `ps2_tx.v`.)
8. **The numeric keypad is dropped, for now** (Luciano): only the digit row
   types digits; the keypad keys (and keypad Enter, `E0 5A`) give nothing.
   (Its `/ * - +` are where they are on both layouts: an easy addition.)
9. **Name `SAPHO/myKeyPS2.v`**, module `myKeyPS2` (Luciano).
10. **Queue size: parameter `DEPTH`, default 4, read synchronously**
    (Luciano, 2026-10-19; first 16 with an unregistered read). Measured on the
    DE2-115: 16 characters read without a register took 128 flip-flops and
    the block 364 LEs, as much as the SAPHO; with a registered read the
    synthesizer may put the queue in a memory block when there is memory to
    spare, and DEPTH 4 keeps it at a few registers when there is not.
11. **A full queue drops the new character** (Luciano): what was typed
    first arrives whole, the excess is lost until the program reads.
12. **A frame timeout** (Luciano): `ps2_clk` still for about 1 ms in the
    middle of a frame restarts the bit count, so a lost bit (noise, a
    keyboard plugged in with the board on) costs one byte, not every byte
    until reset. Hence the parameter `CLK_KHZ`, to count 1 ms.
13. **The block runs on the SAPHO's clock**, read with `cade_sync(k)` (no
    link synchronizer): the case tested and documented as the default
    (Luciano). A block on another clock works with the plain `cade(k)`,
    the same block.
14. **Claude writes the block, its testbench and the whole-processor test**;
    Dênis gets it tested and explained step by step, and checks it on the
    board (Luciano).

## 3. Specification (as implemented)

**File and module.** `SAPHO/myKeyPS2.v`, module `myKeyPS2` (name: Luciano), beside
`myFIFO.v`/`myDCFIFO.v`.

**Parameters.** `NUBITS` (32): width of the word handed to `cade()`, the
character zero-extended. `DEPTH` (4): characters the queue holds. `CLK_KHZ`
(50000): the clock, only to size the frame timeout below.

**Pins.**

| pin | dir | to |
|---|---|---|
| `clk`, `rst` | in | the SAPHO's clock and reset (one clock) |
| `ps2_clk`, `ps2_dat` | in | the keyboard (inputs only: nothing is sent back) |
| `taqui` | out | the SAPHO's `taqui_k` (the block's T bit) |
| `dado` [NUBITS] | out | the SAPHO's `cade_dado_k` |
| `cade` | in | the SAPHO's `cade_k` (the program's C bit) |

**What the program writes.** `c = cade_sync(k);` (same clock, no
synchronizer); plain `cade(k)` also works. The word is always a character:
`a-z`, `A-Z` with Shift, `0-9`, the shifted digits but `^`, space, `- _ = +
, < . >`, Enter = 10, Backspace = 8, Tab = 9, Esc = 27.

**Inside, in order.**
1. Two flip-flops on `ps2_clk`/`ps2_dat`, falling-edge detect (as Dênis).
2. Receiver: 11-bit frame, start/parity/stop checked, a bad frame dropped
   (as Dênis). **New:** a timeout (no `ps2_clk` edge for about 1 ms
   restarts the bit count), so one lost bit cannot shift every later frame;
   Dênis's receiver has none.
3. Decoder: remembers `F0` (next code is a release) and `E0` (next code is
   extended), and the two Shift keys held. A release only clears Shift; an
   extended key, a keypad key, Caps Lock and every key outside the list
   give nothing. The `E0 F0 5A` case of section 1 is handled by keeping both
   flags.
4. Table: a ROM, scancode -> (character, character with Shift), 0 = none.
5. Queue of `DEPTH` characters; a character arriving with the queue full is
   dropped.
6. Link: while the queue is not empty and T == C (the last word was taken),
   put the oldest character on `dado` and flip T; when C flips back to T,
   pop it. `dado` holds still while T != C.

**Tests.** `Scripts/hw/tb_keyps2.v`: PS/2 frames in (bit timing as a
keyboard), the link driven as `CAD` does, characters checked out: every key
of the list with and without Shift, `E0` keys and the `E0 F0 5A` release,
releases, a bad parity, a lost bit (timeout), a full queue. A regress pass
runs it. Then a whole-processor case: a C+- and a C++ program that echo
`cade_sync(0)` to `out(0)`, with the block in the testbench top. Board check
on the DE2-115 with Dênis.

## 4. What was measured and changed while building it

- The queue was first read without a register: 16 characters took 128
  flip-flops and the block 364 LEs. With a registered read and `DEPTH` 4:
  233 LEs, 92 registers (Quartus 24.1, Cyclone IV E). Quartus keeps memories
  this small in logic ("uninferred due to inappropriate RAM size"), at 4 and
  at 16; the registered read lets a synthesizer choose.
- A guard against reading a stale word in the cycle a word is written was
  written, then removed: the testbench passed without it, and the reason is
  structural (the program reads `dado` only after T flips, one cycle after
  the queue changes at the earliest).
- Quartus refuses a `"\\"` string escape that Icarus accepts: the backslash
  and the double quote are `8'h5C` and `8'h22` in the table.
- Dênis's project rebuilt with the block, yanc 8.0 and a 25 MHz PLL for
  everything: 7,792 -> 831 LEs, Fmax 53.4 MHz (was 67.6 MHz at 50 MHz with
  no `.sdc`), the worst path inside the SAPHO.
- An 8-bit echo found a processor defect unrelated to the keyboard: the
  port number was cut to the data-address width (fixed in 849f5cc).

## 5. For the documentation (item 21 in TODO.md)

Written: docs/peripherals.md (pins, parameters, the words `cade()` returns,
the key list, wiring, the clock, the size, 8 bits).
