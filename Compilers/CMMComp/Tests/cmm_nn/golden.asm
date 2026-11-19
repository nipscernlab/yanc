NOP
#PRNAME cmm_nn
#NUIOOU 1
#arrayd v 2 5 5 -2.0 -0.5 0.0 0.75 3.0
#arrayd iv 1 4 4 -3 5 -1 2
#arrayd m 2 6 6 -1.0 2.0 -3.0 4.0 -5.0 6.0
#arrayd g 2 3 3 -1.0 0.0 1.0
#arrayd t 2 3 3 -1.0 0.0 1.0
#arrayd s 2 5 5 1.0 2.0 3.0 2.0 1.0
#arrayd big 2 3 3 200.0 199.0 0.0
#arrayd one 2 1 1 -3.0
#arrayd tie 2 4 4 1.0 5.0 5.0 -2.0
#arrayd neg 2 3 3 -4.0 -1.0 -2.0
#arrayd cnt 1 3 3 0 0 9
#SHARE __exp_n __exp_q
#SHARE __exp_r __exp_x
#SHARE __nn_te __exp_x
#SHARE __nn_tn __exp_q
#SHARE __nn_x __exp_x
#SHARE __nn_b __exp_q
@main LEA v
SET __nn_p
ADD 5
CAL __nn_relu_f
LOD 0
SET main_k
@__Lwh1 LOD main_k
LDI v
F_MLT 1000.0
F2I
OUT 0
INC main_k
LES 4
JIZ __Lwh1
@__Lwh1end LEA iv
SET __nn_p
ADD 4
CAL __nn_relu_i
LOD 0
SET main_k
@__Lwh2 LOD main_k
LDI iv
OUT 0
INC main_k
LES 3
JIZ __Lwh2
@__Lwh2end LEA m
SET __nn_p
ADD 6
CAL __nn_relu_f
LOD 0
SET main_k
@__Lwh3 LOD 0
MLT 3
ADD main_k
LDI m
F_MLT 1000.0
F2I
OUT 0
INC main_k
LES 2
JIZ __Lwh3
@__Lwh3end LOD 0
SET main_k
@__Lwh4 LOD 1
MLT 3
ADD main_k
LDI m
F_MLT 1000.0
F2I
OUT 0
INC main_k
LES 2
JIZ __Lwh4
@__Lwh4end LEA g
SET __nn_p
ADD 3
CAL __nn_sigmoid
LOD 0
SET main_k
@__Lwh5 LOD main_k
LDI g
F_MLT 1000000.0
F2I
OUT 0
INC main_k
LES 2
JIZ __Lwh5
@__Lwh5end LEA t
SET __nn_p
ADD 3
CAL __nn_tanh
LOD 0
SET main_k
@__Lwh6 LOD main_k
LDI t
F_MLT 1000000.0
F2I
OUT 0
INC main_k
LES 2
JIZ __Lwh6
@__Lwh6end LEA s
SET __nn_p
ADD 5
CAL __nn_softmax
LOD 0
SET main_k
@__Lwh7 LOD main_k
LDI s
F_MLT 1000000.0
F2I
OUT 0
INC main_k
LES 4
JIZ __Lwh7
@__Lwh7end LEA big
SET __nn_p
ADD 3
CAL __nn_softmax
LOD 0
SET main_k
@__Lwh8 LOD main_k
LDI big
F_MLT 1000000.0
F2I
OUT 0
INC main_k
LES 2
JIZ __Lwh8
@__Lwh8end LEA one
SET __nn_p
ADD 1
CAL __nn_softmax
LOD_V one 0
F_MLT 1000.0
F2I
OUT 0
LEA s
SET __nn_p
ADD 5
CAL __nn_argmax_f
OUT 0
LEA iv
SET __nn_p
ADD 4
CAL __nn_argmax_i
OUT 0
LEA tie
SET __nn_p
ADD 4
CAL __nn_argmax_f
OUT 0
LEA one
SET __nn_p
ADD 1
CAL __nn_argmax_f
OUT 0
LEA neg
SET __nn_p
ADD 3
CAL __nn_argmax_f
OUT 0
LOD 0
SET main_k
@__Lwh9 LEA cnt
SET __nn_p
ADD 3
CAL __nn_argmax_i
XOR 0
JIZ __Lwh9end
LOD_V cnt 0
ADD 5
SET_V cnt 0
INC main_k
JMP __Lwh9
@__Lwh9end LOD main_k
OUT 0
LOD 10
CAL best
OUT 0
LEA tie
SET __nn_p
ADD 4
CAL __nn_argmax_f
MLT 10
PSH
LEA neg
SET __nn_p
ADD 3
CAL __nn_argmax_f
S_ADD
OUT 0
LOD 0.5
SET main_x
F_NEG
CAL __float_exp
F_ADD 1.0
F_DIV 1.0
NOP
NOP
F_QUO
F_MLT 1000000.0
F2I
OUT 0
F_NEG_M main_x
F_PST
F_MLT 1000.0
F2I
OUT 0
NEG_M 7
PST
OUT 0
PST_M 7
OUT 0
@fim JMP fim
@best SET best_n
#array best_loc 2 3
LOD 0.5
SET_V best_loc 0
LOD -0.5
SET_V best_loc 1
LOD 2.5
SET_V best_loc 2
LEA best_loc
SET __nn_p
ADD 3
CAL __nn_argmax_f
MLT best_n
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
            F_MLT 0.0013888889          // Horner, exp(r) = sum r^k/k! (k=0..6): r * 1/6!
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
// relu on a float array, in place: v[k] = v[k], or 0 when negative --------------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// One copy for every call of the program, in C± and in C++.

@__nn_relu_f    SET   __nn_e                // end
@__nn_relu_f_t  LOD   __nn_p
                PSH                         // the address, for STI
                LDI   0                     // v[k]
                F_PST                       // 0 when negative
                STI   0
                INC __nn_p
                GRE   __nn_e                // more to go while end > p
                JNZ   __nn_relu_f_t
                RET
// relu on an int array, in place: v[k] = v[k], or 0 when negative ---------------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.

@__nn_relu_i    SET   __nn_e                // end
@__nn_relu_i_t  LOD   __nn_p
                PSH                         // the address, for STI
                LDI   0                     // v[k]
                PST                         // 0 when negative
                STI   0
                INC __nn_p
                GRE   __nn_e                // more to go while end > p
                JNZ   __nn_relu_i_t
                RET
// sigmoid on a float array, in place: v[k] = 1 / (1 + exp(-v[k])) ---------------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// Needs float_exp.asm.

@__nn_sigmoid   SET   __nn_e                // end
@__nn_sigmoid_t LOD   __nn_p
                PSH                         // the address, for STI
                LDI   0                     // x = v[k]
                F_NEG
                CAL   __float_exp           // exp(-x)
                F_ADD 1.0
                F_DIV 1.0                   // 1 / (1 + exp(-x))   (F_DIV X = X / acc)
NOP
NOP
F_QUO
                STI   0
                INC __nn_p
                GRE   __nn_e                // more to go while end > p
                JNZ   __nn_sigmoid_t
                RET
// tanh on a float array, in place: v[k] = (exp(2x) - 1) / (exp(2x) + 1) ----------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// Needs float_exp.asm.

@__nn_tanh      SET   __nn_e                // end
@__nn_tanh_t    LOD   __nn_p
                PSH                         // the address, for STI
                LDI   0                     // x = v[k]
                F_MLT 2.0
                CAL   __float_exp           // exp(2x)
                SET   __nn_te
                F_ADD -1.0
                SET   __nn_tn               // exp(2x) - 1
                LOD   __nn_te
                F_ADD 1.0                   // exp(2x) + 1
                F_DIV __nn_tn               // (F_DIV X = X / acc)
NOP
NOP
F_QUO
                STI   0
                INC __nn_p
                GRE   __nn_e                // more to go while end > p
                JNZ   __nn_tanh_t
                RET
// softmax on a float vector, in place: v[k] = exp(v[k] - max) / sum -------------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// The maximum goes first: the SAPHO float has no infinity, and exp passes the
// largest float above about 103. One division, then a product per element.
// Needs float_exp.asm.

@__nn_softmax   SET   __nn_e                // end
                LOD   __nn_p
                SET   __nn_a                // the first element's address
                LDI   0
                SET   __nn_m                // m = v[0]
@__nn_softmax_m INC __nn_p // m = the largest element
                GRE   __nn_e
                JIZ   __nn_softmax_e
                LOD   __nn_p
                LDI   0
                SET   __nn_x
                F_LES __nn_m                // m < v[k]
                JIZ   __nn_softmax_m
                LOD   __nn_x
                SET   __nn_m
                JMP   __nn_softmax_m
@__nn_softmax_e LOD   0.0                   // v[k] = exp(v[k] - m), s = their sum
                SET   __nn_s
                LOD   __nn_a
                SET   __nn_p
@__nn_softmax_x LOD   __nn_p
                PSH
                LDI   0
                F_SU1 __nn_m                // v[k] - m
                CAL   __float_exp
                STI   0                     // keeps the acc
                F_ADD __nn_s
                SET   __nn_s
                INC __nn_p
                GRE   __nn_e
                JNZ   __nn_softmax_x
                LOD   __nn_s
                F_DIV 1.0                   // 1 / s   (F_DIV X = X / acc)
NOP
NOP
F_QUO
                SET   __nn_s
                LOD   __nn_a
                SET   __nn_p
@__nn_softmax_d LOD   __nn_p                // v[k] = v[k] / s
                PSH
                LDI   0
                F_MLT __nn_s
                STI   0
                INC __nn_p
                GRE   __nn_e
                JNZ   __nn_softmax_d
                RET
// argmax of a float vector: the index of the largest element, in the acc --------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// The first of equal elements stays: only a larger one moves the index.

@__nn_argmax_f   SET   __nn_e                // end
                LOD   __nn_p
                SET   __nn_a                // the first element's address
                SET   __nn_b                // the largest one's
                LDI   0
                SET   __nn_m                // m = v[0]
@__nn_argmax_f_t INC __nn_p
                GRE   __nn_e
                JIZ   __nn_argmax_f_e
                LOD   __nn_p
                LDI   0
                SET   __nn_x
                F_LES __nn_m                // m < v[k]
                JIZ   __nn_argmax_f_t
                LOD   __nn_x
                SET   __nn_m
                LOD   __nn_p
                SET   __nn_b
                JMP   __nn_argmax_f_t
@__nn_argmax_f_e LOD   __nn_a                // b - a, the index
                NEG
                ADD   __nn_b
                RET
// argmax of a int vector: the index of the largest element, in the acc --------
// docs/design/nn-functions.md. Called with __nn_p = the address of the first
// element and the acc = the address after the last; at least one element.
// The first of equal elements stays: only a larger one moves the index.

@__nn_argmax_i   SET   __nn_e                // end
                LOD   __nn_p
                SET   __nn_a                // the first element's address
                SET   __nn_b                // the largest one's
                LDI   0
                SET   __nn_m                // m = v[0]
@__nn_argmax_i_t INC __nn_p
                GRE   __nn_e
                JIZ   __nn_argmax_i_e
                LOD   __nn_p
                LDI   0
                SET   __nn_x
                LES   __nn_m                // m < v[k]
                JIZ   __nn_argmax_i_t
                LOD   __nn_x
                SET   __nn_m
                LOD   __nn_p
                SET   __nn_b
                JMP   __nn_argmax_i_t
@__nn_argmax_i_e LOD   __nn_a                // b - a, the index
                NEG
                ADD   __nn_b
                RET
