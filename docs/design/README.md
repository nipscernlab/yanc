# Design notes

The record behind each feature: the problem, the options weighed, what was
decided and why, what was measured, what is still open. They are written
for whoever develops YANC; the user-facing text is the manual
([../index.md](../index.md)). Each note marks what is a fact (read in the
code), what was measured and what was deduced, and most end with a section
"For the documentation" listing what the manual has to say.

| note | about |
|---|---|
| [cli-v7.1.md](cli-v7.1.md) | the v7.1 command line, the `yanc` driver and the output folder, against v6.0 |
| [toma-and-cade.md](toma-and-cade.md) | two SAPHOs passing a value: the protocol, the single-link design (v7.0, superseded in part) |
| [toma-and-cade-links.md](toma-and-cade-links.md) | several links per processor: chains, fan-out, fan-in, two clocks |
| [myfifo.md](myfifo.md) | `myFIFO` fixed, the two-clock `myDCFIFO` |
| [ip-protection.md](ip-protection.md) | handing SAPHO to Quartus or Vivado without showing its source: IEEE 1735, vendor schemes, a service (notes, parked until a partnership) |
| [burst.md](burst.md) | a whole array through a port, one word per clock, stopping while the FIFO is full (design, not implemented) |
| [asm-names.md](asm-names.md) | user names that collide in the assembly |
| [pipeln-and-division.md](pipeln-and-division.md) | the slow dividers: the pipeline tried and dropped, the two register cuts kept |
| [precision-and-width-review.md](precision-and-width-review.md) | the float and word-width review (2026-09): the evidence behind TODO items 1 to 9 |
| [hdl-architecture-audit.md](hdl-architecture-audit.md) | the HDL audit: the evidence behind TODO item 10 |
