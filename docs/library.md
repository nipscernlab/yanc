# The library

The built-in functions of C±. They need no `#include`. A function adds to
the processor only when the program calls it: the cheap ones are an
instruction or two, the mathematical ones a routine of tens of
instructions, added once however many times it is called. C++ has its own
headers (see [The C++ YANC accepts](language-cpp.md#the-standard-headers)).

The costs below were measured on 2026-10-13 (32-bit processor, `#FROUND
0`): the instructions and data words a program `fout(0, f(fin(0)))` has
beyond the same program without `f`. They are program memory, not time:
how many cycles a routine takes depends on its argument.

## Input and output

| function | returns | does |
|---|---|---|
| `in(p)` | `int` | reads input port `p` |
| `fin(p)` | `float` | reads input port `p` and converts it to float |
| `out(p, x)` | | writes the int `x` to output port `p` (a float is converted, with a warning) |
| `fout(p, x)` | | converts the float `x` to an int (truncating) and writes it to output port `p` |

`p` is an integer literal, below `#NUIOIN` or `#NUIOOU`.

## Cheap functions

| function | returns | does | cost |
|---|---|---|---|
| `abs(x)` | as `x` | the absolute value (for a `comp`, its modulus, a `float`) | 1 instruction |
| `pset(x)` | as `x` | `x`, or 0 when `x` is negative | 1 instruction |
| `sign(x, y)` | as `y` | `y` with the sign of `x` | 2 instructions |
| `norm(x)` | `int` | `x / #NUGAIN` as a shift, no divider; `int` only | 1 instruction |
| `copy(x, v)` | | copies the bits of `x` into the variable `v`, with no conversion | a few instructions |

`copy` is how a program looks at a float's bits as an int or builds a float
from bits (see [Floating point](floating-point.md)).

## Mathematical functions

They take a `float` (an `int` is converted) and return a `float`.

| function | cost: instructions, data words | also takes a `comp` |
|---|---|---|
| `sqrt(x)` | 28, 1 | yes (44, 4) |
| `exp(x)` | 28, 9 | yes (68, 19) |
| `log(x)` | 43, 10 | yes (129, 25) |
| `sin(x)` | 32, 10 | yes (80, 20) |
| `cos(x)` | 33, 11 | yes (81, 20) |
| `tan(x)` | 47, 15 | yes (93, 21) |
| `atan(x)` | 37, 11 | yes (148, 27) |
| `sinh(x)`, `cosh(x)` | 34, 11 | no |
| `tanh(x)` | 37, 11 | no |
| `pow(x, y)` | 76, 18 | no |
| `floor(x)` | 10, 2 | no |
| `ceil(x)` | 11, 2 | no |
| `round(x)` | 6, 1 | no |

The complex forms are measured against a program that builds and writes a
`comp` (11 instructions, 3 words). Functions that share a routine share its
cost: `sin` and `cos` together cost
36 instructions, not 65. The methods behind them are described in
*Implementação de Funções Não-Lineares em Processador Soft-Core* (ENMC 2025,
<https://cdn.nipscern.com/publications/enemc-2025-implementacao-de-funcoes.pdf>).
Their accuracy follows the float format: the routines are written for 23
mantissa bits, and a narrower `#NBMANT` loses accuracy in them.

## Complex numbers

| function | returns | does |
|---|---|---|
| `complex(a, b)` | `comp` | `a + bi` from two floats |
| `real(z)`, `imag(z)` | `float` | the real and the imaginary part |
| `abs(z)` | `float` | the modulus |
| `mod2(z)` | `float` | the modulus squared, without a square root |
| `fase(z)` | `float` | the angle, in radians, in all four quadrants |
| `conj(z)` | `comp` | the conjugate |

`+ - * /` work on `comp`, and so do the mathematical functions marked
"yes" in the table of [Mathematical functions](#mathematical-functions).

## Vectors in Dirac notation

C± has a notation for linear algebra on arrays: `|a⟩` is an array as a
column vector, `⟨a|b⟩` the inner product, `|M|` a matrix, and an assignment
`v # ...` computes a vector or a matrix in place:

| form | computes |
|---|---|
| `s = ⟨a\|b⟩` | the inner product of the arrays `a` and `b` |
| `v # \|M\|a⟩` | the matrix `M` times the vector `a` |
| `v # c\|a⟩` | the vector `a` times the scalar `c` |
| `v # \|a⟩ + c\|b⟩` | `a + c·b` |
| `M # \|a⟩⟨b\|` | the outer product |
| `M # \|A\| - \|a⟩⟨b\|` | `A` minus the outer product |
| `M # c\|A\|` | the matrix `A` times `c` |
| `M # c\|I\|` | `c` times the identity |
| `v # \|0⟩` | the zero vector |
| `v # c\|in(p)⟩` | reads input port `p` once per element of `v`, each value times `c` |
| `a # c -> \|a⟩` | shifts `a` by one place, `c` entering (the same array on both sides) |
| `out(p, c\|a⟩)` | writes each element of `a`, times `c`, to output port `p` |

The brackets are the Unicode characters `⟨` (U+27E8) and `⟩` (U+27E9), not
`<` and `>`. The sizes are the arrays' declared sizes; the elements are
`int` or `float`, not `comp`.
