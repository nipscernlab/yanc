<p align="center">
  <img src="https://github.com/nipscernlab/nipscernweb/blob/main/assets/icons/yanc.svg"
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

YANC compiles **CMM** — a small C-like language with first-class fixed-point, floating-point, complex and Dirac-notation operators — all the way down to a synthesizable SAPHO soft-processor, its program/data memory images, and a ready-to-run testbench.

It is the compilation backbone of the [SAPHO](https://github.com/nipscernlab) ecosystem and is consumed by the **Aurora** desktop app.

## Pipeline

```
  foo.cmm  ──►  cmmcomp  ──►  foo.asm  ──►  appcomp  ──►  expanded.asm  ──►  asmcomp  ──►  foo.v  +  *.mif  +  foo_tb.v
              (CMM → ASM)                  (resolve                          (ASM → Verilog HDL +
                                            #USEMAC / macros)                 memory init files + testbench)
```

After `asmcomp`, the generated Verilog is simulated with **Icarus Verilog** (`iverilog` + `vvp`) and visualized in **GTKWave** — both wired up by the provided `go_proc.bat` / `go_proj.bat`.

## Components

| Binary           | Source dir   | Built with    | Role                                                 |
| ---------------- | ------------ | ------------- | ---------------------------------------------------- |
| `cmmcomp`        | `CMMComp/`   | Flex + Bison + GCC | CMM front-end → assembly                        |
| `appcomp`        | `APP/`       | Flex + GCC    | Assembly pre-processor (macro expansion, `#USEMAC`)  |
| `asmcomp`        | `ASM/`       | Flex + GCC    | Assembly → Verilog HDL + memory images + testbench   |
| `comp2gtkw`      | `Scripts/`   | GCC           | Translator: complex-number bit pattern → GTKWave     |

Auxiliary content:

* `HDL/` — reusable Verilog modules (processor core, ALU, instruction decoder, FIFO, ...)
* `Macros/` — assembly macros and lookup tables (`float_sqrt`, `float_sin`, `float_atan`, ...)
* `Scripts/` — Tcl scripts that set up the GTKWave view
* `CMMComp/Tests/` — runnable .cmm example projects (Math, FFT, RLS, DTW, PulseSim, Blind)

## Quick start

### Use pre-built binaries

Download the latest release zip from [Releases](https://github.com/nipscernlab/yanc/releases/latest) and extract it. The zip contains `bin/` (the four executables), `HDL/`, and `Macros/`.

### Build from source

**Requirements (Windows + MSYS2)**

* [MSYS2](https://www.msys2.org/) with the `mingw-w64-x86_64-gcc`, `bison`, and `flex` packages
* Optional, for simulation: [Icarus Verilog](http://iverilog.icarus.com/) and [GTKWave](https://gtkwave.sourceforge.net/)

Edit `build.bat` to point `BLD_DIR` at the install root you want, then run:

```bat
build.bat
```

This compiles all four binaries and stages `HDL/`, `Macros/`, and `Scripts/` next to them.

To run a full pipeline (compile → simulate → open GTKWave) on a single processor or a multi-processor project, edit the `PROJET` / `PROC` / `FNAM` variables at the top of the script and run:

```bat
go_proc.bat       :: single-processor example
go_proj.bat       :: multi-processor project example
```

## CLI flags

All three compilers (`cmmcomp`, `appcomp`, `asmcomp`) take **named options**.
Run any of them with `-h` / `--help` for the full per-tool synopsis, or
`-V` / `--version` for the version string. Diagnostic messages are bilingual:

```
-pt    Portuguese (default)
-en    English
```

Each tool's required options:

```bat
cmmcomp -i <file.cmm> -n <name> -p <proc-dir> -m <macros-dir> -t <temp-dir> [-P]
appcomp -i <file.asm> -t <temp-dir>
asmcomp -i <file.asm> -p <proc-dir> -d <hdl-dir> -m <macros-dir> -t <temp-dir> [-f <MHz>] [-c <clocks>] [-P]
```

`-P` / `--project` selects project mode (multiple processor instances).
Every option also has a long form (`--input`, `--proc-dir`, `--temp-dir`, …).

Example:

```bat
cmmcomp -en -i my_program.cmm -n proc_fft -p C:\proj\proc_fft -m C:\Macros -t C:\Temp\proc_fft
```

## Example CMM

```c
#PRNAME Sqrt
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1

float my_sqrt(float num)
{
    if (num == 0.0) return 0.0;

    int v = (((num << 1) >>> 24) + 22) >>> 1;         // get the exponent
        v = ((((v-22) << 23) + (1 << 22)) << 1) >> 1; // build the float

    float x; copy(v,x);

    x = 0.5 * (x + num/x);   // 4 Newton-Raphson iterations
    x = 0.5 * (x + num/x);
    x = 0.5 * (x + num/x);
    x = 0.5 * (x + num/x);

    return x;
}

void main()
{
    float x[1000] "sqrt_x.txt";
    float a[1000] "sqrt_y.txt";
    float y, t, e;
    int   j = 0;
    while (j < 1000)
    {
        y = sqrt(x[j]);     // built-in (uses macro from Macros/float_sqrt.asm)
        t = a[j];
        e = t - y;
        j++;
    }
}
```

More examples in `CMMComp/Tests/`.

## Project layout

```
yanc/
├── APPComp/              appcomp sources (Headers/ + Sources/)
├── ASMComp/              asmcomp sources
├── CMMComp/              cmmcomp sources + Tests/ (per-proc example projects)
├── CPPComp/              cppcomp sources + Tests/ (per-test C++ programs)
├── HDL/                  reusable Verilog modules
├── Macros/               assembly macros and LUTs
├── Scripts/              regress.sh, comp2gtkw, Tcl viewers
├── build.bat             build all binaries locally
├── go_proc.bat           single-processor end-to-end pipeline
├── go_proj.bat           multi-processor project pipeline
└── .github/workflows/    CI (release on tag push)
```

## Contributing

Issues and pull requests are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md)
for the conventions on commits, comments, and the bilingual `MSG_*`
diagnostic pattern.

## License

MIT — see [LICENSE](LICENSE).
