# `SAPHO/myFIFO.v`: three defects (found 2026-10-09, not fixed)

`myFIFO` replaces the Quartus FIFO IP. It is meant for passing a buffer of
words between SAPHOs (or between a SAPHO and other logic) when word-by-word
`toma`/`cade` (docs/toma-and-cade.md) is not what the design needs. The only
user in this repository is the `DTW` project
(`Compilers/CMMComp/Tests/DTW/TopLevel/top_level.v`), where the FIFO is fed
by the testbench, not by a SAPHO.

Suspected while designing `toma`/`cade` (docs/toma-and-cade.md section 7,
"not simulated"); confirmed in simulation on 2026-10-09. TODO.md item 22.

## The defects

The code (as of commit 50cc7a0):

```verilog
reg wr=0; always @ (posedge clk) if (sclr) wr <= 1'b0; else wr <= wrreq;
...
else if (wr & !full) begin mem[addr_w] <= data; addr_w <= addr_w+1'b1; end
...
assign full  = (addr_w+1'b1 == addr_r) &  wrreq;
assign empty = (addr_w      == addr_r) & ~wr;
```

1. **The word written is the one on `data` one cycle AFTER `wrreq`.** The
   write uses `wr`, which is `wrreq` delayed by one cycle, and samples `data`
   in that later cycle. A SAPHO's `out` bus holds a value only in the cycle
   `out_en` is high (it is the ALU output; the next instruction changes it).
   - Measured with a SAPHO writer (`out(0,1); out(0,2); out(0,3);
     out(0,4);` then `out()` in loops, 12 values, `wrreq = out_en[0]`,
     `data = out`): the bus was right in every `wrreq` cycle, the FIFO held
     `2 3 4 5 5 6 7 8 11 12 13 14` instead of `1 2 3 4 5 6 7 8 11 12 13
     14`. Values written from a loop came out right, back-to-back `out()`
     did not: whether it works depends on the instruction the compiler puts
     after `OUT`.
2. **A write into a full FIFO is not refused, and everything stored is
   lost.** `full` is gated by the current `wrreq`, but the write happens a
   cycle later with `wr`, when `wrreq` is usually low again: `full` reads 0,
   the write goes in, `addr_w` reaches `addr_r` and the FIFO says it is
   empty. Measured (LENGTH 4, three words in, a fourth write): 0 words
   left instead of `1 2 3`. Also, a writer that looks at `full` before
   writing sees 0 on a full FIFO whenever its `wrreq` is low.
3. **A read in the cycle after a write into an empty FIFO returns a stale
   word.** `empty` falls when `wr` rises, before the word is in memory; a
   read in that cycle takes the old content of the slot and moves `addr_r`
   past it. Measured: wrote 7, read 1 (left over from an earlier case), and
   the 7 was lost.

Why the DTW project never showed them (deduced from its testbench,
`top_level_tb.v`): it writes one word every two cycles and holds `data` for
both, so defect 1 is masked, and it never fills the FIFO.

## How to reproduce

`Scripts/hw/tb_fifo.v` checks the three cases and prints PASS/FAIL (today:
3 of 3 FAIL):

```
iverilog -g2012 -s tb_fifo -o f.vvp Scripts/hw/tb_fifo.v SAPHO/myFIFO.v && vvp -n f.vvp
```

The SAPHO-writer measurement of defect 1 used a scratch testbench (a
generated processor with `out` into the FIFO, then the testbench drains it);
it is not in the repository. If the fix goes in, it becomes a regress
fixture: a C+- writer, the FIFO, and the expected sequence.

## Proposed fix (not applied; Luciano decides)

Write in the `wrreq` cycle, and make `full`/`empty` depend on the pointers
only (the behaviour of the Quartus `scfifo` it replaces, show-ahead read):

```verilog
always @ (posedge clk) begin
	if (sclr)
		addr_w <= 0;
	else if (wrreq & !full) begin
		mem[addr_w] <= data;
		addr_w <= addr_w+1'b1;
	end
end
...
assign full  = (addr_w+1'b1 == addr_r);
assign empty = (addr_w      == addr_r);
assign usedw =  addr_w - addr_r;
```

Checked in scratch on 2026-10-09: `tb_fifo.v` 3 of 3 PASS, and the SAPHO
writer gives `1 2 3 4 5 6 7 8 11 12 13 14`. Not checked: the DTW project
(its testbench changes `data` with a blocking assignment at the same
`posedge` where `wrreq` rises, so with a same-cycle write its golden may
move; to be run and looked at before the fix is committed). LENGTH still
holds LENGTH-1 words (one slot tells full from empty), as today.

Other notes on this module (docs/hdl-architecture-audit.md): the read is
combinational (`q = mem[addr_r]`), which keeps it out of block RAM.

## For the documentation (item 21 in TODO.md)

Once fixed, the user-facing text says: wire `wrreq` to the writer's
`out_en` bit and `data` to its `out`; the word is taken in that cycle;
`full` and `empty` are valid at any time; capacity is LENGTH-1 words.
