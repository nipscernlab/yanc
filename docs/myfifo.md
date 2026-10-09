# `SAPHO/myFIFO.v`: three defects, fixed 2026-10-09

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

## For the documentation (item 21 in TODO.md)

The user-facing text says: wire `wrreq` to the writer's `out_en` bit and
`data` to its `out`; the word is taken in that cycle; `q` shows the oldest
word and `rdreq` moves to the next; `full` and `empty` are valid at any
time; capacity is LENGTH-1 words.
