NOP
#PRNAME cmm_comp_fase
#NUBITS 32
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 1.000000
SET main_c
LOD 2.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa1a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa1b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa1z
@Lfa1b LOD fase_t
F_ADD 3.14159265359
JMP Lfa1z
@Lfa1a LOD main_c
F_LES 0.0
JIZ Lfa1c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa1z
@Lfa1c LOD main_c_i
F_LES 0.0
JIZ Lfa1d
LOD 1.57079632679
JMP Lfa1z
@Lfa1d LOD 0.0
F_LES main_c_i
JIZ Lfa1e
LOD -1.57079632679
JMP Lfa1z
@Lfa1e LOD 0.0
@Lfa1z F_MLT 1000.0
F2I
OUT 0
LOD -1.000000
SET main_c
LOD 2.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa2a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa2b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa2z
@Lfa2b LOD fase_t
F_ADD 3.14159265359
JMP Lfa2z
@Lfa2a LOD main_c
F_LES 0.0
JIZ Lfa2c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa2z
@Lfa2c LOD main_c_i
F_LES 0.0
JIZ Lfa2d
LOD 1.57079632679
JMP Lfa2z
@Lfa2d LOD 0.0
F_LES main_c_i
JIZ Lfa2e
LOD -1.57079632679
JMP Lfa2z
@Lfa2e LOD 0.0
@Lfa2z F_MLT 1000.0
F2I
OUT 0
LOD -1.000000
SET main_c
LOD -2.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa3a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa3b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa3z
@Lfa3b LOD fase_t
F_ADD 3.14159265359
JMP Lfa3z
@Lfa3a LOD main_c
F_LES 0.0
JIZ Lfa3c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa3z
@Lfa3c LOD main_c_i
F_LES 0.0
JIZ Lfa3d
LOD 1.57079632679
JMP Lfa3z
@Lfa3d LOD 0.0
F_LES main_c_i
JIZ Lfa3e
LOD -1.57079632679
JMP Lfa3z
@Lfa3e LOD 0.0
@Lfa3z F_MLT 1000.0
F2I
OUT 0
LOD 1.000000
SET main_c
LOD -2.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa4a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa4b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa4z
@Lfa4b LOD fase_t
F_ADD 3.14159265359
JMP Lfa4z
@Lfa4a LOD main_c
F_LES 0.0
JIZ Lfa4c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa4z
@Lfa4c LOD main_c_i
F_LES 0.0
JIZ Lfa4d
LOD 1.57079632679
JMP Lfa4z
@Lfa4d LOD 0.0
F_LES main_c_i
JIZ Lfa4e
LOD -1.57079632679
JMP Lfa4z
@Lfa4e LOD 0.0
@Lfa4z F_MLT 1000.0
F2I
OUT 0
LOD 2.000000
SET main_c
LOD 0.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa5a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa5b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa5z
@Lfa5b LOD fase_t
F_ADD 3.14159265359
JMP Lfa5z
@Lfa5a LOD main_c
F_LES 0.0
JIZ Lfa5c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa5z
@Lfa5c LOD main_c_i
F_LES 0.0
JIZ Lfa5d
LOD 1.57079632679
JMP Lfa5z
@Lfa5d LOD 0.0
F_LES main_c_i
JIZ Lfa5e
LOD -1.57079632679
JMP Lfa5z
@Lfa5e LOD 0.0
@Lfa5z F_MLT 1000.0
F2I
OUT 0
LOD -2.000000
SET main_c
LOD 0.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa6a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa6b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa6z
@Lfa6b LOD fase_t
F_ADD 3.14159265359
JMP Lfa6z
@Lfa6a LOD main_c
F_LES 0.0
JIZ Lfa6c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa6z
@Lfa6c LOD main_c_i
F_LES 0.0
JIZ Lfa6d
LOD 1.57079632679
JMP Lfa6z
@Lfa6d LOD 0.0
F_LES main_c_i
JIZ Lfa6e
LOD -1.57079632679
JMP Lfa6z
@Lfa6e LOD 0.0
@Lfa6z F_MLT 1000.0
F2I
OUT 0
LOD 0.000000
SET main_c
LOD 2.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa7a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa7b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa7z
@Lfa7b LOD fase_t
F_ADD 3.14159265359
JMP Lfa7z
@Lfa7a LOD main_c
F_LES 0.0
JIZ Lfa7c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa7z
@Lfa7c LOD main_c_i
F_LES 0.0
JIZ Lfa7d
LOD 1.57079632679
JMP Lfa7z
@Lfa7d LOD 0.0
F_LES main_c_i
JIZ Lfa7e
LOD -1.57079632679
JMP Lfa7z
@Lfa7e LOD 0.0
@Lfa7z F_MLT 1000.0
F2I
OUT 0
LOD 0.000000
SET main_c
LOD -2.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa8a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa8b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa8z
@Lfa8b LOD fase_t
F_ADD 3.14159265359
JMP Lfa8z
@Lfa8a LOD main_c
F_LES 0.0
JIZ Lfa8c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa8z
@Lfa8c LOD main_c_i
F_LES 0.0
JIZ Lfa8d
LOD 1.57079632679
JMP Lfa8z
@Lfa8d LOD 0.0
F_LES main_c_i
JIZ Lfa8e
LOD -1.57079632679
JMP Lfa8z
@Lfa8e LOD 0.0
@Lfa8z F_MLT 1000.0
F2I
OUT 0
LOD 0.000000
SET main_c
LOD 0.000000
SET main_c_i
LOD 0.0
F_LES main_c
JIZ Lfa9a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD 0.0
F_LES main_c_i
JIZ Lfa9b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa9z
@Lfa9b LOD fase_t
F_ADD 3.14159265359
JMP Lfa9z
@Lfa9a LOD main_c
F_LES 0.0
JIZ Lfa9c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa9z
@Lfa9c LOD main_c_i
F_LES 0.0
JIZ Lfa9d
LOD 1.57079632679
JMP Lfa9z
@Lfa9d LOD 0.0
F_LES main_c_i
JIZ Lfa9e
LOD -1.57079632679
JMP Lfa9z
@Lfa9e LOD 0.0
@Lfa9z F_MLT 1000.0
F2I
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim

// Arctangent function --------------------------------------------------------
// |x| is folded into [0,1] with the 1/x identity: atan(|x|) = pi/2 - atan(1/|x|)
// for |x|>1. atan(t) on [0,1] is a degree-11 odd minimax polynomial (least-
// squares fit, max abs error ~4.9e-6 -- ~6 digits, vs the old 49-point LUT's
// ~4e-5). One shared polynomial for both branches; no table, no new hardware.

@float_atan SET   atan_x                 // save x
            F_ABS_M atan_x               // ax = |x|
            SET   atan_ax
            F_LES 1.0                    // (ax > 1)?  [F_LES true when acc > X]
            SET   atan_big               // flag: 1 if |x|>1 (use 1/x)
            JIZ   L_atan_small
            LOD   atan_ax                // big branch: t = 1/ax
            F_DIV 1.0                    // 1.0 / ax   (F_DIV X = X/acc)
            JMP   L_atan_haveT
@L_atan_small LOD atan_ax               // small branch: t = ax
@L_atan_haveT SET atan_t

            F_MLT atan_t                 // w = t^2
            SET   atan_w
            LOD  -0.012300666580         // Horner in w: atan(t)/t = a1 + a3 w + ... + a11 w^5
            F_MLT atan_w                 // a11
            F_ADD 0.054084934779         // a9
            F_MLT atan_w
            F_ADD -0.117697073484        // a7
            F_MLT atan_w
            F_ADD 0.194020561451         // a5
            F_MLT atan_w
            F_ADD -0.332694520616        // a3
            F_MLT atan_w
            F_ADD 0.999980069822         // a1
            F_MLT atan_t                 // * t  -> atan(t)
            SET   atan_p

            LOD   atan_big
            JIZ   L_atan_done            // |x|<=1 -> result = atan(t)
            LOD   atan_p                 // |x|>1  -> result = pi/2 - atan(1/x)
            F_SU2 1.5707963268           // pi/2 - p   (F_SU2 X = X - acc)
            JMP   L_atan_sign
@L_atan_done LOD atan_p
@L_atan_sign F_SGN atan_x                // apply the sign of x
            RET
