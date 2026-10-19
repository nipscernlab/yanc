# Design notes

The record behind each feature: the problem, the options weighed, what was
decided and why, what was measured, what is still open. They are written
for whoever develops YANC; the user-facing text is the manual
([../index.md](../index.md)). Each note marks what is a fact (read in the
code), what was measured and what was deduced, and most end with a section
"For the documentation" listing what the manual has to say.

## Rules for `SAPHO/`

- **Pay only for what you use.** Every block exists only when an opcode of
  the program needs it: a `generate` on the opcode parameters, never left
  to the synthesiser to prune (measured: it does not, precision review 2026-09-15).
- **No mux input at don't care (Luciano, 2026-10-16).** Every multiplexer
  is guided by the opcode parameters: each input is a term or a row inside
  the `generate` (or guard) of the operation that uses it, and what is not
  generated is a constant 0. No `x` default, no `x` arm, no `x` on an
  absent operator's output. An unused `x` input still costs logic (Yosys:
  +21 % in a float `ula_mux`, pipeln-and-division.md 4.3).

| note | about |
|---|---|
| [cli-v7.1.md](cli-v7.1.md) | the v7.1 command line, the `yanc` driver and the output folder, against v6.0 |
| [toma-and-cade.md](toma-and-cade.md) | two SAPHOs passing a value: the protocol, the single-link design (v7.0, superseded in part) |
| [toma-and-cade-links.md](toma-and-cade-links.md) | several links per processor: chains, fan-out, fan-in, two clocks |
| [myfifo.md](myfifo.md) | `myFIFO` fixed, the two-clock `myDCFIFO` |
| [ps2-keyboard.md](ps2-keyboard.md) | `myKeyPS2`: a PS/2 keyboard read with `cade_sync`, the decisions and what was measured (v9.0) |
| [ip-protection.md](ip-protection.md) | handing SAPHO to Quartus or Vivado without showing its source: IEEE 1735, vendor schemes, a service (notes, parked until a partnership) |
| [burst.md](burst.md) | a whole array through a port, one word per clock, stopping while the FIFO is full (design, not implemented) |
| [asm-names.md](asm-names.md) | user names that collide in the assembly |
| [pipeln-and-division.md](pipeln-and-division.md) | the slow dividers: the pipeline tried and dropped, the register cuts kept (5 and 4 stages since 2026-10-16, section 4.4) |
| [normalisation-survey.md](normalisation-survey.md) | the known ways to normalise a float on an FPGA: eight built, two fitted, none faster; finished, `pnorm` kept |
| [fadd-survey.md](fadd-survey.md) | the known ways to align the exponents and add; the add as a separate circuit built, not fitted; finished, `pnorm` kept |
| [precision-and-width-review.md](precision-and-width-review.md) | the float and word-width review (2026-09): the evidence behind TODO items 1 to 9 |
| [hdl-architecture-audit.md](hdl-architecture-audit.md) | the HDL audit: the evidence behind TODO item 10 |
