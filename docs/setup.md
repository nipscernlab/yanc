# Installing and building YANC

How to get the dependencies, the binaries and a working environment, on
Windows and on Linux. The short version is in the README's
[Quick start](../README.md#quick-start) and [Develop](../README.md#develop).

## Dependencies

YANC runs on **Windows and Linux**, and a single setup script
(`Scripts\setup.bat` on Windows,
`Scripts/setup.sh` on Linux) checks the dependencies and helps you get the
missing ones.

On **Windows**, almost everything comes from a single install:
**[MSYS2](https://www.msys2.org/)**. Its `pacman` packages provide the build
toolchain *and* both simulators, so in practice you only install **two things**:
MSYS2 and the GTKWave build. (`setup.bat` can `pacman -S` the MSYS2 packages and
download GTKWave for you, see [The setup script](#the-setup-script).)

| Dependency | `pacman` package / source | What it's for | When you need it |
| ---------- | ------------------------- | ------------- | ---------------- |
| **MSYS2 — build toolchain** | `mingw-w64-x86_64-gcc`, `make`, `bison`, `flex` | Compiling the YANC binaries from source | Only when building from source (not when using a release zip) |
| **MSYS2 — Icarus Verilog** | `mingw-w64-x86_64-iverilog` | Simulating the generated Verilog | Default flow — `Scripts\single_proc.bat` / `multi_proc.bat` |
| **MSYS2 — Verilator** 5.x | `mingw-w64-x86_64-verilator` | Faster simulation, all user variables in the wave | `--sim verilator` flow — `Scripts\single_proc.bat --sim verilator` |
| **GTKWave** | the [nipscernlab build](https://github.com/nipscernlab/gtkwave-nipscern/releases) (separate download) | Viewing the waveform | To open the trace (any flow) |

> **Why MSYS2 for the simulators too?** `iverilog` and `verilator` are both in
> MSYS2, so a single MSYS2 install covers building *and* simulating — no separate
> Icarus/Verilator installers. The only piece outside MSYS2 is GTKWave.
>
> **GTKWave must be the nipscernlab build** — the runner scripts rely on its
> waveform-formatting behaviour; the generic GTKWave will not work the same way.
>
> ⚠️ **Avoid `mingw-w64-x86_64-gcc` 16.1.0-5.** That MSYS2 package shipped a
> broken `libstdc++` (the `std::string` move constructor symbol is missing), so
> Verilator's `verilated.cpp` fails to link and the `--sim verilator` flow dies
> with `undefined reference to std::__cxx11::basic_string<...>::basic_string(...&&)`.
> Use **gcc 15.1.0-5** (known good). `setup.bat`/`setup.sh` probe for this and
> warn you. To downgrade from the pacman cache:
> ```
> pacman -U /var/cache/pacman/pkg/mingw-w64-x86_64-gcc-15.1.0-5-any.pkg.tar.zst \
>           /var/cache/pacman/pkg/mingw-w64-x86_64-gcc-libs-15.1.0-5-any.pkg.tar.zst
> ```
> The Icarus flow is unaffected.

On **Linux**, the same dependencies come from your distribution's package
manager (`apt`, `dnf`, `pacman`, or `zypper`):

| Package | What it's for |
| ------- | ------------- |
| `gcc`, `bison`, `flex` (Debian/Ubuntu: `build-essential bison flex`) | Compiling the YANC binaries |
| `iverilog` | Default flow — `Scripts/single_proc.sh` / `multi_proc.sh` |
| `verilator` | `--sim verilator` flow — `Scripts/single_proc.sh --sim verilator` |
| `gtkwave` | Viewing the waveform |

> **Note on Linux GTKWave:** there is no Linux build of the nipscernlab fork
> yet, so the Linux flow uses your distro's stock `gtkwave`. It still opens the
> waveform and applies the generated `.gtkw` layout; only the panel chrome
> (dark SST-less view) differs from the Windows fork.

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
