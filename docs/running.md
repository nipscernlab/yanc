# Running the YANC pipeline

What each tool does, what it writes, and the ways to simulate and look at the
result. The commands that take one program all the way are in the README's
[Quick start](../README.md#quick-start).

## Pipeline

YANC has **three compilers** (`cmmcomp`, `cppcomp`, `asmcomp`). Two front-end compilers turn high-level source into assembly (`cppcomp` preprocesses the C++ itself, as gcc does); a single back-end compiler turns that assembly into Verilog, after a first pass over the assembly (`appcomp`).

```
  C±:     foo.cmm  ──►  cmmcomp  ─┐
                                ▼
                             foo.asm  ──►  appcomp  ──►  asmcomp  ──►  foo.v + *.mif + foo_tb.v
                                ▲
  C++:    foo.cpp  ──►  cppcomp  ─┘
```

After `asmcomp`, the generated Verilog can be simulated with **Icarus Verilog** (`iverilog` + `vvp`) or with **Verilator**, and visualized in **GTKWave**. Pre-wired scripts in `Scripts/` cover the whole pipeline end-to-end — `single_proc` (one C± processor), `multi_proc` (C± multi-processor project) and `single_proc_cpp` (one C++ processor), each with a `.bat` and a `.sh` and a `--sim iverilog|verilator` flag (default Icarus).

## What you see in GTKWave

Because the toolchain emits a side-table mapping each PC value to its
originating C± source line, GTKWave shows the executing C± line,
the assembly opcode, and every declared variable evolving in lockstep
with the simulated clock — not just raw bus toggles:

![GTKWave showing the executing C± line, the assembly opcode, and live variable values in lockstep with the clock](images/gtkwave-source-trace.png)

The example above is `proc_fft` mid-run: the C± track shows the
`fout(0, 1000.0*real(data[2]));` statement, the assembly track shows
the matching `LOD / F_MLT+ / F2I / OUT 0` sequence, and the
declared variables (`N`, `ind`, `istep`, `j`, `k`, `m`, `mmax`,
`sind`, `comp temp`) carry their live values.

## Components

Six binaries are produced from source — three compilers, the assembly's first pass, and two helpers:

**Compilers**

| Binary       | Source dir              | Built with         | Role                                                |
| ------------ | ----------------------- | ------------------ | --------------------------------------------------- |
| `cmmcomp`    | `Compilers/CMMComp/`    | Flex + Bison + GCC | C± front-end → assembly                            |
| `cppcomp`    | `Compilers/CPPComp/`    | Flex + Bison + GCC | C++ front-end → assembly                            |
| `asmcomp`    | `Compilers/ASMComp/`    | Flex + GCC         | Back-end: assembly → Verilog HDL + memory images + testbench |

`cppcomp` carries its own C preprocessor (`#include`, `#define`, `#if`, ...; `cppcomp -E` prints its output).

**Assembly first pass**

| Binary       | Source dir              | Built with    | Role                                                                |
| ------------ | ----------------------- | ------------- | ------------------------------------------------------------------- |
| `appcomp`    | `Compilers/APPComp/`    | Flex + GCC    | First pass over the `.asm`: records processor params + resolves variable/label addresses for `asmcomp` |

**Helper**

| Binary       | Source dir   | Built with    | Role                                                |
| ------------ | ------------ | ------------- | --------------------------------------------------- |
| `comp2gtkw`  | `Scripts/`   | GCC           | Translator: complex-number bit pattern → GTKWave    |
| `gen_gtkw`   | `Scripts/`   | GCC           | Reads a VCD header → writes a formatted `.gtkw` view |

Auxiliary content:

* `SAPHO/` — the SAPHO processor itself, in Verilog (core, ALU, instruction decoder, FIFO, ...); asmcomp instantiates it with the parameters each program needs. Up to v5.6 the folder was called `HDL/` (v5.6 ships both names)
* `Compilers/CMMComp/Includes/` — assembly macros and lookup tables for `.cmm` programs (`float_sqrt`, `float_sin`, `float_atan`, ...)
* `Compilers/CPPComp/Includes/` — header shims that `.cpp` programs include
* `Scripts/` — `regress.sh`, `comp2gtkw`, `gen_gtkw` (builds the formatted GTKWave view)
* `Compilers/CMMComp/Tests/` — runnable `.cmm` example projects (Math, FFT, RLS, DTW, PulseSim, Blind, ...)
* `Compilers/CPPComp/Tests/` — per-test C++ programs (`test1` … `test84`), plus the Verilator harness
* `Compilers/yanc_version.h` — single source of truth for the toolchain version, included by the five compiler binaries (the three compilers + the two preprocessors)
## What lands where

The files the user writes stay where they are: the source, the files that
initialise arrays (`int c[3] "coefs.txt";`) and the simulation inputs
(`input_N.txt` feeds input port N). Everything the tools generate goes to one
output folder, `./<name>/` beside the source unless `-o` says otherwise. With
the Quick start's `my_program.cpp`:

| file in `my_program/` | written by | what it is |
|---|---|---|
| `my_program.asm` | cppcomp / cmmcomp | the program in SAPHO assembly |
| `prog_log.txt`, `pc_my_program_mem.txt`, `src_lines.txt` | cppcomp / cmmcomp | the tables asmcomp and the waveform use (variables, PC to source line, the source text); `prog_log.txt` also names the source folder, where asmcomp reads the user's files |
| `app_log.txt` | appcomp | addresses and processor parameters, for asmcomp |
| `my_program.v` | asmcomp | the Verilog top: SAPHO instantiated with this program's parameters |
| `my_program_inst.mif`, `my_program_data.mif` | asmcomp | instruction and data memory images |
| `my_program_tb.v`, `trad_opcode.txt` | asmcomp | the testbench, and the opcode names for the waveform |
| `output_0.txt` | the simulation | what the program wrote to output port 0 |
| `my_program_tb.vcd` | the simulation | the waveform |
| `my_program_tb.gtkw` | gen_gtkw | the formatted GTKWave view of it |

You can stop after asmcomp if all you want are the Verilog and the memory
images (to synthesize, or to feed your own simulator), or swap the simulation
for **Verilator** (below).

## Simulating with Verilator

Verilator 5 compiles the same generated `<proc>_tb.v` directly — `--timing` understands the testbench's clock and `#` delays, so no separate C++ driver is needed:

```bat
:: --- simulate with Verilator, from the output folder (my_program\) --------
cd my_program
verilator --binary --timing --trace +define+YANC_TRACE --top-module my_program_tb ^
          -Wno-lint -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-COMBDLY -Wno-STMTDLY ^
          -Wno-INFINITELOOP -Wno-UNOPTFLAT --Mdir vl ^
          my_program_tb.v my_program.v ^
          %SAPHO%\processor.v %SAPHO%\core.v %SAPHO%\ula.v %SAPHO%\addr_dec.v %SAPHO%\instr_dec.v
vl\Vmy_program_tb.exe   :: runs the sim, writes my_program_tb.vcd here
gtkwave my_program_tb.vcd
```

**The one thing to remember:** `+define+YANC_TRACE` is what makes your variables, arrays, the PC→C± line table and the assembly opcode appear in the waveform. Icarus gets them for free (it predefines `__ICARUS__`); Verilator only compiles that visibility harness when you pass the define. For big multi-millisecond project dumps, use `--trace-fst` instead of `--trace` (compact FST; the file is still named `<tb>.vcd` and GTKWave detects the format). The trace deliberately carries only the `<proc>`-level user signals — the CPU internals below each processor are fenced out with `/* verilator tracing_off */`. **In particular the stack-monitoring flags and the ULA rounding-error taps (`fl_max`, `fl_full`, `pointeri`, `delta_int`, `delta_float`) are intentionally *not* in the Verilator VCD**: keeping them would force Verilator to evaluate the expensive real-valued ULA monitoring logic every cycle, which defeats the whole point of using Verilator (speed). Those debug signals remain available under the Icarus flow (`Scripts\single_proc.bat` / `multi_proc.bat`), where raw speed is not the goal.

## Formatting the waveform — `gen_gtkw` (optional)

After simulating with **either** Icarus or Verilator you have a raw `<tb>.vcd` /
`.fst`: every signal, in scope order, unformatted. If you want the curated view
instead — the **input/output ports**, the **Assembly** and **C±** instruction
tracks (disassembled through their translate files), and the **internal
variables and arrays** grouped per processor — run **`gen_gtkw`** (it is in the
release's `bin/`), then open the waveform with it applied via `-a`. From the
Quick start's output folder:

```bat
gen_gtkw my_program_tb.vcd my_program_tb.gtkw ..
gtkwave --dark --zoom-fit --left-justify my_program_tb.vcd -a my_program_tb.gtkw
```

`gen_gtkw` reads only the VCD **header** (the signal list), classifies each
harness signal by name, and writes a formatted `.gtkw` save file: the right data
format / colour / alias per signal, the `trad_opcode.txt` / `src_lines.txt`
translators on the Assembly / C± tracks, `comp2gtkw.exe` on complex signals, and
one section per processor — any scope that owns both `valr2` and `linetabs` — so
the **same tool handles single- and multi-processor dumps**. Its third argument
is the folder that holds each processor's output folder, where its translate
files are (`..` from inside `my_program/` in the Quick start, the folder that
holds `my_program/`).

Because only the header is needed, the big project FST dumps don't have to be
written in full first: run the sim once with the **`+HEADER_ONLY`** plusarg and
the testbench dumps the header, advances one tick, flushes and `$finish`es — a
tiny header VCD in milliseconds (Verilator's header FST is converted with
`fst2vcd -f`). **The runner scripts wire this whole sequence up
correctly** (header pass → `gen_gtkw` → `gtkwave -a`), so they are the reference
for using it in practice. `--zoom-fit` fits the whole trace to the window.

## Pre-wired scripts

Three scripts in `Scripts/` bundle the Quick start's steps 1–6 with sensible defaults. Each has a
Windows (`.bat`) and a Linux (`.sh`) version, and picks the simulator from a
`--sim iverilog|verilator` flag (default Icarus):

```bat
Scripts\single_proc      .bat / .sh   one C± processor    (edit PROC / FNAM at the top)
Scripts\multi_proc       .bat / .sh   C± multi-proc project
Scripts\single_proc_cpp  .bat / .sh   one C++ processor   (front end: cppcomp)

  ... default (no flag)        -> Icarus Verilog (iverilog + vvp)
  ... --sim verilator          -> Verilator (--binary --timing, +define+YANC_TRACE)
  ... --no-view                -> (single_proc_cpp) run the pipeline, skip GTKWave
```

On Linux make them executable once (`chmod +x Scripts/*.sh`) and run
`Scripts/single_proc.sh` (or `Scripts/single_proc.sh --sim verilator`).

**The C++ runner (`single_proc_cpp`)** feeds a `.cpp` program — the bundled
`Compilers/CPPComp/Tests/proc_cpp` demo — through `cppcomp` instead of
`cmmcomp`, then the *same* assemble / simulate / view back end. cppcomp now emits
the same files cmmcomp does (the `.asm`, `prog_log.txt`, the `pc_<proc>_mem.txt`
PC→source line table and `src_lines.txt`), so the waveform looks just like the C±
flow — **with one rule**: GTKWave only shows C++ variables that live at a **fixed
data-memory address**, because the dump snoops a *constant* write address
(`if (mem_addr_wr == X)`). That covers globals, the locals/params of
non-recursive functions, and `static` locals — int shown as a raw word, float
decoded, arrays element-by-element, exactly as in C±. Stack-frame locals of
recursive functions (no fixed address), pointers (an address, not a value) and
structs (a multi-word aggregate) are deliberately **not** traced. This mirrors
C±, which — for optimization reasons — only ever allocates fixed-address
variables anyway, so both flows show the same kind of thing.
## CLI flags

The five compiler binaries accept **named options** with short and long forms.
Run any of them with `-h` / `--help` for the per-tool synopsis, or
`-V` / `--version` for the version string.

`cmmcomp`, `appcomp` and `asmcomp` produce bilingual diagnostic messages:

```
-pt    Portuguese (default)
-en    English
```

Each tool's options (`[...]` optional; the source file is the argument
without an option):

```bat
:: compilers: the .asm and the tables to the output folder (default ./<name>)
cmmcomp [-o <dir>] [--wave-arrays] <file.cmm>
cppcomp [-o <dir>] [-I <dir>]... [-E] <file.cpp>
:: the assembler: everything to the folder of the .asm
asmcomp [-f <MHz>] [-c <clocks>] <file.asm>

:: the assembly's first pass (addresses and parameters for asmcomp)
appcomp <file.asm>

:: waveform helpers
gen_gtkw <in.vcd> <out.gtkw> <base> [<comp2gtkw>]
comp2gtkw                 (a filter GTKWave runs on complex signals: stdin to stdout)
```

The processor takes the name of the source file (`filtro.cmm` makes the
processor `filtro`, its `filtro.v` and its module `filtro`); for another name,
rename the file. `-o` defaults to the folder `./<name>`, created if missing.
asmcomp's `-f` and `-c`, the testbench's clock and length, default to 100 MHz
and 10000 clocks. The user's
own files (array inits, `input_N.txt`, `cade_k.txt`) are read in the folder of
the source, which the compilers write to `prog_log.txt`. cppcomp preprocesses
the source in memory, as gcc does: `#include "x"` looks in the folder of the
including file, then in the `-I` folders, then in `Header/`; `-E` prints the
preprocessed text and stops; the messages point to the file and line written
(a header's line, too). The folders
that come with the tools are found next to their executable, with no option:
cmmcomp and asmcomp use `Macros/`, asmcomp `SAPHO/`, cppcomp `Header/`, and
`gen_gtkw` uses the `comp2gtkw` beside it. In a
repository build the same search finds `Compilers/CMMComp/Includes`, `SAPHO/`
and `Compilers/CPPComp/Includes`.

`--wave-arrays` (cmmcomp only) shows each array element as its own signal in the waveform; without it the arrays do not appear there (a large array makes many signals).

The short options have a long form (`--output`, `--freq`, `--clocks`, `--help`, `--version`).

Example:

```bat
cmmcomp -en C:\src\proc_fft.cmm
```
