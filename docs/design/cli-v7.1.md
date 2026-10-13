# The command line and the output folder (v7.1)

What changed for whoever runs YANC between v6.0 and v7.1 (2026-10-11 to
2026-10-13), so the manual can say it. The details and the commits are in
the CHANGELOG (`[Unreleased]`); how to run is in
[`running.md`](../simulation.md).

| before (v6.0) | now (v7.1) |
|---|---|
| four or five tools per program: `cmmcomp`, `appcomp`, `asmcomp` (C++: `cpppp`, `cppcomp`, `appcomp`, `asmcomp`) | one command, `yanc prog.cmm` or `yanc prog.cpp`; underneath, `cmmcomp` or `cppcomp`, then `asmcomp` |
| the processor named by `#PRNAME` (C++: `#pragma yanc prname`) or by `-n` | the processor takes the source file's name; the two directives are errors |
| `<proc>/Software`, `Hardware`, `Simulation`, `Temp` | the user's files beside the source; everything generated in one folder, `./<name>` or `-o` |
| `-i`, `-p`, `-t`, `-n`, `-m`, `-d` | gone: the source is the argument, `-o` the output folder, the tools find `SAPHO/` and `Macros/` themselves |
| `-A` / `--array` | `--wave-arrays` |
| `-en` / `-pt`, Portuguese by default | `--lang=en|pt` (or `YANC_LANG`), English by default |
| `cmm_log.txt`, `trad_cmm.txt` | `prog_log.txt`, `src_lines.txt` |
| C++ errors at a line of `pp.cpp` | at the file and line written, a header's too |
| the tools added to the PATH by hand | `install.bat` / `install.sh` in the package |

## For the documentation (item 21 in TODO.md)

The user-facing text says:
- install: unpack the package and run `install.bat` (Windows) or
  `./install.sh` (Linux) once; a new terminal knows `yanc`;
- `yanc prog.cmm` (or `prog.cpp`) makes the processor `prog` in the folder
  `prog/`: `prog.v`, its memories `prog_inst.mif` / `prog_data.mif` and the
  testbench `prog_tb.v`; `-o` picks another folder, `-S` stops at
  `prog.asm`, `yanc prog/prog.asm` assembles only, `-f` / `-c` set the
  testbench clock and length, `-I` adds a C++ include folder,
  `--wave-arrays` shows each array element in the waveform, `-v` shows the
  tools it runs;
- the processor's name is the file's: to rename the processor, rename the
  file;
- the files the program reads (an array initialised from a file,
  `input_N.txt`, `cade_k.txt`) stay beside the source; the simulation runs
  in the output folder and writes `output_N.txt` there;
- the messages are in English; `--lang=pt` or `YANC_LANG=pt` switches the
  C± tools to Portuguese;
- simulating and the waveform are as before, from the output folder
  (README Quick start).
