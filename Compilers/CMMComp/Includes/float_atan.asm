
// Arctangent function --------------------------------------------------------
// |x| is folded into [0,1] with the 1/x identity: atan(|x|) = pi/2 - atan(1/|x|)
// for |x|>1. atan(t) on [0,1] is a degree-11 odd minimax polynomial (least-
// squares fit, max abs error ~4.9e-6 -- ~6 digits, vs the old 49-point LUT's
// ~4e-5). One shared polynomial for both branches; no table, no new hardware.

@__float_atan SET   __atan_x                 // save x
            F_ABS_M __atan_x               // ax = |x|
            SET   __atan_ax
            F_LES 1.0                    // (ax > 1)?  [F_LES true when acc > X]
            SET   __atan_big               // flag: 1 if |x|>1 (use 1/x)
            JIZ   __L_atan_small
            LOD   __atan_ax                // big branch: t = 1/ax
            F_DIV 1.0                    // 1.0 / ax   (F_DIV X = X/acc)
            JMP   __L_atan_haveT
@__L_atan_small LOD __atan_ax               // small branch: t = ax
@__L_atan_haveT SET __atan_t

            F_MLT __atan_t                 // w = t^2
            SET   __atan_w
            LOD  -0.012300666580         // Horner in w: atan(t)/t = a1 + a3 w + ... + a11 w^5
            F_MLT __atan_w                 // a11
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
            SET   __atan_p

            LOD   __atan_big
            JIZ   __L_atan_done            // |x|<=1 -> result = atan(t)
            LOD   __atan_p                 // |x|>1  -> result = pi/2 - atan(1/x)
            F_SU2 1.5707963268           // pi/2 - p   (F_SU2 X = X - acc)
            JMP   __L_atan_sign
@__L_atan_done LOD __atan_p
@__L_atan_sign F_SGN __atan_x                // apply the sign of x
            RET
