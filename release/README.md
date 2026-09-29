# YANC binaries

YANC is the compiler suite for the SAPHO soft-core processor. It turns a C± or
C++ program into the Verilog of a SAPHO processor sized for that program, its
memory images and a testbench. Full documentation:
<https://github.com/nipscernlab/yanc>.

## What is in this package

| folder | contents |
|---|---|
| `bin/` | the compilers (`cmmcomp`, `cppcomp`, `asmcomp`), the preprocessors (`cpppp`, `appcomp`) and two waveform helpers (`comp2gtkw`, `gen_gtkw`). `-h` on any of them shows its options, `-V` its version |
| `SAPHO/` | the SAPHO processor, in Verilog: asmcomp instantiates it with each program's parameters, and a simulator or a synthesizer reads it from here |
| `Macros/` | assembly macros and tables used by C± programs (`-m` of cmmcomp and asmcomp) |
| `Header/` | the C++ headers the programs include (`-I` of cpppp) |
| `example/` | one small program, in C++ (`my_program.cpp`) and in C± (`my_program.cmm`) |

The binaries need nothing else to produce the Verilog. To simulate it you
need a Verilog simulator (Icarus Verilog, or Verilator); to look at the
waveform, GTKWave.

## Quick start

The commands, for Windows `cmd` and for a Linux or MSYS2 shell, are in the
Quick start of the main README:
<https://github.com/nipscernlab/yanc#quick-start>. They run as they are on
`example/my_program.cpp`, which prints `55`.

The C± version compiles with `cmmcomp` instead of `cpppp` + `cppcomp`; the
source goes in `<project>/Software/` first:

```
cmmcomp -en -i my_program.cmm -n my_program -p <project> -m Macros -t <temp>
```

The rest of the pipeline (appcomp, asmcomp, the simulation) is the same.

## License

MIT. SAPHO is described in: SAPHO, Scalable-Architecture Processor for
Hardware Optimization: An FPGA Customizable Implementation Approach. IEEE,
2026. <https://ieeexplore.ieee.org/document/11345120/>
