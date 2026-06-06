
// Natural logarithm ----------------------------------------------------------
// log(x) = e*ln2 + log(m),  x = m*2^e with m in [1, 2).  Range reduction is
// done by value (a loop, like float_sin); log(m) uses the atanh series
// log(m) = 2u(1 + u^2/3 + u^4/5 + u^6/7 + u^8/9), u = (m-1)/(m+1), evaluated
// with Horner (no LUT). x <= 0 returns 0 (guards the loop against running away).
// NB: F_LES X is true when (acc > X) -- it tests "operand < accumulator".

@float_log  SET   log_x                 // save x
            LOD   log_x
            F_LES 0.0                    // (x > 0) ?
            JIZ   L_log_zero             // x <= 0 -> bail out with 0
            LOD   0.0
            SET   log_acc                // accumulates e*ln2, starts at 0

@L_log_a    LOD   log_x                  // while x > 2: x *= 0.5; acc += ln2
            F_LES 2.0                    // (acc > 2) -> true while x > 2
            JIZ   L_log_b                // x <= 2 -> stop halving
            LOD   log_x
            F_MLT 0.5
            SET   log_x
            LOD   log_acc
            F_ADD 0.6931471806
            SET   log_acc
            JMP   L_log_a

@L_log_b    LOD   1.0                    // while x < 1: x *= 2; acc -= ln2
            F_LES log_x                  // (1 > x) -> true while x < 1
            JIZ   L_log_c                // x >= 1 -> reduced to [1, 2]
            LOD   log_x
            F_MLT 2.0
            SET   log_x
            LOD   log_acc
            F_SU1 0.6931471806           // acc = acc - ln2
            SET   log_acc
            JMP   L_log_b

@L_log_c    LOD   log_x                  // u = (m-1)/(m+1)
            F_ADD 1.0
            SET   log_t2                 // m+1
            LOD   log_x
            F_SU1 1.0                     // m-1   (acc - X)
            SET   log_t1
            LOD   log_t2
            F_DIV log_t1                  // F_DIV X = X/acc = (m-1)/(m+1) = u
            SET   log_u
            F_MLT log_u
            SET   log_w                   // w = u^2

            LOD   0.1111111111            // Horner in w (1/9, 1/7, 1/5, 1/3, 1)
            F_MLT log_w
            F_ADD 0.1428571429
            F_MLT log_w
            F_ADD 0.2
            F_MLT log_w
            F_ADD 0.3333333333
            F_MLT log_w
            F_ADD 1.0
            F_MLT log_u                   // * u
            F_MLT 2.0                      // * 2  -> log(m)
            F_ADD log_acc                  // + e*ln2  -> log(x)
            RET

@L_log_zero LOD   0.0
            RET
