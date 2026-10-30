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

C± has a notation for linear algebra on arrays. A chain of array names
between bars is a term: `|a⟩` is the array `a` as a column vector (a ket),
`⟨a|` as a row (a bra), and a name between two bars, `|M|`, is a matrix, a
2D array. In a longer chain the first name is a bra when the chain opens
with `⟨`, the last a ket when it closes with `⟩`, and every other name a
matrix: `|M|a⟩` is `M` times `a`, `⟨a|M|b⟩` is `aᵀ M b`. Two terms side by
side are their product (`c|a⟩`, `|a⟩⟨b|`, `|A||B|`), and `+`, `-` and a
minus in front work as with numbers. `|I|` is the identity, `|0⟩` the zero
vector and `|in(p)⟩` a vector read from input port `p`, one word an
element; each takes its size from the target. The brackets are the
Unicode characters `⟨` (U+27E8) and `⟩` (U+27E9), not `<` and `>`.

A term that is a number, like `⟨a|b⟩` or `⟨a|M|b⟩`, goes anywhere a number
goes. A vector or a matrix goes to an array, whole, with `#`, or to an
output port with `out` or `fout`:

| example | computes |
|---|---|
| `s = ⟨a\|b⟩;` | the inner product |
| `s = 1.0/(0.99 + ⟨x\|P\|x⟩);` | a number from a matrix and a vector |
| `v # \|M\|a⟩ + c\|b⟩;` | `M a + c b` |
| `y # ⟨a\|M\|;` | the row `aᵀ M`, into the vector `y` |
| `A # \|B\| - \|a⟩⟨b\|;` | `B` minus the outer product `a bᵀ` (`A`, `B` not square is fine) |
| `C # \|A\|\|B\|;` | the matrix product |
| `M # c\|I\|;` | `c` times the identity |
| `v # c\|in(0)⟩;` | reads `v`'s size of words from port 0, each times `c` |
| `float D[4] # \|M\|a⟩;` | declares `D` and computes it |
| `out(0, \|M\|a⟩);` | writes each element of `M a` to port 0 |
| `a # c -> \|a⟩;` | shifts `a` by one place, `c` entering (the same array on both sides; not a product) |

The rules:

- **Precedence.** A product binds tighter than `+` and `-` and looser than
  `*`: a coefficient that is a sum needs parentheses, `v # (a+c)|b⟩`,
  because `v # a+c|b⟩` reads as `a + c|b⟩`.
- **Shapes.** Each term has a shape (a number, a vector of N, a row of N,
  an N×M matrix); a sum takes two of one shape, a product needs the
  columns of the first to be the rows of the second, and the target must
  have the shape of the result. When they do not fit, the compiler stops
  and names both.
- **Types.** The elements are `int` or `float`, mixed as in a scalar
  expression: `v # n|a⟩` with an `int n` and a `float a` is a float
  vector, and a float result into an `int` array is converted, with the
  warning of an int receiving a float. Not `comp` yet.
- **Numbers inside.** A coefficient that is not a literal or a variable
  (`(c+d)|a⟩`, `fin(0)*0.001|a⟩`) is computed once, before the loops.
- **`out` takes a number or a vector**, not a matrix, and a vector needs a
  size: `out(0, |0⟩)` has none.
- **Bars.** A bar followed by the name of an array without an index is a
  Dirac bar, so `c | a[k]` stays a bitwise or.
- **Not yet.** `|I|` only on its own or times a number (`M # c|I|`), not
  inside a sum; `|I|` and `|0⟩` not inside a chain.

The code is a loop over the elements, whose size does not grow with the
arrays (an inner product of 4 or of 400 is the same 15 words). It runs
about 2.7 times slower per element than the unrolled code of v9.1 and
before, measured on `proc_rls` (an RLS filter, arrays of N, clocks a
turn of its loop): 855 against 307 at N = 4, 38 067 against 13 971 at
N = 32, while its program goes from 335 to 235 words at N = 4 and from
15 035 to 2335 at N = 32.

**The temporary area.** When the target is also read at another element
than the one being written (`a # |M|a⟩`, `C # |M||C|`), or a product in
the middle of a chain would be computed again for every element
(`|A||B|x⟩` is computed as `|A|(|B|x⟩)`, with `B x` first), the result goes
through `__dirac_tmp`, one area of data memory that every statement of the
program shares, as large as the largest need. The compiler says so on the
line, and sums up at the end what the area costs and how to avoid it: write
to an array that is not on the right-hand side, or compute the middle
product into an array of your own on a line before.

**Results of earlier versions.** Up to v9.1, `A # |a⟩⟨b|` and
`A # |B| - |a⟩⟨b|` computed the outer product transposed (`b aᵀ`),
`a # |M|a⟩` overwrote `a` while still reading it, and an `int` shift of a
computed value (`a # k+1 -> |a⟩`) did nothing. A result computed with
these needs checking. With `K = g P x` and `P` symmetric, as in an RLS
filter, the transposed outer product is the same matrix, up to rounding.
