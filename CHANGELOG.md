# Changelog

All notable changes to YANC are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project adheres to a loose semantic-versioning scheme on the `v*`
tags consumed by Aurora.

## [v4] – 2026-05-28

### Added
- **CPPComp** — new C++ compiler (`cppcomp`) and preprocessor (`cpppp`)
  targeting YANC's 32-bit / IEEE-754 / 4 K core. Implements tiers 1-3:
  classes, single inheritance, virtual functions + vtables, function
  and class templates with real monomorphization (non-type template
  parameters, mixed type/value args), namespaces and `::` qualified
  names, references, `new`/`delete` over a 4 K heap, RAII (stack
  ctor/dtor), operator overloading (binary / unary / compound /
  subscript / call), `enum class`, `using` aliases, C++ casts,
  range-for, default arguments, in-class method declarations with
  out-of-class definitions, static data members, `static` methods,
  `Class<T>::static`, member-init lists, mini-STL written in the
  language (`Vector<T>`, `unique_ptr<T>`, `std::vector`), and
  `<array>` / `<cstddef>` / `<cstdint>` / `<cstring>` / `<cmath>` /
  `<limits>` / `<bit>` shims under `Compilers/CPPComp/Includes/`.
  End-to-end milestone: FISTA `blind_deconvolve` runs on YANC
  (`Compilers/CPPComp/Tests/test48`, `test49`), validated against a
  reference ARM build (`test50`).
- Single shared `YANC_VERSION` (`Compilers/yanc_version.h`) — bumped
  to `"4.0"`. All five binaries (`cmmcomp`, `cppcomp`, `asmcomp`,
  `cpppp`, `appcomp`) read it for `--version`.
- `cppcomp` long-form aliases for every CLI option.
- CMM `#TOAQUI` directive plus a new `cheguei` processor output pin
  that asserts when PC reaches the marked address. The Verilator
  harness terminates the simulation on that pin (replacement for
  manual `$finish` plumbing in heavy sims).
- CMM `for` loop, desugaring to `while` plus an init / step pair.
- ISA: `LDA` / `STA` base-less indirect addressing and a `LEA`
  pseudo-op.
- Verilator runner for the heavy FISTA tests
  (`Compilers/CPPComp/.work/verilator/`), ~seconds instead of minutes
  on iverilog.
- `regress.sh` now drives the full toolchain end-to-end (cmmcomp →
  appcomp → asmcomp → sim) and diffs the produced output files
  against goldens; multi-proc DTW project covered; `num_ins` ratchet
  prevents future refactors from growing the `.asm`.

### Changed
- **Repo layout** — all compilers now live under `Compilers/`
  (`APPComp/`, `ASMComp/`, `CMMComp/`, `CPPComp/`). `Macros/` and
  `include/` consolidated into a single `Includes/` per compiler.
  `Exemplos/` + `Testes/` folded into `CMMComp/Tests/`. `APP/` and
  `ASM/` renamed to `APPComp/` / `ASMComp/`. `build.bat` (repo root)
  renamed to `Scripts/aurora.bat` with relative paths and no
  hardcoded MSYS2 install root.
- **CMMComp** — expression and statement codegen migrated to a real
  AST: POD `expr` is gone, every expression builds an `expr_node`
  tree that the emit walker traverses; `if` / `while` / `switch` /
  function bodies emit through statement-AST walkers; `emit.c`
  (capture-buffer indirection) retired; symbol table moved to
  `struct symbol` + `v_table`, retiring the SoA parallel storage.
  Added a typecheck pass that annotates `BINOP` / `UNOP` /
  `STDLIB_CALL` / `INNER`, plus algebraic identity folding (`x+/-0`,
  `x*1`, `x/1`).
- **CMMComp** — `#INTERPOINT` keyword renamed to `#PRACA`.
- **CLI** — `cmmcomp` / `asmcomp` `-P` / `--project` renamed to
  `-A` / `--array`; `asmcomp`'s `--array` flag dropped entirely;
  `cppcomp` `-p <proc_dir>` is now the only output mode (the old
  `-o` was removed).
- **HW (breaking)** — `JIZ` now tests the whole accumulator word
  (`if_acc = |ula_out`) instead of just bit 0; counting loops
  (`i++` until zero) now terminate correctly.
- **HDL** — combinational `always` blocks use blocking assignments;
  `asmcomp`-generated testbench flushes output on each write
  (kills regress flakiness from buffered tb output).
- `@fim $finish` moved from the generated processor `.v` to the
  auto-testbench.
- README rewritten to reflect the current toolchain shape;
  GTKWave source-trace screenshot added.

### Removed
- Standalone CNIPS C compiler (added during v4 development, then
  removed once CPPComp covered the C++ — and by extension the C —
  use case).
- CMM `#USEMAC` / `#ENDMAC` user-macro feature (unused).
- `cpppp` `-D NAME[=val]` CLI flag.
- `CPPComp/build.bat` and `CPPComp/regress.sh` shim.

### Release packaging
- `release.yml` now builds and ships `cpppp` + `cppcomp` and ships
  `Header/` (CPPComp `Includes/`) alongside `bin/` / `HDL/` /
  `Macros/`. `Macros/` is sourced from `Compilers/CMMComp/Includes/`
  (the old root `Macros/` no longer exists). `Scripts/` in the zip
  is now a single file (`proc2rtl.ys`) — the gtkwave init Tcls and
  `fix.vcd` are no longer bundled, since `gen_gtkw` emits a static
  `.gtkw` from the VCD.

## [v3] – 2026-05-14

### Added
- MIT `LICENSE` file (was previously "all rights reserved" by default).
- `CONTRIBUTING.md` documenting code, commit, and bilingual-message
  conventions.
- Per-push CI workflow (`.github/workflows/ci.yml`) that builds all four
  binaries (`cmmcomp`, `appcomp`, `asmcomp`, `comp2gtkw`) with
  `-O2 -Wall` on Windows + MSYS2, then smoke-tests the full
  `cmmcomp → appcomp → asmcomp` pipeline against every example in
  `Exemplos/`.
- `MSG_ERR_OUT_OF_MEMORY` bilingual diagnostic (replaces
  `MSG_ERR_TOO_MANY_VARS` / `MSG_ERR_TOO_MANY_LABELS`).
- `-h` / `--help` and `-V` / `--version` on all three compilers, plus
  bilingual `MSG_CLI_*` / `MSG_ERR_CLI_*` diagnostics for malformed
  command lines.

### Changed
- All three compilers now take **named command-line options** instead of
  bare positional arguments (`cmmcomp -i file.cmm -n name -p ... -m ... -t ...`,
  `appcomp -i ... -t ...`,
  `asmcomp -i ... -p ... -d ... -m ... -t ... -f ... -c ...`, with
  `-P` / `--project` for project mode). Each compiler validates that every
  required option is present — and `asmcomp` that `-f` / `-c` are integers —
  printing a usage message and exiting instead of dereferencing a missing
  `argv[]` slot. Parsing lives in a new per-compiler `args.c` / `args.h`.
  `go_proc.bat`, `go_proj.bat`, `build.bat`, and the CI invocations were
  updated to the new flag form.
- Symbol and label tables in all three compilers are now grow-on-demand
  via `realloc` (starting at 256 entries, 128 for the label-nesting
  stack), instead of fixed `NVARMAX=999999` / `NLABMAX=99999` BSS
  arrays. Per-process resident memory drops from ~500 MB of zeroed BSS
  to ~128 KB initial.
- All in-source comments translated from Portuguese to English across
  `APP/`, `ASM/`, `CMMComp/`, `HDL/`, `Scripts/`, `Macros/`, the build
  `.bat` files, and the runnable examples in `Exemplos/`.
- README expanded with pipeline diagram, component table, build
  instructions, CLI usage, a runnable CMM example, and project layout.
- `release.yml` now links the new `args.c` into every compiler build
  (was missing it, which would break the release link step); inline
  comments translated to English.
- `.gitignore` comments translated to English; added `/.smoke/` and
  `.vscode/`.

### Removed
- `.vscode/c_cpp_properties.json` is no longer tracked (IDE-specific
  config with hard-coded MSYS2 paths from the original machine).

## [v2] – 2026-05-13

### Added
- Release workflow now bundles `HDL/` and `Macros/` into the release zip
  alongside `bin/`, so Aurora can extract `components/{bin,HDL,Macros}/`
  in one shot.

## [v1] – 2026-05-13

### Added
- Initial release artifact: tag-driven GitHub Actions workflow that
  builds `cmmcomp`, `appcomp`, `asmcomp`, and `comp2gtkw` with MSYS2 +
  MinGW-w64, packages them in `yanc-bin-vN.zip`, and publishes the zip
  as a release asset.

[Unreleased]: https://github.com/nipscernlab/yanc/compare/v4...HEAD
[v4]: https://github.com/nipscernlab/yanc/releases/tag/v4
[v3]: https://github.com/nipscernlab/yanc/releases/tag/v3
[v2]: https://github.com/nipscernlab/yanc/releases/tag/v2
[v1]: https://github.com/nipscernlab/yanc/releases/tag/v1
