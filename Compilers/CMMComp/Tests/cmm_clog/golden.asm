NOP
#PRNAME cmm_clog
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
F_MLT 1.000000
P_LOD 0.000000
F_MLT 0.000000
SF_ADD
CAL float_log
F_MLT 0.5
SET clog_re
LOD 1.000000
P_LOD 0.000000
SET_P fase_im
SET fase_re
LOD 0.0
F_LES fase_re
JIZ Lfa1a
LOD fase_re
F_DIV fase_im
CAL float_atan
SET fase_t
LOD 0.0
F_LES fase_im
JIZ Lfa1b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa1z
@Lfa1b LOD fase_t
F_ADD 3.14159265359
JMP Lfa1z
@Lfa1a LOD fase_re
F_LES 0.0
JIZ Lfa1c
LOD fase_re
F_DIV fase_im
CAL float_atan
JMP Lfa1z
@Lfa1c LOD fase_im
F_LES 0.0
JIZ Lfa1d
LOD 1.57079632679
JMP Lfa1z
@Lfa1d LOD 0.0
F_LES fase_im
JIZ Lfa1e
LOD -1.57079632679
JMP Lfa1z
@Lfa1e LOD 0.0
@Lfa1z SET clog_im
LOD clog_re
P_LOD clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD 0.000000
F_MLT 0.000000
P_LOD 1.000000
F_MLT 1.000000
SF_ADD
CAL float_log
F_MLT 0.5
SET clog_re
LOD 0.000000
P_LOD 1.000000
SET_P fase_im
SET fase_re
LOD 0.0
F_LES fase_re
JIZ Lfa2a
LOD fase_re
F_DIV fase_im
CAL float_atan
SET fase_t
LOD 0.0
F_LES fase_im
JIZ Lfa2b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa2z
@Lfa2b LOD fase_t
F_ADD 3.14159265359
JMP Lfa2z
@Lfa2a LOD fase_re
F_LES 0.0
JIZ Lfa2c
LOD fase_re
F_DIV fase_im
CAL float_atan
JMP Lfa2z
@Lfa2c LOD fase_im
F_LES 0.0
JIZ Lfa2d
LOD 1.57079632679
JMP Lfa2z
@Lfa2d LOD 0.0
F_LES fase_im
JIZ Lfa2e
LOD -1.57079632679
JMP Lfa2z
@Lfa2e LOD 0.0
@Lfa2z SET clog_im
LOD clog_re
P_LOD clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD -1.000000
F_MLT -1.000000
P_LOD 0.000000
F_MLT 0.000000
SF_ADD
CAL float_log
F_MLT 0.5
SET clog_re
LOD -1.000000
P_LOD 0.000000
SET_P fase_im
SET fase_re
LOD 0.0
F_LES fase_re
JIZ Lfa3a
LOD fase_re
F_DIV fase_im
CAL float_atan
SET fase_t
LOD 0.0
F_LES fase_im
JIZ Lfa3b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa3z
@Lfa3b LOD fase_t
F_ADD 3.14159265359
JMP Lfa3z
@Lfa3a LOD fase_re
F_LES 0.0
JIZ Lfa3c
LOD fase_re
F_DIV fase_im
CAL float_atan
JMP Lfa3z
@Lfa3c LOD fase_im
F_LES 0.0
JIZ Lfa3d
LOD 1.57079632679
JMP Lfa3z
@Lfa3d LOD 0.0
F_LES fase_im
JIZ Lfa3e
LOD -1.57079632679
JMP Lfa3z
@Lfa3e LOD 0.0
@Lfa3z SET clog_im
LOD clog_re
P_LOD clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD 3.000000
SET main_z
LOD 4.000000
SET main_z_i
LOD main_z
F_MLT main_z
P_LOD main_z_i
F_MLT main_z_i
SF_ADD
CAL float_log
F_MLT 0.5
SET clog_re
LOD main_z
P_LOD main_z_i
SET_P fase_im
SET fase_re
LOD 0.0
F_LES fase_re
JIZ Lfa4a
LOD fase_re
F_DIV fase_im
CAL float_atan
SET fase_t
LOD 0.0
F_LES fase_im
JIZ Lfa4b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa4z
@Lfa4b LOD fase_t
F_ADD 3.14159265359
JMP Lfa4z
@Lfa4a LOD fase_re
F_LES 0.0
JIZ Lfa4c
LOD fase_re
F_DIV fase_im
CAL float_atan
JMP Lfa4z
@Lfa4c LOD fase_im
F_LES 0.0
JIZ Lfa4d
LOD 1.57079632679
JMP Lfa4z
@Lfa4d LOD 0.0
F_LES fase_im
JIZ Lfa4e
LOD -1.57079632679
JMP Lfa4z
@Lfa4e LOD 0.0
@Lfa4z SET clog_im
LOD clog_re
P_LOD clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD 1.000000
SET main_z
LOD 2.000000
SET main_z_i
LOD 2.000000
SET main_w
LOD 2.000000
SET main_w_i
LOD main_z
F_ADD main_w
P_LOD main_z_i
F_ADD main_w_i
SET_P clog_b
SET   clog_a
F_MLT clog_a
P_LOD clog_b
F_MLT clog_b
SF_ADD
CAL float_log
F_MLT 0.5
SET clog_re
LOD clog_a
P_LOD clog_b
SET_P fase_im
SET fase_re
LOD 0.0
F_LES fase_re
JIZ Lfa5a
LOD fase_re
F_DIV fase_im
CAL float_atan
SET fase_t
LOD 0.0
F_LES fase_im
JIZ Lfa5b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa5z
@Lfa5b LOD fase_t
F_ADD 3.14159265359
JMP Lfa5z
@Lfa5a LOD fase_re
F_LES 0.0
JIZ Lfa5c
LOD fase_re
F_DIV fase_im
CAL float_atan
JMP Lfa5z
@Lfa5c LOD fase_im
F_LES 0.0
JIZ Lfa5d
LOD 1.57079632679
JMP Lfa5z
@Lfa5d LOD 0.0
F_LES fase_im
JIZ Lfa5e
LOD -1.57079632679
JMP Lfa5z
@Lfa5e LOD 0.0
@Lfa5z SET clog_im
LOD clog_re
P_LOD clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
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

// Natural logarithm ----------------------------------------------------------
// log(x) = e*ln2 + log(m),  e = XPO(x) = floor(log2 x),  m = x*2^-e in [1,2).
// O(1): e via XPO, m via a single F_SCL (no halve/double loop). log(m) is the
// atanh series 2u(1 + u^2/3 + u^4/5 + u^6/7 + u^8/9), u = (m-1)/(m+1).
// x <= 0 returns 0.

@float_log  SET   log_x                 // save x (acc still = x)
            F_LES 0.0                    // (x > 0) ?   [F_LES true when acc > X]
            JIZ   L_log_zero             // x <= 0 -> bail out with 0

            LOD   log_x
            XPO                          // e = floor(log2 x)   (int)
            SET   log_e
            NEG                          // -e
            SET   log_ne
            LOD   log_x
            F_SCL log_ne                 // m = x * 2^(-e)  -> [1,2)
            SET   log_m

            F_ADD 1.0                    // m+1   (acc still = m)
            SET   log_t2
            LOD   log_m
            F_SU1 1.0                    // m-1   (acc - X)
            SET   log_t1
            LOD   log_t2
            F_DIV log_t1                 // u = (m-1)/(m+1)   [F_DIV X = X/acc]
            SET   log_u
            F_MLT log_u
            SET   log_w                  // w = u^2

            LOD   0.1111111111           // Horner in w (1/9, 1/7, 1/5, 1/3, 1)
            F_MLT log_w
            F_ADD 0.1428571429
            F_MLT log_w
            F_ADD 0.2
            F_MLT log_w
            F_ADD 0.3333333333
            F_MLT log_w
            F_ADD 1.0
            F_MLT log_u
            F_MLT 2.0                     // log(m)
            SET   log_lm

            LOD   log_e
            I2F                           // float(e)
            F_MLT 0.6931471806            // e * ln2
            F_ADD log_lm                  // + log(m)  -> log(x)
            RET

@L_log_zero LOD   0.0
            RET
