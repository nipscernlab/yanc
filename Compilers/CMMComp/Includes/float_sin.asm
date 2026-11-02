
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
