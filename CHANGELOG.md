# Changelog

All notable changes to YANC are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project adheres to a loose semantic-versioning scheme on the `v*`
tags consumed by Aurora.

## [Unreleased]

### Added
- **`Scripts/gen_gtkw.c`** (groundwork, not yet wired) — a C tool that parses a
  VCD header and emits a pre-formatted GTKWave `.gtkw` save file, porting the
  proven logic of Aurora's wave pipeline. It classifies the harness signals by
  name (I/O mirrors, the `valr2` Assembly / `linetabs` C+- tracks with their
  translate files, int/float/comp variables and arrays, the Stack/ALU flag
  groups) and writes the exact `.gtkw` opcodes. Handles **single and multiple
  processors**: every VCD scope owning both `valr2` and `linetabs` is a proc
  instance and gets its own section, with the type derived from the `p_<type>.core`
  sub-scope (so per-type `trad_opcode.txt`/`trad_cmm.txt` resolve) — single-proc
  is the N=1 case. Replaces the runtime `gtk_proc_init.tcl`/`gtk_proj_init.tcl`
  and lets us move to the nipscern GTKWave v4 (which ignores `--script`), opened
  as `gtkwave <vcd> -a <out.gtkw>`.
- **`+HEADER_ONLY` testbench instrumentation** — both the generated `<proc>_tb.v`
  (hdl.c) and the DTW project `top_level_tb.v` now respond to a `+HEADER_ONLY`
  plusarg: they dump, advance one tick, `$dumpflush` and `$finish`, producing a
  VCD that carries only the header (the signal list `gen_gtkw` needs) without the
  multi-gigabyte body. On the DTW project this is 109 KB in 130 ms instead of a
  641 MB full dump, and the resulting `.gtkw` is byte-identical to the one built
  from the full trace. Plusarg-gated, so normal and regression runs are
  unaffected (regress 71/71).

### Changed
- **All four `go_*.bat` flows now use gen_gtkw + the nipscern GTKWave v4.** Each
  builds `gen_gtkw.exe`, writes the `.gtkw` layout from the (header-only) VCD, and
  opens the waveform with `gtkwave --dark --zoom-fit --left-justify <vcd> -a
  <gtkw>` (no `--script`, no `fix.vcd` tab hack, no `--rcvar` — the nipscern fork
  hides the SST pane and reports that rcvar as not found). FST flows
  (`go_proc`/`go_proj` Icarus, `go_proj_vl` Verilator) add a quick `+HEADER_ONLY`
  pass — Icarus writes the header VCD directly; Verilator's tiny header FST is
  converted with `fst2vcd -f`. The `--zoom-fit` restores the whole-wave view the
  Tcl flow did via `Zoom_Best_Fit`.
- **`go_proj.bat` now sets `TMP`/`TEMP`** to its own Temp dir (like `go_proj_vl`),
  so it no longer inherits a stale temp path from a previous bat run in the same
  cmd window (which broke gcc/iverilog with "cannot create temporary file").

### Removed
- **The runtime GTKWave Tcl formatters and their props** — `gtk_proc_init.tcl`,
  `gtk_proj_init.tcl`, `gtk_almost_proj.tcl`, `pos_gtkw.tcl` and `fix.vcd` (the
  empty-tab crash workaround) are gone, replaced by `gen_gtkw.c`. Also removed the
  long-unused `proc2rtl.ys` Yosys script.

## [v4.4.1] – 2026-06-01

A maintenance release on top of v4.4: a flaky-test fix and a clutter pass over
the Verilator harness in the generated `<proc>.v`. No simulation behaviour
changes (Icarus and Verilator traces are identical to v4.4).

### Fixed
- **Flaky DTW regression** — the multi-proc project pass dumped every signal of
  the heavy `top_level_tb` sim through the FST writer, which intermittently
  crashed `vvp` on Windows (exit 1, empty stderr) even though the design output
  was correct and the regression never reads the waveform. The tb's `$dumpvars`
  is now gated behind a `+WAVE` plusarg: `regress.sh` runs without it (no dump →
  stable and faster), while `go_proj.bat` / `go_proj_vl.bat` pass `+WAVE` to
  keep the GTKWave trace. Verified 0/25 sim failures and 3/3 green regress runs
  (previously ~13% DTW failure rate).

### Changed
- **Less Verilator clutter in the generated `<proc>.v`** — the per-signal
  `/* verilator tracing_off */ … /* verilator tracing_on */` wrappers on the
  `valr` PC-delay registers are now one block fence (valr2 traced, valr1+valr3..10
  fenced together), and the `sm_me2`/`e_me2` float decode helpers are emitted once
  in a single fenced block instead of inline-fenced and duplicated across the
  variable and array loops. The raw `comp` real/imag halves (joined into the
  shown `comp_<name>` wire) are likewise emitted as one fenced block instead of
  fencing each half inline. Pure codegen tidy-up, identical behaviour: regress
  71/71 and the Verilator trace still carries `valr2` (not `valr1/3..10`) with the
  ULA monitors still dead-code-eliminated.
- **`/* verilator public_flat */` only where it's needed** — it was stamped on
  every mirror declaration; now it's dropped from the *traced* signals (the
  waveform dump keeps those on its own) and kept only on the ones fenced out of
  the trace and the hierarchically-referenced `valr10` (the `_tb.v` `$finish`).
  Cuts the attribute roughly in half in the generated `<proc>.v`. Validated by a
  byte-identical Verilator VCD signal diff (same 19 signals on proc_fft) and the
  `$finish` still resolving `proc.valr10`.

## [v4.4] – 2026-05-31

Verilator polish on top of v4.3: a clean lint pass, simulation progress on the
terminal instead of a file, and the visibility/performance trade-off spelled
out.

### Fixed
- **Clean Verilator lint on the sim-visibility code** — the `YANC_SIM_VIS`
  helper signals were assigned across mismatched widths, so Verilator flagged
  `WIDTHEXPAND` (and `REALCVT` on a real `%`). Sized them explicitly with no
  behaviour change (the traced values are identical): `core.v` stack
  `pointeri`; `ula.v` signed mantissa/exponent helpers and the integer-remainder
  `val_mod` (now an integer `%` instead of a real one); and in the generated
  `<proc>.v` the `me2` float decode helpers, the `valr1 <= pc_sim_val` PC tap
  (now zero-extended), and an off-by-one in the `me3` complex `'dx` initialiser.
  The remaining warnings on a project come from user-side HDL (top level,
  wrappers, hand-written testbench), not YANC.

### Changed
- The auto-generated `<proc>_tb.v` now reports progress to the **terminal**
  (`$display "Progress: N% complete"` … `Simulation Complete!`, each in-loop
  line flushed) instead of writing a `progress.txt` file. Tools that drove a
  progress overlay off `progress.txt` (Aurora's `VVPProgressManager`) should
  parse the terminal lines instead — see
  [`docs/aurora-verilator-migration.md`](docs/aurora-verilator-migration.md).
- **The stack/ULA monitor signals are intentionally not in the Verilator VCD.**
  The stack-pointer flags and the ULA rounding-error taps (`fl_max`, `fl_full`,
  `pointeri`, `delta_int`, `delta_float`) sit below the
  `/* verilator tracing_off */` fence, so Verilator drops them — keeping them
  alive would force it to evaluate the costly real-valued ULA monitoring logic
  every cycle, defeating the speed that is the whole point of the Verilator
  backend. They stay available under Icarus (the GTKWave Stack/ALU groups are an
  Icarus-only feature). Documented in the README and the Aurora migration guide.
- Dropped the redundant `YANC_SIM_VIS` guards from the generated testbench — the
  tb is a sim-visibility artifact by definition, so those guards never changed
  anything. No behaviour change.

## [v4.3] – 2026-05-31

The Verilator waveform path now works end-to-end: the same
variables/arrays/PC-line view the Icarus flow always had now appears under
Verilator too. Downstream (Aurora) adopts it with two small changes — see
[`docs/aurora-verilator-migration.md`](docs/aurora-verilator-migration.md).

### Added
- **`go_proc_vl.bat` / `go_proj_vl.bat`** — Verilator siblings of
  `go_proc.bat` / `go_proj.bat`. They feed the generated `<proc>_tb.v` (or the
  project's `top_level_tb`) to Verilator (`--binary --timing --trace[-fst]
  +define+YANC_TRACE`), then open GTKWave the same way the Icarus scripts do,
  keeping each processor's user variables in the waveform.
- `docs/aurora-verilator-migration.md` — migration guide for the Aurora repo
  (pass `+define+YANC_TRACE`; drop the `$finish`-strip workaround).
- A **Simulating with Verilator** section in the README, plus the Verilator 5.x
  MSYS2 install and the `+define+YANC_TRACE` rationale.

### Fixed
- **Waveform visibility under Verilator (GTKWave)** — the sim-visibility
  harness (user variable/array mirrors, the PC→C± line table, the assembly
  opcode tap, the I/O port mirrors) was gated behind `` `ifdef __ICARUS__ ``,
  so Verilator never compiled it and the signals vanished from the trace. The
  harness — and the `pc_sim_val`/`mem_wr`/`mem_addr_wr` plumbing it needs in
  `HDL/processor.v` and `HDL/core.v` — now also compiles under
  `+define+YANC_TRACE` (the new `YANC_SIM_VIS` guard folds `__ICARUS__` *or*
  `YANC_TRACE`). Every mirrored declaration is tagged
  `/* verilator public_flat */`, so Verilator keeps it and the `_tb.v` can
  reach `proc.valr10` hierarchically — which means the **end-of-program
  `$finish` now works under Verilator** (no more running the full cycle budget
  on short programs).
- The **stack-pointer flags** (`core.v`) and the **ULA rounding-error** signals
  (`ula.v`) are no longer Icarus-only — the "Verilator rejects them" premise was
  wrong. They are re-gated to `YANC_SIM_VIS`, so the Stack/ALU waveform groups
  populate under Verilator too.
- **Latch-free under Verilator** — signals that were modelled as
  self-referential `always @(*)` (the `fl_max`/`fl_full` stack high-water marks
  and the `in_sim_*`/`out_sig_*` capture mirrors) are now the clocked
  accumulators / registers they actually are, so Verilator no longer infers
  latches or combinational loops. `fl_max`/`fl_full` evaluate on the pointer's
  next value, so the tracked values are bit-for-bit the same as before, on both
  Icarus and Verilator.

### Changed
- **The Verilator trace now carries only the curated `$dumpvars` set.**
  `--trace` would otherwise dump the whole hierarchy; the generated `<proc>.v`
  and `_tb.v` now fence the CPU internals, the PC-delay intermediates
  (`valr1`, `valr3..valr10`), the raw `comp` halves (`me3_*`), the float decode
  helpers, `linetab`, and the testbench plumbing out of the trace with
  `/* verilator tracing_off */` (no-op comments for Icarus). A `proc_fft` trace
  drops from 1124 signals to 19 — the user variables, `valr2` (Assembly track),
  `comp_*`, `linetabs` and the I/O mirrors. Synthesis and the Icarus flow are
  unchanged.
- Documentation: the language is referred to consistently as **C±** (was a mix
  of "CMM" and "C+-"); the `CMMComp`/`cmmcomp`/`.cmm` names are untouched.

## [v4.2] – 2026-05-28

### Fixed
- **Verilator cleanliness across the HDL** — silence the warning
  storm Verilator emits when running yanc HDL through `--lint-only`
  or the FISTA / Aurora Verilator harness. Each fix is value-
  preserving; iverilog regress stays 71/71 byte-identical.
  - **WIDTHTRUNC in every `generate if`**: Verilog parameters
    default to 32-bit integers, and the HDL uses dozens of
    opcode-enable flags (`ADD`, `F_ADD`, `MLT`, `INN`, `JIZ`, ...)
    as `generate if (FLAG)` conditions. Verilator's GENIF expects
    1 bit. Rewrote every such site (and OR'd combinations like
    `F_ADD | F_SU1 | F_SU2`) to `generate if ((EXPR) != 0)` —
    `HDL/ula.v` (47), `HDL/core.v` (6), `HDL/instr_dec.v` (107).
    Comparison-result conditions (`ITRADD > 0`, `TOAQUIADDR > 0`)
    already produce 1 bit and were left alone.
  - **`HDL/addr_dec.v`**: slice the integer loop variable to the
    index width (`index == i[$clog2(NPORT)-1:0]`) so the EQ no
    longer requires a 32-bit expand on the index side.
  - **`HDL/ula.v` comparison modules** (`ula_les`, `ula_fles`,
    `ula_gre`, `ula_fgre`, `ula_equ`): explicitly zero-extend the
    1-bit comparison result to `NUBITS` (`{{(NUBITS-1){1'b0}}, cmp}`)
    instead of letting an implicit ASSIGNW expand fire.
  - **`ula_denorm`**: zero-pad the MAN-bit mantissas in the COND's
    "false" branch so both ternary arms match the MAN+1-bit signed
    target.
  - **`ula_f2i`**: introduce `m_ext = {{(EXP+1){1'b0}}, m}` so the
    shift operand has the same width as `mag` (MAN+EXP+1).
  - **`su1` / `su2`**: cast the 32-bit `F_SU1` / `F_SU2` parameters
    with `!= 0` so the AND with the 1-bit op-equality stays 1 bit
    and matches the 1-bit LHS.

- **COMBDLY + LATCH on the auto-generated testbench's output
  decoder** — `Compilers/ASMComp/Sources/hdl.c` (`hdl_tb_file`)
  used to emit
  ```
  always @ (*) begin
      if (proc_out_en == N) out_sig_N <= proc_io_out;
      out_en_N = proc_out_en == N;
  end
  ```
  which Verilator flagged twice: `<=` inside a combinational always
  (COMBDLY), and the conditional with no `else` inferred a latch on
  `out_sig_N`. Switched to an unconditional combinational
  `out_sig_N = proc_io_out;` — the file-write block below already
  gates on `out_en_N`, so the captured per-cycle value is
  byte-identical to before, just without the warnings.
  The corresponding proc-side sim block in `hdl_vv_file` is left
  alone: it sits inside `\`ifdef __ICARUS__`, so Verilator never
  sees it, and switching it to the unconditional form broke the
  multi-proc DTW project (its top-level testbench depends on the
  latched per-port semantics of those signals).

### Changed
- The early-`@fim` `$finish` handler and the `integer progress, chrys;`
  declaration now sit AFTER the `// signal registration, progress
  bar and finish` comment in the generated `_tb.v`, grouped with
  the rest of the sim harness. No behavior change — purely a
  layout move so all the simulation-only constructs live in one
  block.

### Release packaging
- `YANC_VERSION` bumped to `"4.2"`.
- Zip content unchanged from v4.1 (25 files, no `Scripts/`).

## [v4.1] – 2026-05-28

### Fixed
- `asmcomp`-generated testbench now closes `progress.txt` on the
  early-`@fim` `$finish` path. The testbench has two `$finish` paths
  — an `always @(posedge clk)` block that fires when the program
  reaches `@fim`, and an `initial` block that runs the cycle-budget
  loop — and only the second one was closing the file handle. On
  programs that actually reach `@fim` (the common case) the
  simulation exited with the file still open. The `integer progress,
  chrys;` declaration was moved above the `@fim` always block so the
  handler can `$fclose` it.

### Changed
- `Scripts/` is no longer copied from yanc into Aurora. Neither the
  local `aurora.bat` deploy nor the release zip ship anything under
  `Scripts/` anymore. Aurora manages its own scripts (`proc2rtl.ys`,
  `copy-components.js`, `download-*.js`, ...), and the GTKWave Tcl
  init / `fix.vcd` path was replaced by `gen_gtkw` emitting a
  static `.gtkw`.
- `aurora.bat` hardened: the cleanup phase now `rmdir` + `del` +
  `mkdir`s each yanc-managed folder (`bin`, `HDL`, `Macros`,
  `Header`), so an interrupted previous run that left a stray FILE
  named `bin` (where the directory should be) is auto-repaired on
  the next invocation. All `move` commands got `/Y` and all `xcopy`
  commands got `/I` so the script never prompts.

### Docs
- README dropped two stale `#USEMAC` references. `appcomp` never had
  a `#USEMAC` directive — that was the CMM user-macro feature
  removed in v4. The `appcomp` row now reads "first pass over the
  `.asm`: records processor params + resolves variable/label
  addresses for `asmcomp`", which is what it actually does.

### Release packaging
- `YANC_VERSION` bumped to `"4.1"`.
- Release zip content shrinks from 31 to 25 entries: `bin/` (6) +
  `HDL/` (6) + `Macros/` (5) + `Header/` (8). No `Scripts/`.

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

[Unreleased]: https://github.com/nipscernlab/yanc/compare/v4.2...HEAD
[v4.2]: https://github.com/nipscernlab/yanc/releases/tag/v4.2
[v4.1]: https://github.com/nipscernlab/yanc/releases/tag/v4.1
[v4]: https://github.com/nipscernlab/yanc/releases/tag/v4
[v3]: https://github.com/nipscernlab/yanc/releases/tag/v3
[v2]: https://github.com/nipscernlab/yanc/releases/tag/v2
[v1]: https://github.com/nipscernlab/yanc/releases/tag/v1
