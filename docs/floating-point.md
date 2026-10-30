# Floating point

A SAPHO `float` is a word of `#NUBITS` bits split into a sign, an exponent
of `#NBEXPO` bits and a mantissa of `#NBMANT` bits, 1 + 8 + 23 by default.
The sizes are those of IEEE 754 single precision; **the encoding is not**.
A YANC float cannot be read or written bit for bit as an IEEE float (C++
`std::bit_cast` of an IEEE pattern gives a wrong value).

## The format

```text
[ sign: 1 ][ exponent: NBEXPO ][ mantissa: NBMANT ]      value = ±m · 2^e
```

| | SAPHO | IEEE 754 |
|---|---|---|
| mantissa `m` | an integer of `NBMANT` bits, its top bit written out (normalised to `2^(NBMANT-1) <= m < 2^NBMANT`) | a fraction with a hidden leading 1 |
| exponent `e` | two's complement, no bias | biased |
| sign | sign and magnitude | sign and magnitude |
| zero | the most negative exponent with `m = 0` | all zeros |
| infinities, NaN, subnormals | none | yes |

With the leading bit written out, the default 23-bit mantissa carries 22
bits after the point, one fewer than IEEE single. With the defaults the
magnitudes run from about `1.2e-32` (`2^-106`) to about `1.4e45` (`2^150`),
with about 7 significant digits (deduced from the format above).

`#NUBITS = #NBMANT + #NBEXPO + 1` and `#NBMANT < 2^(#NBEXPO-1)`, checked
when the program uses float; a smaller format is a smaller and faster
float unit, at the cost of precision (see [Directives](directives.md)).

## Constants

A float constant in the program (`0.1`) becomes the nearest value the
processor's format can hold. The compiler says so for each constant that is
not held exactly (`constant 0.1 on line 1 approximated to 0.099999994039535522`),
and the assembler reports the largest error among the values of an array
file. Measured: with `#FROUND 0`, an input of 5 000 000 read with `fin()`
comes out of `fout()` as -3 388 608; with `#FROUND 1`, as 5 000 000. Constants may be written with an exponent
(`1e-5`).

## How operations round: `#FROUND`

The float unit comes in three levels, chosen per processor. Each level adds
logic only to the float operations the program uses.

| `#FROUND` | what the float unit does | cost |
|---|---|---|
| `0` (C± default) | the original datapath: the operations truncate, and one mantissa bit is lost before normalisation when a result does not fill the top bit, so `(1 + 2^-22) - 1` gives `0` and `x * 1.0` may differ from `x`; the exponent wraps on overflow and underflow; `-0.0 != 0.0` | none |
| `1` | keeps that bit; saturates on overflow and flushes to zero on underflow; one zero. `x * 1.0 == x`. A sum truncates toward zero; a difference cuts the smaller operand first, so it can come out a few units of the last bit above the exact result, and `a - b` is not always exact even when `a/2 <= b <= 2a`. In long sums of mixed signs the two errors cancel: the result is close to level 2's ([measured](design/fround1-subtraction.md)) | about +1.5 % of the ALU |
| `2` (C++ always) | level 1 with three more bits below the mantissa, rounding to the nearest, ties to even, in addition, multiplication, division and conversion: rounding errors on varying data have no bias, and `a - b` is exact when `a/2 <= b <= 2a` | about +5 % of the ALU |

**At level 0, converting an int to float uses only its low `NBMANT` bits:**
`fin()` or an `int` assigned to a `float` gives a wrong value once the int
is beyond ±2^(NBMANT-1) (±4 194 304 by default), and converting a float to
int wraps instead of saturating. Level 1 and above convert the whole word
and saturate. A program that converts large integers needs `#FROUND 1`.

**Dividing a float by zero** gives, at levels 1 and 2, the largest float
with the sign of the quotient (`1.0 / 0.0` and `0.0 / 0.0` alike); at level
0 the result is not defined. An int divided by zero is not defined at any
level.

Even at level 2, adding the same constant again and again drifts: the same
bits are dropped at every step, so the errors do not cancel. Measured, 1000
additions of `0.001` end 240, 177 and 78 units of the last place away from
1.0 at levels 0, 1 and 2. Keep accumulators of phase or time in integer
arithmetic, and accumulate deviations from a nominal value rather than the
value itself. A processor that accumulates can also take a wider mantissa:
32/25/6 keeps two more bits than 32/23/8. A compensated (Kahan) sum does
not help at level 0, because it relies on `t - sum` being exact, which
loses a bit there; from level 1 on it works.

The costs are LUT counts measured with Yosys on a 32-bit processor using
float addition, multiplication, conversions and comparisons, before v6.0
(see `docs/design/precision-and-width-review.md`).

## Looking at a float's bits

`copy(x, v)` copies the bits of `x` into the variable `v` with no
conversion, so a program can read a float's exponent and mantissa as an
int, or build a float from bits: the square-root example of
[A first program](first-program.md) builds its first guess that way. In C++,
`std::bit_cast` does the same between YANC types.

## Seeing the rounding in the waveform

The processor's simulation exposes `delta_float` (and `delta_int`): the
rounding error of each ALU operation against the exact value. Added to the
waveform, it shows the error each operation commits and how it accumulates
along a loop.
