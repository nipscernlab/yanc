NOP
#PRNAME Seno
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE main_y seno_LUT_x
#SHARE main_t seno_LUT_idxf
#SHARE main_e seno_LUT_x
#SHARE __sin_x seno_LUT_idxf
#SHARE __sin_q seno_LUT_x
#SHARE __sin_k seno_LUT_x
#SHARE __sin_r seno_LUT_idxf
#SHARE __sin_w seno_LUT_idx
#SHARE __sin_p seno_LUT_idxf
JMP main
@seno_LUT SET seno_LUT_x
#arrays seno_LUT_Seno_LUT 2 152 "Seno_LUT.txt"
@__Lwh1 F_ABS_M seno_LUT_x
F_LES 3.141592653589793
JIZ __Lwh1end
LOD 6.283185307
F_SGN seno_LUT_x
F_SU2 seno_LUT_x
SET seno_LUT_x
JMP __Lwh1
@__Lwh1end LOD seno_LUT_x
F_MLT 47.746482927568
F_ABS
SET seno_LUT_idxf
F2I_M seno_LUT_idxf
SET seno_LUT_idx
LDI seno_LUT_Seno_LUT
SET seno_LUT_v
LOD seno_LUT_idx
ADD 1
LDI seno_LUT_Seno_LUT
F_SU1 seno_LUT_v
P_I2F_M seno_LUT_idx
F_SU2 seno_LUT_idxf
SF_MLT
F_ADD seno_LUT_v
F_SGN seno_LUT_x
RET
@main #arrays main_x 2 1000 "sin_x.txt"
#arrays main_a 2 1000 "sin_y.txt"
LOD 0
SET main_j
@__Lwh2 LOD main_j
GRE 1000
JIZ __Lwh2end
LOD main_j
LDI main_x
F_ADD 6.283185307
CAL __float_sin
SET main_y
LOD main_j
LDI main_a
SET main_t
F_SU1 main_y
SET main_e
LOD main_j
ADD 1
SET main_j
JMP __Lwh2
@__Lwh2end LOD 0
SET main_j
@__Lwh3 LOD main_j
GRE 1000
JIZ fim
LOD main_j
LDI main_x
CAL seno_LUT
SET main_y
LOD main_j
LDI main_a
SET main_t
F_SU1 main_y
SET main_e
LOD main_j
ADD 1
SET main_j
JMP __Lwh3
@__Lwh3end @fim JMP fim

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
