# YANC binaries

YANC is the compiler suite for the SAPHO soft-core processor. It turns a C± or
C++ program into the Verilog of a SAPHO processor sized for that program, its
memory images and a testbench. Full documentation:
<https://github.com/nipscernlab/yanc>.

## Install

YANC runs with the same commands on Windows and on Linux. On Windows they
are typed in the "MSYS2 UCRT64" terminal of MSYS2
(<https://www.msys2.org/>). From this folder, run once, with no
administrator rights:

```
./install.sh
```

| system | what it does |
|---|---|
| Windows, in the UCRT64 terminal | writes `/etc/profile.d/yanc.sh`, which puts `bin/` on the PATH of every new MSYS2 terminal |
| Linux | a launcher per tool in `~/.local/bin` (`PREFIX=... ./install.sh` for another place) |

Then it checks for Icarus Verilog and GTKWave, and for Verilator, and says
how to get the missing ones; on Windows it installs them with `pacman` if
you answer `y`. `./install.sh --uninstall` undoes it. Open a new terminal:
`yanc -h` answers. Keep this folder where it is: the tools find `SAPHO/`,
`Macros/` and `Header/` next to `bin/` by themselves.

`install.bat` puts `bin\` on your Windows user PATH instead, which `cmd`
and PowerShell see and the MSYS2 terminal does not, to call `yanc` from
there (`uninstall.bat` takes it off).

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
waveform, GTKWave. `install.sh` checks for them; the packages are listed in
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
