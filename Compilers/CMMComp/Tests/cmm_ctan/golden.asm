NOP
#PRNAME cmm_ctan
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE __ctan_eb __ctan_tb
#SHARE __ctan_chb __ctan_ta
#SHARE __ctan_shb __ctan_tb
#SHARE __ctan_d __ctan_ta
#SHARE main_c_i __ctan_ta
#SHARE main_c __ctan_tb
#SHARE main_z __ctan_ta
#SHARE main_z_i __ctan_tb
#SHARE main_w __ctan_emb
#SHARE __ctan_b __ctan_tb
#SHARE __ctan_a __ctan_ta
#SHARE __sin_x main_w_i
#SHARE __sin_q __ctan_emb
#SHARE __sin_k __ctan_emb
#SHARE __sin_r main_w_i
#SHARE __sin_p main_w_i
#SHARE __exp_x __ctan_emb
#SHARE __exp_q main_w_i
#SHARE __exp_n main_w_i
#SHARE __exp_r __ctan_emb
@main @__Lwh1 LOD 0.000000
F_MLT 2.0
SET __ctan_ta
LOD 0.000000
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
F_QUO
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
F_MLT 2.0
SET __ctan_ta
LOD 1.000000
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
F_QUO
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
LOD main_z
F_MLT 2.0
SET __ctan_ta
LOD main_z_i
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
F_QUO
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
SET_P __ctan_b
SET   __ctan_a
F_MLT 2.0
SET __ctan_ta
LOD __ctan_b
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
F_QUO
SET_P main_c_i
SET main_c
F_MLT 1000.0
F2I
OUT 0
LOD main_c_i
F_MLT 1000.0
F2I
OUT 0
LOD 0.785398
F_MLT 2.0
SET __ctan_ta
LOD 0.000000
F_MLT 2.0
SET __ctan_tb
LOD __ctan_ta
CAL __float_sin
SET __ctan_s2a
LOD __ctan_ta
F_SU2 1.570796327
CAL __float_sin
SET __ctan_c2a
LOD __ctan_tb
CAL __float_exp
SET __ctan_eb
F_DIV 1.0
NOP
F_QUO
SET __ctan_emb
F_ADD __ctan_eb
F_MLT 0.5
SET __ctan_chb
LOD __ctan_eb
F_SU1 __ctan_emb
F_MLT 0.5
SET __ctan_shb
LOD __ctan_c2a
F_ADD __ctan_chb
SET __ctan_d
F_DIV __ctan_s2a
NOP
F_QUO
P_LOD __ctan_d
F_DIV __ctan_shb
NOP
F_QUO
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
