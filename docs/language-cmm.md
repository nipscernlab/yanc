# The C± language

C± (read "C plus-minus") is the language YANC was built for: C cut down to
what turns into hardware of a size known at compile time, plus a complex
number type and a notation for vectors. If you know C you can read C±
straight away; what needs learning is mostly what is missing, and why.

Every variable has a fixed address in the data memory: there is no heap, no
pointer, no recursion. That is what lets each variable show up by name in
the simulation waveform, and what lets the compiler build only the hardware
the program uses: a program that never divides gets a processor with no
divider.

## A first program

```c
#NUIOIN 1
#NUIOOU 1

int twice(int x)
{
    return 2 * x;
}

void main()
{
    while (1)
    {
        int a = in(0);       // read input port 0
        out(0, twice(a));    // write output port 0
    }
}
```

`yanc twice.cmm` turns this file into the processor `twice` (the file's
name), in the folder `twice/`. The file's name becomes the name of a Verilog
module, so it has to be one: letters, digits and `_`, not starting with a
digit, and not a Verilog keyword (`begin.cmm` or `my-prog.cmm` are refused;
rename the file). The lines starting with `#` configure the
processor: here, one input port and one output port. The other settings have
defaults; see [Directives](directives.md).

## Comments, names and literals

- Comments: `// to the end of the line` and `/* across lines */`.
- Names: letters, digits and `_`, not starting with a digit, at most 50
  characters (the toolchain's buffers; GitHub issue #66). Names starting
  with `__` belong to the compiler. A name may not be spelt like an
  instruction of the processor once the compiler has prefixed it (a local
  `x` of the function `f` becomes `f_x` in the assembly): the compiler says
  so, and which two names collide.
- **`i` is reserved** for the imaginary unit: `int i;` is an error. Use `k`,
  `n`, `idx`.
- Integer literals, in every form C writes them: `42`, `0x2A`, `052`
  (octal: a leading 0 makes it octal, as in C, so `010` is 8), `0b101010`,
  and the `u`, `U`, `l`, `L` suffixes (they change nothing: there is one
  int). A negative number is the unary minus applied to it: `-42`. A
  decimal literal must fit the `#NUBITS` word: at 8 bits the largest is 127,
  and `200` is an error (arithmetic past the word wraps, as the hardware
  does: `100 + 100` is -56). The one exception is the most negative int:
  `-128` at 8 bits (`-2147483648` at 32) is accepted, so 128 may follow a
  unary minus, but `a - 128` is still an error. An array size, a port and
  a directive are no values of the word and take any literal: `int v[200];`
  at 8 bits compiles, with a warning that an int index reaches only 127. A
  hexadecimal, octal or binary literal is a
  bit pattern of the `#NUBITS` word, as in C: with the sign bit set it is
  negative, so `0xFFFFFFFF` is -1 and `0x80000000` the most negative int at
  32 bits (`0xFF` and `0x80` at 8). Such a literal is accepted in
  expressions and in lists between braces; an array size, a port or a
  `case` takes a positive one.
- Float literals, in every form C writes them: `3.14`, `0.5`, `.5`, `2.`,
  `1e-5`, `2.5E+3`, `007.5`, and with a suffix `1.5f` (`f`, `F`, `l`, `L`;
  the suffix changes nothing, there is one float). Each one is encoded
  exactly to the nearest value of the format, ties to even.
- Imaginary literals: a number followed by `i`, `2i`, `0.5i`, `1e-8i`. A
  complex number is a sum, as in C99: `3 + 4i`, `-1.5 - 2i`, `1e-8 + 2e-8i`,
  `2i`, and with the precedence of any sum, so `y*3 + 2i` is `(y*3) + 2i`.
  A real literal and an imaginary one make one constant, and a real
  expression and an imaginary literal are `complex(x, b)`: no instruction
  adds the zero real part. From two variables, use `complex(re, im)`.

## Types

| type | what it is |
|---|---|
| `int` | signed integer, `#NUBITS` bits wide (32 by default) |
| `float` | the SAPHO floating point: `#NBMANT` mantissa bits and `#NBEXPO` exponent bits (23 and 8 by default). It is **not** IEEE 754; see [Floating point](floating-point.md) |
| `comp` | a complex number: two floats, the real part and the imaginary part |
| `void` | only as the return type of a function |

There is no `char`, `double`, `unsigned`, `bool`, `struct`, string or
pointer. `int` and `float` convert into each other on their own wherever
one is used for the other (an assignment, a function argument, a return, an
output), and the compiler prints a warning at each place; there is no cast.

## Variables and arrays

```c
int a, b;
int n = 0, k, m = n + 1;          // any name of a list can be initialised
float gain = 0.5;
int v[128];                       // an array
float M[4][4];                    // two dimensions at most
float table[64] "table.txt";      // initialised from a file
```

- A variable can be declared at the top of the file (global) or anywhere
  inside a function (local to it). Any name of a declaration list can have
  an initialiser, and a later name can read an earlier one, as in C. A
  local initialiser runs each time the function is called. A global
  initialiser runs once, before `main`, wherever the file declares the
  global, between functions too, and it may call a function defined above
  it.
- The size of an array is a number (or a `#define` name for one).
- An array is initialised between braces, as in C, or from a file:

  ```c
  int   v[5]    = {3, -2, 7};             // v[3] and v[4] are 0
  float g[3]    = {1.5, -0.25, 4};
  comp  z[2]    = {1.5-2i, 3};
  int   m[2][3] = {{1, 2}, {4, 5, 6}};    // or the six values without inner braces
  int   t[256] "tabela.txt";             // one value per line, beside the source
  ```

  Between braces go constants (a minus sign in front is fine): only integers
  in an `int` array, integers and floats in a `float` one, any of the three
  in a `comp` one. Positions left out are zero; more values than positions
  is an error. In a 2D array, inner braces mark the rows: every row in braces
  or none. The file holds one value per line (`int`, `float`, or `re+imi` for
  `comp`, as `1.5-2i`); fewer lines than the size is an error, more lines are
  ignored with a warning.
- Either way the values go straight into the data memory: no instruction
  stores them, so they cost nothing at run time. They are loaded once, with
  the memory, when the FPGA is configured: a local array keeps, from one call
  of its function to the next, what the function wrote into it (like a C
  `static`; the compiler warns), and a reset does not bring the values back.
- `v[k]` reads or writes an element; `M[r][c]` an element of a matrix.
- `v[k)` (a closing parenthesis) reads or writes the element at the
  **bit-reversed** index, the addressing an FFT needs (its width set by
  `#FFTSIZ`).
- An array cannot be a function parameter; share it as a global.

## Operators

| family | operators | notes |
|---|---|---|
| arithmetic | `+` `-` `*` `/` `%` | `%` between ints only; `/` and `%` build a divider into the processor |
| comparison | `<` `>` `<=` `>=` `==` `!=` | 1 or 0; with a `comp`, only `==` and `!=` (see Complex numbers) |
| logical | `&&` `\|\|` `!` | 1 or 0; on ints and floats (not `comp`) |
| bitwise | `&` `\|` `^` `~` | ints |
| shifts | `<<` `>>` `>>>` | `>>` fills with zeros, `>>>` with the sign |
| increment | `x++`, `v[k]++` | also inside an expression |

The precedence is C's. There is no `--`, no compound assignment (`+=`,
`-=`, ...), no `?:`, no `sizeof`. A shift costs almost nothing in hardware,
a division a divider and five clock cycles (four for a float division):
`x >> 2` instead of `x / 4` when `x` is not negative.

## Control flow

`if` / `else`, `while`, `do` ... `while`, `for`, `switch` / `case` /
`default`, `break`, `continue`, `return`, as in C. There is no `goto`.

- `for (init; condition; step)`: `init` is an assignment or a declaration
  with a value (`int k = 0`), `step` an assignment or `k++`; either can be
  empty. One clause each, no commas.
- `case` takes a constant number. A `case` without `break` falls through to
  the next, as in C.
- `break` leaves the innermost loop or `switch`; `continue` goes to the next
  turn of the innermost loop.
- The empty statement `;` and the empty block `{ }` emit nothing, so a
  waiting loop is written as in C: `while (in(1)) ;`.
- A condition (`if`, `while`, `for`, `do`, and the operands of `!`, `&&`,
  `||`) is true when its value is not zero, as in C, also for a float:
  `if (0.5)` is true and `if (0.0)` false. The compiler tests the float's
  mantissa (`AND 2^NBMANT - 1`, one instruction), with no conversion to int.
  A `comp` condition is true when its real or its imaginary part is not
  zero (C99). A `switch` index, by contrast, is converted to int, truncating.

## Functions

```c
float mean(float a, float b)
{
    return (a + b) * 0.5;
}
```

- `main()` is required; the program starts there.
- A function is defined above its first call: there are no prototypes, and
  the whole program is one `.cmm` file.
- Parameters, as many as needed, are passed by value; they can be `int`, `float`
  or `comp`, never an array. The return type is `int`, `float`, `comp` or
  `void`.
- A function that is not `void` returns on every path: the compiler stops
  when a path can reach the end of the body, which would run on into the
  next function. A path ends in a return when its last statement is a
  `return`, an `if`/`else` whose two sides end in one, a `while (1)` left
  only by a `return`, or a `switch` with a `default` and no `break` whose
  last statement does.
- **No recursion.** The locals of a function have fixed addresses, so a
  call to itself would overwrite them; the compiler stops with an error. C++
  (see [The C++ YANC accepts](language-cpp.md)) allows recursion.

## Input and output

The processor talks to the outside through numbered ports, as many as
`#NUIOIN` (inputs) and `#NUIOOU` (outputs) say. The port number is a
literal, not a variable.

```c
int   a = in(0);     // read port 0 as an int
float g = fin(1);    // read port 1 and convert it to float
out(0, a + 1);       // write an int to port 0
fout(1, g * 2.0);    // convert a float to int and write it to port 1
```

A port always carries an integer word. `fin` converts the word it reads to
float; `fout` converts a float to an integer (truncating) before writing.
`out` with a float, or `fout` with an int, still works, with a warning.

In simulation, input port N reads its values from `input_N.txt` beside the
source, one per line, and output port N writes `output_N.txt` in the output
folder.

## Complex numbers

```c
comp z = 3 + 4i;
comp w = complex(1.0, -2.0);
comp p = z * w;
float r = real(p);
float m = abs(z);        // |z| = 5
```

`+ - * /` work on `comp`. `==` and `!=` too, as in C: two complex numbers
are equal when their real parts are equal and their imaginary parts are
equal, and a real is the `comp` with a zero imaginary part (`3+0i == 3`).
They have no order: `<`, `>`, `<=` and `>=` with a `comp` are an error;
compare `abs(z)`, `mod2(z)`, `real(z)` or `imag(z)`. `real(z)`, `imag(z)`, `conj(z)`, `abs(z)`,
`mod2(z)` (|z|²) and `fase(z)` (the angle) take one apart, and `sqrt`,
`exp`, `log`, `sin`, `cos`, `tan`, `atan` accept a `comp`. A `comp` does not
go to an output port as such: write `real(z)` and `imag(z)`.

## Library

The functions that cost little map almost one to one onto instructions:
`abs(x)`, `pset(x)` (0 if negative), `sign(x, y)` (`y` with the sign of
`x`), `norm(x)` (`x` divided by `#NUGAIN`, a power of two, without a
divider) and `copy(x, y)` (copies the bits of `x` into the variable `y`
with no conversion). The mathematical ones become routines of many
instructions: `sqrt`, `exp`, `log`, `pow`, `sin`, `cos`, `tan`, `atan`,
`sinh`, `cosh`, `tanh`, `floor`, `ceil`, `round`. See
[The library](library.md).

## Several processors

`toma(k, x)` hands `x` to the processor at the other end of link `k`;
`cade(k)` and `fcade(k)` take a word from it. See
[Several processors](multiprocessor.md).

## The preprocessor

`#define NAME text` replaces the name by the text from there on:
`#define N 64`. It takes no arguments. There is no `#include` and no
`#if`.

## What does not exist

Pointers, `struct`, strings, recursion, dynamic memory, `#include`, several
source files, arrays of more than two dimensions, arrays as parameters,
`goto`, `--`, compound assignment, `?:`, casts. Most of them are in the C++
YANC accepts, at a cost in instructions; C± keeps the processor small.
