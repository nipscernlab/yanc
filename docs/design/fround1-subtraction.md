# `#FROUND 1`: the subtraction, and why it stays as it is

Found on 2026-10-22 while writing the rounding references of the ALU
testbench (`Scripts/hw/tb_ula.v`, step 2 of writing it). Decided the same day by Luciano: the
hardware stays, the manual changes (option (b) below).

## What the manual said

`docs/floating-point.md` described level 1 as "keeps that bit (exact
truncation toward zero) ... `a - b` is exact when `a/2 <= b <= 2a`". The
second claim is the Sterbenz lemma, and both hold only if the adder keeps
the bits of the smaller operand that the alignment shifts out.

## What the hardware does

`ula_denorm` aligns the smaller operand by shifting it right. At level 2 the
bits that fall off go into G = 3 extra bits (guard, round, sticky); at
levels 0 and 1, G = 0 and they are dropped. A sum is then still the exact
truncation, but a difference computes `big - floor(small)`, which is above
the exact result, and when the difference cancels leading bits the
normaliser shifts that error up.

The case that showed it, at 32/23/8: `1.0 - 0x7FFFFF * 2^-24`
(`1.0 - 0.49999994039535522`). Exact: 0.50000005960464477539, halfway between
0.5 and 0.5 + 2^-23.

| `#FROUND` | result | the level's rule gives |
|---|---|---|
| 0 | 0.5 | 0.5 |
| 1 | 0.50000023841857910156 (mantissa `0x400002`) | 0.5 (truncation) |
| 2 | 0.5 | 0.5 (nearest, ties to even) |

## The fix that was tried

Level 1 on level 2's datapath: G = 3 at levels 1 and 2, and the normaliser
drops the extra bits at level 1 instead of rounding them. With guard, round
and sticky, the truncation toward zero is exact. The whole change, in
`SAPHO/ula.v` (never committed; it would change `cmm_fround1`'s goldens):

```diff
@@ module ula_norm (the ranged branch)
-	// round to nearest even on the extra bits (nothing to do when G = 0)
+	// round to nearest even on the extra bits at level 2; level 1 drops them
 	wire        [MAN-1:0] mnt = nrm[W-1:G];
 	wire                  inc;
-	if (G != 0) begin : rne
+	if (G != 0 && FROUND == 2) begin : rne
@@ module ula
-localparam G = (FROUND == 2) ? 3 : 0;
+localparam G = (FROUND != 0) ? 3 : 0;
```

With it, the case above gives 0.5, and `Scripts/hw/tb_ula.sh` (the 42
operations that do not round) and `Scripts/hw/elab.sh` pass as before.

### Its cost

Yosys (`NOSHARE=1 Scripts/hw/area.sh 1`, 32/23/8, LUT4 / logic depth):

| operators | level 1 today | level 1 with the fix |
|---|---|---|
| `F_ADD`, `F_SU1`, `F_SU2` | 666 / 36 | 753 / 39 |
| `F_MLT` | 1,652 / 34 | 1,674 / 37 |
| every float operator but the division | 2,755 / 40 | 2,899 / 44 |
| int and float, no division | 4,248 / 39 | 3,971 / 43 (abc noise) |
| the whole ALU | 7,548 / 177 | 7,752 / 177 |

Quartus 24.1, `sapho_all` forced to level 1 (`FR=1 Scripts/hw/fmax.sh`),
DE10-Nano (`5CSEBA6U23I7`), seed 1 only:

| | Fmax | ALMs | worst path |
|---|---|---|---|
| today | 35.16 MHz | 3,389 | float add: data memory, denormaliser, adder, normaliser, ALU mux, stack (25.8 ns) |
| with the fix | 35.55 MHz | 3,527 (+4.1 %) | the first part of `F_DIV` (25.6 ns) |

The clock does not move (one seed; seeds spread up to 4.5 %); the logic
grows about 4 %.

### What it changes in the results

`Scripts/hw/fadd_acc/run.sh` (32/23/8; values 1 to 1024, random signs; a
unit is the last bit of the mantissa, about 1.2e-7 of the value).

200,000 random pairs:

| | not the exact truncation | by how much |
|---|---|---|
| level 1 today | 37 %: 3 subtractions in 4, no sum | 1 unit mostly; 2 units in 5,100 pairs, up to 128 units in a few |
| level 1 with the fix | 0 % | |

Of the 18,965 pairs with `b` between `a/2` and `2a` (the documented exact
subtraction), level 1 today gets 4,537 wrong (24 %), with a relative error
up to 1.85e-5; the fix and level 2 get all of them exact; level 0 gets
11,799 wrong, up to 4.1e-4.

300 running sums of 1000 values of mixed signs, error of the final result in
units of its last place:

| | mean | mean of the absolute value | largest |
|---|---|---|---|
| level 0 | -164 | 686 | 42,535 |
| **level 1 today** | +10 | **30** | 1,992 |
| **level 1 with the fix** | -197 | **700** | 49,336 |
| level 2 | +8 | 21 | 2,118 |

The fix makes every single operation right and the long sums about twenty
times worse, as bad as level 0. Exact truncation always pulls toward zero,
and that bias adds up. Level 1 today truncates sums toward zero and rounds
differences away from zero; the two errors cancel, and a long sum comes out
close to level 2's.

## Decision

Luciano, 2026-10-22: option (b). The hardware stays; the manual describes
level 1 as it is: good in long sums, close to level 2, without the exact
subtraction and with a single operation off by a few units. Whoever needs
the exact subtraction uses level 2. The ALU testbench (`Scripts/hw/tb_ula.v`)
checks level 1 against that description, not against the exact
truncation.

The options that were weighed:

- (a) the fix above: every operation right, long sums twenty times worse, +4 %
  of logic, same clock;
- (b) taken: no hardware change, the manual corrected;
- (c) drop level 1 and send its users to level 2, which has both properties
  for about +5 % of the ALU.

## For the documentation (item 21)

The `#FROUND` table of `docs/floating-point.md` was corrected the same day:
level 1 no longer promises the exact truncation nor the exact subtraction,
and says what it is good for (long sums).
