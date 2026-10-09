NOP
#PRNAME cmm_trig
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE __atan_x main_r
#SHARE __atan_t __atan_ax
#SHARE __atan_p __atan_ax
#SHARE __sin_x __atan_big
#SHARE __sin_q main_r
#SHARE __sin_k main_r
#SHARE __sin_r __atan_big
#SHARE __sin_w __atan_ax
#SHARE __sin_p __atan_big
@main @__Lwh1 LOD 0.0
CAL __float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 1.5707963
CAL __float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 0.0
F_SU2 1.570796327
CAL __float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 100.0
CAL __float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 3.1415927
F_SU2 1.570796327
CAL __float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 1.0
CAL __float_atan
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 2.0
CAL __float_atan
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD -1.0
CAL __float_atan
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim

// Arctangent function --------------------------------------------------------
// |x| is folded into [0,1] with the 1/x identity: atan(|x|) = pi/2 - atan(1/|x|)
// for |x|>1. atan(t) on [0,1] is a degree-11 odd minimax polynomial (least-
// squares fit, max abs error ~4.9e-6 -- ~6 digits, vs the old 49-point LUT's
// ~4e-5). One shared polynomial for both branches; no table, no new hardware.

@__float_atan SET   __atan_x                 // save x
            F_ABS_M __atan_x               // ax = |x|
            SET   __atan_ax
            F_LES 1.0                    // (ax > 1)?  [F_LES true when acc > X]
            SET   __atan_big               // flag: 1 if |x|>1 (use 1/x)
            JIZ   __L_atan_small
            LOD   __atan_ax                // big branch: t = 1/ax
            F_DIV 1.0                    // 1.0 / ax   (F_DIV X = X/acc)
NOP
F_QUO
            JMP   __L_atan_haveT
@__L_atan_small LOD __atan_ax               // small branch: t = ax
@__L_atan_haveT SET __atan_t

            F_MLT __atan_t                 // w = t^2
            SET   __atan_w
            LOD  -0.012300666580         // Horner in w: atan(t)/t = a1 + a3 w + ... + a11 w^5
            F_MLT __atan_w                 // a11
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
            SET   __atan_p

            LOD   __atan_big
            JIZ   __L_atan_done            // |x|<=1 -> result = atan(t)
            LOD   __atan_p                 // |x|>1  -> result = pi/2 - atan(1/x)
            F_SU2 1.5707963268           // pi/2 - p   (F_SU2 X = X - acc)
            JMP   __L_atan_sign
@__L_atan_done LOD __atan_p
@__L_atan_sign F_SGN __atan_x                // apply the sign of x
            RET

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
