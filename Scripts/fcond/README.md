# Float as a condition (TODO item 33)

Does a float used as a condition (`if (x)`, `while (x)`, `!x`, `x && y`,
`x ? a : b`, `bool b = x`) give C's answer, true when the value is not zero?

- `gen.py` writes one C++ file, `fcond.cpp`, compiled both by the host g++
  (the reference: `host.h` turns `out()` into `printf`) and by cppcomp, and
  the same cases in C+- at `#FROUND` 0, 1 and 2 (no `?:` and no `bool` in
  C+-). Until TODO 33 is done, today's cmmcomp refuses a float in `&&` and
  `||`, so its programs do not build ("BUILD FAILED"; run.sh goes on).
- Every program runs at the three float formats the fixtures use, 32/23/8,
  16/10/5 and 23/16/6 (`FORMATS` in gen.py; `#pragma yanc` for C++, the
  directives for C+-): the widths are a per-program choice, so every fix
  must hold at each of them.
- Complex conditions (C+- only: C++'s `std::complex` has no `bool`
  conversion): `fcomp.c`, C99 on the host gcc, is the reference (true when
  the real or the imaginary part is not zero), against `fcomp_<f>_l<n>.cmm`.
- `run.sh` builds and simulates them; `compare.py` prints the cases that
  differ from g++ (`SHORT=1`: one line per run).

```sh
bash Scripts/fcond/run.sh                     # today's compilers and SAPHO/
BIN=<dir> HDL=<dir> bash Scripts/fcond/run.sh # a prototype
```

Every case prints 0 or 1 (the do-while 1 or 2), so the outputs compare line
by line whatever the float format; the loops stop one turn too late at most,
so a compiler that takes a zero for true prints one more instead of hanging.
compare.py leaves out, per format, the nonzero values below that format's
smallest normal (2^(NBMANT-1) times 2 to the lowest exponent: 0.0156 at
16/10/5): they become zero there, a format difference, not the compiler's.
