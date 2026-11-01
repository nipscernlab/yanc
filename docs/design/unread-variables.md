# Variables written and never read

A C± variable that the program writes and never reads costs program words
(its `SET`s) and a data word, and shows in the waveform. Since 2026-11-01
it leaves the program; `#WAVE` keeps it when it is there to be seen in the
waveform. Issue #69, step 3.

## The decision

Three options were weighed (Luciano, 2026-11-01):

- (a) keep every variable, as before;
- (b) a variable written and never read leaves everything: its `SET`s, its
  data word, the waveform;
- (c) remove it only when the program asks.

Decided: (b), "so as not to spend memory for nothing; it leaves everything,
as if it did not exist". The examples `ArcTan`, `Seno` and `Sqrt`, and the
manual's first program, compute an error `e = t - y` only to read it in the
waveform. Under (b) both `t` and `e` would leave, so a directive keeps them:
`#WAVE t, e` (Luciano, 2026-11-01).

## The pass

`asm_unread` (`Compilers/common/asm_share.c`), run on the assembly after
`asm_reach` and before `asm_share`, in both compilers.

- A `SET` or `SET_P` on a name of the data class writes it; any other
  instruction that names it reads it. A name in a directive (`#array`,
  `#SHARE`, ...) is read. An array, a `_V` offset or an indirect access
  touches the name through an address, so it counts as read.
- A name written and not read leaves: its `SET`s go, their labels move to
  the next instruction, and a `SET_P` becomes the `POP` it also is. The two
  halves of a comp, `x` and `x_i`, go together or not at all, because
  asmcomp joins them into one waveform signal `comp_x`.
- The pc map (`pc_<proc>_mem.txt`) loses the dropped lines in lockstep, and
  the instruction count falls by as much.
- On a line it does not understand the pass leaves the file untouched.
- cmmcomp passes two callbacks. `gone` marks each removed variable as
  unused, so the existing "never read" warning names it. `keep` answers
  for the variables `#WAVE` named, which count as read.

What counts as a read is a read of the memory word. A value the compiler
takes from the accumulator does not read the variable: in
`t = a[j]; e = t - y;` the subtraction uses the `t` still in the acc, so
`t` is never read. That is why the examples keep `t` as well as `e`.

## What it does not do yet

The computation that fed a removed `SET` stays: in `e = t - y` without
`#WAVE`, the subtraction is still computed and thrown away. Removing it
needs a pass that follows the accumulator's liveness; it is the next step
of issue #69. The same pass would remove the caller's `LOD 0` before a call
to a function whose parameter is never read (14 words in `procBlind`).

cppcomp runs the pass without callbacks: no warning, and no `#WAVE` (a
`#pragma yanc` form is not done).

## Measured (2026-11-01)

On the C± tests, 46 programs lost 960 program words: `cmm_cov_ops` 436,
`sapho_all` 36 (the 29 `SET`s of `main_fw` and `n1_v` to `n7_v`),
`procBlind` 15. No opcode left `sapho_all`, `sapho_all_a` or `sapho_all_b`,
so no hardware block left the processor that is meant to have all of them.
Every simulated output is unchanged; the sim goldens that changed only
gained lines at the end, because the faster programs write more within the
same number of clocks. The C++ tests (129) pass with simulation.

Before this pass a variable never read already shared a data word with
others (`asm_share`, `#SHARE`): the data words gained are few, the program
words are most of the gain.

## For the documentation

- docs/directives.md: the `#WAVE` section (done).
- docs/first-program.md: the `Sqrt` example with `#WAVE t, e` (done).
- docs/messages.md: the new text of the "never read" warning (done).
- README.md, docs/language-cmm.md: "every variable" became "every variable
  the program reads" (done).
