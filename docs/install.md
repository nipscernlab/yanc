# Installing YANC

YANC runs on Windows and Linux with the same commands. On Windows they are
typed in the terminal of [MSYS2](https://www.msys2.org/), in its **UCRT64**
environment: install MSYS2 and open the "MSYS2 UCRT64" terminal, which also
provides the simulators (see [To simulate](#to-simulate)). On Linux, any
shell.

## The release package

Download the latest release from
<https://github.com/nipscernlab/yanc/releases/latest>:
`yanc-bin-<version>.zip` for Windows, `yanc-bin-linux-<version>.tar.gz` for
Linux. Unpack it anywhere and, from the unpacked folder, put its tools on
your PATH, once, with no administrator rights:

```sh
# Windows, in the UCRT64 terminal: bin/ on the PATH of every MSYS2 terminal
echo "export PATH=\"\$PATH:$(pwd)/bin\"" > /etc/profile.d/yanc.sh
# Linux: a launcher per tool in ~/.local/bin (or $PREFIX/bin)
./install.sh
```

Open a new terminal: `yanc -h` answers. `rm /etc/profile.d/yanc.sh`
(Windows) or `./install.sh --uninstall` (Linux) undoes it. The MSYS2
terminal does not see the Windows `PATH`; the package's `install.bat` puts
`bin\` there instead, to call `yanc` from `cmd` or PowerShell
(`uninstall.bat` takes it off). The package holds:

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
[Simulation](simulation.md)).

On **Windows**, [MSYS2](https://www.msys2.org/) provides both simulators, in
its **UCRT64** environment: MSYS2 no longer ships Icarus and Verilator for
MINGW64. GTKWave is a separate download:

| what | `pacman` package / source | for |
| --- | --- | --- |
| Icarus Verilog | `mingw-w64-ucrt-x86_64-iverilog` | simulating the generated Verilog |
| Verilator 5.x | `mingw-w64-ucrt-x86_64-verilator`, with `make`, `mingw-w64-ucrt-x86_64-gcc` and `mingw-w64-ucrt-x86_64-python` | faster simulation, for long runs: Verilator turns the Verilog into C++ and builds it with `make` and `g++` |
| LZ4 | `mingw-w64-ucrt-x86_64-lz4` | Verilator's FST trace (`multi_proc --sim verilator`); the Verilator package does not pull it in |
| GTKWave | the [nipscernlab build](https://github.com/nipscernlab/gtkwave-nipscern/releases) | looking at the waveform |

To use YANC with Icarus only:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-iverilog
```

With Verilator as well:

```sh
pacman -S --needed make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-python \
  mingw-w64-ucrt-x86_64-iverilog mingw-w64-ucrt-x86_64-verilator \
  mingw-w64-ucrt-x86_64-lz4
```

To build YANC and run its scripts as well, the whole list:

```sh
pacman -S --needed make bison flex diffutils \
  mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-python \
  mingw-w64-ucrt-x86_64-iverilog mingw-w64-ucrt-x86_64-verilator \
  mingw-w64-ucrt-x86_64-lz4
```

`diffutils` (`cmp`, `diff`) and Python are what `Scripts/regress.sh` and its
checks run; without them the regression fails in ways that look like
compiler defects.

> **GTKWave: the nipscernlab build** formats YANC's waveform views; the
> generic GTKWave does not show them the same way. Extract the zip to a
> folder of your choice. The MSYS2 terminal does not see the Windows `PATH`,
> so add the folder to MSYS2's, **after** `/ucrt64/bin` (the zip carries its
> own `libstdc++-6.dll` and `libgcc_s_seh-1.dll`, which MSYS2's tools must not
> pick up first), for example in a file `/etc/profile.d/gtkwave.sh`:
> ```sh
> export PATH="$PATH:/c/<your folder>/gtkwave-nipscern"
> ```
> Without GTKWave, `single_proc.sh` and `multi_proc.sh` still simulate and
> write the waveform and its layout; they only do not open it. The Verilator
> flow of `multi_proc.sh` needs `fst2vcd`, which comes with GTKWave.
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

On **Linux**, they come from the distribution's package manager:

| Package | What it's for |
| ------- | ------------- |
| `iverilog` | simulating the generated Verilog |
| `verilator` | faster simulation, for long runs |
| `gtkwave` | looking at the waveform |

> **Note on Linux GTKWave:** there is no Linux build of the nipscernlab fork
> yet, so the Linux flow uses your distro's stock `gtkwave`. It still opens the
> waveform and applies the generated `.gtkw` layout; only the panel chrome
> (dark SST-less view) differs from the Windows fork.

## Building YANC from source

To build the tools from a clone of the repository, or to work on YANC, see
[Building and testing YANC](develop/building.md).
