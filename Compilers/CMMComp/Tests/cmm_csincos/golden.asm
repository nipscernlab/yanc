NOP
#PRNAME cmm_csincos
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE __csin_eb __csin_shb
#SHARE main_c_i __csin_emb
#SHARE __ccos_eb __ccos_shb
#SHARE __ccos_emb __csin_emb
#SHARE main_z_i __csin_emb
#SHARE main_w __csin_a
#SHARE main_w_i main_c
#SHARE __csin_b __csin_emb
#SHARE __sin_x main_c
#SHARE __sin_q __csin_emb
#SHARE __sin_k __csin_emb
#SHARE __sin_r main_c
#SHARE __sin_p main_c
#SHARE __exp_x __csin_emb
#SHARE __exp_q main_c
#SHARE __exp_n main_c
#SHARE __exp_r __csin_emb
@main @__Lwh1 LOD 1.000000
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD 0.000000
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD 0.000000
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
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
CAL __float_exp
SET __ccos_eb
F_DIV 1.0
NOP
F_QUO
SET __ccos_emb
F_ADD __ccos_eb
F_MLT 0.5
SET __ccos_chb
LOD __ccos_eb
F_SU1 __ccos_emb
F_MLT 0.5
SET __ccos_shb
LOD 0.000000
CAL __float_sin
SET __ccos_sa
LOD 0.000000
F_SU2 1.570796327
CAL __float_sin
F_MLT __ccos_chb
P_LOD __ccos_sa
F_MLT __ccos_shb
F_NEG
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
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD main_z
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD main_z
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
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
CAL __float_exp
SET __ccos_eb
F_DIV 1.0
NOP
F_QUO
SET __ccos_emb
F_ADD __ccos_eb
F_MLT 0.5
SET __ccos_chb
LOD __ccos_eb
F_SU1 __ccos_emb
F_MLT 0.5
SET __ccos_shb
LOD main_z
CAL __float_sin
SET __ccos_sa
LOD main_z
F_SU2 1.570796327
CAL __float_sin
F_MLT __ccos_chb
P_LOD __ccos_sa
F_MLT __ccos_shb
F_NEG
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
SET_P __csin_b
SET   __csin_a
LOD __csin_b
CAL __float_exp
SET __csin_eb
F_DIV 1.0
NOP
F_QUO
SET __csin_emb
F_ADD __csin_eb
F_MLT 0.5
SET __csin_chb
LOD __csin_eb
F_SU1 __csin_emb
F_MLT 0.5
SET __csin_shb
LOD __csin_a
F_SU2 1.570796327
CAL __float_sin
SET __csin_ca
LOD __csin_a
CAL __float_sin
F_MLT __csin_chb
P_LOD __csin_ca
F_MLT __csin_shb
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

// Sine function --------------------------------------------------------------
// O(1) range reduction to [-pi/2, pi/2]: k = round(x/pi); r = x - k*pi; then
// sin(x) = (-1)^k * sin(r). sin(r) is a degree-7 odd minimax polynomial (the
// coefficients are a least-squares fit on [0, pi/2], max abs error ~1.6e-6 --
// ~6 digits, vs the old 152-point LUT's ~3). No lookup table, no new hardware.
// NB: F2I truncates toward zero, so round-to-nearest uses a sign-half bias.

@__float_sin  SET   __sin_x                  // save x
            F_MLT 0.3183098862           // q = x / pi   (1/pi)
            SET   __sin_q
            LOD   0.5
            F_SGN __sin_q                  // copysign(0.5, q)
            F_ADD __sin_q                  // q + copysign(0.5,q)
            F2I                          // k = round(q)
            SET   __sin_k                  // keep k (int) for the (-1)^k sign
            I2F                          // float(k)
            F_MLT 3.1415926536           // k * pi
            F_SU2 __sin_x                  // r = x - k*pi  -> [-pi/2, pi/2]
            SET   __sin_r
            F_MLT __sin_r                  // w = r^2
            SET   __sin_w

            LOD  -0.000184472138         // Horner in w: sin(r)/r = a1 + a3 w + a5 w^2 + a7 w^3
            F_MLT __sin_w                  // a7
            F_ADD 0.008309516704         // a5
            F_MLT __sin_w
            F_ADD -0.166651680787        // a3
            F_MLT __sin_w
            F_ADD 0.999997487148         // a1
            F_MLT __sin_r                  // * r  -> sin(r)
            SET   __sin_p

            LOD   __sin_k
            AND   1                      // k & 1  (parity)
            JIZ   __L_sin_even             // k even -> +sin(r)
            LOD   __sin_p
            F_NEG                        // k odd  -> -sin(r)
            RET
@__L_sin_even LOD   __sin_p
            RET

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
