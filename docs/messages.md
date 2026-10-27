# Messages

What the tools say, and how to get past the messages that come up most.
The complete list, generated from the compilers' sources, is in
[the message reference](reference/messages.md).

## Reading them

- An **error** stops the tool: `Error on line 12: ...` (C±, C++ adds the
  file: `filter.cpp:12: error: ...`, a header's name when the line is in a
  header). `yanc` stops with it and returns the tool's exit code.
- A **warning** (`Heads up on line 12: ...` in C±, `warning:` in C++) does not
  stop anything; it points at something that compiles but may not do what
  was meant.
- **Info** lines tell what the tools did: the hardware the program needs,
  the library routines added, the constants approximated, the stack depths.
- `cmmcomp`'s messages have a tone of their own ("You're a confused soul!");
  the information is in the first half of the sentence. `--lang=pt` gives
  them in Portuguese (see [The command line](command-line.md)).

## Syntax error

`Syntax error on line N` in C± (or `syntax error` in C++) means the line
uses something the language does not have. The usual causes in C±:

| written | why | instead |
|---|---|---|
| `a += 2;`, `a--;`, `x ? a : b` | no compound assignment, no `--`, no `?:` | `a = a + 2;`, `a = a - 1;`, an `if` |
| `(int) x` | no casts: conversions are automatic | `int a = x;` |
| `#include`, `#if`, `#define F(x) ...` | the C± preprocessor has only `#define NAME text` | write it out, or use C++ |

In C++, a syntax error usually marks a feature outside what YANC accepts:
`enum` values that are not literals, multiple inheritance, free-function
operators, exceptions, variadic functions (see
[The C++ YANC accepts](language-cpp.md#what-does-not-exist)).

## Common errors

- **`the symbol 'i' is reserved for the imaginary part`**: `i` is the
  imaginary unit in C±. Name the counter `k`.
- **`function 'f' calls itself`**: C± has no recursion (locals have fixed
  addresses). Write it as a loop, or in C++.
- **`... is 'f_x' in the assembly, and that name is already ...`**: two names
  become the same word in the assembly (a local `x` of `f` is `f_x`, so a
  global `f_x` collides with it). Rename one; the message says which two.
- **`the file 'begin.cmm' names the processor, and 'begin' is a Verilog
  keyword`**: the processor takes the file's name, which becomes a Verilog
  module's. Rename the file.
- **`#PRNAME is gone`**, **`#pragma yanc prname is gone`**: since v7.1 the
  name is the file's. Delete the line.
- **`sinh(.) doesn't take a comp`** (also `cosh`, `tanh`, `floor`, `ceil`,
  `round`): these take an int or a float only.
- **`there's no output port N`** (or input port): the port number is at or
  above `#NUIOOU` (`#NUIOIN` for inputs). Raise the directive.
- **`you're N lines short in file '...'`** (asmcomp): an array initialised
  from a file has more elements than the file has lines.
- **`floating-point setup doesn't add up`** (asmcomp): a program with float
  needs `#NUBITS` equal to `#NBMANT + #NBEXPO + 1`. A program without float
  never sees it; with float on a narrow word, give the format too
  (`#NUBITS 16`, `#NBMANT 10`, `#NBEXPO 5`).
- **`the exponent is too short for the mantissa`** (asmcomp): `#NBMANT` must
  be below `2^(#NBEXPO-1)`. Widen `#NBEXPO` or narrow `#NBMANT`.

## Common warnings

- **`variable 'x' is float, but getting an int`** (and the other way round):
  a conversion the compiler inserted. Correct when meant; it costs an
  instruction, and at `#FROUND 0` a large int converts wrongly (see
  [Floating point](floating-point.md)).
- **`if you don't want this warning, use 'fout'`** / **`use 'out'`**: an
  output of the other type; `fout` writes a float (converted to int), `out`
  an int.
- **`global variable 'b' isn't being used`**: it takes a data word for
  nothing.
- **`local array 'table' is initialized from file ... only once`**: a local
  array loaded from a file keeps, between calls, what the function wrote in
  it.
- **`out() sends an integer word; this float is output as its raw bit
  pattern`** (C++): write `out(p, (int)x)`.
- **`'x' asks for 64 bits, but a YANC word has 32`** (C++): `long long` and
  `double` are 32 bits in YANC.

## Information worth reading

- **`constant 0.1 on line N approximated to 0.10000000894070`**: the nearest
  value the float format holds.
- **`ULA resource -> ...`** (asmcomp): each piece of the arithmetic unit the
  program needs; a divider or a float divider is the slowest and largest.
- **`N divisions, 5 words each for DIV/MOD and 4 for F_DIV`**: every
  division is written as five instructions (integer) or four (float),
  because the dividers take that many clock cycles.
- **`N NOP inserted between a float operation and the JIZ/LDI/ILI after
  it`** (asmcomp): a hand-written `.asm` tests a float result with `JIZ`, or
  uses it as the index of `LDI`/`ILI`, right away; the jump and the read
  index take the arithmetic unit before the float normaliser, so a `NOP`
  goes in between (one word and one cycle each).
- **`stack depths from the program: SDEPTH n, NDSTAC n`**: the stacks are as
  deep as the program needs.
