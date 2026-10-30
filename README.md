<p align="center">
  <img src="https://raw.githubusercontent.com/nipscernlab/nipscernweb/main/assets/icons/yanc.svg"
       alt="YANC Icon"
       width="160">
</p>

<h1 align="center">YANC</h1>
<p align="center"><strong>Yet Another Compiler</strong> — toolchain for the SAPHO soft-processor ecosystem</p>

<p align="center">
  <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <a href="https://github.com/nipscernlab/yanc/releases/latest"><img alt="Latest release" src="https://img.shields.io/github/v/release/nipscernlab/yanc"></a>
  <a href="https://github.com/nipscernlab/yanc/actions/workflows/release.yml"><img alt="Release build" src="https://github.com/nipscernlab/yanc/actions/workflows/release.yml/badge.svg"></a>
</p>

---

## What is YANC?

**YANC is the compiler suite for the SAPHO processor.** It takes a program in
**C±** ("C plus-minus": integers, floats in SAPHO's own format, native complex
numbers) or in **C++** and produces the Verilog of a processor sized for that
program, its memory images, a testbench, and the tables that let a waveform
viewer show the source line, the assembly instruction and every variable,
cycle by cycle. It is also the SAPHO compiler inside
**[AURORA](https://github.com/nipscernlab/aurora)**, the NIPS-CERN IDE for
the synthesis and simulation of Verilog circuits, where YANC builds the
SAPHO processors those circuits use.

SAPHO (Scalable-Architecture Processor for Hardware Optimization) is a
soft-core processor for FPGAs whose hardware fits the program it runs: only
the instructions and ALU operators the program uses are synthesized. Its
Verilog is the [`SAPHO/`](SAPHO/) folder of this repository.

## Quick start

**1. Install.** Download the latest
[release](https://github.com/nipscernlab/yanc/releases/latest):
`yanc-bin-<version>.zip` for Windows, `yanc-bin-linux-<version>.tar.gz` for
Linux. Unpack it anywhere and run its install script once: `install.bat` on
Windows, `./install.sh` on Linux. It puts the tools on your PATH, with no
administrator rights. Open a new
terminal: `yanc -h` answers. To simulate you also need
[Icarus Verilog](http://iverilog.icarus.com/), and to see the waveform,
GTKWave ([docs/install.md](docs/install.md)).

**2. Compile.** The package's `example/my_program.cpp`:

```cpp
int main() {
    int acc = 0;
    for (int k = 1; k <= 10; ++k) acc += k;
    out(0, acc);            // writes 55 to output port 0
    return 0;
}
```

In its folder:

<!-- quickstart-sh -->
```sh
yanc my_program.cpp
```

This writes the processor `my_program` into the folder `my_program/`: its
Verilog, its memories and a testbench. A C± program is the same command on
a `.cmm` file.

**3. Simulate.** From the output folder, with `SAPHO` the `SAPHO/` folder of
the package. On Windows (`cmd`):

```bat
set SAPHO=C:\path\to\yanc\SAPHO
cd my_program
iverilog -s my_program_tb -o sim.vvp -y %SAPHO% my_program_tb.v my_program.v
vvp sim.vvp
type output_0.txt
```

On Linux:

<!-- quickstart-sh -->
```sh
export SAPHO=/path/to/yanc/SAPHO
cd my_program
iverilog -s my_program_tb -o sim.vvp -y "$SAPHO" my_program_tb.v my_program.v
vvp sim.vvp
cat output_0.txt
```

`output_0.txt` holds `55`.

**4. See the waveform.** Still in `my_program/`, on either system:

<!-- quickstart-sh -->
```sh
gen_gtkw my_program_tb.vcd my_program_tb.gtkw ..
gtkwave my_program_tb.vcd -a my_program_tb.gtkw
```

GTKWave shows, clock by clock, the source line being run, the assembly
instruction and every variable by name.

`yanc -h` lists the options: `-o` another output folder, `-S` stop at the
assembly, `--lang=pt` the messages in Portuguese, and the rest. Everything
else, from the two languages to simulating with Verilator, is in the
manual: [docs/index.md](docs/index.md).

## Develop

The build needs `gcc`, `make`, `bison` and `flex` (on Windows from
[MSYS2](https://www.msys2.org/), with the `mingw-w64-x86_64-gcc` cross
toolchain; on Linux from the distribution). `Scripts/setup.bat` /
`Scripts/setup.sh` check and install what is missing; the details, including
a broken gcc version to avoid, are in [docs/install.md](docs/install.md).

```sh
make                              # build every tool into bin/
make CC=x86_64-w64-mingw32-gcc    # Windows: standalone .exe, no MSYS2 DLLs
bash Scripts/regress.sh           # the whole regression (~30 min); --no-sim for a quick pass
```

Every change goes through `Scripts/regress.sh`: it compiles each test,
compares the assembly with its golden, simulates, and checks the results
against references computed apart from YANC. The quick form (`--no-sim`)
runs for each change, the whole one for each batch of changes
([AGENTS.md](AGENTS.md), "The one command"). `Scripts/aurora.bat` builds and
deploys the tools into a sibling `Aurora/` checkout, the same files a release
ships. See [CONTRIBUTING.md](CONTRIBUTING.md) for the conventions.

```
yanc/
├── Compilers/      the compilers: CMMComp (C±), CPPComp (C++), ASMComp,
│                   common/ (the ISA table, asm passes), yanc_version.h;
│                   Tests/ and NegTests/ under each front end
├── SAPHO/          the SAPHO processor, in Verilog
├── Scripts/        setup, regress.sh, aurora.bat, the pipeline runners,
│                   hw/ (elaboration, area, Fmax)
├── docs/           the manual (index.md), and design/ with the design notes
├── release/        the README and example/ every release package carries
└── Makefile        the one place the tools are built from
```

## Documentation

The manual is in [docs/](docs/index.md): installing, the C± and C++
languages, the command line, simulating, several processors, the instruction
set. [docs/design/](docs/design/) holds the design notes behind each
feature (why, what was measured, what is open), and
[CHANGELOG.md](CHANGELOG.md) what changed in each release.

## About

YANC is developed by [NIPS-CERN](https://www.nipscern.com), the research
group of the Department of Electrical Engineering of the
[Universidade Federal de Juiz de Fora](https://www.ufjf.br) (UFJF), Brazil,
member of the ATLAS Collaboration at CERN. Project page:
[nipscern.com/projects/yanc](https://www.nipscern.com/projects/yanc).

SAPHO is described in: SAPHO, Scalable-Architecture Processor for Hardware
Optimization: An FPGA Customizable Implementation Approach. *IEEE*, 2026.
<https://ieeexplore.ieee.org/document/11345120/>. Further publications:
[nipscern.com/publications](https://www.nipscern.com/publications).

## License

MIT, see [LICENSE](LICENSE).
