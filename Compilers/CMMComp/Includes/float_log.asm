
// Natural logarithm ----------------------------------------------------------
// log(x) = e*ln2 + log(m),  e = XPO(x) = floor(log2 x),  m = x*2^-e in [1,2).
// O(1): e via XPO, m via a single F_SCL (no halve/double loop). log(m) is the
// atanh series 2u(1 + u^2/3 + u^4/5 + u^6/7 + u^8/9), u = (m-1)/(m+1); its
// factor 2 is in the coefficients (2/9 ... 2/3, 2): a power of two only moves
// the exponent, so each step is exactly twice the one of 2u(...) and the
// result is the same word.
// x <= 0 returns 0.

@__float_log  SET   __log_x                 // save x (acc still = x)
            F_LES 0.0                    // (x > 0) ?   [F_LES true when acc > X]
            JIZ   __L_log_zero             // x <= 0 -> bail out with 0

            XPO_M __log_x                  // e = floor(log2 x)   (int)
            SET   __log_e
            NEG                          // -e
            SET   __log_ne
            LOD   __log_x
            F_SCL __log_ne                 // m = x * 2^(-e)  -> [1,2)
            SET   __log_m

            F_SU1 1.0                    // m-1   (acc - X)
            SET   __log_t1
            LOD   __log_m
            F_ADD 1.0                    // m+1
            F_DIV __log_t1                 // u = (m-1)/(m+1)   [F_DIV X = X/acc]
            SET   __log_u
            F_MLT __log_u
            SET   __log_w                  // w = u^2
            F_MLT 0.2222222222           // Horner in w (2/9, 2/7, 2/5, 2/3, 2): w * 2/9
            F_ADD 0.2857142858
            F_MLT __log_w
            F_ADD 0.4
            F_MLT __log_w
            F_ADD 0.6666666666
            F_MLT __log_w
            F_ADD 2.0
            F_MLT __log_u                   // log(m)
            SET   __log_lm

            I2F_M __log_e                   // float(e)
            F_MLT 0.6931471806            // e * ln2
            F_ADD __log_lm                  // + log(m)  -> log(x)
            RET

@__L_log_zero LOD   0.0
            RET
