# Directives

A C± program configures its processor with directives: lines starting with
`#`, at the top of the file, one per line. Each has a default, so a program
writes only what it changes. A C++ program says the same with
`#pragma yanc <setting> <value>` (see [The C++ YANC accepts](language-cpp.md#configuration)).

```c
#NUBITS 32
#NUIOIN 2
#NUIOOU 1
#FROUND 2
```

## Configuration

| directive | default | sets | C++ |
|---|---|---|---|
| `#NUBITS n` | 32 | the word width, in bits: 2 to 32 | `nubits` |
| `#NBMANT n` | 23 | the float's mantissa bits | `nbmant` |
| `#NBEXPO n` | 8 | the float's exponent bits | `nbexpo` |
| `#NUIOIN n` | 1 | the number of input ports | `nuioin` |
| `#NUIOOU n` | 1 | the number of output ports | `nuioou` |
| `#FROUND n` | 0 (C++: 2) | how float operations round: 0, 1 or 2 (see [Floating point](floating-point.md)) | always 2 |
| `#NUGAIN n` | 128 | the divisor of `norm(x)`; a power of two | (no effect) |
| `#FFTSIZ n` | 3 | how many low bits of the index `v[k)` reverses: 1 to 30, and no more than the data memory's address bits (an FFT array of 2^n words has to fit) | (C± only) |
| `#NDSTAC n` | worked out | the data stack depth | `ndstac` |
| `#SDEPTH n` | worked out | the call stack depth | `sdepth` |

- **`#NUBITS = #NBMANT + #NBEXPO + 1`** (the sign bit), **and `#NBMANT`
  below `2^(#NBEXPO-1)`** (the exponent has to reach 1.0, which is
  `2^(NBMANT-1) · 2^-(NBMANT-1)`): the assembler stops when either fails,
  but only for a program that uses float. The defaults give 32 = 23 + 8 + 1;
  a 16-bit processor with float is, for example, `#NUBITS 16`, `#NBMANT 10`,
  `#NBEXPO 5`, and one without float needs only `#NUBITS 16`. YANC is
  maintained at 32 bits; narrower words work, wider ones are not supported.
- **`#NUGAIN` must be a power of two.** `norm(x)` is `x / NUGAIN` in
  hardware: a power of two is a shift, any other value would be a divider
  on the processor's critical path, so the compilers refuse it.
- **The stacks are sized from the program** when `#NDSTAC` and `#SDEPTH` are
  left out: exactly as deep as the program goes, with no word to spare,
  which is usually a few words. Give
  them only to reserve more. When the depth cannot be worked out (a C++
  function that calls itself back), the stack keeps 128 words and asmcomp
  says so with a warning: give the depth then, since two stacks of 128 words
  can be most of a small processor. In simulation the waveform shows, for
  each stack, the words it holds, `fl_top` while it is full (as a program
  is at its deepest point) and `fl_overflow` from the first push past full or
  pop of an empty stack, an overflow; the regression fails any run that
  raises `fl_overflow`.
- **The processor's name** is the source file's: there is no directive for
  it (`#PRNAME` is an error since v7.1).

## Markers

These two go inside a function, as statements, at most once each per
program; each adds a pin to the processor.

| directive | pin | does |
|---|---|---|
| `#PRACA` | input `itr` | while `itr` is 1 the processor jumps, every clock, to the point marked; when `itr` goes back to 0 it carries on from there |
| `#TOAQUI` | output `cheguei` | `cheguei` is 1 while the processor executes the point marked |

`#PRACA` is a restart point, not an interrupt routine: nothing is saved,
there is no return to where the program was, and holding `itr` at 1 keeps
the processor on that point. `#TOAQUI` lets a testbench or other hardware
see that the program has reached a place, for example the end of an
initialisation.

## Waveform

A variable the program writes and never reads is not in the processor: its
writes leave the program, it takes no data word, and it is not in the
waveform; the compiler warns of each one. `#WAVE` keeps such variables,
to see them in the waveform:

```c
float y, t, e;
#WAVE t, e          // never read: kept for the waveform
```

It names variables already declared, separated by commas: globals at the
top of the file, locals inside their function. It changes nothing for a
variable the program reads. A value the compiler takes from the accumulator,
without reading the variable back, does not count as a read: in
`t = a[j]; e = t - y;` nothing reads `t`. In C++ such a variable leaves
the program too, without a warning, and there is no `#WAVE` yet.

## Constants

`#define NAME text` replaces the name by the text from that line on:
`#define N 64`. It has no arguments; there is no `#include` and no `#if` in
C± (C++ has the whole preprocessor).
