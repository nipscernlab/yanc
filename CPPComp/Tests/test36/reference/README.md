# FISTA reference material (for ../test36.cpp)

`Tests/test36/test36.cpp` is the YANC port of a hosted FISTA blind sparse
deconvolution program. This folder keeps the original and the tooling used to
derive the test's input and golden, so the port stays verifiable.

- **`fista_ref.cpp`** — the ORIGINAL program, verbatim (golden reference, do not
  modify). It reads `N` then `N` float32 **IEEE bit-patterns** (as signed int32)
  from a file via `fbits()`/`std::memcpy`, runs FISTA, and prints `h_hat`. Build
  with g++ and run on `input40.txt`.
- **`input40.txt`** — the original input vector (IEEE bit-patterns), N=40.
- **`gen_scaled.c`** — decodes `input40.txt` and re-emits it as integers scaled
  by a gain (`y * 30000`, kept under 2^22 so the YANC `I2F` does not wrap). Its
  output is `Tests/test36/test36.in`, the vector the YANC port consumes.
- **`fista_scaled.cpp`** — a gcc reference whose I/O matches the port exactly
  (reads the scaled integers, reconstructs `y = i / GAIN_IN`, prints
  `round(h * GAIN_OUT)`). Run on `test36.in` to get the values the YANC output is
  compared against. Algorithm body identical to `fista_ref.cpp`.

The YANC target has no filesystem and no IEEE-754 floats (only the mantissa/
exponent SIZES match IEEE), so the port cannot consume IEEE bit-patterns; the
scaled-integer path is the platform adaptation. The port's output matches the gcc
reference to within float rounding (e.g. `9257` vs `9258`).

Reproduce (from this folder):
```
g++ -O2 -o fista_ref.exe    fista_ref.cpp    && ./fista_ref.exe input40.txt
gcc -O2 -o gen_scaled.exe   gen_scaled.c -lm && ./gen_scaled.exe input40.txt ../test36.in
g++ -O2 -o fista_scaled.exe fista_scaled.cpp && ./fista_scaled.exe ../test36.in
```
