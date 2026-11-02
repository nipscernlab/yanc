
// Arctangent function --------------------------------------------------------
// |x| is folded into [0,1] with the 1/x identity: atan(|x|) = pi/2 - atan(1/|x|)
// for |x|>1. atan(t) on [0,1] is a degree-11 odd minimax polynomial (least-
// squares fit, max abs error ~4.9e-6 -- ~6 digits, vs the old 49-point LUT's
// ~4e-5). Each branch has its own copy of the polynomial, so neither stores a
// flag to test again after it: fewer cycles, a few more words. No table, no
// new hardware.

@__float_atan SET   __atan_x                 // save x
            F_ABS                        // t = |x|
            SET   __atan_t
            F_LES 1.0                    // (|x| > 1)?  [F_LES true when acc > X]
            JIZ   __L_atan_small

            LOD   __atan_t                 // |x| > 1: t = 1/|x|
            F_DIV 1.0                    // 1.0 / |x|   (F_DIV X = X/acc)
            SET   __atan_t
            F_MLT __atan_t                 // w = t^2
            SET   __atan_w
            F_MLT -0.012300666580           // Horner in w: atan(t)/t = a1 + a3 w + ... + a11 w^5
            F_ADD 0.054084934779         // a9
            F_MLT __atan_w
            F_ADD -0.117697073484        // a7
            F_MLT __atan_w
            F_ADD 0.194020561451         // a5
            F_MLT __atan_w
            F_ADD -0.332694520616        // a3
            F_MLT __atan_w
            F_ADD 0.999980069822         // a1
            F_MLT __atan_t                 // * t  -> atan(t)
            F_SU2 1.5707963268           // pi/2 - atan(1/|x|)   (F_SU2 X = X - acc)
            F_SGN __atan_x                 // the sign of x
            RET

@__L_atan_small LOD __atan_t               // |x| <= 1: t = |x|
            F_MLT __atan_t                 // w = t^2
            SET   __atan_w
            F_MLT -0.012300666580           // the same polynomial
            F_ADD 0.054084934779
            F_MLT __atan_w
            F_ADD -0.117697073484
            F_MLT __atan_w
            F_ADD 0.194020561451
            F_MLT __atan_w
            F_ADD -0.332694520616
            F_MLT __atan_w
            F_ADD 0.999980069822
            F_MLT __atan_t                 // atan(|x|)
            F_SGN __atan_x                 // the sign of x
            RET
