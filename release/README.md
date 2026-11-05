# YANC binaries

YANC is the compiler suite for the SAPHO soft-core processor. It turns a C± or
C++ program into the Verilog of a SAPHO processor sized for that program, its
memory images and a testbench. Full documentation:
<https://github.com/nipscernlab/yanc>.

## Install

On Windows, YANC runs in the terminal of MSYS2 (<https://www.msys2.org/>),
in its UCRT64 environment, with the same commands as on Linux. Put the
tools on your PATH once, from this folder, with no administrator rights:

| system | command | what it does |
|---|---|---|
| Windows, in the "MSYS2 UCRT64" terminal | `echo "export PATH=\"\$PATH:$(pwd)/bin\"" > /etc/profile.d/yanc.sh` | adds `bin/` to the PATH of every MSYS2 terminal; `rm /etc/profile.d/yanc.sh` undoes it |
| Linux | `./install.sh` | a launcher per tool in `~/.local/bin` (`PREFIX=... ./install.sh` for another place); `./install.sh --uninstall` removes them |

Then open a new terminal: `yanc -h` answers. Keep this folder where it is:
the tools find `SAPHO/`, `Macros/` and `Header/` next to `bin/` by
themselves. `install.bat` puts `bin\` on your Windows user PATH instead,
which `cmd` and PowerShell see and the MSYS2 terminal does not, to call
`yanc` from there (`uninstall.bat` takes it off).

## What is in this package

| folder | contents |
|---|---|
| `bin/` | `yanc`, the one command that runs the others, the compilers (`cmmcomp`, `cppcomp`, `asmcomp`) and two waveform helpers (`comp2gtkw`, `gen_gtkw`). `-h` on `yanc` or a compiler shows its options, `-V` its version; GTKWave runs `comp2gtkw` itself |
| `SAPHO/` | the SAPHO processor, in Verilog: asmcomp instantiates it with each program's parameters, and a simulator or a synthesizer reads it from here |
| `Macros/` | assembly macros and tables used by C± programs |
| `Header/` | the C++ headers the programs include |
| `example/` | one small program, in C++ (`my_program.cpp`) and in C± (`my_program.cmm`) |

The binaries need nothing else to produce the Verilog. To simulate it you
need a Verilog simulator (Icarus Verilog, or Verilator); to look at the
waveform, GTKWave. On Windows the simulators come from MSYS2 (Icarus:
`pacman -S mingw-w64-ucrt-x86_64-iverilog`); the packages for Verilator,
and GTKWave, are in
<https://github.com/nipscernlab/yanc/blob/main/docs/install.md>.

## Quick start

The commands, the same on Windows (in the UCRT64 terminal) and on Linux,
are in the Quick start of the main README:
<https://github.com/nipscernlab/yanc#quick-start>. They run as they are on
`example/my_program.cpp`, which prints `55`.

The C± version is the same command on the `.cmm`; it writes into
`./my_program/`:

```
yanc example/my_program.cmm
```

The simulation is the same.

## License

MIT. SAPHO is described in: SAPHO, Scalable-Architecture Processor for
Hardware Optimization: An FPGA Customizable Implementation Approach. IEEE,
2026. <https://ieeexplore.ieee.org/document/11345120/>
