# TODO

Open work items for YANC that are not tracked elsewhere. Each item says what
is wrong, why it matters, and what "done" looks like. Remove an item when it
lands (the history stays in git and in the CHANGELOG).

---

## 1. Round-to-nearest mode for the floating-point ALU

**Status:** open · **Area:** `HDL/ula.v` (+ a directive in `cmmcomp` / `cppcomp`)

### The problem

The SAPHO floating-point operators **truncate**, they never round:

| Operator | Where the bits are lost |
|---|---|
| `F_ADD` / `F_SU1` / `F_SU2` | `ula_denorm` right-shifts the mantissa of the smaller operand with no guard bits (`m_in >> shift`); `ula_fadd` then drops the LSB of the sum (`m[MAN:1]`) before `ula_norm` re-normalizes |
| `F_MLT` | `ula_fmlt` keeps only the top `MAN` bits of the `2*MAN`-bit product (`mult[2*MAN-1:MAN]`), before normalization |
| `F_DIV` | `ula_fdiv` keeps the integer quotient bits (`div[MAN-1:0]`), no remainder/sticky information |

All of them round **toward zero in magnitude**. For a single operation that is
at most ~1 ULP, but the error is **biased**: it always goes the same way. In
any long-running accumulation it therefore does not average out, it grows
**linearly** with the number of operations.

Concretely, a phase accumulator `a = a + step` kept in `[1, 2)` with the default
`#NBMANT 23` (22 fractional bits, ULP = 2^-22) loses on average about one ULP per
step. Run at 15.36 kHz as the time base of a 60 Hz resampler, that is a relative
rate error of 2^-22, i.e. an effective frequency error of 60 * 2^-22 = 14.3 uHz,
and the phase of the h-th harmonic drifts without bound (proportionally to h).

### Evidence from real projects

Two independent SAPHO applications of the group have already hit this:

- a CNN inference kernel whose residuals against the double-precision
  reference were **all of the same sign** (systematic bias traced to mantissa
  truncation and to the `I2F`/`F2I` conversions);
- a Farrow/B-spline resampler for harmonic phasor estimation whose phase error
  against the double-precision model **grows linearly with run time**, with
  an effective frequency error that matches the one-ULP-per-step figure above.

The simulator already *measures* this (the `delta_float` / `delta_int` ULA
rounding-error taps visible in GTKWave under Icarus), but there is no way to
*avoid* it in hardware.

### Proposal

- Add an **opt-in** rounding mode, **round to nearest, ties to even**, selected
  per processor by a new directive (working name `#FROUND 1`; default `0`).
  It becomes a `FROUND` parameter of `processor.v` / `core.v` / `ula.v`, so the
  default keeps today's area and stays **bit-identical** to the current goldens.
- `F_ADD` family: carry guard, round and sticky bits out of `ula_denorm`
  (sticky = OR of everything shifted past the round bit), add at the wider
  width, normalize, then round. Rounding must happen **after** normalization, so
  either the extra bits travel through `norm_mux` / `ula_norm`, or `ula_fadd`
  normalizes internally. Handle the mantissa carry-out of the rounding increment
  (shift right, exponent + 1).
- `F_MLT`: keep the full `2*MAN`-bit product, normalize first, then round with
  the discarded low bits as guard/round/sticky.
- `F_DIV`: compute one extra quotient bit and use `remainder != 0` as sticky.
- Cost: a few extra bits in the adder datapath, one `MAN`-bit incrementer and
  an OR-reduction per operator. The incrementer sits on the critical path, so
  check Fmax and consider the pipeline.

### Done when

- A new regression fixture accumulates `x = x + c` for N steps with
  `#FROUND 0` and `#FROUND 1` and compares with a double-precision reference:
  with `0` the error grows ~linearly in N (today's behaviour); with `1` its mean
  stays ~0 and its size grows like sqrt(N) ULP.
- The `delta_float` monitor shows a zero-mean error for `#FROUND 1`.
- Every existing golden is unchanged with the default `#FROUND 0`.
- README (and the C± directive list) documents the directive and its cost.

### Workarounds until then (worth a line in the README)

- Keep long-running accumulators (phase, time base, counters) in **integer /
  fixed point**: `ADD` is exact, and wrap-around modulo 2^NUBITS gives a free
  "mod 2*pi" for binary angles.
- Accumulate **deviations** from a nominal value instead of absolute values, so
  the accumulator stays small and its ULP fine.
- Use a compensated (Kahan) sum where a float accumulator is unavoidable.
- Raise `#NBMANT` / `#NUBITS` for the processor that accumulates.

---

## 2. `I2F` silently wraps integers outside +/-2^(NBMANT-1)

**Status:** open · **Area:** `HDL/ula.v` (`ula_i2f`), possibly a compiler/sim warning

`op_i2f` feeds `ula_i2f` with `in2[NBMANT-1:0]` only: the integer is taken as a
signed `NBMANT`-bit number (+/-2^22 with the default `#NBMANT 23`). A larger
`int` converts to a wrong float with no warning (e.g. `2^24` becomes `0`). Users
currently have to pre-shift (`x >> k`, then scale by `2^k` in float).

**Done when:** either `I2F` converts the full `NUBITS` range (normalizing the
integer instead of truncating its high bits), or out-of-range inputs saturate
and the simulator flags them, and the limitation is documented.
