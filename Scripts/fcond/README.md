# Float as a condition (TODO item 33)

Does a float used as a condition (`if (x)`, `while (x)`, `!x`, `x && y`,
`x ? a : b`, `bool b = x`) give C's answer, true when the value is not zero?

- `gen.py` writes one C++ file, `fcond.cpp`, compiled both by the host g++
  (the reference: `host.h` turns `out()` into `printf`) and by cppcomp, and
  the same cases in C+- at `#FROUND` 0, 1 and 2 (no `?:` and no `bool` in
  C+-; C+- refuses a float in `&&` and `||`, so those are C++ only).
- `run.sh` builds and simulates them; `compare.py` prints the cases that
  differ from g++.

```sh
bash Scripts/fcond/run.sh                     # today's compilers and SAPHO/
BIN=<dir> HDL=<dir> bash Scripts/fcond/run.sh # a prototype
```

Every case prints 0 or 1 (the do-while 1 or 2), so the outputs compare line
by line whatever the float format; the loops stop one turn too late at most,
so a compiler that takes a zero for true prints one more instead of hanging.
Values below SAPHO's smallest normal are left out (they become zero at
`#FROUND 1/2`, a format difference).
