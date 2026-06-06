
// Exponential function -------------------------------------------------------
// exp(x) = 2^n * exp(r),  r = x - n*ln2 in [0, ln2).  Range reduction is done
// by value (a loop, like float_sin) so it is independent of the float layout;
// exp(r) is a degree-8 Taylor polynomial evaluated with Horner (no LUT).
// NB: F_LES X is true when (acc > X) -- it tests "operand < accumulator".

@float_exp  SET   exp_x                 // save x
            LOD   1.0
            SET   exp_2n                // scale factor 2^n, starts at 1

@L_exp_a    LOD   exp_x                 // while x > ln2: x -= ln2; scale *= 2
            F_LES 0.6931471806          // (acc > ln2) -> true while x > ln2
            JIZ   L_exp_b               // x <= ln2 -> stop reducing down
            LOD   exp_x
            F_SU1 0.6931471806          // x = x - ln2   (F_SU1 X = acc - X)
            SET   exp_x
            LOD   exp_2n
            F_MLT 2.0
            SET   exp_2n
            JMP   L_exp_a

@L_exp_b    LOD   0.0                   // while x < 0: x += ln2; scale *= 0.5
            F_LES exp_x                 // (0 > x) -> true while x < 0
            JIZ   L_exp_c               // x >= 0 -> reduced to [0, ln2)
            LOD   exp_x
            F_ADD 0.6931471806
            SET   exp_x
            LOD   exp_2n
            F_MLT 0.5
            SET   exp_2n
            JMP   L_exp_b

@L_exp_c    LOD   0.0000248016          // Horner, exp(r) = sum r^k / k! (k=0..8)
            F_MLT exp_x                 // 1/8!
            F_ADD 0.0001984127          // 1/7!
            F_MLT exp_x
            F_ADD 0.0013888889          // 1/6!
            F_MLT exp_x
            F_ADD 0.0083333333          // 1/5!
            F_MLT exp_x
            F_ADD 0.0416666667          // 1/4!
            F_MLT exp_x
            F_ADD 0.1666666667          // 1/3!
            F_MLT exp_x
            F_ADD 0.5                    // 1/2!
            F_MLT exp_x
            F_ADD 1.0                    // 1/1!
            F_MLT exp_x
            F_ADD 1.0                    // 1/0!  -> exp(r)
            F_MLT exp_2n                 // * 2^n  -> exp(x)
            RET
