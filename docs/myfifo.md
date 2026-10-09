# `SAPHO/myFIFO.v`: three defects, fixed 2026-10-09

(The two-clock version, `myDCFIFO`, is the second half of this note.)

`myFIFO` replaces the Quartus FIFO IP. It is meant for passing a buffer of
words between SAPHOs (or between a SAPHO and other logic) when word-by-word
`toma`/`cade` (docs/toma-and-cade.md) is not what the design needs. Users
in this repository: the `DTW` project
(`Compilers/CMMComp/Tests/DTW/TopLevel/top_level.v`, fed by its
testbench) and the `FIFO` project pass (fed by a SAPHO, below).

Suspected while designing `toma`/`cade` (docs/toma-and-cade.md section 7),
confirmed in simulation and fixed on 2026-10-09 (TODO.md item 22).

## The defects (code up to commit 9c3b2d5)

```verilog
reg wr=0; always @ (posedge clk) if (sclr) wr <= 1'b0; else wr <= wrreq;
...
else if (wr & !full) begin mem[addr_w] <= data; addr_w <= addr_w+1'b1; end
...
assign full  = (addr_w+1'b1 == addr_r) &  wrreq;
assign empty = (addr_w      == addr_r) & ~wr;
```

1. **The word written was the one on `data` one cycle AFTER `wrreq`.** The
   write used `wr`, `wrreq` delayed by one cycle. A SAPHO's `out` bus holds a
   value only in the cycle `out_en` is high (it is the ALU output; the next
   instruction changes it). Measured with a SAPHO writer (`out(0,1);
   out(0,2); out(0,3); out(0,4);` then `out()` in loops, 12 values,
   `wrreq = out_en[0]`, `data = out`): the bus was right in every `wrreq`
   cycle, the FIFO held `2 3 4 5 5 6 7 8 11 12 13 14`. Values written from
   a loop came out right, back-to-back `out()` did not: it depended on the
   instruction the compiler puts after `OUT`.
2. **A write into a full FIFO was not refused, and everything stored was
   lost.** `full` was gated by the current `wrreq` while the write happened
   a cycle later with `wr`, when `wrreq` is usually low again: the write
   went in, `addr_w` reached `addr_r` and the FIFO said it was empty.
   Measured (LENGTH 4, three words in, a fourth write): 0 words left
   instead of `1 2 3`. A writer looking at `full` also saw 0 on a full FIFO
   whenever its `wrreq` was low.
3. **A read in the cycle after a write into an empty FIFO returned a stale
   word.** `empty` fell when `wr` rose, before the word was in memory.
   Measured: wrote 7, read 1 (left over from an earlier case), the 7 lost.

Why the DTW project never showed them (deduced from `top_level_tb.v`): it
writes one word every two cycles, holds `data` for both, and never fills
the FIFO.

## The fix

Write in the `wrreq` cycle; `full`, `empty` and `usedw` from the pointers
only (the behaviour of the Quartus `scfifo` it replaces, show-ahead read):

```verilog
else if (wrreq & !full) begin mem[addr_w] <= data; addr_w <= addr_w+1'b1; end
...
assign full  = (addr_w+1'b1 == addr_r);
assign empty = (addr_w      == addr_r);
assign usedw =  addr_w - addr_r;
```

Capacity stays LENGTH-1 words (one slot tells full from empty), as before.

## Checks (2026-10-09)

- `Scripts/hw/tb_fifo.v`, the FIFO alone, three cases (word of the `wrreq`
  cycle, write into a full FIFO, read right after a write): before the fix
  3 of 3 FAIL, after it 3 of 3 PASS.
- Regress `FIFO` project pass: the C+- program `Tests/fifo_w` writes into
  the FIFO (`Tests/FIFO/TopLevel/fifo_tb.v`, wires only), the testbench
  drains it; it must hold `1 2 3 4 5 6 7 8 11 12 13 14`. The same pass runs
  `tb_fifo.v`. Both checked on their own before the full regress.
- DTW project: its six outputs identical to the golden, `output_dtw_1`
  still 459908.
- Not run yet: the full regress (held by Luciano on 2026-10-09).

Other notes on this module (docs/hdl-architecture-audit.md): the read is
combinational (`q = mem[addr_r]`), which keeps it out of block RAM.

# `SAPHO/myDCFIFO.v`: the same FIFO between two clock domains (2026-10-09)

Asked by Luciano on 2026-10-09: a FIFO with one clock per SAPHO. Choices he
made: the same read as `myFIFO` (combinational, show-ahead) and the name
`myDCFIFO`, with ports in the style of the Quartus `dcfifo` it can stand in
for. `myFIFO` stays as it is for one clock (no synchronizers to pay for).

## Ports

| port | side | what |
|---|---|---|
| `aclr` | both | asynchronous clear; each side leaves it on its own clock (two flip-flops), so hold it for a few cycles of the slower clock |
| `wrclk`, `data`, `wrreq` | writer | the word on `data` is stored in the `wrreq` cycle, unless the FIFO is full |
| `wrfull` | writer | full, as the writer sees it |
| `wrusedw` | writer | words in, as the writer sees them ($clog2(LENGTH)+1 bits: it reaches LENGTH) |
| `rdclk`, `rdreq` | reader | `rdreq` takes the word shown on `q` and moves to the next |
| `q` | reader | the oldest word (show-ahead, combinational read) |
| `rdempty`, `rdalmost_empty` | reader | empty / fewer than ALMOST words, as the reader sees them |
| `rdusedw` | reader | words in, as the reader sees them |

Parameters: `WORD`, `LENGTH` (a power of two, at least 4), `ALMOST`. Unlike
`myFIFO`, it holds LENGTH words, not LENGTH-1 (one more pointer bit tells
full from empty).

## How it works

- Each side keeps its pointer (binary, one bit more than the address) and a
  Gray-code copy of it, and reads the other side's Gray pointer through two
  flip-flops on its own clock. One Gray bit changes per step, so a copy
  taken mid-change is the old value or the new one, never a mix.
- `wrfull` compares the writer's Gray pointer with the reader's copy (equal
  but for the two top bits); `rdempty` compares the reader's Gray pointer
  with the writer's copy. The copies are 2-3 cycles old, so both flags can
  stay on a little longer than true, never turn off early: nothing is
  overwritten, nothing read twice.
- A word written is visible to the reader 2-3 `rdclk` cycles later
  (measured: 2). The word is in memory from the `wrreq` edge on, before the
  reader can see the pointer that announces it, so the data needs no
  synchronizer.

## Wiring two SAPHOs (as `Tests/FIFO/TopLevel/dcfifo_tb.v` does)

```verilog
// writer: out() on port 0
fifo_w W (.clk(clk_a), .rst(rst_a), .out(w_out), .out_en(w_en));
myDCFIFO #(.WORD(16), .LENGTH(16)) D (.aclr(rst),
    .wrclk(clk_a), .data(w_out), .wrreq(w_en[0]), .wrfull(full), ...
    .rdclk(clk_b), .rdreq(r_req[0]), .rdempty(empty), .q(q), ...);
// reader: in(1) is the empty flag, in(0) takes the oldest word
assign r_in = r_req[1] ? {15'd0, empty} : q;
fifo_r R (.clk(clk_b), .rst(rst_b), .in(r_in), .req_in(r_req), .out(r_out), .out_en(r_en));
```

The reader program (`Tests/FIFO/fifo_r.cmm`) waits with
`while (in(1)) ...;` and then takes the word with `x = in(0);`. A writer
that may outrun the reader should read `wrfull` the same way before each
`out()`; `fifo_w` writes 12 words into 16 places and does not need to.

## Checks (2026-10-09)

- `Scripts/hw/tb_dcfifo.v`, the FIFO alone, 20 ns / 27.3 ns clocks both
  ways round: the word of the `wrreq` cycle, LENGTH words fit and the next
  is refused, a word reaches the reader (2 `rdclk` cycles), and 300 words
  with both sides pausing at random arrive once and in order. 4 of 4 PASS
  both ways. The testbench was checked against two broken copies: one that
  never says full (2 of 4 FAIL, one of them a stuck stream caught by the
  testbench's time limit) and one that stores the next cycle's word (4 of 4
  FAIL).
- Regress `FIFO` pass, second part: the SAPHO writer `fifo_w` and the
  SAPHO reader `fifo_r` on 20 ns and 27.3 ns, writer faster and reader
  faster; both readers put out `1 2 3 4 5 6 7 8 11 12 13 14`. The pass also
  runs `tb_dcfifo.v` both ways. Run on its own; the full regress is owed.
- Yosys (`synth`, LENGTH 128, WORD 16): synthesizes; the memory becomes
  2048 flip-flops (combinational read, as in `myFIFO`), the control 66
  flip-flops (4 of them the clear synchronizers).
- What simulation cannot show: metastability. The crossing is safe by
  construction (Gray pointers through two flip-flops), but on the board
  Quartus needs constraints on the two Gray pointer crossings: a
  `set_max_delay` (data path only) of about one period of the destination
  clock, so the Gray bits arrive close together, or the `set_false_path` +
  skew constraint the Intel `dcfifo` uses. The exact `.sdc` lines are not
  written yet; to be done with the board check of `LINKAS`
  (docs/toma-and-cade.md section 8.5).

## For the documentation (item 21 in TODO.md)

The user-facing text says: when the two SAPHOs run on different clocks use
`myDCFIFO` (ports in the table above); wire the writer's `out_en` bit to
`wrreq` and `out` to `data`, give the reader the empty flag on one input
port and `q` on another, with `rdreq` from that port's `req_in`; hold
`aclr` for a few cycles of the slower clock; a word takes 2-3 reader cycles
to show up; capacity is LENGTH words; the Quartus constraints.

## For the documentation (item 21 in TODO.md)

The user-facing text says: wire `wrreq` to the writer's `out_en` bit and
`data` to its `out`; the word is taken in that cycle; `q` shows the oldest
word and `rdreq` moves to the next; `full` and `empty` are valid at any
time; capacity is LENGTH-1 words.
