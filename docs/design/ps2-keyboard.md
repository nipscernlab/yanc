# A PS/2 keyboard block for the SAPHO, read with `cade()`

Status: implemented 2026-10-19 (`SAPHO/myKeyPS2.v`, `Scripts/hw/tb_keyps2.v`,
regress pass `KEYPS2`); checked on the board by Dênis in his keyboard + VGA
example (Luciano, 2026-10-27).
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

Added for v9.1, after an audit of the block (Luciano, 2026-10-20: "implement
everything you suggested"; what was left for later is in GitHub issue #3):

15. **`ps2_clk` is filtered**: the synchronized wire must hold a new level
    for `FILT` = 8 cycles before it counts (320 ns at 25 MHz; the keyboard
    holds each level 30 us at least). The lines are open drain with
    pull-ups; a slow or ringing edge crossing the threshold twice made an
    extra falling edge, one bit too many, the frame lost. Replaces the
    plain edge detect of item 1 of the specification (Dênis's).
16. **Ctrl gives control codes**: either Ctrl (left `14`, right `E0 14`)
    held, a letter gives 1 to 26 (Ctrl+A = 1), Shift or not; Ctrl with any
    other key gives nothing. Ctrl is the second state after Shift; still
    nothing is sent back to the keyboard (decision 7).
17. **On reset T takes the program's C** (`t <= cade`, nothing offered):
    resetting the block alone while C = 1 made T != C at once, and the
    program read a character nobody typed.
18. **An output `tem_tecla`** = the queue is not empty, for an input port:
    the program asks "is there a key?" without blocking in `cade_sync`.
19. **The codes are shared with the screen**: 0-127 ASCII; 128-159 screen
    commands, never from the keyboard; 160-255 for the keyboard's special
    keys (arrows, F-keys, later; decision 6), which the screen always
    ignores. Today the block gives 1-127 only.

## 3. Specification (as implemented)

**File and module.** `SAPHO/myKeyPS2.v`, module `myKeyPS2` (name: Luciano), beside
`myFIFO.v`/`myDCFIFO.v`.

**Parameters.** `NUBITS` (32): width of the word handed to `cade()`, the
character zero-extended. `DEPTH` (4): characters the queue holds. `CLK_KHZ`
(25000): the clock, only to size the frame timeout below.

**Pins.**

| pin | dir | to |
|---|---|---|
| `clk`, `rst` | in | the SAPHO's clock and reset (one clock) |
| `ps2_clk`, `ps2_dat` | in | the keyboard (inputs only: nothing is sent back) |
| `taqui` | out | the SAPHO's `taqui_k` (the block's T bit) |
| `dado` [NUBITS] | out | the SAPHO's `cade_dado_k` |
| `cade` | in | the SAPHO's `cade_k` (the program's C bit) |
| `tem_tecla` | out | optional, to an input port: 1 while the queue is not empty |

**What the program writes.** `c = cade_sync(k);` (same clock, no
synchronizer); plain `cade(k)` also works. The word is always a character:
`a-z`, `A-Z` with Shift, `0-9`, the shifted digits but `^`, space, `- _ = +
, < . >`, Enter = 10, Backspace = 8, Tab = 9, Esc = 27, and 1-26 for
Ctrl+letter (decision 16).

**Inside, in order.**
1. Two flip-flops on `ps2_clk`/`ps2_dat`; `ps2_clk` filtered (a level counts
   after 8 cycles, decision 15) and its falling edge taken from the filtered
   level.
2. Receiver: 11-bit frame, start/parity/stop checked, a bad frame dropped
   (as Dênis). **New:** a timeout (no `ps2_clk` edge for about 1 ms
   restarts the bit count), so one lost bit cannot shift every later frame;
   Dênis's receiver has none.
3. Decoder: remembers `F0` (next code is a release) and `E0` (next code is
   extended), the two Shift keys and the two Ctrl keys held. A release only
   clears Shift or Ctrl; an
   extended key, a keypad key, Caps Lock and every key outside the list
   give nothing. The `E0 F0 5A` case of section 1 is handled by keeping both
   flags.
4. Table: a ROM, scancode -> (character, character with Shift), 0 = none.
5. Queue of `DEPTH` characters; a character arriving with the queue full is
   dropped.
6. Link: while the queue is not empty and T == C (the last word was taken),
   put the oldest character on `dado` and flip T; when C flips back to T,
   pop it. `dado` holds still while T != C. On reset T takes C (decision 17).

**Tests.** `Scripts/hw/tb_keyps2.v`: PS/2 frames in (bit timing as a
keyboard), the link driven as `CAD` does, characters checked out: every key
of the list with and without Shift, `E0` keys and the `E0 F0 5A` release,
releases, a bad parity, a lost bit (timeout), a full queue; since v9.1 also
frames whose `ps2_clk` rings at both edges (glitches of 2 cycles), Ctrl
(Ctrl+c = 3, Ctrl+Shift+a = 1, Ctrl+h = 8, Ctrl+1 and Ctrl+Enter nothing,
right Ctrl `E0 14`), `tem_tecla` with the queue full and empty, and a reset
of the block alone with C = 1. A regress pass
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
- v9.1, mutation-checked (2026-10-20): with `FILT` = 1 (no filter) the
  ringing frames decode wrong (5 errors); with the old reset (`t <= 0`) a
  reset of the block alone hands the program a phantom character.
- v9.1 refit of the DE2-115 system (2026-10-20, one seed): the block went
  from 233 LEs / 92 registers to 294 / 99 (filter, Ctrl). With Ctrl written
  as `k - 96` the worst path of the whole system became the table plus that
  subtraction (frame -> ch, Fmax 88.0 MHz); taking the letter's low 5 bits
  instead (`a` = 0x61) brought it to 100.4 MHz. That was one seed: three
  seeds later gave 88 to 100 MHz, the table still the limit (vga-text.md,
  section 4).

## 5. For the documentation (issue #40)

Written: docs/peripherals.md (pins, parameters, the words `cade()` returns,
the key list, wiring, the clock, the size, 8 bits); v9.1: the code table
shared with the screen, Ctrl, `tem_tecla` with its wiring, reset, the
filter.
