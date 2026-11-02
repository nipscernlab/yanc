NOP
#PRNAME cmm_trig
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE __atan_x main_r
#SHARE __sin_x main_r
#SHARE __sin_q __atan_t
#SHARE __sin_k __atan_t
#SHARE __sin_r main_r
#SHARE __sin_w __atan_t
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

// Sine function --------------------------------------------------------------
// O(1) range reduction to [-pi/2, pi/2]: k = round(x/pi); r = x - k*pi; then
// sin(x) = (-1)^k * sin(r). sin(r) is a degree-7 odd minimax polynomial (the
// coefficients are a least-squares fit on [0, pi/2], max abs error ~1.6e-6 --
// ~6 digits, vs the old 152-point LUT's ~3). No lookup table, no new hardware.
// The parity of k is tested as soon as k is known, and each branch has its
// own copy of the polynomial: an odd k negates r before it, which gives
// -sin(r) because the polynomial is odd (w = r^2 is the same, and only the
// last product by r changes sign). Fewer cycles, a few more words.
// NB: F2I truncates toward zero, so round-to-nearest uses a sign-half bias.

@__float_sin  SET   __sin_x                  // save x
            F_MLT 0.3183098862           // q = x / pi   (1/pi)
            SET   __sin_q
            LOD   0.5
            F_SGN __sin_q                  // copysign(0.5, q)
            F_ADD __sin_q                  // q + copysign(0.5,q)
            F2I                          // k = round(q)
            SET   __sin_k
            AND   1                      // k & 1  (parity)
            JIZ   __L_sin_even             // k even -> +sin(r)

            I2F_M __sin_k                  // k odd: float(k)
            F_MLT 3.1415926536           // k * pi
            F_SU2 __sin_x                  // r = x - k*pi  -> [-pi/2, pi/2]
            F_NEG                        // -r: the polynomial gives -sin(r)
            SET   __sin_r
            F_MLT __sin_r                  // w = r^2
            SET   __sin_w
            F_MLT -0.000184472138        // Horner in w: sin(r)/r = a1 + a3 w + a5 w^2 + a7 w^3: w * a7
            F_ADD 0.008309516704         // a5
            F_MLT __sin_w
            F_ADD -0.166651680787        // a3
            F_MLT __sin_w
            F_ADD 0.999997487148         // a1
            F_MLT __sin_r                  // * r  -> sin(r)
            RET

@__L_sin_even I2F_M __sin_k              // k even: float(k)
            F_MLT 3.1415926536
            F_SU2 __sin_x                  // r
            SET   __sin_r
            F_MLT __sin_r                  // w = r^2
            SET   __sin_w
            F_MLT -0.000184472138        // the same polynomial
            F_ADD 0.008309516704
            F_MLT __sin_w
            F_ADD -0.166651680787
            F_MLT __sin_w
            F_ADD 0.999997487148
            F_MLT __sin_r
            RET
