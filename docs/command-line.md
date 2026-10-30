# The command line

One command turns a program into a processor:

```
yanc my_program.cmm        (or my_program.cpp)
```

`yanc` is a driver, as `gcc` is: it runs the compiler of the language
(`cmmcomp` for C±, `cppcomp` for C++), which writes the program in SAPHO
assembly, and then the assembler `asmcomp`, which writes the processor's
Verilog, its memories and a testbench. It finds them next to itself, stops
at the first one that fails (with that tool's exit code), and stops at the
Verilog: simulating is a separate step (see [Simulation](simulation.md)).

```
  C±:   prog.cmm  ──►  cmmcomp  ─┐
                                 ▼
                              prog.asm  ──►  asmcomp  ──►  prog.v + prog_*.mif + prog_tb.v
                                 ▲
  C++:  prog.cpp  ──►  cppcomp  ─┘
```

## `yanc`

```
yanc [options] <file.cmm | file.cpp | file.asm>
```

| option | does |
|---|---|
| `-o <dir>` | the output folder (default `./<name>`) |
| `-S` | compile only: stop at `<name>.asm` |
| `-I <dir>` | an extra `#include` folder (C++; repeatable) |
| `-f <MHz>` | the testbench clock (default 100) |
| `-c <n>` | how many clocks the testbench simulates (default 10000) |
| `--wave-arrays` | every array element as its own waveform signal (C±) |
| `-v` | show each tool's command line before it runs |
| `--lang=en\|pt` | the language of the messages (default English) |
| `-h`, `-V` | help, version |

A `.asm` is only assembled, in its own folder: `yanc -S prog.cmm` and then
`yanc prog/prog.asm` do in two steps what `yanc prog.cmm` does in one (an
IDE shows the two outputs apart that way). The `.asm`'s file name names the
processor, with the same rule as a source file's.

An `.asm` written by hand assembles the same way. Without the compiler's
tables beside it (`prog_log.txt`, `pc_<name>_mem.txt`) there is no source
behind it: the waveform shows the assembly but no source line and no
variable names, and the files the program reads (`input_N.txt`, array
initialisation) are looked for in the `.asm`'s own folder.

The processor takes the source file's name: `filtro.cmm` makes the processor
`filtro`, its Verilog `filtro.v` and its module `filtro`. The name has to be
a valid Verilog module name (letters, digits and `_`, not starting with a
digit, not a Verilog keyword); for another name, rename the file.

## What lands where

The files you write stay beside the source: the source itself, the files
that initialise arrays (`int c[3] "coefs.txt";`), and the simulation inputs
(`input_N.txt` feeds input port N, `cade_k.txt` link k). Everything the
tools write goes to one output folder, `./<name>/` unless `-o` says
otherwise. For `my_program.cpp`:

| file in `my_program/` | written by | what it is |
|---|---|---|
| `my_program.v` | asmcomp | the processor: SAPHO with this program's parameters |
| `my_program_inst.mif`, `my_program_data.mif` | asmcomp | the instruction and data memory images |
| `my_program_tb.v` | asmcomp | a testbench that feeds the inputs and records the outputs |
| `my_program.asm` | cppcomp / cmmcomp | the program in SAPHO assembly |
| `prog_log.txt`, `pc_my_program_mem.txt`, `src_lines.txt` | cppcomp / cmmcomp | the variables, the line of source each instruction came from, the source text: what the waveform shows (and where the source is) |
| `app_log.txt`, `trad_opcode.txt` | asmcomp | the address of every label, the sizes, the instruction names for the waveform |
| `output_N.txt`, `toma_k.txt`, `my_program_tb.vcd` | the simulation | what the program wrote, and the waveform |

To synthesize, take `my_program.v` and the two `.mif` files, and the SAPHO
modules in the `SAPHO/` folder that comes with YANC (`processor.v`,
`core.v`, `ula.v`, `addr_dec.v`, `instr_dec.v`).

The generated Verilog writes no absolute path: it names its files relative
to the output folder, where the simulation runs (see
[Simulation](simulation.md)). The folder and its source can move together,
and names with accents or spaces are fine on every system.

## The tools under `yanc`

Each can be run alone; the source file is the argument without an option,
and `-h` lists the options.

```
cmmcomp [-o <dir>] [--wave-arrays] <file.cmm>         the .asm and the tables into <dir> (default ./<name>)
cppcomp [-o <dir>] [-I <dir>]... [-E] <file.cpp>      the same for C++; -E prints the preprocessed source
asmcomp [-f <MHz>] [-c <clocks>] <file.asm>           the Verilog, memories and testbench beside the .asm
gen_gtkw <in.vcd> <out.gtkw> <base> [<comp2gtkw>]      a formatted GTKWave view of a dump (see Simulation)
```

`--lang`, `-h`/`--help` and `-V`/`--version` work in the three compilers;
`gen_gtkw` takes `-h` and `--lang`. In
`cmmcomp`, `cppcomp` and `asmcomp`, `-o`, `-f` and `-c` also have the long
forms `--output`, `--freq`, `--clocks`.

The tools find the folders that come with them (`SAPHO/`, `Macros/` with
the library routines, `Header/` with the C++ headers) next to their own
`bin/` folder: there is no option to name them. In a build of the
repository they find `SAPHO/` and `Compilers/` the same way.

## Messages and their language

The messages are in English. `--lang=pt` switches every tool (`cmmcomp`,
`cppcomp`, `asmcomp`, `gen_gtkw` and `yanc`'s own) to Portuguese; so does
the environment variable `YANC_LANG=pt`, set once for the terminal
(`--lang=en` wins over it). In C++ the word after the file and line
follows: `filter.cpp:12: erro: ...`, `atenção:` for a warning. An error
names the line (for C++, the file and line, a header's too) and stops the
tool; a warning (`Heads up`) does not.
