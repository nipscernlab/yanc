NOP
#PRNAME cmm_exp
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE __exp_x main_r
#SHARE __exp_n __exp_q
#SHARE __exp_r main_r
#SHARE __log_x __exp_q
#SHARE __log_e main_r
#SHARE __log_m __log_ne
#SHARE __log_t2 __exp_q
#SHARE __log_t1 __log_ne
#SHARE __log_u __exp_q
#SHARE __log_w __log_ne
#SHARE __log_lm __exp_q
@main @__Lwh1 LOD 0.0
CAL __float_exp
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 1.0
CAL __float_exp
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 1.0
F_ADD 1.0
CAL __float_exp
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD -1.0
CAL __float_exp
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 3.0
CAL __float_exp
CAL __float_log
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim

// Exponential function -------------------------------------------------------
// exp(x) = 2^n * exp(r),  n = round(x/ln2),  r = x - n*ln2 in [-ln2/2, ln2/2].
// O(1): n via F2I (round-to-nearest with a sign-half bias, since F2I truncates),
// 2^n via a single F_SCL (no loop). exp(r) is a degree-6 Taylor polynomial.

@__float_exp  SET   __exp_x                 // save x
            F_MLT 1.4426950409          // q = x / ln2   (1/ln2 = log2 e)
            SET   __exp_q
            LOD   0.5
            F_SGN __exp_q                 // copysign(0.5, q)
            F_ADD __exp_q                 // q + copysign(0.5,q)
            F2I                         // n = round(q)   (F2I truncates toward 0)
            SET   __exp_n
            I2F                         // float(n)
            F_MLT 0.6931471806          // n * ln2
            F_SU2 __exp_x                 // r = x - n*ln2   (F_SU2 X = X - acc)
            SET   __exp_r

            LOD   0.0013888889          // Horner, exp(r) = sum r^k/k! (k=0..6)
            F_MLT __exp_r                 // 1/6!
            F_ADD 0.0083333333          // 1/5!
            F_MLT __exp_r
            F_ADD 0.0416666667          // 1/4!
            F_MLT __exp_r
            F_ADD 0.1666666667          // 1/3!
            F_MLT __exp_r
            F_ADD 0.5                    // 1/2!
            F_MLT __exp_r
            F_ADD 1.0                    // 1/1!
            F_MLT __exp_r
            F_ADD 1.0                    // 1/0!  -> exp(r)
            F_SCL __exp_n                  // * 2^n  -> exp(x)
            RET

// Natural logarithm ----------------------------------------------------------
// log(x) = e*ln2 + log(m),  e = XPO(x) = floor(log2 x),  m = x*2^-e in [1,2).
// O(1): e via XPO, m via a single F_SCL (no halve/double loop). log(m) is the
// atanh series 2u(1 + u^2/3 + u^4/5 + u^6/7 + u^8/9), u = (m-1)/(m+1).
// x <= 0 returns 0.

@__float_log  SET   __log_x                 // save x (acc still = x)
            F_LES 0.0                    // (x > 0) ?   [F_LES true when acc > X]
            JIZ   __L_log_zero             // x <= 0 -> bail out with 0

            LOD   __log_x
            XPO                          // e = floor(log2 x)   (int)
            SET   __log_e
            NEG                          // -e
            SET   __log_ne
            LOD   __log_x
            F_SCL __log_ne                 // m = x * 2^(-e)  -> [1,2)
            SET   __log_m

            F_ADD 1.0                    // m+1   (acc still = m)
            SET   __log_t2
            LOD   __log_m
            F_SU1 1.0                    // m-1   (acc - X)
            SET   __log_t1
            LOD   __log_t2
            F_DIV __log_t1                 // u = (m-1)/(m+1)   [F_DIV X = X/acc]
NOP
NOP
F_QUO
            SET   __log_u
            F_MLT __log_u
            SET   __log_w                  // w = u^2

            LOD   0.1111111111           // Horner in w (1/9, 1/7, 1/5, 1/3, 1)
            F_MLT __log_w
            F_ADD 0.1428571429
            F_MLT __log_w
            F_ADD 0.2
            F_MLT __log_w
            F_ADD 0.3333333333
            F_MLT __log_w
            F_ADD 1.0
            F_MLT __log_u
            F_MLT 2.0                     // log(m)
            SET   __log_lm

            LOD   __log_e
            I2F                           // float(e)
            F_MLT 0.6931471806            // e * ln2
            F_ADD __log_lm                  // + log(m)  -> log(x)
            RET

@__L_log_zero LOD   0.0
            RET
