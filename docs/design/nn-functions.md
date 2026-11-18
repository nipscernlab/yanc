# Functions for neural networks

`relu`, `sigmoid`, `tanh`, `softmax` and `argmax` on an array, in C± and in
C++, for the forward pass of a dense network on a SAPHO. A student of the
laboratory has that task, and Luciano agreed to add the functions to YANC
(2026-11-18). This note covers C±; the C++ part (the same calls as cppcomp
built-ins, with `exp`, `tanh` and `sigmoid` of a number) is still to be
written.

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
- At walker time `nn.c` builds the statements a program would write by
  hand, `for (k = 0; k < N; k++) v[k] = pset(v[k]);`, as the parser builds a
  `for`: an assignment, then a `STMT_WHILE` with the init on `then_body`
  and the step on `else_body`, so that the loop gets the bottom test of
  `for_bottom_test`. `stmt_emit` emits them. No instruction is written by
  hand except `argmax`'s two below; every other one comes from the code
  paths of `pset`, `tanh`, `exp`, the comparisons and the array accesses.
- A matrix gets two loops. `softmax` is a running maximum, a loop of
  `v[k] = exp(v[k] - m); s = s + v[k];`, one division `s = 1.0 / s` and a
  loop of products. `argmax` is `b = 0; m = v[0];` and a loop of
  `if (v[k] > m) {m = v[k]; b = k;}`.
- The counters, the maximum and the sum are variables of the compiler,
  `__nn<stem><n>_<function>`, one set per use, as `dirac.c` names its own.
- `argmax` inside an expression: a live partial result (`x + argmax(v)`)
  is pushed first (`PSH`), as for a call of a function, and the result is
  loaded into the acc (`LOD b`). Its loops then run where the expression
  is, so in a `while` condition they run every turn.
- `Scripts/wave_map.py` accepts the jumps of these loops (`__Lwh`, and
  `__Lif` for `softmax` and `argmax`) on the line that calls them; its
  `--selftest` on `sapho_all` still catches broken maps (2026-11-18).

## What was measured

On 2026-11-18, 32-bit processor, `#FROUND 0`.

- The 126 C± tests give the same `.asm` as their goldens.
- What each call costs, beyond the same program without it, and its cycles
  on a vector of 16 floats from -2 to 2:

  | call | instructions | data words | cycles per element |
  |---|---|---|---|
  | `relu(v);` | 10 (19 for a matrix) | 0 | 8.1 |
  | `sigmoid(v);` | 42 | 10 | 40.1 |
  | `tanh(v);` | 46 | 12 | 44.1 |
  | `softmax(v);` | 71 | 14 | 55.4 |
  | `argmax(v)` | 18 | 3 | 11.6 |
  | `sigmoid(x)` | 33 | 10 | |
  | `relu(x)` | 1 | 0 | |

  The sizes are the same at 4 and at 64 elements.
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

- `Compilers/CMMComp/Tests/cmm_nn`: every form, a tie, one element,
  negatives, `softmax` of `{200, 199, 0}`, `argmax` in a `while` condition,
  on a local array and twice in one expression (the second finds the acc
  live). `Scripts/nn_ref.py` computes the same program in Python and holds
  the sim golden to it; the regression runs it, and the test is in
  `verified.txt`. Changing the reference (a sum without the last element,
  the last of equal elements, an `argmax` computed once before the
  `while`, `sigmoid(-x)`, `tanh(2x)`, a `relu` that does nothing) makes the
  check fail.
- `NegTests`: `relu(x);` on a number, `softmax` of an `int` array and of a
  matrix, `argmax` of a matrix, `relu` of a `comp` array, `sigmoid` of a
  `comp`.

## Open

- The C++ part.
- The fused form `h # relu(|W|x⟩ + |b⟩);`.
- `std::vector` and `std::array` in C++.

## For the documentation

`docs/library.md`, "Neural networks": the calls, a two-layer network, why
`softmax` subtracts the maximum and when to leave it out, the costs.
`docs/language-cmm.md`, "Library": the calls and the reserved names.
