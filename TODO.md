# TODO

Open work items for YANC that are not tracked elsewhere. This file is the
**list**: what is wrong, why it matters, what "done" looks like. The evidence
(file:line, measurements, the full inventory of 32-bit assumptions) lives in
[`docs/design/precision-and-width-review.md`](docs/design/precision-and-width-review.md) and
[`docs/design/hdl-architecture-audit.md`](docs/design/hdl-architecture-audit.md); each item
links to its section there. The measurements themselves are reproducible with
[`Scripts/hw/`](Scripts/hw/README.md) (elaboration, Yosys area/depth, Quartus
Fmax, the divider testbench). Remove an item when it lands (the
history stays in git and in the CHANGELOG).

## Where things stand (2026-10-27)

**v9.1 is the last release (2026-10-20):** the keyboard and the screen
(`myKeyPS2`, `myVGAText`) and C++ `<iostream>`, `<cstdio>` and `<cstring>`
on them. v9.0 (2026-10-19) opened the peripherals with the keyboard; v8.0
(2026-10-18) brought the faster clock of item 29. Each release publishes
the manual from docs/ (GitHub Pages and nipscern.com/library/yanc). Items
2 and 3 closed in v9.0; since v9.1, in the CHANGELOG [Unreleased], items 5,
6(a) and 34 (exact float constants and C± literals in every C form, the
ALU testbench, an initializer on any name of a declaration list).

**Parked on 2026-10-26, documented to be taken up later** (Luciano): two
designs for faster array code, each with its decisions, measurements and
the questions left open, and their code kept in archive tags, not in a
branch. The Dirac notation as expressions computed in loops, with
pointers and `RPT` (docs/design/dirac-expressions.md; tags
`archive/dirac-parser`, `archive/dirac-loops-proto`), and the array unit,
a memory for arrays with a MAC (docs/design/array-unit.md; tags
`archive/array-unit-design`, `archive/array-unit-proto`). Items 28 and 31
wait for them.

**Left on boards:** from v7.1, the run of Sabrina and Pedro Henrique (the
`LINKALL` pair with the `.sdc` on both boards; the Fmax is measured, the
link costs no clock). Items 35 and 37, the keyboard and the screen, closed
on 2026-10-27: Dênis ran both blocks on the board, read and written with
`cade` and `toma`, in his keyboard + VGA example.

**Waiting on a decision:** item 21 (documentation that cannot fall behind
a release): Chrysthofer's proposal arrived on 2026-10-13; its phase 0 is
Luciano and Chrysthofer agreeing on it. The GitHub plan: the proposal says GitHub
Education gives teachers Team for free (Luciano's application).

**Moved to GitHub issues (2026-10-27):** 6(b) and 7, the word beyond 32
bits (#29, parked); 9, the math library keyed on `NBMANT` and shared with
C++ (#31); 10, architecture hardening, as #32 (an instruction-set
simulator and random differential test), #33 (the decode rows from
`isa.tsv`), #34 (the undefined cases, division by zero first) and #35
(HDL hygiene), its interrupt rework, stack overflow pin, I/O contract and
registered-branch option dropped (Luciano); 12, the run-time exception
strobe (#36, parked); 13, what was left of the pre-assembly optimizer, as
#37 (the C++ heap arena, 77 % of the data memory of the programs that
allocate) and #38 (wasted instructions left in the C± goldens).

**Open items, below:** 14 the zero-fill at 8 words a turn, or close it; 15 the
regress on this machine (mitigated); 21 documentation that cannot fall
behind a release (waiting on a decision); 26 arrays in the waveform
without a signal per element; 27 the C++ gaps that real embedded code
hits; 32 the divider stage count from the widths (recorded; on the ZYBO
`F_DIV` limits).

**Parked by decision:** 28 and 31 (the array unit and the Dirac
expressions, above), 24 (processors with different float formats on one
link: AURORA's check), 30 (SAPHO without showing its source, until a
partnership), 33 (the float zero as an all-zero word in hardware), the
reorganisation of the runner/setup scripts, a macOS build.

**Closed, in the CHANGELOG:** 1 and 4 (`#FROUND 1`), 2 (round to nearest
unbiased), 3 (the float format checked only with float), 5 (exact float
constants), 6(a) (the ALU testbench), 8 (the ALU datapath restructuring,
its six steps), 11 (consistency at 32 bits), 16, 18
(hardware for code that never runs), 19 (the dividers, v6.0), 20, 22 and
23 (v7.1), 25 (appcomp folded into asmcomp, v7.1), 29 (a faster clock,
v8.0), 34 (initialisers in a declaration list), the user names that
collide in the `.asm`; 17 merged into 28 (2026-10-18).

---

## Decisions taken (2026-09-18)

- **Word width: 32 bits, for now.** The width sweep
  ([`Scripts/hw/width_sweep.sh`](Scripts/hw/width_sweep.sh)) found Icarus 13.0
  miscomputing division above 32 bits: unsigned `/` from 36 bits up, signed `/`
  in a procedural block above 64. The one form the HDL uses (signed `/` in a
  continuous assign) is right at every width, but a design that only works
  because it avoids three simulator bugs is not a base to build on. Nothing
  above 32 bits is maintained, and no code has to be written to be ready for
  it. Since then the datapath has lost its last `/`; where that leaves the
  question, and what reopening involves, is GitHub issue #29 (parked; it
  took over items 6(b) and 7 on 2026-10-27). [§3, decision A]
- **Hidden mantissa bit: no.** The format keeps its explicit leading one.
  [§1.8]

---

## 14. Inline small leaf accessors (`cppcomp`): what is left

**Status:** both landed -- the core in `fdcc070`, the two remaining cases on
2026-09-24; the profile is done, what is left is below · **Area:** `Compilers/CPPComp/Sources/codegen.c`

`fdcc070` expands a tiny accessor at the call site, with copy propagation:
1.32x on a loop of element accesses. Two cases still called; both are now
fixed:

- **`operator[]` of a class template.** The instantiated methods are clones
  kept in `g_inst[]`, not in `cg_unit->funcs`, so `func_by_label()` never
  found them. Fix: search `g_inst` too (its declaration moves up next to
  `cg_unit`, and the three inliner helpers get forward declarations).
- **The write path, `a[i] = x`.** `gen_addr`'s own overloaded-subscript site
  now expands too: a reference-returning accessor yields the element's
  address, which is exactly the lvalue's.

Regress with it: the C++ phase 76/76; three C± failures that passed when
re-run (item 15 -- cmmcomp is untouched by this change).

**The measurement that matters:**

| program | before | after |
|---|---|---|
| template benchmark, 3232 accesses | 78 991 cycles | 59 791 |
| `test46` (blind deconvolution), real | 76 360 cycles | **76 261 (-0.13 %)** |

`test46` loses all five `operator[]` calls for +2 instructions, but gains
almost nothing: it spends its time in float arithmetic, the linear solve and
the convolution, and its five accesses sit in loops of fifteen. The change is
correct and nearly free, and it removes a real inconsistency (identical code
expanded for a plain class and not for a template, which is the form real C++
uses) -- but as an optimisation it does not pay on the one real program
measured. Committed for the consistency, with the 0.13 % stated.

**The lesson for every optimisation after this one:** a benchmark written to
isolate a cost overstates the gain. Measure on a real program, in cycles.
The cycle-counting tools are in [`Scripts/perf/`](Scripts/perf/README.md)
(`cycles.sh`, the per-address profile, `prof_report.py`, and the test46
recipe and history).

**The profile of `test46` (2026-09-24).** A Verilator harness counts the
cycles the fetch address (`pc_sim_val`, exposed with `+define+YANC_TRACE`)
spends on each program word; a script maps the words to functions and source
lines through the `.asm`, `pc_*_mem.txt` and `src_lines.txt`. Both are in the
cycle bench (`build_prof.sh`, `prof_report.py`). Of 76 311 cycles:

| function | before | after the zero-fill fix |
|---|---|---|
| `main`, all of it `float scratch[2400] = {0.0f}` | 38.2 % | 26.6 % |
| `solve_spd` (Cholesky) | 28.0 % | 33.2 % |
| `inverse_tikhonov_calibrated` | 15.2 % | 18.0 % |
| `sqrt` (the user's 24-step Newton loop) | 9.0 % | 10.7 % |
| `convolve_full` | 7.9 % | 9.4 % |

The zero-fill loop was 12 instructions a word and is now 7: 76 261 -> 64 184
cycles, 15.8 %, in one small change -- against 0.13 % for the whole inliner.

**Done 2026-09-24:** the Cholesky inner loop recomputed `i*n` and `j*n` for
every `k`. A `for` now hoists a loop-invariant address `p + i*n` into a
temporary (`lih_*` in codegen.c), and a one-word element indexed by a plain
variable is `base; ADD idx`: 64 184 -> 60 890 cycles (-5.1 %), the inner
loop 7 280 -> 4 550.

**Also done 2026-09-24/25** (each in the CHANGELOG): the `for` tests at the
bottom (cppcomp always; cmmcomp only when a literal start already satisfies a
literal bound, so C± code never grows -- Luciano's choice); an int literal
takes the memory form; the zero-fill stores four words a turn; the hoist sees
by-value calls and lone variables; integer constant folding. **test46:
76 261 -> 49 684 cycles (-35 %), 1 754 -> 1 628 instructions.**

**Where test46 spends its 49 734 cycles now** (`Scripts/perf/cycles.sh ...
prof`): `solve_spd` 35 %, `main` 25 % (the `scratch[2400]` zero-fill, 5
instructions a word), `inverse_tikhonov_calibrated` 18 %, `sqrt` 11 % (the
user's 24-step Newton loop: nothing for the compiler), `convolve_full` 9 %.

**Next candidates, smallest first:**
- DONE 2026-09-26: a `<fn>_this` word nothing reads loses every SET (peephole
  pass 0b). It covered the out-of-line methods too (`POP; SET <fn>_this;
  LDI k; RET`), not only the expanded accessors: 384 instructions fewer over
  the C++ tests (26 173 -> 25 789, 37 programs), 12 data words; test46 1 613 ->
  1 581 instructions but 49 678 -> 49 661 cycles, its accessors are not in
  the hot loops;
- ~~the backward solve `a[k*n + i]`: step a pointer by `n` instead of `MLT`~~
  -- measured 2026-09-26, not worth it. The inner loop is 18 instructions a
  turn; a pointer stepped by `n` costs the same five as `P_LOD k; MLT n;
  S_ADD` plus the load (the ALU is combinational, `MLT` is one cycle like
  `ADD`). One instruction a turn comes only by also replacing `b[k]` with a
  pointer and eliminating `k` (the end test on the pointer): 18 -> 17, 105
  turns at n = 15, ~105 of test46's 49 711 cycles (0.2 %), for an induction
  variable elimination in cppcomp;
- the zero-fill at 8 words a turn: ~4.5 instructions a word, for more code;
- the Cholesky inner loop is 16 instructions a turn and near the floor of
  this ISA: `k < j` with a variable bound needs `GRE j; LIN; JIZ` (only JIZ).
Declined, do not propose again: SHL for a power-of-two 2D row size
(Luciano: the multiplier is in most programs anyway).

## 15. The regress is not trustworthy on this machine

**Status:** mitigated 2026-09-25, cause not fixed · **Area:** `Scripts/regress.sh`, the machine · **Evidence:** below

Seven full runs in one session; only ONE came back clean. Each other run
failed on a DIFFERENT set of 3-8 heavy float tests, with `vvp exited non-zero`
or empty/truncated output. That matters beyond the annoyance: a run no longer
distinguishes a real regression from noise, and a whole-program pass like the
data-word sharing (`asm_share`) is exactly the kind of change that can
miscompile silently.

What is known, measured:
- the same `.vvp`, re-run in the same directory, alternates exit 0 / exit 1;
- nothing on stderr and no Icarus error, so the process is dying, not failing;
- a failed run leaves the VCD truncated (`cmm_tan`: 4.47 MB of the 5.36 MB a
  good run writes), i.e. it dies mid-simulation;
- it tracks machine memory: the one clean run was with the machine free (19
  min for 134 tests); the bad ones had 1-2 GB free of 15.5 GB and a single
  simulation took 70 s, about 8x slower.

**NOT the waveform writer, measured:** the same testbench with all 16
`$dumpvars` commented out runs in the same time (69.9/69.0 s against
72.0/68.2 s). Gating the dump behind `+WAVE` would save 215 MB of disk per run
and nothing else. The truncated VCD is a symptom of dying mid-run, not the
cause; do not repeat that inference.

**Related, and free:** the working tree carries 288 MB that is not versioned
and regenerates: `.smoke/` (284 MB, of which 215 MB is 118 VCD files nobody
reads), `Teste/`, and `bin/`. `bin/` is worse than bloat: it goes stale (an
`appcomp` from June against sources from September) and then silently
mis-assembles, which has already cost one wrong investigation.

**Latest (2026-09-24), and what the machine looked like:** another run, three
C± failures (`cmm_fround1`, `cmm_fround2`, `cmm_hyper`), all passing when
re-run. At that moment the whole machine used about 3 % of its 22 cores, but
had **1.0 GB free of 15.5 GB**: the slowness is paging, not CPU. What held the
memory: VS Code 4.6 GB (41 processes, five windows), Claude Code 2.6 GB (20
processes: seven sessions in VS Code plus the desktop app), `node` 2.2 GB,
Chrome 1.2 GB. Some of it was idle for hours -- the desktop app (619 MB) and
two old sessions, found by the last-write time of each session's transcript
under `~/.claude/projects/<folder>/<session-id>.jsonl`. That is the quickest
way to see which sessions are only holding memory.

**Mitigated (2026-09-25):** `regress.sh` now re-runs a simulation that DIED
-- vvp exiting non-zero, or in the C++ phase an empty or truncated output (a
strict prefix of the golden) -- up to twice, and lists every retried test in
the summary (`retried (simulation died, TODO.md item 15): ...`). A wrong but
complete output still fails at once. Tested with a fake vvp: a test that dies
once passes and is listed, one that always dies fails. Two deaths in a row
were seen once, hence two retries. The board is repeatable again; the cause
(memory pressure) is not fixed, so a long `retried` list still means: free
memory before trusting timings.

**Meanwhile:** re-run a failing heavy test on its own; if it passes, it is
this. Free memory on the machine before trusting a board.

**Done when:** a full run is repeatable, or the cause is found and named.

## 21. Documentation that cannot fall behind a release

**Status:** Chrysthofer answered on 2026-10-13 with a proposal, "Diretrizes
e arquitetura dos projetos do nipscernlab" (PDF, 28 pages, attached to the
email thread); its phase 0 is Luciano and him agreeing on it. Nothing is
decided yet (opened 2026-10-09) · **Area:** this repo,
nipscernlab/docs_aurora, nipscernlab/.github

The published manual (nipscernlab/docs_aurora, nipscern.com/library/sapho)
stopped at AURORA 6.11.0.5 (2026-08-31, about yanc v5.3: it lists LDA/STA and
old directive defaults), and Aurora Intelligence quotes it. Luciano wants hard
guidelines for every public nipscernlab repo (each repo an independent
project, the guidelines global, in nipscernlab/.github): no release without
green CI, rigorous per-repo tests and coherent documentation; English the
source language, Portuguese generated and reviewed by a person. For yanc: the
yanc chapters of the manual living in this repo and published on the site,
the reference pages generated from `isa.tsv` and the other tables, a release
routine that refuses to tag without them. Every feature since v6.0 has its
docs/ note ending in "For the documentation (item 21)": `command-line.md`
(the v7.1 command line and output folder), `toma-and-cade-links.md` 11
(with three points kept from `toma-and-cade.md` 8.7), `myfifo.md`,
`asm-names.md`.

What the proposal asks of yanc (to weigh, not decided): the language file
(today AURORA's hand-generated `sapho_rules.json`) generated by yanc's CI
at each release and attached to it, the release failing without it (its
phase 4); the manual in English in docs_aurora, built and published by CI,
the SAPHO release locked while the manual is behind; and for the repo
itself `AGENTS.md`, `ROADMAP.md` (from issues and milestones instead of
TODO.md), ADRs, a ruleset on main without exceptions, lint, secret scanning,
a third-party list and `CITATION.cff`.

**Done when** the guidelines are agreed (the email "Diretrizes para os
repositórios do nipscernlab", 2026-10-09, is the basis) and yanc follows them.

## 24. Processors with different float formats on one link

**Status:** parked (Luciano, 2026-10-11: the check belongs to AURORA, which
sees every program of a project; a conversion maybe in the future) ·
**Evidence:** docs/design/toma-and-cade.md section 3,
docs/design/toma-and-cade-links.md section 9

`toma` sends a float's word unconverted and `fcade` reads it as a float:
exact when both processors have the same `#NBMANT`/`#NBEXPO`, garbled with
no warning when they differ (each program is compiled alone); ints with
different `#NUBITS` only get the tools' port-width warning.

## 28. Burst: a whole array through a port, and arrays filled and copied fast

**Status:** design written (Luciano's block, 2026-10-14), nothing
implemented; part of the array unit since 2026-10-24, parked with it on
2026-10-26 · **Area:** `SAPHO/core.v`, `instr_dec.v`, `isa.tsv`, asmcomp,
then cmmcomp and cppcomp · **Evidence:** docs/design/burst.md,
docs/design/array-unit.md (decision 14: bursts, copy, fill)

Sending an array to a FIFO costs 8 instructions a word in a loop, or 2 to 3
a word unrolled (code growing with the array). A `burst` block (size
register, pointer, an `out` that waits in place until the end) sends one
word a clock with fixed code, and stops while the FIFO is full through a new
`out_ok[p]` pin; the same block fills an array from an input FIFO. Not to
be implemented yet (Luciano, 2026-10-14): the idea is recorded. Open: the
accumulator after a burst, the one-word margin against a full FIFO (and an
empty one on input), the element type, the compiler passes that drop and
share arrays, names and opcodes (they fit at the end of the table), the
language forms (note sections 5 and 5a).

**One block for both (Luciano, 2026-10-18).** The burst block and the fast
fill/copy of arrays (item 17 until then, below) are to be designed together:
the same size register and auto-incremented pointer that stream an array
to or from a port can stream it from memory to memory (a copy, one word a
clock, the simple dual-port RAM's limit: one read and one write a cycle)
or write one value into every element (a fill). Study first: what the
block needs for each use (a second pointer for the copy's destination, the
read-to-write pipeline register), what pays only when used, and the
language forms; then decide the ISA once for all three.

### From item 17: filling and copying arrays faster (2026-09-28)


**Status:** open, design discussion (2026-09-28); nothing measured, every
number below is deduced from the `.asm` · **Area:** `SAPHO/core.v`
(`mem_ctrl`, `rel_addr`), `SAPHO/processor.v` (`mem_data`), `instr_dec.v`,
the ISA, cppcomp · **Evidence:** test46's profile (item 14)

**Why.** `float scratch[2400] = {0.0f}` is 24 % of test46's cycles (11 991 of
49 711). The zero-fill loop is 20 instructions for 4 words, 5 a word, because
`STI` writes `mem[base + stack top] <= acc` and POPS: every word reloads the
index, adds its offset, pushes it and loads the zero again. A copy
`b[i] = a[i]` is `LOD i; PSH; LDI a; STI b`, 4 a word.

**The memory's limit.** `mem_data` is a simple dual-port RAM: one registered
read port and one write port, separate addresses. One read and one write a
cycle, so a copy can never beat one word a cycle.

**Options, smallest hardware first** (instructions a word, fill / copy):

| option | hardware | fill | copy |
|---|---|---|---|
| today | -- | 5 | 4 |
| cppcomp: 8 words a turn | none | ~4.5 | 4 |
| assembler: `array+k` operands (LDI/STI take a name OR a number today) | none | ~3.75 | 4 |
| new store, address from the acc, data from the stack, NO pop | write-address mux (stack/acc) + write-data mux (acc/stack), generated only if used | ~1.6 | none (the stack grows) |
| the same with POP (a second decode row, same datapath) | as above | ~3 | 3 |
| index register `X` with post-increment (`SETX`, `LDX a`: acc <= mem[a+X], `STX b`: mem[b+X] <= acc, X++) | an MDATAW register, an incrementer, a third index source in both address adders | 1 (`LOD 0` once, then `STX`) | 2 |
| pipelined copy instruction (read now, write the read word next cycle) | a data pipeline register, the destination base from the stack or a second register, overlap and flush care | -- | 1 (the RAM's limit) |

**Rejected: redefining `STI` as address-from-acc, data-from-stack.** Plain
stores tie, but a read-modify-write at a COMPUTED address needs the address
in a temporary and reloaded: +1 instruction and +1 word each time, in loops.
test46's `out[i + j] += ai * b[j]` (convolution, 5.4 % of its cycles) is
exactly that. Keep `STI`; any new store is a separate opcode.

**Two full memory ports (true dual-port).** Do not speed a copy (a word is
still one read + one write, two accesses a cycle either way). They give two
writes a cycle (fill at 2 words a cycle) or two READS a cycle, the DSP dual
data bus: `acc += mem[a + X] * mem[b + Y]` for FIR / convolution / dot
product, which needs two index registers and a two-address instruction (the
operand field holds one base). Cost on Cyclone V: an M10K is 256x40 in simple
dual-port but 512x20 in true dual-port, so 32-bit memories up to 256 words
take 2 blocks instead of 1 (512 words: 2 either way). A read on the second
port cannot share a cycle with a write, and the mixed-port read-during-write
behaviour has to be defined.

**Compiler side.** Loops cppcomp writes itself (the `{0}` fill,
`copy_to_block`) can use any of these directly. User loops need the
compiler to see that `i` walks the array and replace it with `X` (the
induction-variable elimination item 14 found expensive).

**Recommendation (not decided, 2026-09-28):** if the goal is only fill and copy, the
store options; if SAPHO is to be stronger at DSP, the index register `X`
first (it covers fill, copy and sequential loops, and later a modulo mode
for circular FIR buffers), then `Y` with two full ports if the MAC loops of
real group programs are the bottleneck. Each step measured first: ALUTs and
Fmax in Quartus (a program that uses it and one that must pay nothing), and
test46's cycles.



## 30. SAPHO synthesized without showing its source (if a partnership comes)

**Status:** parked (Luciano, 2026-10-14): to look at only if UFJF partners
with a company that would invest in SAPHO · **Evidence:**
docs/design/ip-protection.md

Shipping `SAPHO/` so Quartus or Vivado synthesize it without anyone reading
it. IEEE 1735 keeps the per-program parameters working, but Quartus
decrypts it only in Pro (the lab's Cyclone V is Lite/Standard; Intel's
Ampcrypt through sales instead), and Vivado needs the AMD key and an
encryption license; a netlist service is the vendor-free option. SAPHO up
to v7.1 is public under MIT and stays so. If it comes: ask Intel and AMD,
test with a small module then `sapho_all`, decide the licence of future
versions with UFJF's technology transfer office first.

## 31. The Dirac output of an array: a wasted multiply, a missing form

**Status:** open, found 2026-10-14 while measuring item 28, not fixed;
since 2026-10-25 part of the Dirac redesign, parked with it on 2026-10-26,
not to be fixed on its own · **Area:** cmmcomp (Luciano's code: the diff
goes to him first) · **Evidence:** docs/design/burst.md section 1,
docs/design/dirac-expressions.md sections 2 and 3.4

`out(0, 1|a⟩);` writes each element as `LOD_V a k; MLT 1; OUT 0`: the
`MLT 1` costs a cycle a word and synthesizes the integer multiplier for a
multiplication by one (pay only for what you use). And `out(0, |a⟩);`,
without the scalar, is a syntax error. Fixed, the form is two instructions a
word (`LOD_V`, `OUT`), the least today's ISA allows, independent of the
burst. A fixture with the scalar 1 and with none, the golden checking that
no `MLT` and no multiplier remain.

In the Dirac redesign (dirac-expressions.md) both defects go without code
of their own: `out(p, X)` takes any vector expression, `|a⟩` included, and a
literal 1 drops out by the folding the expression walker already does.
**Done when** the redesign lands with the fixture of its section 4.

## 32. Divider stage count computed from the widths

**Status:** recorded 2026-10-16, for later (Luciano: 5/4 stay fixed for
now, item 29, closed) · **Area:** `SAPHO/ula.v` (`div_array`), `asm_divseq`,
asmcomp, `hdl.c` · **Evidence:** docs/design/pipeln-and-division.md
section 4

The cut positions already follow the widths (`first_row()` spreads PRE + R
rows + POST evenly over `STG` cycles); the number of cuts does not. The
other direction: fix how much logic fits in a cycle, a budget B in rows,
and take `STG = ceil((PRE + R + POST) / B)`, R = `NUBITS` for `DIV`/`MOD`
and `NBMANT` + 0/1/3 for `F_DIV` at `#FROUND` 0/1/2, PRE/POST the ones in
the code (integer 1.5/2.8, float 0.8/3.6 rows). The measured 32-bit
minima (5 and 4) hold for B between 7.6 and 9.07 rows; with B = 8.5:
integer 32/24/16 bits -> 5/4/3, float `NBMANT` 23 at `#FROUND` 2 or 0 ->
4, `NBMANT` 15 at `#FROUND` 2 -> 3. The gain is in narrower words only (a
16-bit processor saves two NOPs and two cycles a division, and two cuts).
Compute it once, in C, and pass `STG` to the generated Verilog like the
opcode parameters, so the HDL and `asm_divseq` cannot disagree (`hdl.c`
not checked yet). Weak point, deduced and not measured: the rest of the
processor also shortens at narrower widths, so one B in rows may over- or
under-cut; a fit at 16 bits would settle it. Done: the formula in the
compilers, `STG` a parameter of both dividers, regress, one narrow fit.

Seen 2026-10-17 on the ZYBO (Vivado, one run, after item 29): the worst
path is the first part of `F_DIV` (data memory -> array up to its second
cut, 26.5 ns; 37.26 MHz), not the float add as on the DE10-Nano. A fifth
`F_DIV` stage, tried on a copy: 37.79 MHz (noise), the next path (the
result written back to memory) only 0.4 ns behind; Luciano kept 4. The
budget per cycle differs per FPGA family, so a computed `STG` would need a
per-target B, or the larger of the two counts.

## 33. The float zero as an all-zero word, in hardware (parked)

**Status:** parked (Luciano, 2026-10-17: not now) · **Area:** `SAPHO/ula.v`
(normaliser, add alignment, comparisons, `F_NEG`/`F_ABS`, `F_SCL`/`XPO`),
the constant encoder, the compilers · **Evidence:** the float-condition
study of 2026-10-17 (CHANGELOG [Unreleased], `Scripts/fcond`)

What was item 33, a float as a condition, is solved in software (approach
1 of that study, in both compilers: `AND 2^NBMANT - 1` before the `JIZ`,
one word and one cycle per float condition; checked against the host
C/C++ compiler by `Scripts/fcond`, in the regress). Of the other
approaches, `EQU 0.0; LIN` was wrong for -0 at `#FROUND 0` and cost two
words; keeping C+-'s truncation was declined. The one left, kept as an
option: with the explicit leading one a float is zero exactly when its
mantissa is, so the all-zero word can be chosen as the zero's code. Output:
the normaliser emits all zeros for a zero (the sign at 0, also at
`#FROUND 0`); `F_NEG`/`F_ABS` keep a zero a zero. Input: whoever reads the
exponent treats mantissa 0 as the minimum exponent (the add's alignment,
`F_LES`/`F_GRE`, `F_SCL`/`XPO`). Then `JIZ` (the OR of all bits) would be
right for floats with no `AND`, 0 and 0.0 would share a code, and old data
with the old zero code would still read as zero. The input fix sits on the
add, the worst path: it needs a fit. Done: `Scripts/fcond` passing with the
compilers' `AND` removed, the regress, a fit that does not lose clock.

## 27. C++ gaps that real embedded code hits

**Status:** open, noted 2026-10-13 while writing docs/language-cpp.md (each
checked with a program) · **Area:** `Compilers/CPPComp/`

Accepted by C++ compilers, a syntax error in cppcomp: an `enum` value that
is not a plain literal (`= -1`, `= A + 1`) and an underlying type
(`enum class E : int`); operator overloading as a free function, and of
`%`, the shifts, the bitwise operators, `()`; multiple inheritance;
anonymous namespaces; variadic functions; lambdas inside functions or with
captures; template specialization and default template arguments. Also: no
`fin`/`fout`, and `out()` of a float writes its raw bits (C± converts);
`#pragma yanc nugain` is accepted and changes nothing. Each is a candidate
when a real program needs it (the missing-feature protocol: ask first).

## 26. Arrays in the waveform without one signal per element

**Status:** open, noted 2026-10-12 (Luciano) · **Area:** cmmcomp
`--wave-arrays`, asmcomp `hdl.c` (the `$dumpvars` of `sim_name_arr`),
`gen_gtkw`

**Why.** With `--wave-arrays` the testbench dumps every element of every
array as its own variable, so the `.vcd` and the GTKWave signal list grow
with the array sizes (a 2400-word buffer is 2400 signals); without it the
arrays do not show at all. The option exists only to keep that cost away.

**Done when** an array can be watched without a variable per position: for
example one signal for the last write (address and value), or a dump of
the array memory read back by the viewer, chosen after measuring the
`.vcd` size and the simulation time on a test with large arrays (test46).

## Workarounds at `#FROUND 0` (worth a line in the README)

- Keep long-running accumulators (phase, time base, counters) in **integer /
  fixed point**: `ADD` is exact, and wrap-around modulo 2^NUBITS gives a free
  "mod 2*pi" for binary angles.
- Accumulate **deviations** from a nominal value instead of absolute values.
- Raise `#NBMANT` for the processor that accumulates (32/25/6 keeps two
  more mantissa bits than 32/23/8; its constants are encoded exactly).
- A compensated (Kahan) sum does **not** help at level 0: it relies on
  `t - sum` being exact, which loses a bit there. It works from level 1 on.
