# Building and testing YANC

For whoever builds YANC from source or changes it. Installing a release is
in [Installing YANC](../install.md); the conventions for a change are in
[CONTRIBUTING.md](https://github.com/nipscernlab/yanc/blob/main/CONTRIBUTING.md).

## The repository

```
yanc/
├── Compilers/      the compilers: CMMComp (C±), CPPComp (C++), ASMComp,
│                   Driver (yanc), common/ (the ISA table, the shared
│                   passes), yanc_version.h; Tests/ and NegTests/ under each
│                   front end
├── SAPHO/          the SAPHO processor, in Verilog
├── Scripts/        setup, regress.sh, aurora.bat, the runner scripts,
│                   gen_gtkw.c, comp2gtkw.c, hw/ (elaboration, area, Fmax)
├── docs/           this manual; design/ holds the design notes
├── release/        the README, example/ and install scripts of the package
└── Makefile        the one place the tools are built from
```

| tool | source | built with | does |
|---|---|---|---|
| `yanc` | `Compilers/Driver/` | GCC | the driver: runs the others |
| `cmmcomp` | `Compilers/CMMComp/` | Flex, Bison, GCC | C± to assembly |
| `cppcomp` | `Compilers/CPPComp/` | Flex, Bison, GCC | C++ to assembly (with its own preprocessor) |
| `asmcomp` | `Compilers/ASMComp/` | Flex, GCC | assembly to Verilog, memories and testbench, in two passes |
| `gen_gtkw` | `Scripts/gen_gtkw.c` | GCC | a formatted GTKWave view from a dump's header |
| `comp2gtkw` | `Scripts/comp2gtkw.c` | GCC | the filter GTKWave runs on complex signals |

`Compilers/CMMComp/Includes/` becomes the package's `Macros/`,
`Compilers/CPPComp/Includes/` its `Header/`. `Compilers/yanc_version.h` is
the version `yanc`, `cmmcomp`, `cppcomp` and `asmcomp` print with `-V`.

## Dependencies for building

The build needs `gcc`, `make`, `bison` and `flex`: on Windows from MSYS2's
UCRT64 environment (the package list is in
[Installing YANC](../install.md#to-simulate)), on Linux from the
distribution (Debian/Ubuntu: `build-essential bison flex`). The simulators
and GTKWave are the same as for using YANC ([Installing YANC](../install.md#to-simulate)).

On Windows, clone and run git from one side only, preferably MSYS2's git in
the UCRT64 terminal (`pacman -S git`). Git for Windows installs with
`core.autocrlf=true` and MSYS2's git without it. `.gitattributes` makes
both check text out with LF (the `.bat`, `.cmd` and `.ps1` with CRLF), so
a clone made after that rule (#119) holds the same bytes in either git. A
clone made before it by Git for Windows still has CRLF in its files: once
it has the rule, `git status` is clean in both gits, but the files are
rewritten with LF only by a fresh clone or, with no uncommitted work, by
`git rm --cached -r -q . && git reset --hard`.

## Testing

```sh
bash Scripts/regress.sh            # the whole regression (about 30 minutes)
bash Scripts/regress.sh --no-sim   # without simulating anything (a few minutes)
```

The regression builds the tools, compiles every test and compares its
assembly with the golden one, assembles it, checks with
`Scripts/wave_map.py` that the tables the waveform is built from agree with
each other and with the assembly and put each instruction on a source line
that can have produced it, compares what the waveform will show (each
source line with its instructions, the variables and the messages) with
`golden_map.txt`, simulates, checks that the waveform agrees with the
program (the source-line and instruction tracks show the same address at
every change, a variable changes only in the cycle of a `SET` of it, and
where `out(p, x)` writes a variable the waveform's `x` is the word that
comes out), and compares the outputs with
references computed apart from YANC, checks that the bad programs of
`NegTests/` are refused with the right message, holds the instruction set's
copies and the manual's instruction page to `Compilers/common/isa.tsv`,
holds the manual's message reference to the compilers' sources, holds the
link lines at the foot of `CHANGELOG.md` to its version headings, runs
every combination of the Dirac notation in
`Compilers/CMMComp/DiracCases/cases.tsv` through cmmcomp, holds the
simulated outputs of `dirac_shapes` and `proc_rls` to values computed in
Python (`Scripts/dirac_ref.py`, `Scripts/rls_ref.py`), counts which of
cmmcomp's sites that emit assembly the tests reach, and which a test with
an independent reference checks (`Scripts/cmm_cov.sh`; a site is a
call that writes assembly (`add_instr`, `add_sinst`, the append of a float
routine of `Includes/`), or a cell of an operator table, since every cell
goes out through the one call in `tpl.c`, and a cell that loads differently
with the accumulator free or busy counts once for each state; the tests that
count as checked are in `Compilers/CMMComp/Tests/verified.txt`, and
neither count may grow past `cov_baseline.txt`), and runs `yanc`
itself. Every change goes through it: the quick form (`--no-sim`) once at
the end of a batch, the whole one before the batch's last merge
(AGENTS.md, "Work in batches"). `bash Scripts/cmm_cov.sh --list`
lists every site no test checks. `Scripts/cov_tests.py` writes the generated tests
and their expected output, computed in Python; `Scripts/stack_balance.py`
works out the data stack's depth through every test's assembly and fails
when a statement leaves a word on it. A change to the instruction
table or to a message regenerates its page:
`python3 Scripts/gen_isa_doc.py`, `python3 Scripts/gen_messages_doc.py`.

## The setup script

You don't have to install and wire these up by hand. Run **once**:

```sh
bash Scripts/setup.sh  # Linux, and Windows in the MSYS2 UCRT64 terminal
```

It **checks** every dependency and **helps you get the missing ones**, through
the package manager (`pacman` in the MSYS2 terminal, the distribution's on
Linux):

* finds, or offers to install, the `gcc` / `make` / `bison` / `flex` build
  toolchain;
* produces the YANC binaries in `bin/`: it keeps the ones already there,
  compiles them from source when the toolchain is present, and otherwise
  downloads the prebuilt ones of this system from the latest release
  (`yanc-bin-linux-*.tar.gz`, or under MSYS2 `yanc-bin-*.zip`, which it
  unpacks with `unzip`);
* finds, or offers to install, Icarus, Verilator (with `lz4`, `make`, `gcc`
  and `python` on MSYS2) and GTKWave;
* caches every resolved path in `Scripts/tools.local.sh` (gitignored), which
  `Scripts/env.sh` loads for each runner, falling back to a `PATH` lookup for
  anything not cached.

Re-run with `--rebuild` to recompile or `--download` to refetch. If something
can't be resolved automatically (e.g. MSYS2 itself isn't installed yet), setup
prints the link and command to fix it, then you re-run it.

`Scripts\setup.bat` does the same from `cmd`, for the `.bat` runners: it finds
MSYS2 by its `pacman.exe`, takes the tools from its `ucrt64\bin`, offers to
`pacman -S` what is missing, and caches the paths in `Scripts\tools.local.bat`
for `Scripts\env.bat`. `Scripts\setup.bat --rebuild` recompiles and
`--download` refetches the prebuilt binaries.

## Building from source

Requirements (Windows + [MSYS2](https://www.msys2.org/)):

* The MinGW-w64 cross toolchain of MSYS2's UCRT64 environment — install with `pacman -S mingw-w64-ucrt-x86_64-gcc`. The build calls `x86_64-w64-mingw32-gcc.exe` (the cross tuple, not plain `gcc.exe`) on purpose: it produces stand-alone Windows `.exe`s with no MSYS2 DLL runtime dependency (only the UCRT, part of Windows 10 and later), so the deployed binaries run without MSYS2.
* `make`, `bison` and `flex` — install with `pacman -S make bison flex`.
* These tools must be reachable on your `PATH`. The script bails early with the exact line to add if any of them is missing. Typical setup:
  ```
  set PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%
  ```
* Optional, only needed if you want to simulate the generated Verilog. MSYS2
  ships both simulators only in UCRT64 (no longer in MINGW64), and
  `setup.bat` looks for them in `ucrt64\bin`. The `.bat` runners pass
  Verilator the same gcc 16 workaround as the bash scripts
  (`YANC_VL_CFLAGS`, set in `Scripts\env.bat`; see
  [Installing YANC](../install.md#to-simulate)):
  * [Icarus Verilog](http://iverilog.icarus.com/) (`iverilog` + `vvp`) — `pacman -S mingw-w64-ucrt-x86_64-iverilog`, and/or
  * [Verilator](https://verilator.org/) **5.x** — `pacman -S mingw-w64-ucrt-x86_64-verilator mingw-w64-ucrt-x86_64-lz4`. Its `--binary` mode drives the `g++`, `python3` and `make` of the same MSYS2; the FST trace of `multi_proc` includes `lz4.h`, which the Verilator package does not pull in.
  * GTKWave, to view the waveform — `pacman -S mingw-w64-ucrt-x86_64-gtkwave`.

The actual compile commands live in a single top-level **`Makefile`** (the
source of truth shared by the setup scripts, `aurora.bat`, and the CI/release
workflows). To build the binaries by hand — on Linux, or on Windows under
MSYS2:

```sh
make                              # build all into bin/
make cmmcomp                      # build one
make clean                        # remove bin/ and the flex/bison outputs
make CC=x86_64-w64-mingw32-gcc    # Windows standalone .exe (no MSYS2 DLLs)
```

On Windows `make` emits `.exe` automatically (it keys off `$(OS)`); on Linux the
binaries have no suffix. `Scripts/setup.bat` / `setup.sh` just drive this
Makefile, so you normally don't call it directly.

`Scripts/aurora.bat` builds all binaries (via the Makefile) and deploys them into a sibling `Aurora/components/` checkout. It assumes the two repos sit side by side under a common parent — no absolute paths, no editing required:

```
<parent>\
   yanc\        (this repo)
   Aurora\
      components\   <-- deploy target
```

Run it from anywhere (it derives both paths from `%~dp0`):

```bat
Scripts\aurora.bat
```

It runs `make stage`, the same recipe the release uses, so a local deploy carries exactly what a release would.
