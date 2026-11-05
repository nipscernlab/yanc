# Installing YANC

YANC runs on Windows and Linux with the same commands. On Windows they are
typed in the terminal of [MSYS2](https://www.msys2.org/), in its **UCRT64**
environment: install MSYS2 and open the "MSYS2 UCRT64" terminal, which also
provides the simulators and GTKWave (see [To simulate](#to-simulate)). On
Linux, any terminal.

## The release package

Download the latest release from
<https://github.com/nipscernlab/yanc/releases/latest>:
`yanc-bin-<version>.zip` for Windows, `yanc-bin-linux-<version>.tar.gz` for
Linux. Unpack it anywhere and, from the unpacked folder (in the UCRT64
terminal `C:\Users\me\yanc` is `/c/Users/me/yanc`), run once, with no
administrator rights:

```sh
./install.sh
```

It puts the tools on your PATH: on Windows a file `/etc/profile.d/yanc.sh`
puts the package's `bin/` on the PATH of every new MSYS2 terminal, on Linux
a launcher per tool goes in `~/.local/bin` (`PREFIX=... ./install.sh` for
another place). Then it checks for Icarus Verilog, GTKWave and Verilator
and says how to get the missing ones; on Windows it installs them with
`pacman` if you answer `y`. Open a new terminal: `yanc -h` answers.
`./install.sh --uninstall` undoes it.

The MSYS2 terminal does not see the Windows `PATH`, nor `cmd` and
PowerShell MSYS2's. The package's `install.bat` puts `bin\` on the Windows
user `PATH`, to call `yanc` from `cmd` or PowerShell (`uninstall.bat` takes
it off). The package holds:

| folder | contents |
|---|---|
| `bin/` | `yanc` and the tools under it (`cmmcomp`, `cppcomp`, `asmcomp`), and the waveform helpers `gen_gtkw` and `comp2gtkw` |
| `SAPHO/` | the SAPHO processor in Verilog, which a simulator or a synthesizer reads with the generated `.v` |
| `Macros/` | the assembly routines of the C± library |
| `Header/` | the C++ headers |
| `example/` | `my_program.cmm` and `my_program.cpp`, the program of the README's Quick start |

The tools find `SAPHO/`, `Macros/` and `Header/` next to their own `bin/`,
so keep the package's folders together.

## To simulate

YANC writes the processor's Verilog and a testbench; running it takes a
Verilog simulator, and looking at the result a waveform viewer (see
[Simulation](simulation.md)). `install.sh` checks for these; the packages
are:

| what | MSYS2 UCRT64 (`pacman -S`) | Debian / Ubuntu (`apt install`) | for |
| --- | --- | --- | --- |
| Icarus Verilog | `mingw-w64-ucrt-x86_64-iverilog` | `iverilog` | simulating the generated Verilog |
| GTKWave | `mingw-w64-ucrt-x86_64-gtkwave` | `gtkwave` | looking at the waveform |
| Verilator 5 | `mingw-w64-ucrt-x86_64-verilator`, with `make`, `mingw-w64-ucrt-x86_64-gcc`, `mingw-w64-ucrt-x86_64-python` and `mingw-w64-ucrt-x86_64-lz4` | `verilator`, with `make`, `g++` and `python3` | faster simulation, for long runs: Verilator turns the Verilog into C++ and builds it with `make` and `g++`; its FST trace (`multi_proc --sim verilator`) needs `lz4.h`, which MSYS2's Verilator does not pull in |

On Windows, MSYS2 ships Icarus and Verilator only for UCRT64: it no longer
does for MINGW64. Everything, in the UCRT64 terminal:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-iverilog mingw-w64-ucrt-x86_64-gtkwave \
  mingw-w64-ucrt-x86_64-verilator mingw-w64-ucrt-x86_64-lz4 \
  make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-python
```

To build YANC and run its scripts as well, add `bison flex diffutils`:
`diffutils` (`cmp`, `diff`) and Python are what `Scripts/regress.sh` and
its checks run, and without them the regression fails in ways that look
like compiler defects. `pacman -Syu` keeps MSYS2 up to date.

> **GTKWave.** YANC's scripts give GTKWave the options it knows:
> `--dark`, and `--zoom-fit` and `--left-justify` when it lists them (the
> [nipscernlab build](https://github.com/nipscernlab/gtkwave-nipscern/releases)
> does; MSYS2's and the distributions' do not, and refuse an option they do
> not know). Any of them applies the layout `gen_gtkw` writes. Without
> GTKWave, `single_proc.sh` and `multi_proc.sh` still simulate and write the
> waveform and its layout; they only do not open it. The Verilator flow of
> `multi_proc.sh` needs `fst2vcd`, which comes with GTKWave.
>
> **gcc 16 and Verilator.** MSYS2's gcc 16 (16.1 and 16.2) ships a
> `libstdc++` without the `std::string` move constructor at `-Os`, which
> Verilator's build uses: linking fails with `undefined reference to
> std::__cxx11::basic_string<...>::basic_string(...&&)`. YANC's scripts pass
> `-D_GLIBCXX_ASSERTIONS` to Verilator under MSYS2 (`YANC_VL_CFLAGS` in
> `Scripts/env.sh` and `Scripts\env.bat`), with which the compiler builds
> the constructor itself; no downgrade is needed. Verilator run by hand
> takes it as `-CFLAGS -D_GLIBCXX_ASSERTIONS` (see
> [Simulation](simulation.md#simulating-with-verilator)).
> `Scripts/setup.sh` and `Scripts\setup.bat` check it. The Icarus flow is
> unaffected.

## Building YANC from source

To build the tools from a clone of the repository, or to work on YANC, see
[Building and testing YANC](develop/building.md).
