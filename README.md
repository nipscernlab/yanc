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
[release](https://github.com/nipscernlab/yanc/releases/latest)
(`yanc-bin-<tag>.zip` on Windows, `yanc-bin-linux-<tag>.tar.gz` on Linux,
`<tag>` being the version, for example `yanc-bin-v6.0.zip`), unpack it
anywhere and run the install script in it once: `install.bat` on Windows,
`./install.sh` on Linux. It puts the tools on your PATH (no administrator
rights); open a new terminal and `cmmcomp -h` answers. The tools find the
folders that come with them (`SAPHO/`, `Macros/`, `Header/`) by themselves.
To simulate you also need [Icarus Verilog](http://iverilog.icarus.com/) (or
Verilator), and to look at the waveform GTKWave
([docs/setup.md](docs/setup.md)).

**2. Run a program.** Four commands take a source file down to the
processor's Verilog and its testbench; a simulator and a viewer do the rest.
The compilers print in Portuguese by default; `-en` switches them to English.
The program, `my_program.cpp`:

```cpp
int main() {
    int acc = 0;
    for (int k = 1; k <= 10; ++k) acc += k;
    out(0, acc);            // writes 55 to output port 0
    return 0;
}
```

**Windows** (`cmd`), in the folder that holds `my_program.cpp`:

```bat
:: 1. compile to assembly: my_program\my_program.asm and its tables (the
::    processor takes the source file's name; a .cmm source: cmmcomp my_program.cmm)
cppcomp my_program.cpp
:: 2. addresses and processor parameters
appcomp my_program\my_program.asm
:: 3. the processor's Verilog, memory images and testbench, all in my_program\
asmcomp my_program\my_program.asm

:: 4. simulate (Icarus); the processor's Verilog is the package's SAPHO folder
set SAPHO=C:\path\to\yanc\SAPHO
cd my_program
iverilog -s my_program_tb -o sim.vvp my_program_tb.v my_program.v ^
         %SAPHO%\processor.v %SAPHO%\core.v %SAPHO%\ula.v %SAPHO%\addr_dec.v %SAPHO%\instr_dec.v
vvp sim.vvp
type output_0.txt

:: 5. the waveform, formatted (.. holds the output folder my_program\)
gen_gtkw my_program_tb.vcd my_program_tb.gtkw ..
gtkwave my_program_tb.vcd -a my_program_tb.gtkw
```

**Linux** (or MSYS2 on Windows), with the package unpacked in `$YANC`. This
block installs, writes the program and checks the output, so it can be pasted
as is (the CI runs it on every push):

<!-- quickstart-sh: .github/workflows/ci.yml runs the next block -->
```sh
set -e
YANC=${YANC:-/path/to/yanc}                 # the unpacked package
"$YANC/install.sh"                          # the tools into ~/.local/bin
export PATH="$HOME/.local/bin:$PATH"

cat > my_program.cpp <<'EOF'
int main() {
    int acc = 0;
    for (int k = 1; k <= 10; ++k) acc += k;
    out(0, acc);            // writes 55 to output port 0
    return 0;
}
EOF

cppcomp my_program.cpp
appcomp my_program/my_program.asm
asmcomp my_program/my_program.asm

SAPHO=$YANC/SAPHO
cd my_program
iverilog -s my_program_tb -o sim.vvp my_program_tb.v my_program.v \
         "$SAPHO/processor.v" "$SAPHO/core.v" "$SAPHO/ula.v" "$SAPHO/addr_dec.v" "$SAPHO/instr_dec.v"
vvp -n sim.vvp
tr -d '\r' < output_0.txt      # 55
[ "$(tr -d '\r' < output_0.txt)" = 55 ]
gen_gtkw my_program_tb.vcd my_program_tb.gtkw ..
test -s my_program_tb.gtkw
# gtkwave my_program_tb.vcd -a my_program_tb.gtkw   # to look at the waveform
```

Everything generated lands in `my_program/`, beside the source: the
processor (`my_program.v`, `my_program_inst.mif`, `my_program_data.mif`), the
assembly, the testbench and the simulation outputs. Files the program reads
(arrays initialised from a file, `input_N.txt` for the simulation) stay beside
the source. Stop after `asmcomp` if all you want are the Verilog and the memory
images. Verilator instead of Icarus, the formatted waveform and the scripts
that run the whole pipeline: [docs/running.md](docs/running.md). Writing
programs: [docs/programming.md](docs/programming.md).

## Develop

The build needs `gcc`, `make`, `bison` and `flex` (on Windows from
[MSYS2](https://www.msys2.org/), with the `mingw-w64-x86_64-gcc` cross
toolchain; on Linux from the distribution). `Scripts/setup.bat` /
`Scripts/setup.sh` check and install what is missing; the details, including
a broken gcc version to avoid, are in [docs/setup.md](docs/setup.md).

```sh
make                              # build every tool into bin/
make CC=x86_64-w64-mingw32-gcc    # Windows: standalone .exe, no MSYS2 DLLs
bash Scripts/regress.sh           # the whole regression (~30 min); --no-sim for a quick pass
```

Every change goes through `Scripts/regress.sh`: it compiles each test,
compares the assembly with its golden, simulates, and checks the results
against references computed apart from YANC. `Scripts/aurora.bat` builds and
deploys the tools into a sibling `Aurora/` checkout, the same files a release
ships. See [CONTRIBUTING.md](CONTRIBUTING.md) for the conventions.

```
yanc/
├── Compilers/      the compilers: CMMComp (C±), CPPComp (C++), ASMComp, APPComp,
│                   common/ (the ISA table, asm passes), yanc_version.h;
│                   Tests/ and NegTests/ under each front end
├── SAPHO/          the SAPHO processor, in Verilog
├── Scripts/        setup, regress.sh, aurora.bat, the pipeline runners,
│                   hw/ (elaboration, area, Fmax)
├── docs/           setup, running, programming, and the design notes
├── release/        the README and example/ every release package carries
├── Makefile        the one place the tools are built from
└── TODO.md         open work items
```

## Documentation

| file | what it covers |
|---|---|
| [docs/setup.md](docs/setup.md) | dependencies, the setup scripts, building from source |
| [docs/running.md](docs/running.md) | the pipeline, each tool and its options, what lands where, Icarus and Verilator, the formatted waveform, the runner scripts |
| [docs/programming.md](docs/programming.md) | an example in C±, the float rounding levels (`#FROUND`), several processors (`toma` / `cade`) |
| [CHANGELOG.md](CHANGELOG.md) | what changed in each release |
| `docs/*.md` (the rest) | design notes behind each feature: why, what was measured, what is open |

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
