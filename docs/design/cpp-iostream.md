# `<iostream>`, `<cstdio>` and `<cstring>` for CPPComp, on the keyboard and the screen

Status: implemented 2026-10-20 (v9.1). Manual: [The C++ language](../language-cpp.md),
section "The standard headers", and [Peripherals](../peripherals.md).

## 1. Where it comes from

With `myKeyPS2` and `myVGAText` a C++ program read keys with
`cade_sync(0)` and wrote them with `toma_sync(1, c)`, a character at a time,
and had to turn numbers into digits itself. Luciano asked (2026-10-20) how a
PC does it and whether the SAPHO's C++ could follow the same pattern: "pode
atualizar o compilador sim, vamos fazer o mais completo", with `printf` and
`std::string` left for later as long as they are recorded and the ground is
prepared for them (GitHub issue #5).

On a PC, C++ writes with `std::cout << x` and reads with `std::cin >> x` or
`std::cin.getline(buf, n)`; C has `putchar`, `getchar`, `puts`, `fgets`,
`printf`. Between the keyboard and the program sits the terminal, which
echoes what is typed and lets backspace edit the line; the program gets the
line only when Enter is pressed. The SAPHO has no terminal, so `std::cin`
does that work itself.

## 2. Decided

1. **The standard names and behaviour**, so a program written for a PC
   reads the same: `std::cout`, `std::cin`, `std::endl`, the C functions,
   the `<cstring>` functions. Where C++ has a well-known trap, YANC keeps it
   (`cin >> n` leaves the Enter for the next `getline`, as on a PC).
2. **The compiler learns what the headers need** (Luciano, 2026-10-20),
   rather than the headers dodging it:
   - an overloaded method is called by the overload its arguments pick (it
     was called by its bare name, a label that does not exist: the program
     compiled and did not assemble);
   - `char`, `bool` and `unsigned` are told apart from `int` when an
     overload is chosen, by C++'s ranking (the same type, then a promotion
     char/bool to int, then a conversion); `'A'` has type `char` and
     `true`/`false` type `bool`; a reference parameter (`int&`) matches by
     the type it refers to; a pointer to `char` is apart from other pointers;
   - `operator<<` and `operator>>` can be overloaded (members, as the other
     operators), and a reference result chains (`cout << a << b`).
   Template instances keep the old coarse codes: `f<char>` is still `f<int>`.
3. **The links are set by `#define` before the include** (defaults:
   keyboard on link 0, screen on link 1, both `_sync`; `YANC_ECHO`,
   `YANC_LINE_MAX`). A `#pragma yanc` was proposed first; a `#define` needs
   nothing from the compiler (the link number must be a literal, and the
   macro expands to one).
4. **Three layers in one internal header, `<yanc_term>`**, so `printf`,
   `scanf` and `std::string` stand on them later: one character on the
   links; numbers as text (integer in a base, with width, fill and sign; a
   float with significant digits); reading (the edited line, blanks, an
   integer, a float, a word, a line).
5. **No division anywhere**: printing a number must not give the processor
   a divider (large, and a long path). Decimal digits are made by doubling a
   digit array one bit at a time; a float is printed with powers of 10 built
   by multiplication and digits counted by subtraction; reading multiplies
   by 10 as `(v << 3) + (v << 1)`.
6. **A float prints as C++ prints it by default** (6 significant digits,
   `%g`): `7.5`, `3.14159`, `1e+10`, `0.00015`, `1e-07`.
7. **`<cstring>` complete now** (Luciano: the base of `std::string`):
   `strlen strcmp strncmp strcpy strncpy strcat strncat strchr strrchr
   strstr memcmp memmove memchr`, beside `memcpy memset`.
8. **Later** (GitHub issue #5): `printf`/`scanf` (need variadic functions),
   `std::string` and `std::getline(cin, s)`, `setw`, `setprecision`,
   `if (cin >> x)` (needs a conversion operator).

## 3. What came up while building it

- **Namespaces are transparent in CPPComp**: a name in `std` or in any
  namespace is global. The internal names of `<yanc_term>` all start with
  `yanc_` so they cannot collide with a program's; the standard names
  (`cout`, `endl`, `hex`, `dec`...) are global once `<iostream>` is included.
- **A link used only by code that is never called**: `<iostream>` holds the
  keyboard's code, so a program with `cout` alone left a `#LKSYNC 0` behind
  after `asm_reach` removed that code, and asmcomp refused the program.
  `asm_reach` (shared by both compilers) now drops a `#LKSYNC k` when no
  `TOM k`/`CAD k` is kept. Fixture test93.
- **A float's digits**: scaling a large number down by 0.1 piled up the
  error (`1e10` printed `9.99999e+09`); building the power of 10 up by
  multiplication fixed it.
- **The old overload codes**: compiled with the previous cppcomp, test92
  stops at "redefinition of function 'f'" (`f(char)` beside `f(int)`). Every
  other C++ and C± program of the regress compiles to the same `.asm` as
  before (93 C++, 93 C±, compared one by one).

## 4. Tests

- test92: the overload rules (hand-computed golden).
- test93: a program that only writes, with `<cstdio>` and the string
  functions (hand-computed golden).
- Regress pass VGATEXT, system C: a 32-bit C++ SAPHO with `<iostream>` and
  `<cstring>` on a `myKeyPS2` and a `myVGAText` of its own; the keyboard
  types a name with a backspace and two numbers, and the bench reads the
  screen memory: `Nome? Ana`, `Oi, Ana!`, `a b? 12 30`, `12 + 30 = 42`,
  `-7 ff 18 3.14159 1e+10 0.00015`, `Ana tem 3 letras`.

## 5. For the documentation (item 21 in TODO.md)

Written: docs/language-cpp.md (the headers, overloading, the operators) and
docs/peripherals.md (the keyboard and the screen from C++).
