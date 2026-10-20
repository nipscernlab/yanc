# The C++ YANC accepts

`cppcomp`, YANC's C++ compiler, takes a large part of C and C++: classes,
inheritance and virtual functions, templates, references, `new` and `delete`,
recursion, function pointers, a few standard headers. It is for code written
elsewhere (a filter, an estimator, a control loop) that should run on a
SAPHO with few changes. What it costs is size and speed: the same algorithm
written in C± (see [The C± language](language-cmm.md)) usually takes fewer
instructions and cycles.

```cpp
#pragma yanc nuioin 1
#pragma yanc nuioou 1

int fact(int n) { return n <= 1 ? 1 : n * fact(n - 1); }

void main(void)
{
    int n = in(0);
    out(0, fact(n));
}
```

`yanc fact.cpp` makes the processor `fact` (the file's name, which has to be
a valid Verilog module name: see [The C± language](language-cmm.md)). The
program is one source file, which can `#include` others.

## The program

- `main` is required. `void main(void)` and `int main()` both work; the
  value `main` returns is discarded.
- One source file per processor. Other files come in with `#include`,
  searched in the including file's folder, then in the folders given with
  `yanc -I <dir>`, then in the headers that come with YANC.
- The processor is configured with `#pragma yanc` lines, the C++ form of the
  C± directives (see [Configuration](#configuration)).

## Input and output

| call | does |
|---|---|
| `in(p)` | reads input port `p` (an integer literal) as an `int` |
| `out(p, v)` | writes the integer `v` to output port `p` |
| `toma(k, x)`, `cade(k)`, `fcade(k)` | pass a word over link `k` to another processor (see [Several processors](multiprocessor.md)), with the `_sync` forms for a partner on the same clock |

A port carries an integer word. There is no `fin`/`fout` in C++: to read a
float, read an int and scale it; to write one, cast it, `out(0, (int)x)`.
`out` with a `float` writes the float's raw bits, not its value, and the
compiler warns.

With a keyboard and a screen beside the processor (`myKeyPS2` and
`myVGAText`, see [Peripherals](peripherals.md)), a program reads and writes
text as on a PC:

```cpp
#include <iostream>
using namespace std;

int main()
{
    char nome[20];
    int a, b;
    cout << "Nome? ";
    cin.getline(nome, 20);
    cout << "Oi, " << nome << '!' << endl;
    cout << "a b? ";
    cin >> a >> b;
    cout << a << " + " << b << " = " << a + b << endl;
    return 0;
}
```

See [Text on the keyboard and the screen](#text-on-the-keyboard-and-the-screen).

## Types

| type | in YANC |
|---|---|
| `int`, `short`, `long`, `char`, `signed`, `unsigned` | one word, 32 bits; `unsigned` compares and divides as unsigned |
| `int8_t`, `uint8_t`, `int16_t`, `uint16_t` | stored in a word, wrapping at their width as on a PC |
| `long long`, `int64_t`, `uint64_t` | one 32-bit word: the compiler warns that the 64 bits are ignored |
| `bool` | a word holding 0 or 1 |
| `float` | the SAPHO float, 23 mantissa and 8 exponent bits by default: the sizes of IEEE 754 single, **not** its encoding, no infinity, no NaN (see [Floating point](floating-point.md)) |
| `double`, `long double` | the same 32-bit float, with a warning |
| pointers, references | one word; a pointer counts in words, and `p + n` steps `n` elements of the type it points to |
| arrays | up to 8 dimensions; `int t[N] "file.txt";` initialises one from a file, as in C± |
| `enum`, `enum class` | the values are integer literals (`enum E { A = 1 }`); `= -1`, `= A + 1` and an underlying type (`enum class E : int`) are not accepted |
| `struct`, `class`, `union`, bitfields | as in C++ |
| `auto` | for a local with an initializer, and in a range-for |

**Constant data in the data memory.** A string literal, and a global
`const` array whose initializer is made of constants (`const int t[4] = {1,
2, 3};`, `const float k[] = {0.5f, -1.0f};`, `const char msg[] = "Oi";`),
go straight into the data memory: no instruction stores them at the start
of the program. Write a table that never changes as `const`: without it, a
global array with an initializer is stored by code when `main` starts (3
instructions a word), because the data memory is loaded once, when the FPGA
is configured, and a value the program changed would stay changed after a
reset. An array of classes or of pointers is always stored by code.

`sizeof` counts words, not bytes: `sizeof(int) == sizeof(char) == 1`.
`const` objects cannot be assigned; `constexpr` is read as `const`, and a
`const int` with a constant value can size an array or be a template
argument. Every float operation rounds to the nearest (`#FROUND 2`): C++
always compiles at that level.

## Statements and expressions

Everything C has: `if`, `switch` (with fall-through), `while`, `do`, `for`,
`break`, `continue`, `goto`, `return`; compound assignment (`+=`, `<<=`,
...), `++`/`--`, `?:`, the comma, casts (C casts and `static_cast`,
`reinterpret_cast`, `const_cast`, `dynamic_cast`, the last with no runtime
check), `sizeof`, `static_assert`, designated initializers, compound
literals. A range-for (`for (auto x : c)`) walks an array, or an object with
`size()` and `operator[]` such as `std::vector` and `std::array`. A number
starting with `0` is octal. A float used as a condition (`if (x)`, `!x`,
`x && y`, `x ? a : b`) is true when it is not zero, as in C: the compiler
tests its mantissa, because SAPHO's zero float is not an all-zero word.

## Functions

- Overloading by the number and type of the parameters, and default
  arguments, for functions and methods. The overload is chosen as C++
  ranks it: the same type first, then a promotion (`char` or `bool` to
  `int`), then a conversion (between integer types, integer and float, one
  pointer type to another). `char`, `bool`, `unsigned` and `int` are told
  apart (`f('A')` calls `f(char)`), `'A'` is a `char`, `true` a `bool`, and
  a reference parameter (`int&`) matches by the type it refers to.
- **Recursion works**: a function that can reach itself gets a stack frame
  on a 1024-word call stack, built only when the program has one. Inside a
  recursive function a local cannot be an array or a struct; the compiler
  says so. Functions that do not recurse keep their locals at fixed
  addresses, as in C±.
- Function pointers, including arrays of them, as parameters and as struct
  fields.
- No variadic functions (`f(int n, ...)`).

## Classes

- Data members, methods (inline or defined outside the class), access
  labels, static data members and static methods.
- Constructors, overloaded, with member-init lists and default member
  initializers; copy constructors; destructors, run at the end of the scope
  and on `return`, `break` and `continue`. Every object is constructed:
  globals, locals, array elements, members, `new` and `new[]`.
- **Single** inheritance (one base class), virtual functions, pure virtual
  functions, virtual destructors.
- Operator overloading, as member functions only, of `+ - * / == != < > <=
  >= << >> = [] += -= *= /=` and unary `-` and `+`. Not `%`, `<<=`, `>>=`,
  the bitwise operators, `!`, `++`, `--`, `->`, `()`, conversion operators,
  and not as free functions. An operator that returns a reference to its
  object chains: `s << a << b` calls `operator<<` on what the first returned.
- Templates of functions and of classes, with type parameters
  (`typename`/`class`) and `int` parameters, explicit `f<T>(...)`. No
  specialization and no default template argument.
- Namespaces, nested; a qualified name and `using namespace` work. No
  anonymous `namespace {`. A namespace only groups names: every name in it
  is also global, so `std::cout` and `cout` are one object, and a program
  that includes `<iostream>` cannot name its own variable `hex` or `dec`.
- A lambda only as an immediately-invoked global initializer with no
  captures and no parameters (`int k = []() { ... }();`).

## Dynamic memory

`new`, `new[]`, `delete`, `delete[]`, and `malloc(nwords)` / `free(p)`
(sizes in words). They share a heap of 2048 words, which the processor
gets only when the program allocates; when it is full, the allocation
returns 0. The heap's size is fixed.

## The standard headers

| header | gives |
|---|---|
| `<cmath>` | `fabs`, `sqrt`, `isfinite`, `isnan`, `isinf` (the last two always 0: there is no infinity or NaN) |
| `<vector>` | `std::vector<T>`: `size()`, `data()`, `operator[]`, `reserve`, `resize`, `push_back`; its storage is never freed, and there is no `erase`, `insert`, `clear` or iterator |
| `<array>` | `std::array<T, N>`: `operator[]`, `data()`, `size()` |
| `<cstring>` | `strlen`, `strcmp`, `strncmp`, `strcpy`, `strncpy`, `strcat`, `strncat`, `strchr`, `strrchr`, `strstr`, as C's (a char is a word, a string ends with a 0); `memcpy`, `memmove`, `memset`, `memcmp`, `memchr`, `n` in words |
| `<iostream>` | `std::cout`, `std::cin` on the screen and the keyboard: [below](#text-on-the-keyboard-and-the-screen) |
| `<cstdio>` | `putchar`, `puts`, `fputs`, `fputc`, `putc`, `getchar`, `fgetc`, `getc`, `fgets`, with `stdin`, `stdout`, `stderr`: [below](#text-on-the-keyboard-and-the-screen) |
| `<cstdint>`, `<cstddef>` | the integer types, `size_t`, `ptrdiff_t` |
| `<limits>` | `std::numeric_limits<T>`, with `infinity()` = 1e30 |
| `<bit>` | `std::bit_cast` |

There is no `<string>`, `<algorithm>`, `<memory>` or `<utility>`. A header
the program includes but does not use costs nothing: the code nobody calls
leaves the processor.

## Text on the keyboard and the screen

`<iostream>` and `<cstdio>` read the keyboard block `myKeyPS2` and write
the screen block `myVGAText` ([Peripherals](peripherals.md)), over two
links. By default the keyboard is on link 0 and the screen on link 1, both
blocks on the processor's clock. A program changes that with `#define`
before its first `#include`:

| define | default | means |
|---|---|---|
| `YANC_STDIN_LINK` | 0 | the keyboard's link |
| `YANC_STDOUT_LINK` | 1 | the screen's link |
| `YANC_LINK_SYNC` | 1 | 1: the blocks on the processor's clock (`cade_sync`/`toma_sync`); 0: on another clock (`cade`/`toma`) |
| `YANC_ECHO` | 1 | 1: what is typed shows on the screen |
| `YANC_LINE_MAX` | 80 | the longest line read |

**Writing.** `cout <<` takes `const char*`, `char`, `bool` (written as 1 or
0, as C++ does), `int`, `unsigned` and `float`, and `std::endl`,
`std::flush`, `std::hex`, `std::dec`, `std::oct`. A float is written as C++
writes it by default, with 6 significant digits: `7.5`, `3.14159`, `1e+10`,
`0.00015`. The SAPHO float carries about 7 decimal digits, so the sixth can
be off by one. `cout.put(c)` and `cout.write(s, n)` also work. In C:
`putchar(c)`, `puts(s)` (adds the new line), `fputs(s, stdout)`.

**Reading.** As on a PC, nothing reaches the program before Enter: the
characters typed show on the screen and Backspace takes the last one back.
On a PC the terminal does that; here `std::cin` and `getchar` do. Then:

| call | reads |
|---|---|
| `cin >> x` | `int`, `unsigned`, `float` (`2.5`, `-1e3`), `char` (the next one that is not blank) or `char*` (a word); blanks and new lines before it are skipped |
| `cin.getline(s, n)` | the rest of the line, up to `n - 1` characters; the new line is taken and not stored |
| `cin.get()`, `cin.peek()` | the next character, taken or left |
| `cin.ignore(n, c)` | drops up to `n` characters, stopping after `c` |
| `getchar()`, `fgets(s, n, stdin)` | as C's (`fgets` keeps the new line) |

A number that is not there (a letter where `cin >> n` wants digits) sets
`cin.fail()`, gives 0 and takes nothing; `cin.clear()` resets it. As on a
PC, `cin >> n` leaves the Enter in the line, so a `getline` right after it
reads an empty line: `cin.ignore(100, '\n')` between them.

**What it costs.** Only what the program calls: a program that writes but
never reads gets no keyboard link. Printing a number divides nothing (the
processor gets no divider for it); printing or reading a float uses the
float adder and multiplier. The line typed takes `YANC_LINE_MAX + 2` words
of data memory.

**Not yet** (GitHub issue #5): `printf` and `scanf` (they need variadic
functions), `std::string` and `std::getline(cin, s)`, `setw`,
`setprecision`, and `if (cin >> x)` or `!cin` (use `cin.fail()`).

## The preprocessor

`#include "..."` and `<...>`, `#define` with or without arguments (variadic
macros, `#` and `##`), `#undef`, `#if`/`#elif`/`#else`/`#endif` with
`defined`, `#ifdef`/`#ifndef`, `#pragma once`, `#error` (stops) and
`#warning`. A line cannot continue onto the next with a backslash, and there
are no predefined macros (`__FILE__`, `__cplusplus`). `yanc -S` keeps the
assembly; `cppcomp -E file.cpp` prints the preprocessed source.

## Configuration

`#pragma yanc <setting> <value>` sets what the C± directives set (see
[Directives](directives.md)):

| setting | sets | default |
|---|---|---|
| `nubits` | the word width | 32 |
| `nbmant`, `nbexpo` | the float's mantissa and exponent bits | 23 and 8 at 32 bits |
| `nuioin`, `nuioou` | the number of input and output ports | 1 and 1 |
| `nugain` | C±'s `norm()` divisor; C++ has no `norm()`, so it changes nothing | 128 |
| `ndstac`, `sdepth` | the data and call stack depths | worked out from the program |

An unknown setting gives a warning. `#pragma yanc prname` (the name is the
file's), `fftsiz` and `itradd` (bit-reversed addressing and interrupts
exist in C± only) are errors.

## What does not exist

Exceptions (`try`, `catch`, `throw`), `typeid` and RTTI, `friend`,
`decltype`, multiple inheritance, variadic functions, lambdas inside a
function or with captures, template specialization and default template
arguments, `std::string`, file streams, `printf`, `enum` values that are not plain
literals, and more than one source file (use `#include`). Most of these
stop the compiler with "syntax error" at the line.
