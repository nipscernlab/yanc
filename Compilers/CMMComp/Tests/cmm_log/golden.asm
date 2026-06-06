NOP
#PRNAME cmm_log
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
LOD 1.0
CAL float_log
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 2.0
CAL float_log
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 8.0
SET main_x
F_ADD 2.0
CAL float_log
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 8.0
CAL float_log
CAL float_exp
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 0.0
CAL float_log
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim

// Exponential function -------------------------------------------------------
// exp(x) = 2^n * exp(r),  n = round(x/ln2),  r = x - n*ln2 in [-ln2/2, ln2/2].
// O(1): n via F2I (round-to-nearest with a sign-half bias, since F2I truncates),
// 2^n via a single F_SCL (no loop). exp(r) is a degree-6 Taylor polynomial.

@float_exp  SET   exp_x                 // save x
            F_MLT 1.4426950409          // q = x / ln2   (1/ln2 = log2 e)
            SET   exp_q
            LOD   0.5
            F_SGN exp_q                 // copysign(0.5, q)
            F_ADD exp_q                 // q + copysign(0.5,q)
            F2I                         // n = round(q)   (F2I truncates toward 0)
            SET   exp_n
            I2F                         // float(n)
            F_MLT 0.6931471806          // n * ln2
            F_SU2 exp_x                 // r = x - n*ln2   (F_SU2 X = X - acc)
            SET   exp_r

            LOD   0.0013888889          // Horner, exp(r) = sum r^k/k! (k=0..6)
            F_MLT exp_r                 // 1/6!
            F_ADD 0.0083333333          // 1/5!
            F_MLT exp_r
            F_ADD 0.0416666667          // 1/4!
            F_MLT exp_r
            F_ADD 0.1666666667          // 1/3!
            F_MLT exp_r
            F_ADD 0.5                    // 1/2!
            F_MLT exp_r
            F_ADD 1.0                    // 1/1!
            F_MLT exp_r
            F_ADD 1.0                    // 1/0!  -> exp(r)
            F_SCL exp_n                  // * 2^n  -> exp(x)
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
