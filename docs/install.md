# Installing YANC

YANC runs on Windows and Linux.

## The release package

Download the latest release from
<https://github.com/nipscernlab/yanc/releases/latest>:
`yanc-bin-<version>.zip` for Windows, `yanc-bin-linux-<version>.tar.gz` for
Linux. Unpack it anywhere and run its install script once:

```bat
install.bat          :: Windows: puts bin\ on your user PATH, no administrator rights
```
```sh
./install.sh         # Linux: a launcher per tool in ~/.local/bin (or $PREFIX/bin)
```

Open a new terminal: `yanc -h` answers. `uninstall.bat` (Windows) or
`./install.sh --uninstall` (Linux) undoes it. The package holds:

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

On **Windows**, [MSYS2](https://www.msys2.org/) provides both simulators;
GTKWave is a separate download:

| what | `pacman` package / source | for |
| --- | --- | --- |
| Icarus Verilog | `mingw-w64-x86_64-iverilog` | simulating the generated Verilog |
| Verilator 5.x | `mingw-w64-x86_64-verilator` | faster simulation, for long runs |
| GTKWave | the [nipscernlab build](https://github.com/nipscernlab/gtkwave-nipscern/releases) | looking at the waveform |

> **GTKWave: the nipscernlab build** formats YANC's waveform views; the
> generic GTKWave does not show them the same way.
>
> ⚠️ **Avoid `mingw-w64-x86_64-gcc` 16.1.0-5.** That MSYS2 package shipped a
> broken `libstdc++` (the `std::string` move constructor symbol is missing), so
> Verilator's `verilated.cpp` fails to link and the `--sim verilator` flow dies
> with `undefined reference to std::__cxx11::basic_string<...>::basic_string(...&&)`.
> Use **gcc 15.1.0-5** (known good). To downgrade from the pacman cache:
> ```
> pacman -U /var/cache/pacman/pkg/mingw-w64-x86_64-gcc-15.1.0-5-any.pkg.tar.zst \
>           /var/cache/pacman/pkg/mingw-w64-x86_64-gcc-libs-15.1.0-5-any.pkg.tar.zst
> ```
> The Icarus flow is unaffected.

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
