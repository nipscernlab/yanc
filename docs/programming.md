# Writing programs for SAPHO

An example in C±, the float rounding levels, and programs that run on several
processors. The language itself is described in the SAPHO manual
(<https://nipscern.com/library/sapho>); the examples in
`Compilers/CMMComp/Tests/` and `Compilers/CPPComp/Tests/` show every construct
the compilers accept.

## An example in C±

A hand-written square root against the built-in one. The program is the
`Sqrt` test (`Compilers/CMMComp/Tests/Sqrt/`), which also holds the two data
files: 1000 inputs and their exact square roots.

```c
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128

// square root by Newton-Raphson, from a first guess built on the exponent
float my_sqrt(float num)
{
    if (num == 0.0) return 0.0;

    int v = (((num << 1) >>> 24) + 22) >>> 1;         // get the exponent
        v = ((((v-22) << 23) + (1 << 22)) << 1) >> 1; // build the float

    float x; copy(v,x);                               // the same bits, as a float

    x = 0.5 * (x + num/x);   // 4 Newton-Raphson iterations
    x = 0.5 * (x + num/x);
    x = 0.5 * (x + num/x);
    x = 0.5 * (x + num/x);

    return x;
}

void main()
{
    float x[1000] "sqrt_x.txt";   // the inputs, loaded into data memory
    float a[1000] "sqrt_y.txt";   // their exact square roots
    float y, t, e;

    int j = 0;
    while (j < 1000)              // the built-in sqrt (macro in Includes/float_sqrt.asm)
    {
        y = sqrt(x[j]);
        t = a[j];
        e = t - y;                // its error
        j++;
    }

    j = 0;
    while (j < 1000)              // the hand-written one
    {
        y = my_sqrt(x[j]);
        t = a[j];
        e = t - y;                // its error
        j++;
    }
}
```

Nothing is written to an output port: the result is read in the waveform,
where `e` shows the error of each version, sample by sample, next to the
source line being executed. The header sets the processor: word width
(`#NUBITS`), float format (`#NBMANT`, `#NBEXPO`), depth of the data stack
(`#NDSTAC`) and of the instruction stack (`#SDEPTH`), number of input and
output ports (`#NUIOIN`, `#NUIOOU`), and the divisor of `norm()`
(`#NUGAIN`, a power of two).

More examples in `Compilers/CMMComp/Tests/` and `Compilers/CPPComp/Tests/`.

## Float rounding level (`#FROUND`)

The float datapath of the ALU comes in three levels, selected per processor by
the `#FROUND` directive (default `0`). Each level adds logic only to the float
operators the program actually uses; the costs below are LUT4 counts measured
with Yosys on a 32/23/8 processor that uses `F_ADD`, `F_MLT`, `I2F`, `F2I`,
comparisons and the integer ALU (no `F_DIV`).

| `#FROUND` | What the ALU does | Cost |
|---|---|---|
| `0` | Legacy datapath: operators truncate, one mantissa bit is lost before normalization whenever the result does not fill the top bit (so `(1+2^-22) - 1 == 0` and `x * 1.0` may differ from `x`), the exponent wraps on overflow/underflow, `-0.0 != 0.0`. Bit-identical to previous releases. | — |
| `1` | Keeps that bit (exact truncation toward zero), saturates on overflow, flushes to zero on underflow, canonical `+0.0`. `x * 1.0 == x`, `a - b` is exact for `a/2 <= b <= 2a`. `I2F` (and `fin()`) converts the whole `NUBITS` word instead of its low `NBMANT` bits — at level 0 an `int` beyond ±2^(NBMANT-1) converts to garbage — and `F2I` saturates to the `int` range instead of wrapping. | ≈ +1.5 % |
| `2` | Level 1 plus round to nearest, ties to even (guard/round/sticky bits through the adder, the multiplier, the divider and `I2F`). Rounding errors on varying data are unbiased. | ≈ +5 % (≈ +20 % of the float part) |

With `F_DIV` also instantiated the whole ALU (19 integer + 12 float operators)
goes from 8487 LUT4 to 8271 (level 1, within synthesis noise) and 8691 (level
2, +2.4 %); the divider itself grows the most at level 2 (2682 → 3162, three
extra quotient bits). These counts predate v6.0, which cut each divider by
two registers: a division is now the three-word sequence `<div> x; NOP;
<read>` ([`pipeln-and-division.md`](pipeln-and-division.md)).

`cppcomp` always emits `#FROUND 2`. The fixtures `Compilers/CMMComp/Tests/cmm_fround0`,
`cmm_fround1` and `cmm_fround2` run the same program at each level; their
1000-step accumulation of `0.001` ends 240 / 177 / 78 ULP away from `1.0`. Even
at level 2 a **constant** addend drifts (the same bits are discarded every
step, so the rounding is correlated) — keep phase/time accumulators in integer
arithmetic regardless of the level.

## Several processors (`toma` / `cade`)

SAPHOs pass values to each other over links, with wires only between them.
`toma(k, x);` hands `x` over the processor's link `k`; `y = cade(k);` (int)
or `fcade(k)` (float) on the partner takes it; whoever arrives first waits.
A processor can have any number of links, so chains, fan-out and fan-in are
written directly, and a link works across clocks (the `_sync` forms,
`toma_sync` / `cade_sync` / `fcade_sync`, declare the partner on the same
clock and save 4 cycles a value). Both processors of a link need the same
`#NUBITS`, and the same float format to pass floats. The pins, the wiring and
the two-clock constraint: [`toma-and-cade-links.md`](toma-and-cade-links.md).
