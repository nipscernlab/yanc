NOP
#PRNAME cmm_hyper
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
LOD 0.0
SET cosh_x
CAL float_exp
SET cosh_t
F_NEG_M cosh_x
CAL float_exp
F_ADD cosh_t
F_MLT 0.5
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 1.0
SET cosh_x
CAL float_exp
SET cosh_t
F_NEG_M cosh_x
CAL float_exp
F_ADD cosh_t
F_MLT 0.5
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
F_NEG_M 1.0
SET cosh_x
CAL float_exp
SET cosh_t
F_NEG_M cosh_x
CAL float_exp
F_ADD cosh_t
F_MLT 0.5
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 0.0
SET sinh_x
CAL float_exp
SET sinh_t
F_NEG_M sinh_x
CAL float_exp
F_SU2 sinh_t
F_MLT 0.5
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 1.0
SET sinh_x
CAL float_exp
SET sinh_t
F_NEG_M sinh_x
CAL float_exp
F_SU2 sinh_t
F_MLT 0.5
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
F_NEG_M 1.0
SET sinh_x
CAL float_exp
SET sinh_t
F_NEG_M sinh_x
CAL float_exp
F_SU2 sinh_t
F_MLT 0.5
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 0.0
F_MLT 2.0
CAL float_exp
SET tanh_e
F_ADD -1.0
SET tanh_n
LOD tanh_e
F_ADD 1.0
F_DIV tanh_n
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 1.0
F_MLT 2.0
CAL float_exp
SET tanh_e
F_ADD -1.0
SET tanh_n
LOD tanh_e
F_ADD 1.0
F_DIV tanh_n
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 3.0
F_MLT 2.0
CAL float_exp
SET tanh_e
F_ADD -1.0
SET tanh_n
LOD tanh_e
F_ADD 1.0
F_DIV tanh_n
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
