# Functions for neural networks

`relu`, `sigmoid`, `tanh`, `softmax` and `argmax` on an array, in C± and in
C++, for the forward pass of a dense network on a SAPHO. A student of the
laboratory has that task, and Luciano agreed to add the functions to YANC
(2026-11-18), in C± and, as cppcomp built-ins, in C++ with `exp`, `tanh`
and `sigmoid` of a number.

## The problem

In C± a layer is one statement, `h # |W|x⟩ + |b⟩;` (the Dirac notation,
[dirac-expressions.md](dirac-expressions.md)), and the weights come from
files. What follows a layer is not: an activation applied to every element,
the probabilities of the output, the index of the largest one. C± has no
arrays as parameters ([../language-cmm.md](../language-cmm.md), "What does
not exist"), so a program cannot write these once as functions of its own:
each is a loop repeated after every layer.

## The decisions

- The array forms take the name of the array and change it in place:
  `relu(h);`, `sigmoid(h);`, `tanh(h);`, `softmax(y);`, and `argmax(y)` is
  an expression (decided 2026-11-18, against functions inside a Dirac
  expression, `h # relu(|W|x⟩ + |b⟩)`, which would compute the activation
  in the layer's own loop but needs more work in `dirac.c` and in the shape
  check; that form stays open).
- `relu` and `sigmoid` also take a number; `relu(x)` is `pset(x)`. `tanh(x)`
  already existed.
- `relu` takes `int` and `float` arrays, `sigmoid` and `tanh` `float` ones
  (they write floats), all three 1D or 2D. `softmax` takes a `float` vector
  and `argmax` an `int` or `float` vector: on a matrix neither says along
  which side. No `comp`: it has no order.
- `argmax` gives the first of equal elements, as numpy does.
- `softmax` subtracts the largest element before `exp`, see "What was
  measured".
- `relu`, `sigmoid`, `softmax` and `argmax` become names of the library: a
  program cannot give them to a variable. No test of `Compilers/CMMComp/Tests`
  used them (checked 2026-11-18).

## How it is built

Fact, read in the code (`Compilers/CMMComp/Sources/nn.c`).

- The grammar has a statement `std_nn` for the four array forms, which
  checks the array at parse time (`nn_check`) and appends a `STMT_NN`; and
  three expressions, `relu(x)` (`OP_STD_RELU`), `sigmoid(x)`
  (`OP_STD_SIGM`, `builtin_sigmoid` in `stdlib.c`: `F_NEG`, `CAL
  __float_exp`, `F_ADD 1.0`, `F_DIV 1.0`) and `argmax(v)`
  (`OP_STD_ARGMAX`, the array in the node's `id`).
- Each array form is a call of a routine of `Compilers/CMMComp/Includes/`
  (`Macros/` in a release): `nn_relu_f.asm`, `nn_relu_i.asm`,
  `nn_sigmoid.asm`, `nn_tanh.asm`, `nn_softmax.asm`, `nn_argmax_f.asm`,
  `nn_argmax_i.asm`. A call is four words, `LEA v; SET __nn_p; ADD N; CAL
  __nn_<f>`: the address of the first element in `__nn_p`, the address after
  the last in the acc. `macros.c` appends each routine the program calls
  once, after the program, as `float_exp.asm` (which `sigmoid`, `tanh` and
  `softmax` bring); cppcomp appends the same files. A matrix is its rows one
  after the other, so `relu`, `sigmoid` and `tanh` walk it as one vector of
  rows times cols elements.
- The routines walk the array with an address, `LDI 0` and `STI 0`, and
  their own variables, `__nn_p`, `__nn_e`, `__nn_a`, `__nn_b`, `__nn_m`,
  `__nn_x`, `__nn_s`, `__nn_te` and `__nn_tn`. They call `__float_exp` and
  nothing of the program, so they need no stack frame; the call stack is
  2 deep where one of them calls `exp` (worked out by the tools, and no
  `fl_overflow` in the simulations of the tests).
- `softmax` is a running maximum, a loop of `v[k] = exp(v[k] - m)` and their
  sum, one division `s = 1.0 / s` and a loop of products. `argmax` keeps the
  address of the largest element and returns it minus the first's.
- `argmax` inside an expression: a live partial result (`x + argmax(v)`)
  is pushed first (`PSH`), as for a call of a function, and the routine
  leaves the index in the acc. In a `while` condition it runs every turn.
- The waveform shows the four words of the call on the line of the call;
  the routines, after the program, have no source line, as the library's
  float routines.

## Loops in place, then routines

The first form of this change (2026-11-18) wrote the loops in place for each
call: in C±, the statements a program would write by hand, built at walker
time and emitted by `stmt_emit`; in C++, loops in assembly. Each call then
cost the whole loop again: `cmm_nn` was 552 instructions and `test117` 885,
against 495 and 793 for the same programs written by hand without the new
functions (whose C++ version writes `softmax` and `argmax` once, as
functions, and calls them). A network calls `relu` once per layer and
`argmax` at least once per output, so the routines (2026-11-19)
replace the loops: `cmm_nn` is now 386 instructions and `test117` 623, with
the same outputs.

A routine costs more than a loop in place for one call (4 to 8 words, see
below) and less from the second call on. Choosing per program (in place when
called once) was left out: one code path, as for `exp`.

## C++

Fact, read in the code (`Compilers/CPPComp/Sources/codegen.c`, the section
"relu, sigmoid, tanh, softmax, argmax and exp, built in").

- Built-ins, as `in` and `out`, not a header of templates (decided
  2026-11-18): C± and C++ take the same calls, and `exp` has to be one
  anyway. Typing and emission reach them after every function, overload,
  template and method of the program, so a program's own `argmax` is the
  one called: tests 39, 44, 46, 48, 49 and 50 define
  `int argmax(const float *a, int n)`, and the 117 C++ tests give the same
  `.asm` before and after the change (2026-11-18). The recursion analysis
  does not take a call of a built-in for a call through a pointer.
- `exp` is C±'s routine: `Macros/float_exp.asm` (`EXE_MACROS_DIRS`, the
  folder cmmcomp reads) is appended after `@fim` once, as the other
  helpers, without its comments and never fused. Issue #31 agreed on
  2026-10-27 that the two compilers share the routines. `tanh` and
  `sigmoid` of a number are the sequences of cmmcomp's `stdlib.c`.
- An array goes to the same routines as in C±, `nn_*.asm`, appended after
  `@fim` once: its address from `gen_addr` (so it does not matter where the
  array lives: global, local, member) in `__nn_p`, the address after its
  last element in the acc, and a `CAL`.
- A pointer, a reference, a `std::vector` and a `std::array` are refused
  (no size the compiler knows there); `relu` and `argmax` take signed ints
  and floats (an unsigned word has no negative, nor the order of `LES`),
  `sigmoid`, `tanh` and `softmax` floats.

## What was measured

32-bit processor, `#FROUND 0`.

- The 126 C± tests give the same `.asm` as their goldens, and the 117 C++
  tests the same `.asm` as `main` (2026-11-19).
- What a call costs, beyond the same program without it, on a vector of 16
  floats from -2 to 2 (2026-11-19, the routines): the first call, which
  brings the routine; each call after it; the cycles a call takes per
  element. The loops in place of the first form (2026-11-18) cost the
  number in the last column at every call.

  | call | first call: instructions, data words | each further call | cycles per element | in place, every call |
  |---|---|---|---|---|
  | `relu(v);` | 14, 4 | 4 | 8.4 | 10 |
  | `sigmoid(v);` | 46, 14 | 4 | 40.4 | 42 |
  | `tanh(v);` | 50, 16 | 4 | 44.4 | 46 |
  | `softmax(v);` | 78, 18 | 4 | 56.9 | 71 |
  | `argmax(v)` | 26, 7 | 4 | 13.2 | 18 |
  | `sigmoid(x)` | 33, 10 | 7 | | |
  | `relu(x)` | 1, 0 | 1 | | |

  The sizes do not depend on the number of elements. Data words of
  routines are shared: `sigmoid` and `tanh` together bring `exp` once.
- `log(exp(x))` gives `x` back up to 103 and less from 104: `exp` passes
  the largest float there. A `softmax` that did not subtract the maximum
  (the compiler changed for the test, then restored) gives the same values
  for `{1, 2, 3, 2, 1}` but 0, 0 and 1 for `{200, 199, 0}`, instead of
  0.731, 0.269 and 0.
- Measured the same way, the costs of `docs/library.md`'s mathematical
  functions do not match the page on `main` without this change (`sqrt` 32
  instructions and 2 data words against 28 and 1, `tanh` 37 and 12 against
  36 and 11): the page or its method is out of date. Not part of this
  change.

## The tests

- `Compilers/CPPComp/Tests/test117`: the program of `cmm_nn` in C++, the
  same 42 values, and `exp`, `std::exp`, `tanh`, `std::tanh` and `sigmoid`
  of a number. `Scripts/nn_ref.py` holds its `golden.txt` to Python too.
  `NegTests`: a pointer, a `std::vector`, `softmax` of an `int` array,
  `argmax` of a matrix, `exp` of an array, two arguments (in English and in
  Portuguese).
- `Compilers/CMMComp/Tests/cmm_nn`: every form, a tie, one element,
  negatives, `softmax` of `{200, 199, 0}`, `argmax` in a `while` condition,
  on a local array and twice in one expression (the second finds the acc
  live). 386 instructions, 210 of them in the program and the rest the
  routines. `Scripts/nn_ref.py` computes the same program in Python and holds
  the sim golden to it; the regression runs it, and the test is in
  `verified.txt`. Changing the reference (a sum without the last element,
  the last of equal elements, an `argmax` computed once before the
  `while`, `sigmoid(-x)`, `tanh(2x)`, a `relu` that does nothing) makes the
  check fail.
- `NegTests`: `relu(x);` on a number, `softmax` of an `int` array and of a
  matrix, `argmax` of a matrix, `relu` of a `comp` array, `sigmoid` of a
  `comp`.

## Open

- The fused form `h # relu(|W|x⟩ + |b⟩);`.
- `std::vector` and `std::array` in C++.

## For the documentation

`docs/library.md`, "Neural networks": the calls, a two-layer network, why
`softmax` subtracts the maximum and when to leave it out, the costs.
`docs/language-cmm.md`, "Library": the calls and the reserved names.
`docs/language-cpp.md`, "Neural networks": the built-ins, what arrays they
take, and that a function of the program hides them.
