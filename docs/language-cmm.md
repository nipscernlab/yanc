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
- Names: letters, digits and `_`, not starting with a digit. Names starting
  with `__` belong to the compiler. A name may not be spelt like an
  instruction of the processor once the compiler has prefixed it (a local
  `x` of the function `f` becomes `f_x` in the assembly): the compiler says
  so, and which two names collide.
- **`i` is reserved** for the imaginary unit: `int i;` is an error. Use `k`,
  `n`, `idx`.
- Integer literals: `0`, `42`. A negative number is the unary minus applied
  to it: `-42`.
- Float literals: `3.14`, `0.5`, `2.`. There is no exponent form: write
  `0.00001`, not `1e-5`.
- Complex literals: a real part and an imaginary part, both written:
  `3 + 4i`, `-1.5 - 2i`, `0 + 1i`. From variables, use `complex(re, im)`.

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
float gain = 0.5;
int v[128];                       // an array
float M[4][4];                    // two dimensions at most
float table[64] "table.txt";      // initialised from a file
```

- A variable can be declared at the top of the file (global) or anywhere
  inside a function (local to it). Only one variable per declaration can be
  initialised: `int a = 1;`, not `int a = 1, b = 2;`.
- The size of an array is a number (or a `#define` name for one).
- An array is initialised from a file only: the file's name in quotes after
  the size. There is no `= {1, 2, 3}`. The file holds one value per line
  (`int`, `float`, or `re+imi` for `comp`, as `1.5-2i`) and sits beside the
  source. Fewer lines than the size is an error; more lines are ignored with
  a warning. The values are loaded once, with the memory: a local array
  initialised from a file keeps, from one call of its function to the next,
  what the function wrote into it (like a C `static`; the compiler warns).
- `v[k]` reads or writes an element; `M[r][c]` an element of a matrix.
- `v[k)` (a closing parenthesis) reads or writes the element at the
  **bit-reversed** index, the addressing an FFT needs (its width set by
  `#FFTSIZ`).
- An array cannot be a function parameter; share it as a global.

## Operators

| family | operators | notes |
|---|---|---|
| arithmetic | `+` `-` `*` `/` `%` | `%` between ints only; `/` and `%` build a divider into the processor |
| comparison | `<` `>` `<=` `>=` `==` `!=` | 1 or 0 |
| logical | `&&` `\|\|` `!` | 1 or 0 |
| bitwise | `&` `\|` `^` `~` | ints |
| shifts | `<<` `>>` `>>>` | `>>` fills with zeros, `>>>` with the sign |
| increment | `x++`, `v[k]++` | also inside an expression |

The precedence is C's. There is no `--`, no compound assignment (`+=`,
`-=`, ...), no `?:`, no `sizeof`. A shift costs almost nothing in hardware,
a division a divider and three clock cycles: `x >> 2` instead of `x / 4`
when `x` is not negative.

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
- There is no empty statement: `while (busy()) ;` and `{ }` are syntax
  errors. A waiting loop needs a body, even a harmless one:
  `while (in(1)) { x = 0; }`.

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
- Parameters (16 at most) are passed by value; they can be `int`, `float`
  or `comp`, never an array. The return type is `int`, `float`, `comp` or
  `void`.
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

`+ - * /` work on `comp`. `real(z)`, `imag(z)`, `conj(z)`, `abs(z)`,
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
