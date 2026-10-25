# The YANC manual

YANC compiles programs written in C± or C++ into a SAPHO processor: the
Verilog of a processor built for that program, its memories and a
testbench. This manual covers installing YANC, the two languages, running
the tools, simulating the result and putting several processors together.

It describes YANC 9.0. The SAPHO manual (<https://nipscern.com/library/sapho>)
covers the processor and the AURORA IDE.

## Getting started

- [Installing YANC](install.md): the release package, the simulators and
  GTKWave.
- [A first program](first-program.md): from an empty folder to a simulated processor, in C± and in
  C++.
- [The command line](command-line.md): `yanc` and the tools under it, the output folder, the
  options, `--lang`.

## Writing programs

- [The C± language](language-cmm.md): types (`int`, `float`, `comp`), arrays, functions,
  operators, control flow, input and output, the traps.
- [The C++ YANC accepts](language-cpp.md): what works, the `#pragma yanc` settings, what does
  not exist.
- [Directives](directives.md): `#NUBITS`, `#NBMANT`, `#NBEXPO` and the rest, with their
  defaults and their C++ pragmas.
- [The library](library.md): `sqrt`, `sin`, `exp`, ... and what each one costs.
- [Floating point](floating-point.md): the SAPHO float format (not IEEE 754) and the rounding
  levels of `#FROUND`.

## Several processors

- [Several processors](multiprocessor.md): the processor's pins, `toma` and `cade` over links, FIFOs,
  interrupts.

## Peripherals

- [Peripherals](peripherals.md): blocks beside the processor, wired in your
  top level: the FIFOs `myFIFO` and `myDCFIFO`, the PS/2 keyboard
  `myKeyPS2`, the VGA text screen `myVGAText`.

## Simulating

- [Simulation and the waveform](simulation.md): Icarus and Verilator,
  GTKWave and `gen_gtkw`, the runner scripts.
- [Messages](messages.md): the errors and warnings, what they mean and how to fix them.

## Reference

- [Message reference](reference/messages.md): every message the tools
  print, generated from their sources (the regress fails when it falls
  behind).
- [Instructions](reference/instructions.md): the SAPHO instruction set,
  generated from `Compilers/common/isa.tsv` (the regress fails when the page
  falls behind the table).

## Developing YANC

- [Building and testing YANC](develop/building.md): the repository, building
  the tools, the regression, deploying to AURORA.
- [Design notes](design/README.md): why each feature is built the way it is, what was
  measured, what is open.

```{toctree}
:hidden:
:caption: Getting started

install
first-program
command-line
```

```{toctree}
:hidden:
:caption: Writing programs

language-cmm
language-cpp
directives
library
floating-point
multiprocessor
```

```{toctree}
:hidden:
:caption: Peripherals

peripherals
```

```{toctree}
:hidden:
:caption: Simulating

simulation
messages
```

```{toctree}
:hidden:
:caption: Reference

reference/messages
reference/instructions
```

```{toctree}
:hidden:
:caption: Developing YANC

develop/building
design/README
design/cli-v7.1
design/toma-and-cade-links
design/toma-and-cade
design/myfifo
design/fround1-subtraction
design/ps2-keyboard
design/vga-text
design/cpp-iostream
design/burst
design/dirac-expressions
design/ip-protection
design/asm-names
design/pipeln-and-division
design/normalisation-survey
design/fadd-survey
design/precision-and-width-review
design/hdl-architecture-audit
```
