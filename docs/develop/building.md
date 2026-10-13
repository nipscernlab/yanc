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
├── Makefile        the one place the tools are built from
└── TODO.md         open work items
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
the version every tool prints.

## Dependencies for building

The build needs `gcc`, `make`, `bison` and `flex`: on Windows from MSYS2
(`pacman -S mingw-w64-x86_64-gcc make bison flex`), on Linux from the
distribution (Debian/Ubuntu: `build-essential bison flex`). The simulators
and GTKWave are the same as for using YANC ([Installing YANC](../install.md#to-simulate)).

## Testing

```sh
bash Scripts/regress.sh            # the whole regression (about 30 minutes)
bash Scripts/regress.sh --no-sim   # without simulating the C± tests (a few minutes)
```

The regression builds the tools, compiles every test and compares its
assembly with the golden one, simulates and compares the outputs with
references computed apart from YANC, checks that the bad programs of
`NegTests/` are refused with the right message, holds the instruction set's
copies and the manual's instruction page to `Compilers/common/isa.tsv`,
holds the manual's message reference to the compilers' sources, and runs
`yanc` itself. Every change goes through it. A change to the instruction
table or to a message regenerates its page:
`python3 Scripts/gen_isa_doc.py`, `python3 Scripts/gen_messages_doc.py`.

## The setup script

You don't have to install and wire these up by hand. Run **once**:

```bat
Scripts\setup.bat      :: Windows
```
```sh
bash Scripts/setup.sh  # Linux
```

It **checks** every dependency and **helps you get the missing ones**:

* finds (or, with MSYS2, offers to `pacman -S`) the `gcc` / `bison` / `flex`
  build toolchain;
* produces the YANC binaries in `bin\` — **either** keeping the `.exe` a release
  zip already shipped / downloading the prebuilt ones from the latest release
  (no build toolchain needed), **or** compiling them once from source;
* finds the simulators, offering to `pacman -S` **Icarus** and/or **Verilator**
  from MSYS2 if they're missing;
* **downloads the nipscernlab GTKWave** portable bundle if it isn't already
  present;
* remembers every resolved path so the runners need no manual configuration.

On **Linux**, `setup.sh` does the equivalent through your package manager: it
offers to install `gcc`/`bison`/`flex` and the simulators, and produces the
binaries in `bin/` — building them from source when the toolchain is present, or
downloading the prebuilt Linux tarball from the latest release when it isn't. It
uses the distro `gtkwave` and caches the paths in `Scripts/tools.local.sh` for
the runners. Re-run with `--rebuild` to recompile or `--download` to refetch.

If something can't be resolved automatically (e.g. MSYS2 itself isn't installed
yet), setup prints the exact link and command to fix it, then you re-run it.

### What `setup.bat` does on Windows

On **Linux** `setup.sh` installs the dependencies through your package manager,
then builds the binaries from source into `bin/` (or downloads the prebuilt
Linux tarball from the release when no toolchain is present; `--download` forces
it), uses the distro `gtkwave`, and caches the paths in
`Scripts/tools.local.sh` for `Scripts/env.sh`. The rest of this section
describes the richer Windows `setup.bat`, which covers both ways of getting
YANC and picks automatically:

* **Binary mode** — if `bin\` already contains the `.exe` (e.g. you extracted a
  release zip), it keeps them; no `bison`/`flex`/`gcc` needed. If the build
  toolchain is *missing*, it offers to **download** the prebuilt binaries from
  the latest GitHub release into `bin\`.
* **Source mode** — if you cloned the repo and have the MSYS2 toolchain (or let
  setup install it via `pacman`), it **compiles** the six binaries straight into
  `bin\` once, so later runs never recompile.

It then locates the simulators and **GTKWave** (which must be the
[nipscernlab build](https://github.com/nipscernlab/gtkwave-nipscern/releases) —
setup offers to download its portable Windows bundle if missing), and caches
every resolved path in `Scripts\tools.local.bat` (gitignored). `Scripts\env.bat`
loads that cache for each runner, falling back to a `PATH` lookup for
anything not cached — so if the tools are already on your `PATH`, the scripts
work even before running setup.

Force a specific mode with `Scripts\setup.bat --rebuild` (recompile from source)
or `Scripts\setup.bat --download` (refetch the prebuilt binaries).
## Building from source

Requirements (Windows + [MSYS2](https://www.msys2.org/)):

* The MinGW-w64 cross toolchain — install with `pacman -S mingw-w64-x86_64-gcc`. The build calls `x86_64-w64-mingw32-gcc.exe` (the cross tuple, not plain `gcc.exe`) on purpose: it produces stand-alone Windows `.exe`s with no MSYS2 DLL runtime dependency, so the deployed binaries work on any Windows machine.
* `make`, `bison` and `flex` — install with `pacman -S make bison flex`.
* These tools must be reachable on your `PATH`. The script bails early with the exact line to add if any of them is missing. Typical setup:
  ```
  set PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%
  ```
* Optional, only needed if you want to simulate the generated Verilog — both
  are MSYS2 packages, so they install the same way as the build toolchain:
  * [Icarus Verilog](http://iverilog.icarus.com/) (`iverilog` + `vvp`) — `pacman -S mingw-w64-x86_64-iverilog`, and/or
  * [Verilator](https://verilator.org/) **5.x** — `pacman -S mingw-w64-x86_64-verilator`. Its `--binary` mode drives the MinGW `g++` and `python3` from your `mingw64` toolchain, so keep `…\mingw64\bin` on `PATH`.
  * GTKWave — the [nipscernlab build](https://github.com/nipscernlab/gtkwave-nipscern/releases) to view the waveform.

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
