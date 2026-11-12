NOP
#PRNAME ArcTan
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main #arrays main_x 2 1000 "atan_x.txt"
#arrays main_a 2 1000 "atan_y.txt"
#SHARE main_e main_y
#SHARE arctan_LUT_x main_y
#SHARE arctan_LUT_signo main_t
#SHARE __atan_x main_y
#SHARE __atan_t main_t
#SHARE __atan_w arctan_LUT_v0
LOD 0
SET main_j
@__Lwh1top LOD main_j
LDI main_x
CAL __float_atan
SET main_y
LOD main_j
LDI main_a
SET main_t
F_SU1 main_y
SET main_e
INC main_j
LES 999
JIZ __Lwh1top
@__Lwh1end LOD 0
SET main_j
@__Lwh2top LOD main_j
LDI main_x
CAL arctan_LUT
SET main_y
LOD main_j
LDI main_a
SET main_t
F_SU1 main_y
SET main_e
INC main_j
LES 999
JIZ __Lwh2top
@__Lwh2end @fim JMP fim
@arctan_LUT SET arctan_LUT_x
#arrays arctan_LUT_atan_lut 2 49 "Arctan_LUT.txt"
SET arctan_LUT_signo
F_ABS_M arctan_LUT_x
SET arctan_LUT_x
LOD 0.0
SET arctan_LUT_v0
LOD arctan_LUT_x
F_LES 1.0
JIZ __Lif1else
LOD arctan_LUT_x
F_DIV 1.0
NOP
NOP
F_QUO
SET arctan_LUT_x
LOD 1.57079632679
SET arctan_LUT_v0
@__Lif1else LOD 47.0
F_MLT arctan_LUT_x
SET arctan_LUT_idxf
F2I_M arctan_LUT_idxf
SET arctan_LUT_idx
LDI arctan_LUT_atan_lut
SET arctan_LUT_x
LOD arctan_LUT_idx
ADD 1
LDI arctan_LUT_atan_lut
F_SU1 arctan_LUT_x
P_I2F_M arctan_LUT_idx
F_SU2 arctan_LUT_idxf
SF_MLT
F_ADD arctan_LUT_x
F_SU2 arctan_LUT_v0
F_SGN arctan_LUT_signo
RET

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
NOP
NOP
F_QUO
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
