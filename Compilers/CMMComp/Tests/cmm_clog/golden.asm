NOP
#PRNAME cmm_clog
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE __fase_t __fase_re
#SHARE __clog_im __fase_re
#SHARE main_c_i __fase_re
#SHARE main_w __clog_b
#SHARE main_w_i __clog_b
#SHARE __atan_x __fase_re
#SHARE __atan_t main_c
#SHARE __log_x main_c
#SHARE __log_e __fase_re
#SHARE __log_ne __atan_w
#SHARE __log_m main_c
#SHARE __log_t1 __atan_w
#SHARE __log_u main_c
#SHARE __log_w __atan_w
#SHARE __log_lm main_c
@main @__Lwh1 LOD 1.0
P_LOD 0.0
F_MLT 0.0
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD 1.0
P_LOD 0.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa1a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa1b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa1z
@__Lfa1b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa1z
@__Lfa1a LOD __fase_re
F_LES 0.0
JIZ __Lfa1c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa1z
@__Lfa1c LOD __fase_im
F_LES 0.0
JIZ __Lfa1d
LOD 1.57079632679
JMP __Lfa1z
@__Lfa1d LOD 0.0
F_LES __fase_im
JIZ __Lfa1e
LOD -1.57079632679
JMP __Lfa1z
@__Lfa1e LOD 0.0
@__Lfa1z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD 0.0
F_MLT 0.0
P_LOD 1.0
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD 0.0
P_LOD 1.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa2a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa2b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa2z
@__Lfa2b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa2z
@__Lfa2a LOD __fase_re
F_LES 0.0
JIZ __Lfa2c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa2z
@__Lfa2c LOD __fase_im
F_LES 0.0
JIZ __Lfa2d
LOD 1.57079632679
JMP __Lfa2z
@__Lfa2d LOD 0.0
F_LES __fase_im
JIZ __Lfa2e
LOD -1.57079632679
JMP __Lfa2z
@__Lfa2e LOD 0.0
@__Lfa2z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD -1.0
F_MLT -1.0
P_LOD 0.0
F_MLT 0.0
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD -1.0
P_LOD 0.0
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa3a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa3b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa3z
@__Lfa3b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa3z
@__Lfa3a LOD __fase_re
F_LES 0.0
JIZ __Lfa3c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa3z
@__Lfa3c LOD __fase_im
F_LES 0.0
JIZ __Lfa3d
LOD 1.57079632679
JMP __Lfa3z
@__Lfa3d LOD 0.0
F_LES __fase_im
JIZ __Lfa3e
LOD -1.57079632679
JMP __Lfa3z
@__Lfa3e LOD 0.0
@__Lfa3z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD 3.0
SET main_z
LOD 4.0
SET main_z_i
LOD main_z
F_MLT main_z
P_LOD main_z_i
F_MLT main_z_i
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD main_z
P_LOD main_z_i
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa4a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa4b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa4z
@__Lfa4b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa4z
@__Lfa4a LOD __fase_re
F_LES 0.0
JIZ __Lfa4c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa4z
@__Lfa4c LOD __fase_im
F_LES 0.0
JIZ __Lfa4d
LOD 1.57079632679
JMP __Lfa4z
@__Lfa4d LOD 0.0
F_LES __fase_im
JIZ __Lfa4e
LOD -1.57079632679
JMP __Lfa4z
@__Lfa4e LOD 0.0
@__Lfa4z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD 1.0
SET main_z
LOD 2.0
SET main_z_i
SET main_w
SET main_w_i
F_ADD main_z
P_LOD main_z_i
F_ADD main_w_i
SET_P __clog_b
SET __clog_a
F_MLT __clog_a
P_LOD __clog_b
F_MLT __clog_b
SF_ADD
CAL __float_log
F_MLT 0.5
SET __clog_re
LOD __clog_a
P_LOD __clog_b
SET_P __fase_im
SET __fase_re
LOD 0.0
F_LES __fase_re
JIZ __Lfa5a
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
SET __fase_t
LOD 0.0
F_LES __fase_im
JIZ __Lfa5b
LOD __fase_t
F_ADD -3.14159265359
JMP __Lfa5z
@__Lfa5b LOD __fase_t
F_ADD 3.14159265359
JMP __Lfa5z
@__Lfa5a LOD __fase_re
F_LES 0.0
JIZ __Lfa5c
LOD __fase_re
F_DIV __fase_im
NOP
NOP
F_QUO
CAL __float_atan
JMP __Lfa5z
@__Lfa5c LOD __fase_im
F_LES 0.0
JIZ __Lfa5d
LOD 1.57079632679
JMP __Lfa5z
@__Lfa5d LOD 0.0
F_LES __fase_im
JIZ __Lfa5e
LOD -1.57079632679
JMP __Lfa5z
@__Lfa5e LOD 0.0
@__Lfa5z SET __clog_im
LOD __clog_re
P_LOD __clog_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
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
NOP
NOP
F_QUO
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
