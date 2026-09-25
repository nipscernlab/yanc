# Changelog

All notable changes to YANC are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project adheres to a loose semantic-versioning scheme on the `v*`
tags consumed by Aurora.

## [Unreleased]

### Added
- **An exact constant encoder, `Compilers/common/yanc_num.c`** (TODO 5,
  step 1 of 5; not wired into any compiler yet). It reads decimal text (with
  an exponent: `1e-8`, `.5`, `-2.5e-3`) as a ratio of big integers and
  encodes it into the SAPHO word `{s, e, m}` rounded to nearest, ties to
  even, with no host `float`/`double` on the way and no C integer ceiling
  (the word comes out as a bit string); it flags overflow, underflow,
  denormals and the `#FROUND >= 1` flush. `Scripts/check_yanc_num.py` holds
  it to an exact Python model (3 750 cases over 32/23/8, 32/25/6, 16/10/5
  and two narrow formats, including constructed ties and the denormal and
  overflow edges); `regress.sh` runs it. Measured on the constants of the
  test programs: 45 of 264 come out one unit lower in the last mantissa bit
  than today's `f2mf`, which rounds twice (to a host float, then half-up) --
  e.g. pi/2 at 23 bits is 6 588 397.33 * 2^-22, and `f2mf` gives 6 588 398.

### Fixed
- **The regress tells a real failure from a simulation that died**
  (`Scripts/regress.sh`, TODO 15). On a machine short of memory vvp can die
  mid-run with no message, and a different 3-8 heavy tests failed each run.
  A simulation that died -- vvp exiting non-zero, or in the C++ phase an
  empty or truncated output (a strict prefix of the golden) -- is now run
  again, up to twice, and every retried test is listed in the summary; a
  wrong but complete output fails at once. Tested with a fake `VVP` that dies
  once (the test passes and is listed) or always (it fails). Also: the
  summary's `failed:` list printed one name per line (a global `IFS`); both
  lists are one line now. The cause (memory pressure) is not fixed.
- **`aurora.bat` deploys exactly what a release ships** (`Makefile`,
  `release.yml`, `Scripts/aurora.bat`). The release built every binary and
  copied HDL/, Macros/ and Header/ whole; `aurora.bat` copied a fixed list of
  six executables (leaving `gen_gtkw` out, which the Aurora Intelligence
  lists) and the folders without their subfolders. Both now use one recipe,
  `make stage STAGE=<dir>`: every binary the Makefile builds plus the three
  folders with `cp -r`, rebuilt from empty. `aurora.bat` copies that tree
  whole (`xcopy /E`), and only wipes Aurora's folders after the build
  succeeded (a failed build used to leave Aurora with no YANC). The Makefile
  header documented a `make install` that did not exist; it documents
  `stage` now. Checked: 27 files, the same set as `yanc-bin-v5.4.zip`.
- **A C++ reference operand gives its referent's value** (`cppcomp`,
  `codegen.c`). The memory-operand shortcuts for a binary op and for
  `++`/`--` took a reference variable's word as the value, but that word
  holds the referent's address: with `int& r = x;` (x = 5), `z + r` gave
  104 for 105, `z * r` 400 for 500, and `++r` stepped the stored address,
  leaving `x` alone and the reference pointing elsewhere. Those paths now
  skip references, as the store path already did. `test79`.
- **A program with 2 or fewer data words compiles** (`appcomp`, `asmcomp`).
  appcomp refused any program whose data memory had 2 words or fewer ("this
  processor is totally useless"), which caught the smallest real program, a
  button-to-LED loop (`main_x` and the constant `1`), on a new student's
  first task. The limit was off by one and in the wrong tool: 2 words work
  in hardware; what breaks is 1 or 0, where the data address,
  `$clog2(MDATAS)` bits, would be 0 bits wide (`[-1:0]`, refused by Icarus).
  asmcomp now pads the data memory to 2 words with zeros the program never
  addresses, so every size compiles, and the check and its message are
  gone. New `cmm_tiny2` (the reported program), `cmm_tiny1` and `cmm_tiny0`
  (no data word at all), checked by value. Found by Sabrina Amaral and
  Pedro Henrique. Also: asmcomp ran the `#NUGAIN` power-of-two check twice.
- **A value-initialized local is zeroed on every call** (`cppcomp`,
  `CPPComp.y` + `codegen.c`, TODO 16). `int a[8] = {};`, `P q = {};`,
  `P p{};`, `int x{};` and `float f{};` as locals emitted nothing: the grammar
  left them to the `.mif`'s zero, which a local, with its fixed storage, only
  has on the first call. The declaration is now marked value-initialized; a
  type that is constructed (a constructor, or a vtable) is zeroed and then
  constructed, so a user default constructor still runs; any other type goes
  through the empty braced list (zeros plus default member initializers).
- **A float zeroed by a brace initializer equals `0.0f`** (`cppcomp`,
  `emit_zero_words`). The zero-fill stored the integer word 0, which is zero
  in arithmetic but not the `0.0` constant (the encoder gives it the most
  negative exponent, and `EQU` compares words): in `float a[6] = {1.0f};`,
  `a[3] == 0.0f` was false. The fill now picks the zero the way the `.mif`
  of a global does (`agg_fill_code`), so a local and a global agree.
- **A class template's constructor runs for a local** (`cppcomp`,
  `resolve_ctor`). Its constructors are clones in `g_inst`, which the lookup
  did not search: `TC<int> t;` and `TA<int> u(5);` ran no constructor at all.
  `test78` covers the three.
- **A mid-run reset now restarts the program in every case** (`HDL/core.v`,
  `prefetch`). Instruction memory reads synchronously, so the word fetched on
  the reset edge is the first one executed after it -- and during `rst` the
  fetch address was the old PC. When that word was a `JMP` / taken `JIZ` /
  `CAL` / `RET`, it ran right after the reset and sent the program back where
  it was (typically into its `while (1)`). The fetch address is now 0 while
  `rst` is high, so every reset behaves like the boot one. Measured before the
  fix: `ResetCheck` failed in 3 of the 5 phases of its spin loop and passed
  only because its single reset happened to land on a good one. Its
  testbench now resets five times, one cycle later each time, and the regress
  requires all six bursts equal. Fmax not re-measured (one extra term on the
  instruction-address mux).
- **A call with no argument no longer overwrites a partial result**
  (`cmmcomp`, `EXPR_FUNC_CALL` in `ast.c`). An argument's load pushes a live
  accumulator (`P_LOD`); with no argument nothing did, so `(x+5) - f()` lost
  `x+5` and popped garbage: -100 instead of -95. A `PSH` now goes before the
  `CAL` when the accumulator is live. Older than the optimisations below.
- **The Sethi-Ullman reorder no longer moves a call across its sibling**
  (`cmmcomp`, `ast.c`, from `75d18d2`). `f(1) + (f(2)+1)*(f(3)+1)` ran the
  calls as f(2), f(3), f(1), so a callee with a side effect gave 303 instead
  of 1613; and moving a call after a heavy subtree ran it with a live
  accumulator (the bug above). Operands are no longer swapped when either
  side has a call, a `++` or an `in()`/`fin()`. A plain variable next to a
  call is still read as the op's memory operand, after the call: `g + f(1)`
  sees f's write to g, as gcc does (C leaves that order unspecified).
  `cmm_callseq` checks both by value.
- **`m[1] = 5` on a 2D array is refused** with a constant index too
  (`ass_array_const`, the store half of the entry below).
- **`cos()` wrote two instructions on one asm line** (`exec_cos` in
  `stdlib.c`: `F_ADD 1.570796327CAL float_sin`, four missing `\n`; the
  assembler's lexer happened to split them, so the result was right). Also a
  stray space after a `\n` in `oper.c`.
- **An array read only through constant indices is no longer reported as
  unused, and a constant index is checked like a variable one** (`cmmcomp`,
  `arr_1d2exp_const` in `array_index.c`, `ass_array_const` in
  `data_assign.c`). The direct-addressing fast path (`LOD_V`/`SET_V`, commit
  `d4d9f06`) copied only the emit of the general path: it never set
  `v_table[].used`, so `out(0, a[0])` warned that `a` was unused, and it
  skipped the checks, so `x[0]` on a scalar and `m[1]` on a 2D array compiled
  silently where `x[k]` and `m[k]` are refused. The assembly of a valid program
  is unchanged. The CMM negative phase of `regress.sh` gains three rejected
  fixtures and a second manifest, `NegTests/nowarn.txt`: valid programs that
  must compile without a given message.

### Added
- **`sapho_all`: one program that instantiates every SAPHO block**
  (`Compilers/CMMComp/Tests/sapho_all/`). At 32/23/8, `#FROUND 2`, it makes
  the processor generate 100 of the 104 opcode blocks (every int and float
  operator in its memory, stack and push+memory forms, the conversions, both
  input reads), the data and instruction stacks, FFT addressing, the
  `#PRACA` interrupt entry (pin tied to 0 in simulation) and the `#TOAQUI`
  pin. Inside a `while (1)` it reads a constant (7) from port 0, folds every
  result into a 32-bit sum and outputs it reduced to 4 bits (XOR of its
  nibbles), for a board with four LEDs: `4` every turn. The expected value
  comes from `model.py`, written from the operator definitions in
  `HDL/ula.v`, not from a run. `Scripts/check_blocks.py` checks the
  generated `.v`: every opcode parameter set except the four with no C±
  form (`SF_SCL`, `XPO_M`, `LDA`, `STA`, listed in `blocks_except.txt`), and
  it also fails if one of those becomes instantiated. `regress.sh` step 4a
  runs it and requires every sim-golden line to equal the model's value.
- **cmmcomp warns when a local array is initialized from a file**
  (`int t[4] "t.txt";` inside a function, `main` included). The file is
  `.mif` content, loaded once, so the array keeps what the function wrote
  into it from one call to the next, like a C `static`. A global array
  gives no warning. `NegTests/nowarn.txt` now also takes `|+<text>`: a
  valid program that must print that text (two fixtures, local and global).

### Changed
- **Integer constant expressions are folded** (`cppcomp`, `cfold`). An int
  expression of literals only -- a `constexpr` like `L >> 1` or `FL - 1`
  arrives as literals -- is computed at compile time in the word's
  arithmetic (32-bit two's complement, wrapping; `/` `%` truncate; signed
  `>>` arithmetic) and becomes one literal, so a compare against it takes
  the memory form too (`k < FL - 1` is `GRE 28`). Left to the ALU: division
  by zero, `INT_MIN / -1`, shifts out of range, unsigned literals. `test46`
  1 667 -> 1 628 instructions. `test83` checks the edges against host gcc.
- **The loop-invariant hoist sees more loops** (`cppcomp`, `lih_*`). A
  variable passed to a function whose parameter is by value can no longer be
  written by it, so it does not block the hoist (only a reference parameter,
  or an unknown callee, does); and a lone invariant variable next to a
  varying part is hoisted too (`h[k + d]` -> `t[k]`). `test46`: 51 540 ->
  49 710 cycles, 3.5 % fewer (the Toeplitz build and the autocorrelation).
  `test80` gains a by-value call in the loop and a lone-variable index.
- **Zeroing a large local brace initializer stores four words a turn**
  (`cppcomp`, `emit_zero_words`). From 16 words up, the `n % 4` lowest words
  are stored singly and the loop covers groups of four top-down, paying its
  control once per four stores: 5 instructions a word where one a turn took 7.
  `test46` (`float scratch[2400] = {0.0f}`): **56 345 -> 51 540 cycles, 8.5 %
  fewer**, for 22 more instructions. `test82` covers the four remainders,
  several start words, int and float, and the words next to the run.
- **A C± `for` with a literal start and bound tests only at the bottom**
  (`cmmcomp`, `for_bottom_test` in `ast.c`). For `for (k = c0; k OP c1; ...)`
  with int literals and `c0 OP c1` true, the entry test is known to pass, so
  the loop drops it and tests right after the step, jumping back while the
  condition holds: `k < 10` is `LES 9; JIZ top`, with the `LOD k` dropped by
  the peephole. Smaller AND faster, and only then: any other loop keeps its
  top test, so no program grows (a rotation of every loop would have cost
  one instruction each). 8 examples shrink, 42 instructions in all
  (`for_loop` 88 -> 78, `cmm_break` 110 -> 100), 2 cycles fewer per turn;
  7 sim goldens grew because more turns fit the cycle budget (each checked
  to start with the whole old output). New `cmm_forbottom`, values by hand.
- **A C++ `for` tests its condition at the bottom** (`cppcomp`, `S_FOR` +
  `gen_jump_true`). The condition is checked once on entry and then right
  after the step, jumping back while it holds: no `JMP` per turn, and with
  no label between the step and the test the peephole drops the reload of
  the stepped variable. The ISA only has `JIZ`, so the bottom test is the
  inverted comparison (`k < 10` tests `k >= 10`, i.e. `LES 9`). An int
  literal operand also takes the memory form now (`k < 24` was
  `P_LOD 24; S_LES`), and `a <= c` / `a >= c` become `a < c+1` / `a > c-1`
  with a literal, one instruction each. `test46`: **60 890 -> 56 415 cycles,
  7.3 % fewer**, and smaller (1 682 -> 1 653 instructions). When the init
  gives the variable a literal that already satisfies a literal bound
  (`for (int k = 0; k < 10; ...)`), the entry test is dropped too, as in
  cmmcomp: test46 1 653 -> 1 635 instructions, 56 345 cycles. `test81` covers
  the loop shapes (every comparison, `continue`, `break`, `&&`/`||`, a call
  in the condition, float, nested, zero trips, a bound at `INT_MAX`),
  against host gcc.
- **A loop-invariant address is computed once, before the `for`** (`cppcomp`,
  `lih_*` in `codegen.c`). In `for (...) s -= a[i*n + k] * a[j*n + k];` the
  part `a + i*n` does not change while `k` runs; it is now hoisted into a
  pointer temporary and the access becomes `t[k]`. Only when provably safe:
  a plain pointer local or parameter, an invariant part of int locals /
  parameters and literals with `+ - *` (and at least one operator), none of
  them written in the loop (assignment, `++`, declared inside, passed to a
  call, address taken) or escaping the function; never in a recursive
  function or one with `goto` / labels / inline asm. And a one-word element
  indexed by a literal or a plain int variable is now `base; ADD idx`
  instead of the push / load / `S_ADD` path. `test46` (blind deconvolution):
  **64 184 -> 60 890 cycles, 5.1 % fewer**; its Cholesky inner loop 7 280
  -> 4 550. `test80` covers the cases that must NOT be hoisted (the variable
  written in the loop, through a pointer, through a reference call, the base
  pointer moved), plus a zero-trip and a nested loop, against host gcc.
- **A 2D array index multiplies by the row size as a constant** (`cmmcomp`,
  `array_index.c`, `data_declar.c`). Each 2D array had a data word
  `<name>_arr_size`, filled at run time where the array was declared
  (`LOD 75; SET dtw_arr_size`), and every index read it (`MLT dtw_arr_size`).
  The row size is a constant of the declaration, so the index now multiplies
  by it directly (`MLT 75`): two instructions and one data word fewer per 2D
  array, no cycle more per access. In `dirac_assign` and `proc_rls` the word
  was filled and never read.
- **cmmcomp emits 209 fewer instructions over 53 examples, about 6.5 % of
  its own output** (`ast.c`, `oper.c`; C±'s point is lean code):
  - `while (1)` / `do ... while (1)` no longer test the constant (2
    instructions, and 2 cycles a turn);
  - an int `switch` dispatches on the differences between its cases with the
    value kept in the accumulator (`ADD d; JIZ body` per case, taken modulo
    2^NUBITS), with no copy in `switch_exp`: `cmm_switch` 88 -> 68;
  - `if (a == b) break;`, `if (a != b) continue;` and the like, when the body
    is only a jump, are one `JIZ` straight to its target (`acc = a ^ b`, or
    `acc = (a == b)`);
  - a comparison with one operand in the accumulator uses the memory form
    (`X op acc`, the relation reversed when the acc holds the left operand):
    `GRE b` for `P_LOD b; S_LES`, and a float against a float in the acc is
    one instruction where it was four;
  - the first parameter is not reloaded at the top of the function body.
  Every simulated output is unchanged; 42 sim goldens grew because the loops
  now fit more turns into the same number of cycles (checked: each new
  output starts with the whole old one). New `cmm_cmpforms` (every operand
  shape of the comparison, <, = and >) and `cmm_ctlforms` (the loops, the
  jump-only ifs, switch cases out of order, a far case value at 32 bits).
- **Zeroing what a local brace initializer leaves out takes 7 instructions a
  word instead of 12** (`cppcomp`, `emit_zero_words` in `codegen.c`). The loop
  kept two counters, an index going up and a count going down; it now keeps
  one index walking down to the first word, entering the loop in the
  accumulator (`SET`, `STI` and `JIZ` leave it alone): 7 instructions a word
  when the run starts at word 0 or 1, 8 otherwise. Found by a cycle profile
  of `test46`, where `float scratch[2400] = {0.0f}` was 38 % of the run:
  **76 261 -> 64 184 cycles, 15.8 % fewer**, same output, 14 instructions
  smaller. `test77` covers the three shapes of the exit test.
- **The inliner also expands a class template's `operator[]`, and the write
  side `a[i] = x`** (`cppcomp`, `codegen.c`). A class template's methods are
  clones kept in `g_inst[]`, not in the unit's function list, so the lookup
  never found them: identical code was expanded for a plain class and called
  for a template, which is the form real C++ uses. The address path
  (`gen_addr`) had its own subscript call site; a reference-returning
  accessor's expanded body yields the element's address, which is the
  lvalue's. On a template benchmark: 78 991 -> 59 791 cycles. On `test46`
  (the blind deconvolution): all five `operator[]` calls are gone for +2
  instructions, but **76 360 -> 76 261 cycles, only 0.13 %** -- that program
  spends its time in float arithmetic, the linear solve and the convolution,
  not in element access. Kept for the consistency; a cycle profile of
  `test46` is the next step (`TODO.md` item 14).
- **A tiny accessor is pasted at the call site instead of called** (`cppcomp`,
  `codegen.c`). A call to a small function costs far more than the work it
  does: `a[i]` through a class `operator[]` was four instructions at the call
  site plus six in the callee. A non-virtual, non-recursive, non-template
  function whose body is exactly one `return <expr>;` -- scalar parameters, no
  call inside -- is now expanded in place, on the method-call path and on the
  overloaded-subscript path. With **copy propagation**: an argument that is
  already a plain scalar variable of exactly the parameter's type is not
  copied, the parameter is bound to that variable's own word (only when the
  body writes nothing). `this` is computed last, so the existing peephole
  drops the body's reload of it. The access is now five instructions where
  the call was ten. Measured on a loop of 3232 accesses, same output:
  **79 003 -> 59 799 cycles, 24.3 % fewer, 1.32x**, closing 59 % of the gap
  to a native array (46 703). The price is size where a call site runs once:
  +19 instructions over eight C++ tests (0.34 %), all in `test21` and
  `test23`; the other six are unchanged. Not yet expanded: `operator[]` of a
  class template, and the write path `a[i] = x`. `TODO.md` item 14.
- **A label rides on the next instruction, and unreachable code is gone**
  (`cppcomp`, `codegen.c`). Two passes at the end of the peephole. A label
  used to be carried by a `NOP` of its own (`@Cnt__ctor NOP`), which cost
  one instruction -- one cycle, the ALU being combinational -- per label; it
  now rides on the instruction that follows (`@Cnt__ctor POP`), which is how
  `cmmcomp` has always written labels and what the assembler expects. And an
  instruction that follows an unconditional `JMP` or a `RET` without carrying
  a label cannot be reached, so it is dropped. Measured over eight C++ tests:
  6307 -> 5577 instructions, **11.6 % fewer**, from 7.2 % (`test70`) to
  13.0 % (`test46`). No output changes -- the same 141 tests pass. The new
  passes run last, because the older ones match on a bare mnemonic that a
  label in front would hide. C± assembly is unaffected: it never had either
  pattern. `TODO.md` item 13 records what is left, which needs a separate
  whole-program tool.
- **`real()` / `imag()` hand back a memory operand** (cmmcomp `stdlib.c`): for
  a `comp` variable or constant, the half is returned as the variable's own
  word (`c` / `c_i`) instead of being loaded into the accumulator, so the
  consumer fuses it (`F2I_M c`, `F_NEG_M c_i`, `F_ADD c_i`, `F_MLT c`, ...).
  `real(c*c)` (comp already in the accumulator) is unchanged. No output value
  changes; a 45-context probe shrinks 350 → 325 instructions, and seven
  fixtures shrink (`cmm_comp_arith` 109 → 103, `cmm_comp_div` 107 → 104,
  `cmm_comp_func` 126 → 117, `cmm_comp_mix` 269 → 245, `cmm_comp_sqrt`
  180 → 175, `cmm_conj` 65 → 60, `func_combos` 68 → 66), so their `golden.asm`
  and `golden_sim` (more iterations in the same clock budget) were re-blessed.
  One shape grows by two: `complex(real(d), expr)` now spills `expr` to
  `aux_var` before loading `real(d)`.
- **Float comparisons without the denormaliser** (`ula_fcmp`): `F_LES` and
  `F_GRE` now compare the raw words lexicographically — the magnitude key is
  the exponent with its sign bit flipped followed by the mantissa, and the
  sign selects or reverses that order — instead of aligning both operands and
  comparing the aligned magnitudes. A zero mantissa is still the value zero
  whatever the exponent carries, so `+0 == -0` as before. A processor that
  only compares floats no longer builds the alignment shifter: 314 → 132 LUT4
  and 20 → 10 LUT4 levels at `#FROUND 0`, 403 → 132 / 18 → 10 at level 2
  (Yosys, no resource sharing, 32/23/8). One that also adds floats pays ~2 %
  more LUT4 at the same depth, because there the comparator used to ride on
  the adder's denormaliser. Step 3 of the ALU restructuring (`TODO.md` item 8).
  The order assumes what every word the machine holds satisfies — a normalised
  mantissa, or the minimum exponent, which is what `ula_norm` produces and what
  the constant encoder clamps denormals to; a raw unnormalised word from an
  input port can be misordered, as it already breaks `F_MLT`/`F_DIV` at
  `#FROUND >= 1`.
- **One shifter for `SHL`/`SHR`/`SRS`, one for `F2I`** (`ula_shift`,
  `ula_f2i`): only one shift executes per cycle, so the three barrel shifters
  are folded into one right shifter — a left shift is a right shift of the
  bit-reversed word (the reversal is wiring), and the arithmetic shift only
  changes the bit that enters from the top. `F2I`'s `<<`/`>>` pair is one
  `>>` the same way. Same results bit for bit, including amounts beyond the
  word width. A processor that uses the three shifts: 504 → 363 LUT4 at the
  same depth; `F2I` 387 → 350; the whole division-free ALU −3.5 % (level 0) /
  −5.6 % (level 2) LUT4 (Yosys, no resource sharing, 32/23/8). A processor
  with a single shift opcode builds exactly what it built before. Step 5 of
  the ALU restructuring.

- **One default parameter set, everywhere** (audit 1.7). The defaults in
  `processor.v`, `core.v`, `ula.v`, `asmcomp` and `cmmcomp` had drifted apart
  — `processor.v` said `NUBITS 16` next to `NBMANT 23` + `NBEXPO 8`, a format
  that does not add up, and `asmcomp`/`cmmcomp` carried a second float format
  (23/16/6) that nothing reached. They now all carry `cppcomp`'s set, which is
  the one actually exercised (every C++ test omits the `#pragma` lines and
  runs on it): `NUBITS 32 = NBMANT 23 + NBEXPO 8 + 1`, `NUGAIN 128`,
  `SDEPTH`/`DDEPTH` 128, `FFTSIZ 3`, `FROUND 0`, named in one comment block at
  the top of `processor.v`. No generated processor changes: the `<proc>.v` the
  assembler writes passes every parameter explicitly. What changes is a hand
  instantiation, and a directive a program forgot to write. The unused
  `NBOPCO` parameter of `ula` (and the connection feeding it) is gone.

- **`#NUGAIN` must be a power of two.** `norm(x)` is `x / NUGAIN` in hardware;
  a power of two is a shift, anything else infers a constant divider that
  becomes the ALU's critical path (measured at 32 bits: 100 → 70 LUT4 levels,
  3 → 38, against 12 for 64 — the clock halved). `asmcomp` now refuses any
  other value, so every front end is covered, and `cmmcomp` refuses it with
  the source line. Every shipped example uses 128. `NUGAIN` is typed
  `signed [NUBITS-1:0]` in `processor.v`, `core.v` and `ula_nrm` as it already
  was at the ALU top. Step 6, the last of the ALU restructuring (`TODO.md`
  item 8).

### Added
- **The instruction set in one file** (`Compilers/common/isa.tsv`,
  `Scripts/check_isa.py`, a regress phase). The ISA was written out in four
  places that nothing kept in step -- `ASMComp.l`, `instr_dec.v`, `ula.v` and
  `core.v` -- where a disagreement is silent: a program assembles and then
  runs as a different program. The table lists all 116 mnemonics with their
  opcode, operand class, and what each does to the accumulator, the stack,
  the data word its operand names, the control flow and the ports. The
  regress now holds `ASMComp.l` and `core.v` to it, and re-derives the effect
  columns from the naming convention so a typo cannot survive. It
  deliberately does not parse `instr_dec.v` or `ula.v` line by line: a regex
  over Verilog breaks on innocuous edits, and generating all four copies from
  the table is the real answer (the rest of `TODO.md` item 10.2). The effect
  columns are a careful reading of the naming convention and of `ASMComp.l`'s
  own comments, not a measurement -- the simulator of item 10.1 is what will
  validate them. They are what a whole-program optimiser reads to know
  whether an instruction can be removed.
- **`cppcomp` warns when a variable asks for more than a word.** YANC stays at
  32 bits, so `long long`, `unsigned long long`, `int64_t`, `uint64_t`,
  `double` and `long double` are one 32-bit word, as they always were — but
  now each variable declared with one (global, local, loop variable,
  parameter, field, array, static member, or through a `typedef`) gets
  `warning: 'x' asks for 64 bits, but a YANC word has 32: the requested size
  is ignored and 'x' is a 32-bit integer` (or `float`). Pointers, typedefs,
  casts and `sizeof` do not warn, nor does `long`, whose 32 bits are what C++
  requires. `<cstdint>` defines `int64_t`/`uint64_t` through `long long` so
  they warn too. New fixture `test67`, and `regress.sh` now checks compiler
  warnings: a C++ test with a `warnings.txt` must produce exactly the warnings
  listed there.
- **`Scripts/hw/width_sweep.sh`** — checks whether Icarus and Verilator
  compute every integer operator right at a given word width, in continuous
  assigns and in procedural blocks, against a Python-bigint oracle. Its first
  run (Icarus 13.0, Verilator 5.048) decided that YANC **stays at 32 bits**:
  Icarus miscomputes unsigned `/` from 36 bits up and procedural signed `/`
  above 64, so work beyond 32 bits is parked until a simulator upgrade passes
  the sweep (`TODO.md` item 6(b)). At 32 bits it found one real inconsistency:
  `DIV` of `INT_MIN` by `-1` is `INT_MIN` under Icarus and `0` under Verilator
  (`TODO.md` item 11).
- **`Scripts/hw/tb_alu.sh`** — a self-checking unit testbench for the shared
  shifter, `F2I` and the float comparison, in four formats (8/4/3 to 64/52/11)
  and at every `#FROUND` level, with the expected values derived in the
  testbench (`<<`/`>>`/`>>>`, a native two-direction shift model, real-valued
  arithmetic) rather than blessed. Five injected bugs are each caught. It is
  the seed of the ALU testbench `TODO.md` item 6 asks for. `elab.sh` now also
  elaborates the shifts and both dividers, which its opcode list had never
  covered. It also covers the integer `DIV`/`MOD` at the signed edges
  (`INT_MIN`, `INT_MIN / -1`, division by one), where it caught a Verilog trap
  worth knowing: a ternary with an unsigned operand turns a signed division
  unsigned. `area.sh` gains `div` and `mod` configurations.

### Fixed
- **A `static` local is built the first time control reaches it**
  (`cppcomp`, `codegen.c`). Every static local used to be initialised at
  program start together with the globals, so a function that was never
  called still ran its static's constructor, an initialiser that reads a
  global saw the value it had before `main` did anything, and the
  construction order did not match C++. Each static now carries a `__once`
  word, zeroed at `main` entry (so a reset starts over) and set after the
  initialiser, which is emitted at the declaration instead. A static with
  nothing to run keeps costing nothing. `test76` runs the timing cases —
  never-called, first call, second call, a static inside a branch, an
  initialiser reading a global, a brace-initialised array — against a `g++`
  run of the same program. `test70` is unchanged: it compares how many
  objects were built, which this timing does not alter. `TODO.md` item
  11(d), the last one, so item 11 is closed.
- **An input word at or above 2^31 keeps its bits under Verilator**
  (`Compilers/CPPComp/Tests/Verilator/sim_main.cpp`). The harness read the
  input file with `fscanf("%d")` into an `int`, which saturates at
  `INT_MAX` on overflow: `3000000000`, `2^31` and `4294967295` all reached
  the processor as `0x7fffffff`, three different words collapsed into one.
  Icarus's `$fscanf` takes the low 32 bits instead, so the same input file
  drove the two simulators differently. The harness now reads wide and
  truncates to the word, matching Icarus. `cmm_bigin` (Icarus) and `test75`
  (Verilator) feed the same six words and expect the same six results.
  `TODO.md` item 11(b).
- **The signed rendering of an output word is now written down** (not a
  behaviour change): a port carries `NUBITS` raw bits and has no type, so
  both testbenches print it as signed decimal and `out(0, 3000000000u)`
  reads back as `-1294967296` — the same bits, the other rendering. The two
  simulators always agreed here; what bites is comparing against a host run
  printed with `%u`. The note sits where the value is written, in
  `ASMComp/hdl.c` and in the Verilator harness. Giving the port a signedness
  was considered and rejected: the same port can carry both kinds in one
  program, so the information does not belong to it.
- **`T x(N::v);` declares an object instead of failing to parse**
  (`cppcomp`, `CPPComp.l`/`CPPComp.y`). With a namespace-qualified first
  argument the declaration was a syntax error: after `T x(` the parser has
  one token of lookahead, and a qualified name could still begin a parameter
  *type* (`Filter f(std::uint8_t v)` is a prototype), so it took the
  prototype path. The lexer now classifies the **last** component of a
  qualified chain and hands the first name a token that already says which
  it is — a chain ending in a type keeps `NS_IDENT`, one ending in a value
  gets the new `NS_VIDENT` — so `Filter f(geo::scale)` and
  `Filter f(outer::inner::k)` declare objects. A chain whose first name is
  itself a type (`Color::Red`, `Filter::apply`) is untouched, and the
  grammar keeps its 16 shift/reduce conflicts. `test74` covers the cases
  against a `g++` run of the same program. `TODO.md` item 11(e); the plain
  `T x(v);` form landed earlier as `test71`.
- **`INT_MIN / -1` now gives the same answer under both simulators**
  (`HDL/ula.v`, `ula_div`). It is the one signed quotient that does not fit
  in a word: Verilog defines it as the wrapped result (`INT_MIN`), which is
  what Icarus computed and what every other YANC integer operator does on
  overflow, while Verilator's runtime guards the host divide trap and
  returned `0`. The same program therefore printed different numbers
  depending on which simulator ran it. `ula_div` now names the case and
  returns the wrapped quotient, so both simulators — and synthesis — agree.
  Costs 8 LUT4 on a `DIV`-only processor (1796 → 1804) with the critical
  path unchanged at depth 373 (Yosys, no resource sharing, 32/23/8); a
  processor without `DIV` is untouched. `test72` (Icarus) and `test73`
  (Verilator) run the same nine divisions and share a golden. `TODO.md`
  item 11(a). Division **by zero** is still undefined and still differs
  (`x` under Icarus, `0` under Verilator) — that is item 10.5.
- **C± `++` on floats, on array elements, and inside expressions**
  (`cmmcomp`, `data_use.c`). Three faults in one operator, found by a probe
  of every conversion, arithmetic and I/O form against the same program
  compiled by gcc (everything else matched):
  - `x++` on a float added the *int* literal `1` (`F_ADD 1`): the assembler
    encodes it as an integer, whose bits the float adder reads as an
    unnormalised number, so `1.5` became `2.0`. The literal is now `1.0`.
  - `a[i]++` and `m[i][j]++` reloaded the index into the accumulator *after*
    the sum, but `STI` takes the index from the stack: the incremented value
    was thrown away and a stray word was stored at whatever the stack held
    (an `x` in simulation). A `hist[k]++` histogram stayed all zeros. The
    index now goes to the acc and onto the stack (`PSH` keeps the acc) before
    `LDI`, and `STI` finds it there — same length in 1-D, two instructions
    shorter in 2-D, since the index is no longer computed twice. The
    `pplus_arr` fixture had this frozen in its goldens (`0 0 0 …` where
    `1 2 3 …` was due); they are re-blessed.
  - `x++` inside an expression yielded the *new* value: `buf[n++] = v` wrote
    one slot too far and `while (k++ < 3)` ran twice. It is now C's old
    value, only in expression form: a scalar keeps a copy on the stack and
    `SET_P` (SET + POP) brings it back (+1 instruction, int or float), an
    int element re-subtracts (`ADD -1`, exact modulo 2^NUBITS, +1), a float
    element keeps a copy in `aux_var` (+2). The statement form `x++;` — every
    `++` the examples use — generates exactly what it did: the other 53 C±
    goldens are byte-identical. New fixture `cmm_pplus`, expected values
    from gcc.
- **C++ exact-width 8- and 16-bit integers wrap like on a PC** (`cppcomp`).
  `int8_t`, `uint8_t`, `int16_t` and `uint16_t` were plain 32-bit words, so
  `uint8_t b = 255; b++;` gave 256 and a `uint8_t` checksum never wrapped.
  They are now types of their own (`<cstdint>` spells them with the builtin
  `__yanc_int8` ... `__yanc_uint16`): a value stored in one — assignment,
  initializer, argument, return, cast, `++`/`--`, `+=` — wraps to its width
  (`AND 255`; a signed one is also sign-extended), a literal is wrapped at
  compile time, and a value that already fits (from a narrower type) costs
  nothing. In arithmetic they are ints, as C++ promotes them, so
  `uint8_t x = 5; x > -1` is true, as on the host. `char` and `short` stay one
  32-bit word, which C++ allows (a `CHAR_BIT == 32` target, like several
  DSPs). Also fixed: `++`/`--` on a bitfield stepped the whole word that holds
  it, carrying into the neighbouring fields (`x.b++` on a field above bit 0
  changed `x.a`); it now steps the field. `test66` grows to 93 lines, against
  host g++.
- **`T x(v);` parses when the first argument starts with a variable**
  (`cppcomp`): `Filter f(k);`, `Filter f(k + 1);` and `Filter g_f(g);` were
  syntax errors, while a literal argument worked. After `T x(` an identifier
  could still have begun a namespace-qualified parameter type
  (`std::uint8_t`), so the parser took the function-prototype path. The
  lexer now returns a name followed by `::` as a token of its own
  (`NS_IDENT`), so a plain identifier there starts the arguments; qualified
  names, chains like `blind::dsp::f` included, parse as before. A type name
  as the first argument still reads as a prototype, as in C++; a
  namespace-qualified variable there (`T x(N::v);`) does too, which C++
  would not (`TODO.md` 11(e)). New fixture `test71`.
- **Every C++ object is constructed** (`cppcomp`). A constructor ran only for
  a plain local object and `new T`; a global object (`Filter g_f;`,
  `G g2(9);`), a static local, an array element, `new T[n]` and a member
  object all stayed zero — including a global polymorphic object, whose vptr
  was never set. And a class without a constructor got no implicit one, so
  its default member initializers (`struct Cfg { int gain = 5; };`) were
  never applied. Now the implicit default constructor is synthesized when it
  has something to run; every constructor first builds its base (unless its
  member-init list does) and its member objects — one the member-init list
  names with the list's arguments (`: inner(5)` lowered to `inner = 5`,
  which copied the object at address 5); arrays and `new T[n]`
  construct each element in a loop; globals are constructed at program
  start, after every global value, in declaration order; static locals there
  too. A braced initializer gives what it leaves out its default member
  initializer or zero — also on a second call: a local keeps fixed storage,
  so `float buf[8] = {0};` used to zero only `buf[0]` from the second call
  on. New fixture `test70`, against host g++, counts constructions so a
  missing or doubled one shows.
- **A C++ `struct` is a class** (`cppcomp`): a tagged `struct` accepted
  nothing but fields, so a method, a constructor, a base class or an access
  label in it was a syntax error, and a `struct` template was never
  instantiated. It now takes the same body as a `class` (C++ tells the two
  apart only by the default access, which is not enforced); a struct defined
  inside a class restores the enclosing class when it closes. Anonymous and
  C-style structs are unchanged. New fixture `test69`, against host g++.
- **C++ bitfields laid out and masked at the 32-bit word** (`cppcomp`). Without
  `#pragma yanc nubits`, struct layout packed bitfields into 16-bit words
  while everything else ran at 32: four 8-bit fields took two words, so a
  `union` of them with an `unsigned` read `513` where the host reads
  `67305985`. Layout now falls back to the target width, like the rest of the
  compiler. And a field as wide as the word (`unsigned w : 32`) always read
  and stored 0: its mask was `(1L << 32) - 1`, undefined with Windows' 32-bit
  `long`, which came out as 0; masks are now computed in 64 bits. Programs
  whose bitfields fit in 16 bits get the same code as before. New fixture
  `test65`, against host g++ values (the old compiler got 8 of its 11 lines
  wrong).
- **Signed C++ bitfields keep their sign** (`cppcomp`): a read shifted and
  masked the field but never sign-extended it, so `int s : 4` holding `-1`
  read `15` and `-8` read `8`, and the error carried into arithmetic. A read
  of a signed field narrower than the word now ends with `(v ^ s) - s`, `s`
  the field's sign bit (two instructions, only on such fields). New fixture
  `test66`, the first test dedicated to signed/unsigned on the signed-only
  ALU: besides the bitfields it pins the compensations cppcomp already made
  for `unsigned` and had no test for (bit-31-flipped comparisons, the
  software divider behind `/` and `%`, `SHR` vs `SRS`, C's int/unsigned
  mixing). Expected values from host g++; the old compiler gets 10 of its 34
  lines wrong. What still differs from the host is listed in `TODO.md`
  item 11.
- **C++ values are converted to the type they are stored in** (`cppcomp`).
  Every destination — assignment, initializer, argument, `return`, cast,
  aggregate slot, bitfield — now converts to its own type, where before only
  int/float was converted and only at some of them:
  - **arguments were never converted**: a float passed to an `int` parameter
    arrived as its raw bits (`plus1(2.9f)` gave 1977404622, not 3); an int
    passed to a `float` one only worked for small positive values, which
    happen to read the same in the YANC float format;
  - **`unsigned` ↔ `float`** went through the signed `I2F`/`F2I`, so
    `float f = 3000000000u` gave `-1294967296.0` and `unsigned u = 4e9f`
    saturated to `2147483647`. Two helpers, emitted only when used, fix it:
    `u2f` halves a word with bit 31 set (keeping its low bit as a sticky bit,
    so the rounding stays exact) and doubles the result; `f2u` takes 2^31 off
    a value at or above it and puts bit 31 back;
  - **`bool`** held whatever was stored in it (`bool b = 5` held 5,
    `bool b = 0.25f` held 0); a conversion to `bool` is now `!= 0` — two
    instructions for an integer, a mantissa test for a float — and none when
    the value is already 0 or 1 (a comparison, `&&`, `!`, a literal);
  - integer literals with a `u` suffix, or too big for `int`, are `unsigned`,
    as in C++ (they were `int`, so `float f = 4294967295u` gave `-1.0`).

  The runtime helpers (`udivmod`, the heap, `u2f`, `f2u`) are now emitted
  after the template instances: an unsigned division used only inside a
  template left `CAL udivmod` without its helper, and the program printed
  nothing. `test66` grows to 62 lines (the old compiler gets 31 wrong); new
  `test68` covers the helpers used only in templates.
- **Comparison of a very small value against a much larger one** at `#FROUND`
  0 and 1: the alignment shifted the smaller operand out of existence, so
  anything more than `#NBMANT` binary orders below the other operand compared
  **equal** to it — most visibly `x < 0.0` was false for `|x| < 2^-NBMANT`.
  The lexicographic compare orders them. Level 2 was already correct (the
  sticky bit kept the shifted-out operand visible).

## [v5.4] – 2026-09-15

The floating-point datapath of the ALU, reworked. A new `#FROUND` directive
picks how precise the float operators are (0 = the v5.3 datapath, bit for
bit; 1 = no bit lost, saturation, canonical zero; 2 = round to nearest even),
and `cppcomp` always asks for level 2, so every C++ processor now rounds
correctly. Along the way the adder, the comparators, the divider and the
normaliser were restructured — same results, fewer LUTs, higher Fmax (a
division-free float processor goes 45 → 51 MHz on a Cyclone V; one that
divides, 8 → 14 MHz) — two old bugs in the constant encoder were fixed, and
every float block is now gated by the opcodes the program uses.

### Changed
- **Float adder restructured** (`ula_denorm`, `ula_fadd`): the operands are
  ordered by exponent with both differences computed in parallel (no negation
  in series), only the smaller mantissa goes through a barrel shifter (one
  shifter instead of two), and the addition is done in sign-magnitude form —
  equal signs add the magnitudes, different signs subtract both ways in
  parallel and keep the non-negative one — instead of converting both operands
  to two's complement, adding, and converting back. Same results bit for bit
  at every `#FROUND` level (all goldens unchanged). `F_LES`/`F_GRE` now share
  one comparison unit (`ula_fcmp`) working on the aligned sign-magnitude
  operands instead of a subtraction each. Yosys LUT4 (32/23/8): `F_ADD`
  684 → 628 (level 0), 907 → 812 (level 2); `F_LES`+`F_GRE` 439 → 314;
  depth `F_ADD` 37 → 35 levels at 32 bits, 64 → 52 at 64/52/11. Quartus
  (Cyclone V C6): a division-free float processor (`cmm_cexp`) goes from
  45.1 to 51.0 MHz (+13 %) with 3 % fewer ALMs; processors that use
  `F_DIV` stay at ~8 MHz, where the combinational divider sets the clock.
- **Shorter rounding path at `#FROUND 1/2`** (`norm_mux`, `ula_norm`,
  `ula_denorm`): `F_MLT` and `F_DIV`, which their operators already align to
  the top bit, bypass the leading-zero count and enter the rounding/range
  stage directly; the exponent and the overflow/underflow checks are
  evaluated for both outcomes of the rounding carry in parallel with the
  increment (carry-select); the level-2 sticky bit of the denormaliser comes
  from a thermometer mask instead of a second barrel shifter. Level 0 is
  untouched (Quartus produces the identical netlist). At levels 1/2 the results
  are the same bit for bit for normalised operands, which is everything the
  ALU produces there; denormal constants no longer exist at those levels (see
  the encoder entry below). Quartus (Cyclone V C6), `cmm_cexp` built at level 2:
  35.0 → 40.3 MHz (+15 %), 903 → 916 ALMs. The level-2 Fmax cost on a
  division-free processor drops from −31 % to −21 % of level 0 (51.0 MHz).
- **`F_DIV` is an explicit restoring divider array** instead of the `/`
  operator. With a normalised divisor the quotient has only `NBMANT+1(+G)`
  bits, so the array keeps that many rows (23 / 24 / 26 at levels 0 / 1 / 2)
  where `/` built one per dividend bit (45 / 46 / 49). Same quotient bit for
  bit at levels 0 and 1 (20 000 random operands checked against `/`, at
  32/23/8 and 16/10/5). At level 2 the array exposes the remainder, so the
  sticky bit is exact (`remainder != 0`) and every division is correctly
  rounded — the approximate third quotient bit is gone (goldens of level-2
  divisions may move by 1 ULP, toward the exact value). Division by zero:
  levels ≥ 1 saturate to ±max (the ALU's ±∞); level 0 gives a defined
  all-ones quotient where `/` gave `x`. Quartus (Cyclone V C6), a processor
  that divides (`cmm_fround0`): **8.1 → 14.3 MHz (+77 %)**, 1743 → 1323 ALMs
  (−24 %). Yosys (32/23/8, no sharing): `F_DIV` 2678 / 511 → 1825 / 339 LUT4 /
  levels at level 0, 2243 / 396 → 1983 / 384 at level 2.
- **The normaliser only builds the paths some opcode needs.** The
  leading-zero counter exists only if `F_ADD`/`F_SU*`/`I2F`/`F_ROT` (or, at
  level 0, `F_MLT`/`F_DIV`) is instantiated, and the direct path only if
  `F_MLT`/`F_DIV` at level ≥ 1 is. Before, an `F_MLT`-only processor at level
  2 carried an unused counter the synthesiser did not prune: 1827 → 1658 LUT4,
  50 → 38 levels (Yosys). Every float block of the ALU is now behind a
  `generate` keyed on the opcodes of the program, as the integer ones always
  were.

### Fixed
- **Constants below the smallest normal float were mis-encoded by the
  assembler** (`asmcomp` `f2mf`, and the `cmmcomp` copy used for its
  diagnostics). At 23-bit mantissas the denormal shift was skipped, so a
  constant between 2.9e-39 and 1.2e-32 kept its full mantissa at the clamped
  exponent (`1e-33` was encoded as `1.6e-32`, `1e-34` as `1.28e-32`); at any
  width a constant far below the range needed a shift of 32+ bits, undefined in
  C (`1e-32` at 16/10/5 came out as `7.6e-5` instead of 0). Normal constants
  are encoded exactly as before.

### Changed
- **At `#FROUND >= 1` constants below the smallest normal float are encoded as
  zero** (flush-to-zero, the constant counterpart of the ALU flushing any
  result that small at those levels), and `cmmcomp` warns when a nonzero
  literal is flushed. The bound depends on the format: 2^(NBMANT-1) ·
  2^(-2^(NBEXPO-1)), i.e. 1.2e-32 at 32/23/8 but **0.0078 at 16/10/5** —
  where `0.001` becomes 0; use `#FROUND 0` or a wider `#NBEXPO` there.
  `appcomp` now records `#FROUND` in `app_log.txt` so `asmcomp` knows the
  level before it encodes the first constant.

### Added
- **`#FROUND` directive — float rounding level (0/1/2) of the ALU**, a
  `FROUND` parameter of `processor.v`/`core.v`/`ula.v`. Level `0` (default)
  is the legacy datapath, bit-identical to previous releases. Level `1`
  keeps the mantissa bit that `F_ADD`/`F_MLT`/`F_DIV` used to drop before
  normalization (so `(1+2^-22) - 1` is no longer 0 and `x * 1.0 == x`),
  saturates the exponent on overflow instead of wrapping, flushes to zero
  on underflow, and keeps zero canonical (`-0.0 == 0.0`). Level `2` adds
  round to nearest, ties to even, with guard/round/sticky bits carried from
  the denormalizer and the operators to a single rounding stage after the
  normalizer. Each level only adds logic to the float operators the program
  instantiates (Yosys LUT4, 32/23/8, int + float without `F_DIV`: level 1
  ≈ +1.5 %, level 2 ≈ +5 %). `cmmcomp`, `appcomp` and `asmcomp` accept
  `#FROUND` (0..2, validated); **`cppcomp` always emits `#FROUND 2`**, so
  the C++ goldens moved to the rounded datapath. Fixtures
  `cmm_fround0/1/2` run one program at each level (lost bit, tie cases,
  1000-step accumulation drift, overflow/underflow/negative zero, int/float
  conversions out of range).
- **`I2F` converts the whole word and `F2I` saturates at `#FROUND >= 1`.**
  `I2F` (also behind `fin()`/`F_INN`) used to take only the low `NBMANT`
  bits of the `int`, so `5000000` became `-3388608.0` and `2^24` became
  `0.0`; it now pre-aligns the full magnitude (a leading-zero count on the
  `NBEXPO+1` bits above the mantissa) and lets `ula_norm` truncate (level 1)
  or round (level 2) it. `F2I` returns `INT_MAX`/`INT_MIN` instead of
  wrapping for |x| >= 2^(NUBITS-1). Level 0 keeps both legacy behaviours.
- `docs/precision-and-width-review.md` — review of the float datapath and of
  every 32-bit assumption in the toolchain: what limits precision today (a
  mantissa bit lost before normalisation, truncation, exponent wrap, the
  host-`float` constant encoder) and what is needed to run any
  `NBMANT`/`NBEXPO`. `TODO.md` is now the short list that points into it.

### Removed
- `docs/aurora-verilator-migration.md` — the v4.3 migration guide for Aurora.
  Aurora adopted both changes (`+define+YANC_TRACE`, no `$finish` strip) and
  the `go_proc_vl.bat`/`go_proj_vl.bat` scripts it referenced were folded into
  `Scripts/single_proc.bat --sim verilator`. Still in git history.

## [v5.3] – 2026-07-24

### Changed
- **Synchronous global reset across the whole HDL library** — FPGA-recommended
  reset style: the async resets (`pc`, stack pointers, `racc`) became
  synchronous, and every state register that had no reset at all is now linked
  to the global `rst` — `ula_in1_ctrl` (`popr`/`stkr`), `ula_in2_ctrl`
  (`req_inr`/`ior`), `io_ctrl` (`en_out`/`addr_out`, killing any spurious boot
  `out_en` pulse) and `instr_dec`'s `ula_op` (its `rst` port was unused).
  `myFIFO`'s delayed write strobe `wr` now clears on `sclr`. Deliberately left
  without reset: memory arrays and their read-data registers (a reset there
  blocks block-RAM inference) and the sim-only stack stats (`fl_full`/`fl_max`).
  Testbenches already hold `rst` across a clock edge, so nothing changes on the
  simulation side — all sim goldens are bit-identical.

### Added
- **C++ object-model features (`cppcomp`)** — out-of-class destructor
  definition (`Class::~Class() { body }`), destructors running on early
  `return`/`break`/`continue` (RAII), static data member access through an
  instance or pointer (`obj.s`, `p->s`), array-of-function-pointers as a
  struct field, zero-fill aggregate `= {}` default member initializers,
  using-declaration (`using N::name;`), and lambda IIFE as a global
  initializer (`[](){...}()`). Realistic fixtures test58–test61 (CRC-16/CCITT
  telemetry link, debounced-input function-local statics, POD struct by-value
  semantics, lambda-IIFE global init).
- **Directed reset regression pass (`ResetCheck`)** — a fixture that emits a
  deterministic output burst, parks in `while(1)`, gets a single-cycle `rst`
  pulse mid-run from a dedicated testbench, and must repeat the burst
  bit-identically (data memory is NOT reset, so the program re-assigns
  everything it outputs). `regress.sh` grew pass 3b with a golden-independent
  anchor: the first half of `output_reset.txt` must equal the second half, so
  a blind `--update` cannot bless a broken reset.

### Fixed
- **`cur_base` clobbered by array-dimension size expressions (`cppcomp`)** —
  a `base_type` reduction inside an array-size expression retyped the
  declarator being built (the v5.2 `cur_base` bug class, now closed:
  `array_suffix` saves/restores `cur_base`; locked by test64).
- **`std::array<float>` zero-fill used int-0 bits (`cppcomp`)** — aggregate
  zero-fill of a float array now emits float `0.0` in the YANC format instead
  of an all-zeros integer word.
- **`f2mf` rounding carry at power-of-two boundaries (`asmcomp`)** — the
  float-to-mini-float conversion renormalizes the mantissa when the rounding
  carry crosses a power of two, instead of shipping a doubled mantissa.
- **`config.h` defaulted to a non-YANC target (`cppcomp`)** — the bundled
  header now defaults to the fixed 32-bit/IEEE-sizes/4K YANC target.
- **UTF-8 comment dashes mangled by an ASCII rewrite (`cppcomp`)** — restored
  after the lambda-IIFE commit accidentally transcoded them.

## [v5.2] – 2026-06-08

### Added
- **C++ simulation/GTKWave pipeline parity (`cppcomp`)** — a cppcomp-generated
  design now tracks its variables and source lines in the waveform exactly like a
  C±-generated one. cppcomp emits the two GTKWave-support files cmmcomp always
  produced: `pc_<proc>_mem.txt` (one 20-bit two's-complement source line per
  instruction, written in lockstep with the instruction count; program scaffolding
  tagged −1/−3) and `trad_cmm.txt` (the numbered preprocessed source those line
  numbers index into). `cmm_log.txt` is now restricted to genuine scalar statics —
  `int → 1`, `float → 2`, and anything else (pointer/struct/…) is no longer
  published, so asmcomp never builds a wrong integer mirror at a fixed address; a
  function parameter is logged under its asm label so overloaded-function params
  still reconstruct to the `<func>_<var>` operand asmcomp matches. All of this is
  simulation-only (behind `` `ifdef YANC_SIM_VIS `` in the generated core), so it
  costs nothing in synthesis.
- **`single_proc_cpp.bat`** — end-to-end C++ runner mirroring `single_proc.bat`
  (cpppp → cppcomp → appcomp → asmcomp → iverilog/verilator → GTKWave), with
  `--sim iverilog|verilator` and `--no-view`, plus a minimal `proc_cpp` demo.
- **`out()` warns on a float value (`cppcomp`)** — the hardware OUT port is an
  integer word; `out(port, x)` with a float `x` ships its raw bit pattern (a
  wrong number). cppcomp now emits a compile *warning* pointing at the cast
  (`out(port, (int)x)`) but still compiles and does **not** convert — the
  convention every existing test already follows is the explicit cast. (C±
  warns and truncates via `F2I` and offers `fout`; cppcomp warns without
  converting.)

### Changed
- **Runner scripts moved under `Scripts/`** — the single/multi-proc runners
  (`single_proc.bat`/`.sh`, `multi_proc.bat`/`.sh`, `single_proc_cpp.bat`) now
  live in `Scripts/` next to `env`/`setup` instead of the repo root, and a new
  `Scripts/single_proc_cpp.sh` gives the C++ pipeline a Linux runner to match the
  `.bat`. Aurora's deploy step (`Scripts/aurora.bat`) was trimmed to copy only the
  binaries Aurora actually consumes (no `gen_gtkw`); the release archives are
  unchanged.

### Fixed
- **Runner scripts trusted phantom tool paths (`Scripts/env.bat`)** — an
  IDE/launcher (the VS Code integrated terminal under Aurora) can pre-set
  `IVERILOG`/`VVP`/`GTKWAVE`/`MINGW_BIN` to bundled tools that are not actually
  installed. env.bat only filled a tool *if not defined*, so it trusted those
  non-existent paths and never reached its PATH fallback — every runner died at
  `iverilog` with *"The system cannot find the path specified"* and never built the
  `.vvp`. env.bat now drops any inherited tool path that does not exist (so the
  PATH lookup finds the real MSYS2 tool while a present nipscernlab GTKWave is
  kept), and derives `MINGW_BIN` from a resolved tool when setup's cache is absent.
- **`--version` printed `5.0`** — `yanc_version.h` was left at `5.0` after the v5.1
  release; bumped to match the tag and CHANGELOG.
- **Dead / double-evaluated left operand on `-` `/` `%` `<<` `>>` (`cppcomp`)** —
  the memory-operand fast path evaluated the left operand and only then found
  these non-commutative ops aren't handled there, falling through to the stack
  path which re-evaluated it: a dead `LOD` for a plain variable, and a genuine
  double-evaluation for a side-effecting left operand (`f() - b` called `f()`
  twice). Now gated on the operator, so the left operand is emitted exactly once.
- **Constant-index array access uses the `_V` (base+offset) opcodes (`cppcomp`)** —
  `arr[k]` with a compile-time integer `k` now bakes the offset into the
  instruction instead of computing the index and going through the indirect
  `STI` / `LDI` path: a store `SET_V arr k` (4 → 2 instructions), a load
  `LOD_V arr k` (2 → 1), and when `arr[k]` is the right operand of `+`/`*` the
  fused `ADD_V` / `MLT_V` (and float `F_ADD_V` / `F_MLT_V`) collapses load+op too
  (`g[0] + g[1]` → `LOD_V g 0; ADD_V g 1`, 6 → 2). Non-constant indices keep the
  indirect path. Same opcodes cmmcomp's constant-index path emits. Regress: 51/51 cpp.
- **Unary ops / int↔float casts of a plain variable use the `_M` form (`cppcomp`)** —
  `-x`, `~x`, `!x` and `(int)x` / `(float)x` on a simple scalar memory variable
  now read the operand straight from memory (`NEG_M` / `F_NEG_M` / `INV_M` /
  `LIN_M` / `F2I_M` / `I2F_M`) instead of `LOD x; <op>`, one instruction less
  each. Operands that aren't a plain memory variable (members, references, frame
  locals, expressions) keep the `LOD; <op>` path. Regress: 51/51 cpp.
- **Post-emit peephole (`cppcomp`)** — cppcomp now buffers emitted instructions
  and runs a peephole before writing them (the instruction-selection layer it
  lacked; cmmcomp tracks the accumulator inline). Two passes, no fusion across a
  label or into verbatim inline asm, and the `pc_<proc>_mem.txt` line table is
  regenerated from the fused stream so `num_ins` stays exact:
    1. `LOD <var>; <unary>` → `<unary>_M <var>` — a unary acc op (NEG/ABS/PST/
       NRM/I2F/F2I/INV/LIN and `F_` variants) after loading a *named* variable
       reads it straight from memory. Literal loads (`LOD 5`) are left alone.
    2. bare `PSH` + a load-class op → the op's `P_` / `PF_` variant that pushes
       the accumulator as part of the same instruction (`PSH; LOD x` → `P_LOD x`,
       `PSH; NEG_M x` → `P_NEG_M x`, ...); and a plain `SET x; POP` →
       `SET_P x` (store-then-pop fused), which collapses the argument-unpacking
       in a function prologue (`POP; SET c; POP; SET b; POP; SET a` →
       `POP; SET_P c; SET_P b; SET a`).
  Together they collapse e.g. `a - b` to `LOD a; P_NEG_M b; S_ADD` (3 instrs).
  Regress: 51/51 cpp.
- **Redundant store-then-reload elimination (`cppcomp`)** — a `LOD x` (or
  `LOD_V a k`) immediately after a `SET x` (or `SET_V a k`) of the *same* cell
  is dropped: `SET` leaves the value in the accumulator, so reloading it is a
  no-op. This was pervasive — `int x = e; <use x>` emitted `<e>; SET x; LOD x;
  <use>` — and removing it shed ~470 instructions across the test suite (test50
  alone ~60). New peephole pass 0. Regress: 51/51 cpp.
- **Dead `JMP` to the next line dropped (`cppcomp`)** — a bare `JMP L`
  immediately followed by `@L` (exact label, not a prefix) just falls through,
  so it is removed — e.g. the `JMP main` a helper-function-less program emits
  right before `@main`. Regress: 51/51 cpp.

## [v5.1] – 2026-06-07

### Added
- **Math-library built-ins (`cmmcomp`)** — `pow(x, y)` (an integer-constant
  exponent expands to exact square-and-multiply, an integer variable to a runtime
  multiply loop, otherwise `exp(y·ln x)`); `tan(x)` (a dedicated degree-11 minimax
  fit, range-reduced mod π with a cotangent fold for the pole — *not* `sin/cos`);
  `floor` / `ceil` / `round` (inline via `F2I`, returning an integral float, ties
  away from zero like C `round()`); and the hyperbolics `cosh` / `sinh` / `tanh`
  (composed from the real exponential, `tanh` via the one-`exp` `(e^2x−1)/(e^2x+1)`
  identity). Each is validated by value with a new fixture. No new hardware.
- **Complex arguments across the transcendentals (`cmmcomp`)** — `exp`, `sin`,
  `cos`, `tan`, `log` and `atan` now accept a `comp` argument and return a `comp`
  (joining `sqrt`), composed from real ops via the canonical library identities:
  `exp(a+bi)=eᵃ(cos b+i·sin b)`, `sin(a+bi)=sin a·cosh b+i·cos a·sinh b`,
  `log(a+bi)=½·ln|z|²+i·arg z`, `tan` via the real-denominator closed form
  `(sin 2a+i·sinh 2b)/(cos 2a+cosh 2b)`, and `atan` via
  `½·atan2(2a,1−a²−b²)+i·¼·ln(…)`. The `cosh`/`sinh` of the imaginary part come
  from a single `exp` + reciprocal. Also adds **`conj(z)`** (complex conjugate
  `a−bi`; a real `x` becomes `x+0i`; always returns a `comp`). New `cmm_cexp` /
  `cmm_csincos` / `cmm_clog` / `cmm_catan` / `cmm_ctan` / `cmm_conj` fixtures
  validate every form (const / memory / accumulator) by value.

### Fixed
- **`fase()` / `atan2` returned the wrong quadrant (`cmmcomp`)** — all five `F_LES`
  sign tests in `exec_fase` had been written assuming `F_LES X` is true when
  `acc < X`, but the hardware semantics are `acc > X`, so the routine silently
  computed `fase(−z)` (every quadrant off by ±π). It went unnoticed for a long
  time because the `cmm_comp_fase` golden had been blessed with the wrong output
  (the fixture's own comment had the right values). Fixed by swapping the operand
  on each test; `cmm_comp_fase` now matches the true `atan2`
  (`fase(1+2i)=1107`, `fase(2+0i)=0`, …), and complex `log` / `atan`, which build
  on it, are correct.

### Changed
- **`comp` argument → `comp` result is now explicit in the type pass (`cmmcomp`)** —
  the typecheck annotates `sqrt`/`exp`/`sin`/`cos`/`tan`/`log`/`atan` as `comp`
  when the argument is `comp` (it always said `float` before). Cosmetic — codegen
  already used the function's runtime return type, so no emitted code changed — but
  it makes the rule "real argument → `float`, `comp` argument → `comp`" explicit.
- **Codegen now uses the fused load+op instructions (`cmmcomp`)** — after a full
  audit of the assembler ISA, wasteful `(P_)LOD`/`PSH` + op pairs were collapsed
  into the single-cycle fused forms (`F_NEG_M`, `F_ABS_M`, `PF_NEG_M`, `P_LOD`,
  `I2F_M`, …), shaving an instruction off `cosh`/`sinh`, the `pow` loop, and the
  complex `sqrt`/`log` branches. Values are unchanged (the affected goldens only
  grow — shorter code loops more times in the fixed sim window).

### Changed
- **`float_atan` now uses a minimax polynomial instead of a LUT** — the arctangent
  macro drops the 49-entry table for a degree-11 odd minimax polynomial
  (least-squares fit on `[0,1]`, max abs error ~4.9e-6, ~6 digits vs the table's
  ~4e-5). `|x|` is still folded into `[0,1]` with the `1/x` identity
  (`atan(|x|)=π/2−atan(1/|x|)`), but now a single shared polynomial serves both
  branches instead of duplicating the lookup — so the macro is actually *smaller*
  in code (34 vs 39 instructions) while freeing ~98 words of data memory
  (`Arctan_LUT.txt` removed). A few more instructions execute per call (a Horner
  poly vs a table lookup), negligible on the single-cycle ULA. No new hardware.
  `cmm_trig` gains `atan(1)=π/4`, `atan(2)` (the 1/x branch) and `atan(-1)` (sign);
  `cmm_comp_fase` (atan via `fase`) is unchanged in value. With this, **no
  transcendental macro uses a lookup table anymore**.
- **`float_sin` / `float_cos` now use a minimax polynomial instead of a LUT** — the
  sine macro drops the 152-entry lookup table for a degree-7 odd minimax
  polynomial (least-squares fit on `[0, π/2]`, max abs error ~1.6e-6, ~6 digits
  vs the table's ~3). Range reduction folds to `[-π/2, π/2]` (`k = round(x/π)`,
  `r = x - k·π`) and the `(-1)^k` sign is applied from the parity of `k`. Costs
  just +3 instructions over the LUT version (31 vs 28) while freeing ~152 words
  of data memory (`Sin_LUT.txt` removed), and uses only existing instructions (no
  new HW). `cos` reuses it. `cmm_trig` golden updated (`cos(0)` now 999 — the
  polynomial's uniform 0.9999975 vs the table's lucky-exact node).
- **`float_sin` / `float_cos` range reduction is now O(1)** — the sine macro no
  longer subtracts `2π` in a loop to bring the argument into range; it computes
  `k = round(x/2π)` and does `x -= k·2π` in one shot (round-to-nearest via a
  sign-half bias, since `F2I` truncates), landing in `[-π, π]` before the usual
  table lookup. `cos` reuses it (`sin(x+π/2)`), so both are now fixed-cost
  regardless of argument magnitude — `sin(100 rad)` no longer loops ~16 times.
  Same table, same ~3–4 digit precision; uses only existing instructions (no new
  HW). First by-value sin/cos coverage added as the `cmm_trig` fixture
  (`sin(0)=0`, `sin(π/2)≈1`, `cos(0)=1`, `sin(100)≈-0.506`, `cos(π)≈-1`).
- **`float_exp` / `float_log` range reduction is now O(1) via `F_SCL`/`XPO`** — the
  exp and log macros no longer reduce the argument with a value loop (subtract
  `ln2` / halve repeatedly). `float_exp` computes `n = round(x/ln2)` directly and
  scales `exp(r)` by `2^n` with one `F_SCL`; `float_log` gets the exponent with
  `XPO` and the mantissa `m = x·2^-e` with one `F_SCL`. Both are now fixed-cost
  (~25–33 instructions) regardless of argument magnitude, and the reduction is
  exact (no bit loss from repeated halving), so the inverse round-trips land
  cleaner — `exp(log(8))` now hits 8.000 exactly (was 7.999). `float_sqrt` keeps
  its `F_ROT` estimate. Same values, regenerated `cmm_exp`/`cmm_log` goldens.

### Added
- **ISA: `F_SCL`/`SF_SCL` and `XPO`/`XPO_M` — float exponent surgery (HW + assemblers)** —
  four new ULA operations. `F_SCL X` / `SF_SCL` scale a float by a power of two
  (`acc·2^k`, k a signed int from memory or the stack) by adding k to the
  exponent field (saturating); `XPO` / `XPO_M` extract the base-2 exponent of a
  float as an int (`floor(log2|x|)`, from the accumulator or memory). They are
  the `ldexp`/`frexp` primitives for O(1), exact range reduction in the
  transcendental macros (the value-loop reductions in `float_exp`/`float_log` and
  the `F_ROT` estimate in `float_sqrt` are being migrated to them). Implemented as
  `ula_scl`/`ula_xpo` in `HDL/ula.v` (mux/params/decode in `instr_dec.v`,
  `core.v`, `processor.v`; `F_ROT` kept for now), and taught to the assemblers
  (`ASMComp.l`, `app.l`, with resource-report messages in `opcodes.c`). Verified
  by value through the sim: `F_SCL(1,3)=8`, `F_SCL(8,-3)=1`, `XPO(8)=3`,
  `XPO(16)=4`, `SF_SCL(1,3)=8`, `XPO_M(32)=5`. The exponent field is signed two's
  complement (not IEEE-biased), which makes both ops a thin exponent-field add /
  read.
- **Real `exp(x)` and `log(x)` (cmmcomp)** — two new built-ins: `exp` is e^x and
  `log` is the natural logarithm (ln). Both are backed by new assembly macros
  (`Includes/float_exp.asm`, `Includes/float_log.asm`) auto-included on demand
  through the same `find_opc`/`mac_add` hook as `float_sqrt`/`float_sin`. The
  macros use **no new hardware and no lookup table**: range reduction is done by
  value with a loop (like `float_sin`), independent of the non-IEEE float layout,
  and the reduced interval is evaluated with a Horner polynomial — `exp(r)` a
  degree-8 Taylor series on `[0, ln2)`, `log(m)` the atanh series
  `2u(1+u²/3+u⁴/5+…)` with `u=(m-1)/(m+1)` on `[1, 2)`. `log` guards `x<=0`
  (returns 0). Complex arguments are rejected for now (a later step). Wired
  through the lexer (`exp`/`log` keywords), grammar (`std_exp`/`std_log`),
  AST (`OP_STD_EXP`/`OP_STD_LOG`) and `exec_exp`/`exec_log` in stdlib. Validated
  by value with the new `cmm_exp`/`cmm_log` fixtures (scaled ×1000 since `fout`
  truncates to int): `exp(0)=1`, `exp(1)=e`, `exp(2)=e²`, `exp(-1)=1/e`,
  `log(2)=ln2`, `log(10)=ln10`, and the inverse round-trips `log(exp(3))` and
  `exp(log(8))`.
- **Complex `sqrt(z)` (cmmcomp)** — `sqrt` now accepts a `comp` argument and
  returns the principal complex square root, instead of rejecting it as an error.
  The `comp` branch of `exec_sqrt` is composed entirely from existing real ops
  (no new assembly instructions): with `z = a + b i` and `r = |z| = sqrt(a²+b²)`,
  it emits `sqrt(z) = sqrt((r+a)/2) + sign(b)·sqrt((r-a)/2) i` (`F_SU1` for the
  subtraction, `F_MLT 0.5` for the halving, `F_SGN` for the sign transfer). All
  three argument forms are handled — `comp` constant, `comp` in memory, and a
  `comp` already in the accumulator (spilled to temps with `SET_P`/`SET`, like
  `exec_fase`). The accumulator peephole drops the redundant `LOD csqrt_r` /
  `LOD csqrt_a` after each `SET`, so the live half stays in the acc. Locked down
  by the new `cmm_comp_sqrt` fixture (by-value): `sqrt(3+4i)=2+1i`,
  `sqrt(-3+4i)=1+2i`, `sqrt(z+w)=sqrt(3+4i)=2+1i` (acc path), and the real-axis
  edges `sqrt(4)=2`, `sqrt(-4)=2i` (principal root; `F_SGN(0)` yields `+`).
- **Accumulator-aware redundant-load elimination (cmmcomp codegen)** — the
  streaming peephole in `add_instr` now tracks which operand is in the
  accumulator (`acc_name`, set by a plain `LOD`/`P_LOD`/`SET`, cleared by every
  other op) and drops a `LOD x` whenever the accumulator already holds `x`. This
  generalises the old "`LOD x` right after `SET x`" drop to any basic-block
  position; `acc_name` is reset at every control-flow boundary
  (`emit_peephole_reset`), so a drop never crosses a label. Value-preserving and
  byte-identical on the suite (the existing codegen had no extra redundant loads
  to drop). To turn the new capability into a real win, every `c²+d²` the codegen
  builds — the denominator of all complex divisions (`oper_divi`: comp÷comp,
  scalar÷comp and the acc variants) and the `|comp|²` in `mod2`/`abs`
  (`exec_mod2`) — now goes through one `emit_sq_sum(er, ei)` helper that squares
  the half the accumulator already holds first. `c²+d²` is commutative, so when
  `d` is live (e.g. `r = x / y` or `mod2(x)` right after `x = …`) squaring `d²`
  first lets the peephole drop its `LOD`. So `r = x / y` right after `y = …` is
  21 instructions instead of 22. Value-preserving and byte-identical on the suite
  (no existing test has the divisor/operand live in the acc at the operation).
  Locked down by the new `cmm_comp_div` fixture, which also adds the first
  by-value coverage of `comp_var / comp_var` ((4+2i)/(1+1i)=3-1i,
  (8+6i)/(1-1i)=1+7i, (6+8i)/(2+0i)=3+4i).

### Fixed
- **Walker-time diagnostics report the right line and variable name (cmmcomp)** —
  every warning / info message emitted during the deferred-AST walk (comp/float
  conversions, complex array index, comp-in-condition, comp/float `RECV`, the
  function-call parameter conversions, …) printed the **EOF line** and the
  **mangled `<fn>_<var>`** name (e.g. `line 21: variable 'main_c' …` instead of
  `line 15: variable 'c' …`). Both come from globals that are stale during the
  walk: `line_num` (the lexer line) has run to EOF, and `fname` (the current
  function, used by `rem_fname` to strip the prefix) was reset to `""` at each
  function's end. Fix: the walker wrappers (`stmt_emit` / `ast_emit_expr`) now
  keep `line_num` in sync with the node's source line (mirroring `emit_line`,
  with save/restore so parse-time diagnostics are untouched), and the
  `STMT_FUNC` walker sets a new display-only global `emit_fname` to the
  function's name for `rem_fname`. `emit_fname` is display-only — `exec_id` still
  keys off the global `fname`, so temp-variable names and the **assembly are
  byte-identical**. Messages stay correctly bilingual (`-pt` / `-en`). Verified
  byte-identical on the suite and by a probe in both languages (warnings now read
  the real line and the bare local name).
- **Arithmetic between two `comp` literals is no longer dropped (cmmcomp)** —
  `(1+2i)*(3+4i)` returned `3+4i` (and `(4+2i)/(1+1i)` returned `4+2i`, and
  `(0+2i)+(3+4i)` returned `3+4i`) instead of computing the operation. The
  algebraic-identity folder (`x*1→x`, `x/1→x`, `x+0→x`) used `is_const_value`,
  which only compared the literal's *real-part* cell against the scalar 0/1 — so
  a comp literal like `1+2i` (real part 1) was mistaken for the constant 1 and
  the operation was folded away. `is_const_value` now rejects non-scalar
  literals (`n->type > 2`), so only a genuine int/float literal can be the
  algebraic 0/1; comp literals go through the normal `oper_mult`/`oper_divi`/
  `oper_soma` path. Scalar folding is unchanged (every existing test stays
  byte-identical). Locked down by the new `cmm_comp_arith` fixture (`×`/`÷`/`+`/
  `−` between comp literals, the `0+x`/`1*x` decoy cases, and a comp-variable
  regression — output values checked against hand computation).
- **`fase()` (complex phase) is now a real `atan2` (cmmcomp)** — it previously
  computed just `atan(imag/real)`, a 2-quadrant angle that was wrong for
  `real < 0` (off by ±π) and **divided by zero when `real == 0`** (the sim
  produced no output). `exec_fase` now emits proper `atan2(imag, real)`: it
  branches on the signs of real and imag, computes `atan(imag/real)` only when
  `real ≠ 0` (with the ±π quadrant correction for `real < 0`), and returns ±π/2
  on the imaginary axis and 0 at the origin — correct in all four quadrants and
  on the axes, no division by zero. Uses only existing opcodes (`F_LES`/`F_DIV`/
  `F_ADD`/`CAL float_atan`/`LOD`/`JIZ`/`JMP`), no hardware change. `fase` was
  unused by any test/example, so no golden changed. Locked down by the new
  `cmm_comp_fase` sim fixture (all four quadrants, all four axes and the origin,
  checked as `atan2×1000` against hand computation — e.g. `-2+0i → π`,
  `0±2i → ±π/2`).

### Added
- **`do { } while ()` loop (cmmcomp)** — C± gains the third C loop. The body runs
  first and the condition is tested at the bottom, so it always executes at least
  once. STMT_DO shares the `while` label namespace (`push_while`), so `break`
  (-> `Lwh<n>end`) and `continue` (-> `Lwh<n>cont`, the bottom test) ride the
  same break/continue stacks as while/for — a do-while continue re-evaluates the
  condition. Uses only `JIZ`/`JMP`, no hardware change. Locked down by the
  `cmm_dowhile` sim fixture (body-runs-once, normal loop, break, continue, and a
  nested do-while). `goto` is now the only C control-flow keyword still missing
  from C±.
- **`continue` statement (cmmcomp)** — C± gains `continue`, binding to the
  innermost enclosing loop (a `switch` does not catch it, matching C). It rides
  the same machinery as `break`: a **continue-target stack** in `saltos.c` that
  only loops push to. A `while`-continue jumps to the loop top (re-test the
  condition); a `for`-continue jumps to a step label (`Lwh<n>cont`) placed just
  before the step, so the step still runs and the index advances — a top jump
  would hang the loop. The `for` step now lives on the while node's `else_body`
  and is emitted after the body (instead of glued into it), and the
  `Lwh<n>cont` label is emitted only when a `continue` actually targets that
  loop, so **every existing test stays byte-identical**. Uses only `JMP` — no
  hardware change. Locked down by the `cmm_continue` sim fixture (continue in
  while, in for with the step-runs check, nested inner-only, and inside a switch
  inside a for). `goto` and `do/while` remain unimplemented in C±.
- **Real C `switch` fall-through (cmmcomp)** — a `case` without `break` now falls
  through into the next case body, and empty cases stack onto the following one
  (`case 1: case 2: case 3: foo();` runs `foo` for all three) — standard C
  semantics. The old codegen re-tested `switch_exp` at every case label, so a
  no-break case was *skipped* instead of falling through, and an empty case was
  a syntax error. The `STMT_SWITCH` walker now emits a **dispatch block** (one
  compare per case, jump to that case's body) followed by the **case bodies laid
  out contiguously**, so control falls from one body into the next with no
  re-test. The grammar's switch body became a flat `sw_body` (labels and
  statements interleaved, the C model), which also allows the empty-case stack.
  Uses only existing opcodes (`LOD`/`EQU`/`JIZ`/`JMP`) — no hardware change. Cost
  is about one extra `JMP` per switch; the assembly is no longer byte-identical,
  so the switch goldens and `size_baseline` were regenerated. Locked down by the
  `cmm_switch_fall` sim fixture (cascade, empty-case stacking and partial
  fall-through, output values checked against hand computation). Note: a jump
  table (O(1) dispatch for large dense switches) would need an indirect jump,
  which the ISA does not have — but that is an optimisation, not correctness.
- **Nested `switch` and correct `break` binding (cmmcomp)** — a `switch` can now
  appear inside a `case`, and `break` binds to the innermost enclosing loop *or*
  switch (the standard C rule). Previously the grammar had a separate
  "statements allowed in a case" tier (`stmt_case`) that omitted `switch`, so a
  nested switch was a syntax error; worse, a `break` inside an `if` inside a
  `case` was emitted as a loop break (`JMP Lwh<n>end`) and jumped out of the
  enclosing `while` instead of the switch. The fix replaces the grammar tier
  with a semantic **break-target stack** in `saltos.c`: every loop and switch
  pushes one entry, and `exec_break` resolves to the top, so a case body is now
  an ordinary statement list (blocks and nested switches included). Per-switch
  case numbering moved onto the switch's `stmt_node` so nested switches no longer
  share a global counter. **Byte-identical assembly** for every existing test
  (flat switches, loops and breaks emit exactly as before — verified by a
  zero-diff `--update` and the `size_baseline` ratchet); the new behaviour is
  locked down by the `cmm_switch_nest` sim fixture (nested switch + a
  break-in-if-in-case, output values checked against hand computation).
  (Fall-through was a limitation at this point; it is fixed by the
  fall-through entry above.)
- **Object-like `#define` in `cmmcomp`** — the C± lexer now handles
  `#define NAME body` directly, with no separate preprocessor stage: a later use
  of `NAME` is replaced by re-lexing its body (flex `yy_scan_string` + a buffer
  stack), so `#define LIMIT 256` lets you write `LIMIT` anywhere the literal
  would go. Nested defines expand; a self-referential define is expanded once
  and then left alone (a per-macro active flag prevents an infinite loop).
  Function-like macros, `#ifdef` and `#include` are out of scope. Locked down by
  a positive asm-golden fixture (`cmm_define`) and a recursion-guard negative
  test.
- **CMM negative phase in `regress.sh`** — malformed programs (syntax error,
  missing `main()`, undeclared variable, garbage, recursive `#define`) are now
  asserted to be rejected with a clean non-zero exit and the right diagnostic,
  never a crash or silent accept. Fixtures live in `Compilers/CMMComp/NegTests/`.
- **Sethi-Ullman operand ordering (cmmcomp codegen)** — when a commutative
  integer `+` / `*` has two complex operands, the AST walker now evaluates the
  heavier subtree first, so its result spends less time pushed on the shallow
  NDSTAC hardware stack (fewer pushes / spills). The reorder is value-identical
  (`S_ADD` / `S_MLT` are symmetric) and the AST is what makes it possible — the
  old parse-order emit was locked left-to-right. Validated functionally by the
  `cmm_reorder` sim fixture (output values checked against hand computation, not
  golden.asm). Scoped to int `+`/`*` for now; float is excluded (reassociation
  would change rounding).
- **Direct access for constant array indices (cmmcomp codegen)** — `arr[k]` with
  a compile-time-constant index into an int array now uses the `SET_V` (store)
  and `LOD_V` / `P_LOD_V` (read) pseudo-instructions: the assembler bakes the
  offset into a plain `SET`/`LOD` at `arr_base + k`, so there is no index to
  materialise, no stack push, and no indirect `STI`/`LDI` -- one fewer
  instruction per access, no stack slot, no indirect addressing. The deferred
  AST makes it clean: the index subtree is inspected before the access is
  emitted. Scoped to 1D-forward int arrays (int RHS for stores); 2D / reversed /
  float / comp fall back to the indirect path. Validated by the `cmm_arridx` sim
  fixture, which writes distinct values to distinct constant indices via `SET_V`
  and reads them back via `LOD_V` -- the readback values are checked against
  hand computation, so a wrong offset on either side would be caught.
- **Integer constant folding (cmmcomp codegen)** — a fully-constant int
  subexpression (`+ - * & | ^` of int literals) is evaluated at compile time and
  emitted as a single literal, folding deeply through nesting (`(2+3)*4` -> 20).
  Only when every intermediate stays within the signed NUBITS range, so the
  compile-time arithmetic matches the hardware. A non-negative result becomes a
  literal; a negative one has no literal form, so it is emitted as a `NEG_M` of
  its magnitude (`3-5` -> `NEG_M 2`) -- negatives compose too (`(3-5)+10` folds
  to `8`). An out-of-range/overflowing result falls back to the runtime op
  (which does the NUBITS wraparound), e.g. `30000+30000` is left to the runtime.
  Float is never folded (the YANC float is not IEEE-bit-compatible); bitwise
  ops fold only on non-negative operands. Composes with the constant-index array
  opt (`arr[2+3]` folds to `arr[5]`, then uses `LOD_V` / `SET_V`). Validated by
  the `cmm_cfold` sim fixture (folded output values checked against hand
  computation: 5 20 63 10 35 -7 8).

### Fixed
- **Memory-safety pass over the older compilers** — every `fopen` in
  cmmcomp/asmcomp/appcomp is now NULL-checked (a missing input file is a clean
  error instead of a segfault), and the path-building `sprintf`s became
  bounds-checked `snprintf`.

## [v5.0] – 2026-06-03

YANC v5.0 is the **first cross-platform release** — a big milestone. The whole
toolchain now builds and runs on both **Linux** (native `gcc`) and **Windows**
(MSYS2 / MinGW-w64) from one shared top-level `Makefile`, verified on every push
by CI on Ubuntu *and* Windows, with prebuilt binaries published for both
platforms. It also lands the `gen_gtkw` wave-view generator (replacing the
runtime Tcl formatters), named HDL generate blocks, and a batch of
cross-platform build hardening.

### Added
- **Cross-platform build from a single `Makefile`** — one top-level `Makefile`
  is now the single source of truth for every binary (cmmcomp, appcomp, asmcomp,
  cpppp, cppcomp, comp2gtkw, gen_gtkw). It emits `.exe` under Windows
  (`$(OS)=Windows_NT`) and bare binaries on Linux, and CI builds + smoke-tests
  the full pipeline on **both** Ubuntu and Windows/MSYS2 with `-Werror`.
- **Per-OS setup + runners** — `Scripts/setup.sh` (Linux) and `Scripts/setup.bat`
  (Windows) resolve the build / simulation / GTKWave tools and cache them via
  `env.sh` / `env.bat`; the example runners ship as both `single_proc.{sh,bat}`
  and `multi_proc.{sh,bat}` and pick the simulator with `--sim`. No hardcoded
  tool paths anywhere.
- **Prebuilt Linux binaries** — the release workflow now also builds on Ubuntu
  and publishes `yanc-bin-linux-vX.tar.gz` alongside the Windows
  `yanc-bin-vX.zip`.
- **`.gitattributes`** pins the shell scripts to LF (and the Windows scripts to
  CRLF), so a fresh checkout runs under MSYS2 / git-bash regardless of the
  cloning machine's `core.autocrlf` setting.
- **`Scripts/gen_gtkw.c`** — a C tool that parses a
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
- **Runners reorganised** — `go_proc` / `go_proj` are renamed to `single_proc` /
  `multi_proc`, the per-simulator variants are merged behind a
  `--sim {icarus,verilator}` flag, and each ships as both `.bat` and `.sh`. The
  runners now copy only the inputs (the `.cmm` / project inputs), not the whole
  project tree.
- **Build hardening** — `make` is invoked with bare `BISON=bison FLEX=flex` so an
  inherited backslash Windows path can't reach MSYS2's `/bin/sh`; `setup` probes
  for the broken MSYS2 gcc 16.1.0-5 libstdc++ (which kills Verilator) and warns;
  `regress.sh` resolves its tools through `env.sh` and links with `-lm` instead
  of hardcoded paths.
- **Named HDL generate blocks** — `core.v` and `instr_dec.v` name every
  `generate` block, so the simulation hierarchy shows meaningful names instead of
  `genblk2` / `genblkN`.
- **The runner wave flows now use gen_gtkw + the nipscern GTKWave v4.** Each
  builds `gen_gtkw.exe`, writes the `.gtkw` layout from the (header-only) VCD, and
  opens the waveform with `gtkwave --dark --zoom-fit --left-justify <vcd> -a
  <gtkw>` (no `--script`, no `fix.vcd` tab hack, no `--rcvar` — the nipscern fork
  hides the SST pane and reports that rcvar as not found). FST flows
  (the Icarus `single_proc`/`multi_proc` runs and the Verilator `multi_proc
  --sim verilator` run) add a quick `+HEADER_ONLY` pass — Icarus writes the
  header VCD directly; Verilator's tiny header FST is converted with
  `fst2vcd -f`. The `--zoom-fit` restores the whole-wave view the
  Tcl flow did via `Zoom_Best_Fit`.
- **`multi_proc.bat` sets `TMP`/`TEMP`** to its own Temp dir, so it no longer
  inherits a stale temp path from a previous bat run in the same cmd window
  (which broke gcc/iverilog with "cannot create temporary file").

### Fixed
- **`cpppp` realpath buffer overflow** — `path_canon`'s POSIX branch could
  overflow its buffer while canonicalising paths; fixed so the preprocessor is
  safe on Linux.
- **`racc` declared before use in the HDL** — `racc` is now declared ahead of the
  `uic_in2` generate block that references it, so iverilog v13 (which rejects
  forward references) builds the generated Verilog.

### Removed
- **The runtime GTKWave Tcl formatters and their props** — `gtk_proc_init.tcl`,
  `gtk_proj_init.tcl`, `gtk_almost_proj.tcl`, `pos_gtkw.tcl` and `fix.vcd` (the
  empty-tab crash workaround) are gone, replaced by `gen_gtkw.c`. Also removed the
  long-unused `proc2rtl.ys` Yosys script.
- **The old `go_*.bat` runners** (`go_proc` / `go_proj` / `go_proc_vl` /
  `go_proj_vl`) — replaced by `single_proc` / `multi_proc` (`.bat` + `.sh`,
  `--sim`-selected).
- **`HANDOFF.md`** — the merged branch's working notes.

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
  parse the terminal lines instead — see `docs/aurora-verilator-migration.md`
  (removed after Aurora adopted it; in git history).
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
`docs/aurora-verilator-migration.md` (removed after Aurora adopted it; in git
history).

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

[Unreleased]: https://github.com/nipscernlab/yanc/compare/v5.0...HEAD
[v5.0]: https://github.com/nipscernlab/yanc/releases/tag/v5.0
[v4.4.1]: https://github.com/nipscernlab/yanc/releases/tag/v4.4.1
[v4.4]: https://github.com/nipscernlab/yanc/releases/tag/v4.4
[v4.3]: https://github.com/nipscernlab/yanc/releases/tag/v4.3
[v4.2]: https://github.com/nipscernlab/yanc/releases/tag/v4.2
[v4.1]: https://github.com/nipscernlab/yanc/releases/tag/v4.1
[v4]: https://github.com/nipscernlab/yanc/releases/tag/v4
[v3]: https://github.com/nipscernlab/yanc/releases/tag/v3
[v2]: https://github.com/nipscernlab/yanc/releases/tag/v2
[v1]: https://github.com/nipscernlab/yanc/releases/tag/v1
