# Aurora ⇄ YANC: Verilator waveform migration (v4.3)

This note is for the **Aurora** maintainers. YANC v4.3 makes the Verilator
waveform path work end-to-end — the same variables/arrays/PC-line view the
Icarus flow has always shown now appears under Verilator too. Adopting it in
Aurora is **two small changes** (and lets you delete one fragile workaround).

## TL;DR

| # | Change in Aurora | File |
| - | ---------------- | ---- |
| 1 | **Add `+define+YANC_TRACE`** to the Verilator *Wave* build args | `js/compilation/builders/verilator.js` → `buildVerilatorBuildSpec` |
| 2 | **Stop stripping the `$finish` block** — delete the `stripVerilatorIncompatibleLines` workaround | `js/wave/testbench_instrumenter.js` + its call in `js/compilation/compilation_module.js` |

Everything else (warnings, `--binary --main --timing --trace-fst`,
`--no-trace-top`, the 2-pass header/run) stays exactly as it is today.

---

## Why

### The harness is now compiled under Verilator

YANC's generated `<proc>.v` mirrors every user variable/array, the PC→C±
line table, the assembly opcode tap and the I/O ports into dedicated
"sim-visibility" signals. That whole block used to be gated behind
`` `ifdef __ICARUS__ ``, so **Verilator never compiled it** — which is why:

- the waveform had no variables, and
- the testbench's end-of-program handler
  `always @(posedge clk) if (proc.valr10 == N) $finish;` failed with
  `%Error: Can't find definition of 'valr10' in dotted variable 'proc.valr10'`
  (the reg was optimized away before elaboration resolved the reference).

YANC v4.3 folds the guard into `` `ifdef YANC_SIM_VIS ``, which is turned on by
**either** `__ICARUS__` (Icarus predefines it) **or** `+define+YANC_TRACE`.
Every mirrored declaration is tagged `/* verilator public_flat */`, so
Verilator keeps the signal and the `_tb.v` can still reach `proc.valr10`
hierarchically.

**Consequence:** pass `+define+YANC_TRACE` and the harness compiles, the
variables show up, **and `proc.valr10` resolves — so the `$finish` works**.
That is exactly the workaround you no longer need (see change #2).

### The end-of-program `$finish` now works under Verilator

Your `stripVerilatorIncompatibleLines` regex deleted the
`if (proc.valr10 == N) … $finish` block from the Verilator copy of the tb,
with the documented limitation:

> Verilator perde o early-finish de fim-de-programa: a pass-2 roda o cycle
> budget inteiro do .spf (numClocks), mesmo que o programa acabe antes.

With `+define+YANC_TRACE` that limitation is gone. `proc.valr10` is a real,
reachable signal, so the testbench stops the moment the program ends — under
Verilator just like under Icarus. No more running the full cycle budget on
short programs. (Your own code comment listed "ask yanc to gate the handler"
as future-improvement #1; v4.3 does one better and makes it *work* instead of
skipping it.)

---

## Change #1 — pass `+define+YANC_TRACE`

In `buildVerilatorBuildSpec` (`js/compilation/builders/verilator.js`), add the
define to the args array. It is a Verilog define token — one entry, no quoting:

```diff
   const args = [
     ctx.verilatorScript,
     '--binary',
     '--main',
     '--trace-fst',
     '-j', '0',
     ...warnings,
     '--timing',
     '--x-assign', 'fast',
     '--no-trace-top',
+    '+define+YANC_TRACE',
     '-CFLAGS', '-O3',
     '-CFLAGS', '-fstrict-aliasing',
     '--top-module', ctx.simTopModule,
     '-Mdir', ctx.objDir,
     '-y', ctx.hdlPath,
     ...ctx.sourceFiles,
   ];
```

It is safe to pass unconditionally: for a pure-Verilog user testbench with no
YANC harness it just defines an unused macro.

> Note: `+define+YANC_TRACE` is **Verilator-only**. The Icarus flow must **not**
> get it — Icarus already turns the harness on through its predefined
> `__ICARUS__`, and the two guards coexist by design.

---

## Change #2 — delete the `$finish` strip workaround

Once `+define+YANC_TRACE` is in place, the `$finish` block compiles and runs,
so the strip is not just unnecessary — keeping it **removes the early-finish
you now get for free**.

1. In `js/compilation/compilation_module.js` (~line 1380), stop calling
   `stripVerilatorIncompatibleLines`. Feed the **instrumented** testbench to
   the Verilator build unchanged — i.e. `tbForVerilator` becomes the
   instrumented tb itself, no separate stripped copy.
2. Delete `stripVerilatorIncompatibleLines` and `PROC_VALR10_FINISH_BLOCK`
   from `js/wave/testbench_instrumenter.js`, and the matching cases in
   `tests/unit/testbenchInstrumenter.test.js`.

The rest of `testbench_instrumenter.js` (the `$dumpfile`/`$dumpvars` injection
for *user* testbenches that have none) is unaffected — keep it.

---

## What you do NOT need to change

- **Warnings.** Your set (`-Wno-fatal -Wno-TIMESCALEMOD -Wno-DECLFILENAME
  -Wno-STMTDLY`) already covers the project's timescale/initial-delay noise —
  `-Wno-fatal` is what keeps the multi-proc top-level (`TIMESCALEMOD`,
  `INITIALDLY`) from aborting the build. Nothing to add.
- **`--no-trace-top`.** Keep it — it complements the change below: YANC fences
  everything that is not a curated signal out of the trace, and `--no-trace-top`
  drops the testbench scope on top of that.
- **The 2-pass header/run** (`+AURORA_HEADER_ONLY`) and the FST progress
  overlay — unchanged.
- **The "Verilator (top-level)" button** (`--json-only` → `--cc --exe --build`
  with your C++ harness in `verilator_tb.js`) — a different flow (file I/O, not
  waveform). It does not use the YANC harness and needs no change.

---

## Good to know: the trace is now curated

YANC v4.3 also makes the Verilator trace carry **only the signals the
testbench's `$dumpvars` lists** — the user variables/arrays, `valr2` (the
Assembly track), `comp_*`, `linetabs`, the I/O mirrors. It fences the CPU
internals, the PC-delay intermediates, the raw `comp` halves and the
connection/plumbing wires out of the trace with `/* verilator tracing_off */`
(no-op comments for Icarus). The result is a small FST that matches the Icarus
waveform.

**The stack/ULA monitor signals are intentionally absent from the Verilator
trace.** The stack-pointer flags and the ULA rounding-error taps (`fl_max`,
`fl_full`, `pointeri`, `delta_int`, `delta_float`) live deep inside the CPU,
below the fence, so Verilator drops them. This is by design: Verilator is the
**performance** backend, and keeping those taps alive would force it to evaluate
the costly real-valued ULA rounding-error logic on every cycle. So in the
Verilator wave flow the GTKWave **Stack** / **ALU** groups come up empty — those
are an Icarus-only debug feature (`go_proc.bat` / `go_proj.bat`). Bringing them
back under Verilator means paying that speed cost; ask and we can wire a per-proc
`.vlt` opt-in.

One interaction to be aware of for the **Wave Configuration picker**: those
fences are unconditional in the generated `.v`, so under **Verilator** only the
curated set is traceable. If a user overrides the dump list and picks a signal
YANC has fenced (a CPU-internal wire, `valr5`, a raw `comp` half, …), it will
be absent from the Verilator FST even though `$dumpvars` names it — the fence
wins. Under **Icarus** every signal is still dumpable (the fences are
Verilator-only comments). If exposing fenced signals under Verilator ever
matters, it needs a per-proc Verilator config (`.vlt`) on YANC's side — ask and
we will add it.

---

## Follow-up (post-v4.3): progress moved to the terminal

The auto-generated `<proc>_tb.v` no longer writes a `progress.txt` file. The
progress loop now prints to **stdout** instead:

```
Progress: 10% complete
Progress: 20% complete
…
Simulation Complete!
```

Each progress line is `$fflush`-ed so it streams live through the pipe (the
final `Simulation Complete!` / `Info: end of program!` needs no flush — `$finish`
exits normally and the C runtime flushes stdout).

**Aurora impact:** `VVPProgressManager` detects `progress.txt` (the `chrys`
10→100 loop) to drive the percentage overlay; the heuristic lives in
`js/compilation/compilation_module.js`. With the file gone, that detection fails
and the overlay no longer advances for YANC procs. To keep it, parse the
terminal lines instead — you already stream/scan the simulator's stdout for
`$display` output, so match `^Progress:\s*(\d+)%` and feed the captured
percentage to the overlay; treat `Simulation Complete!` / `Info: end of
program!` as 100%. The `progress.txt` detection + the file-size plumbing can
then be deleted.

## Reference: the standalone commands

The repo ships two end-to-end reference scripts that wire this exact flow:

- `go_proc_vl.bat` — one processor: `verilator --binary --timing --trace
  +define+YANC_TRACE --top-module <proc>_tb …` → `V<proc>_tb.exe` → GTKWave.
- `go_proj_vl.bat` — multi-proc project: same with `--trace-fst` and the
  `top_level_tb` as top.

See the **Simulating with Verilator** section of the README for the full
command and the `+define+YANC_TRACE` rationale.
