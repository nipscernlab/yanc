
// Tangent function -----------------------------------------------------------
// tan has period pi, so x is reduced to r = x - round(x/pi)*pi in [-pi/2, pi/2].
// Since +-pi/2 are poles, r is folded into [0, pi/4] with the cotangent identity
// tan(r) = 1/tan(pi/2 - |r|) for |r| > pi/4, and one degree-11 odd minimax
// polynomial (least-squares fit of tan(u)/u on [0, pi/4], max abs error
// ~6.5e-7) serves both branches; each branch has its own copy of it, so
// neither stores a flag to test again after it (fewer cycles, a few more
// words). tan is odd, so the sign of r is applied at the end with F_SGN. No
// lookup table, no new hardware.
// NB: F2I truncates toward zero, so round-to-nearest uses a sign-half bias.

@__float_tan  SET   __tan_x                  // save x
            F_MLT 0.3183098862           // q = x / pi   (1/pi)
            SET   __tan_q
            LOD   0.5
            F_SGN __tan_q                  // copysign(0.5, q)
            F_ADD __tan_q                  // q + copysign(0.5,q)
            F2I                          // k = round(q)
            I2F                          // float(k)
            F_MLT 3.1415926536           // k * pi
            F_SU2 __tan_x                  // r = x - k*pi  -> [-pi/2, pi/2]  (F_SU2 X = X - acc)
            SET   __tan_r
            F_ABS                        // a = |r|
            SET   __tan_u
            F_LES 0.785398163            // a > pi/4 ?  [F_LES true when acc > X]
            JIZ   __L_tan_inner

            LOD   __tan_u                  // a > pi/4: u = pi/2 - a   (u in [0,pi/4))
            F_SU2 1.5707963268           // pi/2 - a   (F_SU2 X = X - acc)
            SET   __tan_u
            F_MLT __tan_u                  // w = u^2
            SET   __tan_w
            F_MLT 0.019691612742           // Horner in w: tan(u)/u = a1 + a3 w + ... + a11 w^5
            F_ADD 0.013512594373         // a9
            F_MLT __tan_w
            F_ADD 0.056704928952         // a7
            F_MLT __tan_w
            F_ADD 0.132942907215         // a5
            F_MLT __tan_w
            F_ADD 0.333353228230         // a3
            F_MLT __tan_w
            F_ADD 0.999999838962         // a1
            F_MLT __tan_u                  // * u  -> tan(u) = P  (>= 0)
            F_DIV 1.0                    // 1.0 / P: the cotangent   (F_DIV X = X/acc)
            F_SGN __tan_r                  // tan is odd -> the sign of r
            RET

@__L_tan_inner LOD  __tan_u                  // a <= pi/4: u = a
            F_MLT __tan_u                  // w = u^2
            SET   __tan_w
            F_MLT 0.019691612742           // the same polynomial
            F_ADD 0.013512594373
            F_MLT __tan_w
            F_ADD 0.056704928952
            F_MLT __tan_w
            F_ADD 0.132942907215
            F_MLT __tan_w
            F_ADD 0.333353228230
            F_MLT __tan_w
            F_ADD 0.999999838962
            F_MLT __tan_u                  // tan(u) = P
            F_SGN __tan_r                  // the sign of r
            RET
