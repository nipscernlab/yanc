NOP
#PRNAME ArcTan
#NUBITS 32
#NDSTAC 5
#SDEPTH 5
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
JMP main
@arctan_LUT SET arctan_LUT_x
#arrays arctan_LUT_atan_lut 2 49 "Arctan_LUT.txt"
LOD arctan_LUT_x
SET arctan_LUT_signo
F_ABS_M arctan_LUT_x
SET arctan_LUT_x
LOD 0.0
SET arctan_LUT_v0
LOD arctan_LUT_x
P_LOD 1.0
SF_GRE
JIZ Lif1else
I2F_M 1
P_LOD arctan_LUT_x
SF_DIV
SET arctan_LUT_x
LOD 1.57079632679
SET arctan_LUT_v0
@Lif1else LOD 47.0
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
@main #arrays main_x 2 1000 "atan_x.txt"
#arrays main_a 2 1000 "atan_y.txt"
LOD 0
SET main_j
@Lwh1 LOD 1000
LES main_j
JIZ Lwh1end
LOD main_j
LDI main_x
CAL float_atan
SET main_y
LOD main_j
LDI main_a
SET main_t
F_SU1 main_y
SET main_e
LOD main_j
ADD 1
SET main_j
JMP Lwh1
@Lwh1end LOD 0
SET main_j
@Lwh2 LOD 1000
LES main_j
JIZ Lwh2end
LOD main_j
LDI main_x
CAL arctan_LUT
SET main_y
LOD main_j
LDI main_a
SET main_t
F_SU1 main_y
SET main_e
LOD main_j
ADD 1
SET main_j
JMP Lwh2
@Lwh2end @fim JMP fim

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
