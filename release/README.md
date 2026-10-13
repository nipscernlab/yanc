# YANC binaries

YANC is the compiler suite for the SAPHO soft-core processor. It turns a C± or
C++ program into the Verilog of a SAPHO processor sized for that program, its
memory images and a testbench. Full documentation:
<https://github.com/nipscernlab/yanc>.

## Install

Run the install script once, from this folder:

| system | command | what it does |
|---|---|---|
| Windows | `install.bat` | puts `bin\` on your user PATH (no administrator rights); `uninstall.bat` takes it off |
| Linux | `./install.sh` | a launcher per tool in `~/.local/bin` (`PREFIX=... ./install.sh` for another place); `./install.sh --uninstall` removes them |

Then open a new terminal: `cmmcomp -h` answers. Keep this folder where it is:
the tools find `SAPHO/`, `Macros/` and `Header/` next to `bin/` by
themselves.

## What is in this package

| folder | contents |
|---|---|
| `bin/` | the compilers (`cmmcomp`, `cppcomp`, `asmcomp`) and two waveform helpers (`comp2gtkw`, `gen_gtkw`). `-h` on any of them shows its options, `-V` its version |
| `SAPHO/` | the SAPHO processor, in Verilog: asmcomp instantiates it with each program's parameters, and a simulator or a synthesizer reads it from here |
| `Macros/` | assembly macros and tables used by C± programs |
| `Header/` | the C++ headers the programs include |
| `example/` | one small program, in C++ (`my_program.cpp`) and in C± (`my_program.cmm`) |

The binaries need nothing else to produce the Verilog. To simulate it you
need a Verilog simulator (Icarus Verilog, or Verilator); to look at the
waveform, GTKWave.

## Quick start

The commands, for Windows `cmd` and for a Linux shell, are in the Quick start
of the main README: <https://github.com/nipscernlab/yanc#quick-start>. They
run as they are on `example/my_program.cpp`, which prints `55`.

The C± version compiles with `cmmcomp` instead of `cppcomp`; it
writes into `./my_program/`:

```
cmmcomp example/my_program.cmm
```

The rest of the pipeline (asmcomp, the simulation) is the same.

## License

MIT. SAPHO is described in: SAPHO, Scalable-Architecture Processor for
Hardware Optimization: An FPGA Customizable Implementation Approach. IEEE,
2026. <https://ieeexplore.ieee.org/document/11345120/>
