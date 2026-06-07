NOP
#PRNAME cmm_csincos
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
CAL float_exp
SET csin_eb
F_DIV 1.0
SET csin_emb
LOD csin_eb
F_ADD csin_emb
F_MLT 0.5
SET csin_chb
LOD csin_eb
F_SU1 csin_emb
F_MLT 0.5
SET csin_shb
LOD 0.000000
CAL float_sin
SET csin_sa
LOD 0.000000
F_NEG
F_ADD 1.570796327CAL float_sin
SET csin_ca
LOD csin_sa
F_MLT csin_chb
SET csin_re
LOD csin_ca
F_MLT csin_shb
SET csin_im
LOD csin_re
P_LOD csin_im
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
CAL float_exp
SET ccos_eb
F_DIV 1.0
SET ccos_emb
LOD ccos_eb
F_ADD ccos_emb
F_MLT 0.5
SET ccos_chb
LOD ccos_eb
F_SU1 ccos_emb
F_MLT 0.5
SET ccos_shb
LOD 0.000000
CAL float_sin
SET ccos_sa
LOD 0.000000
F_NEG
F_ADD 1.570796327CAL float_sin
SET ccos_ca
F_MLT ccos_chb
SET ccos_re
LOD ccos_sa
F_MLT ccos_shb
F_NEG
SET ccos_im
LOD ccos_re
P_LOD ccos_im
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
LOD 1.000000
SET main_z_i
CAL float_exp
SET csin_eb
F_DIV 1.0
SET csin_emb
LOD csin_eb
F_ADD csin_emb
F_MLT 0.5
SET csin_chb
LOD csin_eb
F_SU1 csin_emb
F_MLT 0.5
SET csin_shb
LOD main_z
CAL float_sin
SET csin_sa
LOD main_z
F_NEG
F_ADD 1.570796327CAL float_sin
SET csin_ca
LOD csin_sa
F_MLT csin_chb
SET csin_re
LOD csin_ca
F_MLT csin_shb
SET csin_im
LOD csin_re
P_LOD csin_im
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
LOD 1.000000
SET main_z_i
CAL float_exp
SET ccos_eb
F_DIV 1.0
SET ccos_emb
LOD ccos_eb
F_ADD ccos_emb
F_MLT 0.5
SET ccos_chb
LOD ccos_eb
F_SU1 ccos_emb
F_MLT 0.5
SET ccos_shb
LOD main_z
CAL float_sin
SET ccos_sa
LOD main_z
F_NEG
F_ADD 1.570796327CAL float_sin
SET ccos_ca
F_MLT ccos_chb
SET ccos_re
LOD ccos_sa
F_MLT ccos_shb
F_NEG
SET ccos_im
LOD ccos_re
P_LOD ccos_im
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD 0.500000
SET main_z
LOD 0.500000
SET main_z_i
LOD 0.500000
SET main_w
LOD 0.500000
SET main_w_i
LOD main_z
F_ADD main_w
P_LOD main_z_i
F_ADD main_w_i
SET_P csin_b
SET   csin_a
LOD csin_b
CAL float_exp
SET csin_eb
F_DIV 1.0
SET csin_emb
LOD csin_eb
F_ADD csin_emb
F_MLT 0.5
SET csin_chb
LOD csin_eb
F_SU1 csin_emb
F_MLT 0.5
SET csin_shb
LOD csin_a
CAL float_sin
SET csin_sa
LOD csin_a
F_NEG
F_ADD 1.570796327CAL float_sin
SET csin_ca
LOD csin_sa
F_MLT csin_chb
SET csin_re
LOD csin_ca
F_MLT csin_shb
SET csin_im
LOD csin_re
P_LOD csin_im
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

// Sine function --------------------------------------------------------------
// O(1) range reduction to [-pi/2, pi/2]: k = round(x/pi); r = x - k*pi; then
// sin(x) = (-1)^k * sin(r). sin(r) is a degree-7 odd minimax polynomial (the
// coefficients are a least-squares fit on [0, pi/2], max abs error ~1.6e-6 --
// ~6 digits, vs the old 152-point LUT's ~3). No lookup table, no new hardware.
// NB: F2I truncates toward zero, so round-to-nearest uses a sign-half bias.

@float_sin  SET   sin_x                  // save x
            F_MLT 0.3183098862           // q = x / pi   (1/pi)
            SET   sin_q
            LOD   0.5
            F_SGN sin_q                  // copysign(0.5, q)
            F_ADD sin_q                  // q + copysign(0.5,q)
            F2I                          // k = round(q)
            SET   sin_k                  // keep k (int) for the (-1)^k sign
            I2F                          // float(k)
            F_MLT 3.1415926536           // k * pi
            F_SU2 sin_x                  // r = x - k*pi  -> [-pi/2, pi/2]
            SET   sin_r
            F_MLT sin_r                  // w = r^2
            SET   sin_w

            LOD  -0.000184472138         // Horner in w: sin(r)/r = a1 + a3 w + a5 w^2 + a7 w^3
            F_MLT sin_w                  // a7
            F_ADD 0.008309516704         // a5
            F_MLT sin_w
            F_ADD -0.166651680787        // a3
            F_MLT sin_w
            F_ADD 0.999997487148         // a1
            F_MLT sin_r                  // * r  -> sin(r)
            SET   sin_p

            LOD   sin_k
            AND   1                      // k & 1  (parity)
            JIZ   L_sin_even             // k even -> +sin(r)
            LOD   sin_p
            F_NEG                        // k odd  -> -sin(r)
            RET
@L_sin_even LOD   sin_p
            RET

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
