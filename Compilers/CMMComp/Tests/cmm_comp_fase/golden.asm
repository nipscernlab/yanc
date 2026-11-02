NOP
#PRNAME cmm_comp_fase
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE __fase_t main_c
#SHARE __atan_x main_c
@main @__Lwh1 LOD 1.0
SET main_c
LOD 2.0
SET main_c_i
LOD 0.0
F_LES main_c
JIZ __Lfa1a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa1b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa1z
@__Lfa1b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa1z
@__Lfa1a LOD main_c
F_LES 0.0
JIZ __Lfa1c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa1z
@__Lfa1c LOD main_c_i
F_LES 0.0
JIZ __Lfa1d
LOD 1.57079632679
JMP __Lfa1z
@__Lfa1d LOD 0.0
F_LES main_c_i
JIZ __Lfa1e
LOD -1.57079632679
JMP __Lfa1z
@__Lfa1e LOD 0.0
@__Lfa1z F_MLT 1000.0
F2I
OUT 0
LOD -1.0
SET main_c
LOD 2.0
SET main_c_i
LOD 0.0
F_LES main_c
JIZ __Lfa2a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa2b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa2z
@__Lfa2b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa2z
@__Lfa2a LOD main_c
F_LES 0.0
JIZ __Lfa2c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa2z
@__Lfa2c LOD main_c_i
F_LES 0.0
JIZ __Lfa2d
LOD 1.57079632679
JMP __Lfa2z
@__Lfa2d LOD 0.0
F_LES main_c_i
JIZ __Lfa2e
LOD -1.57079632679
JMP __Lfa2z
@__Lfa2e LOD 0.0
@__Lfa2z F_MLT 1000.0
F2I
OUT 0
LOD -1.0
SET main_c
LOD -2.0
SET main_c_i
LOD 0.0
F_LES main_c
JIZ __Lfa3a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa3b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa3z
@__Lfa3b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa3z
@__Lfa3a LOD main_c
F_LES 0.0
JIZ __Lfa3c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa3z
@__Lfa3c LOD main_c_i
F_LES 0.0
JIZ __Lfa3d
LOD 1.57079632679
JMP __Lfa3z
@__Lfa3d LOD 0.0
F_LES main_c_i
JIZ __Lfa3e
LOD -1.57079632679
JMP __Lfa3z
@__Lfa3e LOD 0.0
@__Lfa3z F_MLT 1000.0
F2I
OUT 0
LOD 1.0
SET main_c
LOD -2.0
SET main_c_i
LOD 0.0
F_LES main_c
JIZ __Lfa4a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa4b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa4z
@__Lfa4b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa4z
@__Lfa4a LOD main_c
F_LES 0.0
JIZ __Lfa4c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa4z
@__Lfa4c LOD main_c_i
F_LES 0.0
JIZ __Lfa4d
LOD 1.57079632679
JMP __Lfa4z
@__Lfa4d LOD 0.0
F_LES main_c_i
JIZ __Lfa4e
LOD -1.57079632679
JMP __Lfa4z
@__Lfa4e LOD 0.0
@__Lfa4z F_MLT 1000.0
F2I
OUT 0
LOD 2.0
SET main_c
LOD 0.0
SET main_c_i
F_LES main_c
JIZ __Lfa5a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa5b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa5z
@__Lfa5b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa5z
@__Lfa5a LOD main_c
F_LES 0.0
JIZ __Lfa5c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa5z
@__Lfa5c LOD main_c_i
F_LES 0.0
JIZ __Lfa5d
LOD 1.57079632679
JMP __Lfa5z
@__Lfa5d LOD 0.0
F_LES main_c_i
JIZ __Lfa5e
LOD -1.57079632679
JMP __Lfa5z
@__Lfa5e LOD 0.0
@__Lfa5z F_MLT 1000.0
F2I
OUT 0
LOD -2.0
SET main_c
LOD 0.0
SET main_c_i
F_LES main_c
JIZ __Lfa6a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa6b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa6z
@__Lfa6b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa6z
@__Lfa6a LOD main_c
F_LES 0.0
JIZ __Lfa6c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa6z
@__Lfa6c LOD main_c_i
F_LES 0.0
JIZ __Lfa6d
LOD 1.57079632679
JMP __Lfa6z
@__Lfa6d LOD 0.0
F_LES main_c_i
JIZ __Lfa6e
LOD -1.57079632679
JMP __Lfa6z
@__Lfa6e LOD 0.0
@__Lfa6z F_MLT 1000.0
F2I
OUT 0
LOD 0.0
SET main_c
LOD 2.0
SET main_c_i
LOD 0.0
F_LES main_c
JIZ __Lfa7a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa7b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa7z
@__Lfa7b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa7z
@__Lfa7a LOD main_c
F_LES 0.0
JIZ __Lfa7c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa7z
@__Lfa7c LOD main_c_i
F_LES 0.0
JIZ __Lfa7d
LOD 1.57079632679
JMP __Lfa7z
@__Lfa7d LOD 0.0
F_LES main_c_i
JIZ __Lfa7e
LOD -1.57079632679
JMP __Lfa7z
@__Lfa7e LOD 0.0
@__Lfa7z F_MLT 1000.0
F2I
OUT 0
LOD 0.0
SET main_c
LOD -2.0
SET main_c_i
LOD 0.0
F_LES main_c
JIZ __Lfa8a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa8b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa8z
@__Lfa8b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa8z
@__Lfa8a LOD main_c
F_LES 0.0
JIZ __Lfa8c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa8z
@__Lfa8c LOD main_c_i
F_LES 0.0
JIZ __Lfa8d
LOD 1.57079632679
JMP __Lfa8z
@__Lfa8d LOD 0.0
F_LES main_c_i
JIZ __Lfa8e
LOD -1.57079632679
JMP __Lfa8z
@__Lfa8e LOD 0.0
@__Lfa8z F_MLT 1000.0
F2I
OUT 0
LOD 0.0
SET main_c
SET main_c_i
F_LES main_c
JIZ __Lfa9a
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES main_c_i
JIZ __Lfa9b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa9z
@__Lfa9b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa9z
@__Lfa9a LOD main_c
F_LES 0.0
JIZ __Lfa9c
LOD main_c
F_DIV main_c_i
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa9z
@__Lfa9c LOD main_c_i
F_LES 0.0
JIZ __Lfa9d
LOD 1.57079632679
JMP __Lfa9z
@__Lfa9d LOD 0.0
F_LES main_c_i
JIZ __Lfa9e
LOD -1.57079632679
JMP __Lfa9z
@__Lfa9e LOD 0.0
@__Lfa9z F_MLT 1000.0
F2I
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim

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
