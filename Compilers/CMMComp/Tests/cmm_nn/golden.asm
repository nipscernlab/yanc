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
#SHARE __nnk1_main __nnk0_main
#SHARE __nnk2_main __nnk0_main
#SHARE __nnk3_main main_k
#SHARE __nnk4_main __nnk0_main
#SHARE __tanh_e main_k
#SHARE __nnk10_main __nns7_main
#SHARE __nnk9_main __nnm11_main
#SHARE __nnk15_main __nns12_main
#SHARE __nnk14_main __nnm16_main
#SHARE __nnk20_main __nns17_main
#SHARE __nnk19_main main_k
#SHARE __nnb21_main main_k
#SHARE __nnm22_main __tanh_n
#SHARE __nnb24_main main_k
#SHARE __nnm25_main __tanh_n
#SHARE __nnk26_main __nnk23_main
#SHARE __nnb27_main main_k
#SHARE __nnm28_main __tanh_n
#SHARE __nnk29_main __nnk23_main
#SHARE __nnb30_main main_k
#SHARE __nnm31_main __tanh_n
#SHARE __nnk32_main __nnk23_main
#SHARE __nnb33_main main_k
#SHARE __nnm34_main __tanh_n
#SHARE __nnk35_main __nnk23_main
#SHARE __nnb36_main __tanh_n
#SHARE __nnm37_main __nnk23_main
#SHARE __nnb39_main main_x
#SHARE __nnm40_main main_k
#SHARE __nnk41_main __tanh_n
#SHARE __nnb42_main main_x
#SHARE __nnm43_main main_k
#SHARE __nnk44_main __tanh_n
#SHARE best_n main_k
#SHARE __nnb45_best __tanh_n
#SHARE __nnm46_best __nnk23_main
#SHARE __nnk47_best __nnk38_main
#SHARE __exp_x main_k
#SHARE __exp_q __tanh_n
#SHARE __exp_n __tanh_n
#SHARE __exp_r main_k
@main LOD 0
SET __nnk0_main
@__Lwh10 LOD __nnk0_main
P_LOD __nnk0_main
LDI v
F_PST
STI v
INC __nnk0_main
LES 4
JIZ __Lwh10
@__Lwh10end LOD 0
SET main_k
@__Lwh1 LOD main_k
LDI v
F_MLT 1000.0
F2I
OUT 0
INC main_k
LES 4
JIZ __Lwh1
@__Lwh1end LOD 0
SET __nnk1_main
@__Lwh11 LOD __nnk1_main
P_LOD __nnk1_main
LDI iv
PST
STI iv
INC __nnk1_main
LES 3
JIZ __Lwh11
@__Lwh11end LOD 0
SET main_k
@__Lwh2 LOD main_k
LDI iv
OUT 0
INC main_k
LES 3
JIZ __Lwh2
@__Lwh2end LOD 0
SET __nnk2_main
@__Lwh13 LOD 0
SET __nnk3_main
@__Lwh12 LOD __nnk2_main
MLT 3
ADD __nnk3_main
P_LOD __nnk2_main
MLT 3
ADD __nnk3_main
LDI m
F_PST
STI m
INC __nnk3_main
LES 2
JIZ __Lwh12
@__Lwh12end INC __nnk2_main
LES 1
JIZ __Lwh13
@__Lwh13end LOD 0
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
@__Lwh4end LOD 0
SET __nnk4_main
@__Lwh14 LOD __nnk4_main
P_LOD __nnk4_main
LDI g
F_NEG
CAL __float_exp
F_ADD 1.0
F_DIV 1.0
NOP
NOP
F_QUO
STI g
INC __nnk4_main
LES 2
JIZ __Lwh14
@__Lwh14end LOD 0
SET main_k
@__Lwh5 LOD main_k
LDI g
F_MLT 1000000.0
F2I
OUT 0
INC main_k
LES 2
JIZ __Lwh5
@__Lwh5end LOD 0
SET __nnk5_main
@__Lwh15 LOD __nnk5_main
P_LOD __nnk5_main
LDI t
F_MLT 2.0
CAL __float_exp
SET __tanh_e
F_ADD -1.0
SET __tanh_n
LOD __tanh_e
F_ADD 1.0
F_DIV __tanh_n
NOP
NOP
F_QUO
STI t
INC __nnk5_main
LES 2
JIZ __Lwh15
@__Lwh15end LOD 0
SET main_k
@__Lwh6 LOD main_k
LDI t
F_MLT 1000000.0
F2I
OUT 0
INC main_k
LES 2
JIZ __Lwh6
@__Lwh6end LOD_V s 0
SET __nnm6_main
LOD 1
SET __nnk10_main
@__Lwh18 LOD __nnk10_main
LDI s
F_LES __nnm6_main
JIZ __Lif1else
LOD __nnk10_main
LDI s
SET __nnm6_main
@__Lif1else INC __nnk10_main
LES 4
JIZ __Lwh18
@__Lwh18end LOD 0.0
SET __nns7_main
LOD 0
SET __nnk8_main
@__Lwh16 LOD __nnk8_main
P_LOD __nnk8_main
LDI s
F_SU1 __nnm6_main
CAL __float_exp
STI s
F_ADD __nns7_main
SET __nns7_main
INC __nnk8_main
LES 4
JIZ __Lwh16
@__Lwh16end LOD __nns7_main
F_DIV 1.0
NOP
NOP
F_QUO
SET __nns7_main
LOD 0
SET __nnk9_main
@__Lwh17 LOD __nnk9_main
P_LOD __nnk9_main
LDI s
F_MLT __nns7_main
STI s
INC __nnk9_main
LES 4
JIZ __Lwh17
@__Lwh17end LOD 0
SET main_k
@__Lwh7 LOD main_k
LDI s
F_MLT 1000000.0
F2I
OUT 0
INC main_k
LES 4
JIZ __Lwh7
@__Lwh7end LOD_V big 0
SET __nnm11_main
LOD 1
SET __nnk15_main
@__Lwh21 LOD __nnk15_main
LDI big
F_LES __nnm11_main
JIZ __Lif2else
LOD __nnk15_main
LDI big
SET __nnm11_main
@__Lif2else INC __nnk15_main
LES 2
JIZ __Lwh21
@__Lwh21end LOD 0.0
SET __nns12_main
LOD 0
SET __nnk13_main
@__Lwh19 LOD __nnk13_main
P_LOD __nnk13_main
LDI big
F_SU1 __nnm11_main
CAL __float_exp
STI big
F_ADD __nns12_main
SET __nns12_main
INC __nnk13_main
LES 2
JIZ __Lwh19
@__Lwh19end LOD __nns12_main
F_DIV 1.0
NOP
NOP
F_QUO
SET __nns12_main
LOD 0
SET __nnk14_main
@__Lwh20 LOD __nnk14_main
P_LOD __nnk14_main
LDI big
F_MLT __nns12_main
STI big
INC __nnk14_main
LES 2
JIZ __Lwh20
@__Lwh20end LOD 0
SET main_k
@__Lwh8 LOD main_k
LDI big
F_MLT 1000000.0
F2I
OUT 0
INC main_k
LES 2
JIZ __Lwh8
@__Lwh8end LOD_V one 0
SET __nnm16_main
LOD 1
SET __nnk20_main
@__Lwh24 LOD __nnk20_main
GRE 1
JIZ __Lwh24end
LOD __nnk20_main
LDI one
F_LES __nnm16_main
JIZ __Lif3else
LOD __nnk20_main
LDI one
SET __nnm16_main
@__Lif3else INC __nnk20_main
JMP __Lwh24
@__Lwh24end LOD 0.0
SET __nns17_main
LOD 0
SET __nnk18_main
@__Lwh22 LOD __nnk18_main
P_LOD __nnk18_main
LDI one
F_SU1 __nnm16_main
CAL __float_exp
STI one
F_ADD __nns17_main
SET __nns17_main
INC __nnk18_main
LES 0
JIZ __Lwh22
@__Lwh22end LOD __nns17_main
F_DIV 1.0
NOP
NOP
F_QUO
SET __nns17_main
LOD 0
SET __nnk19_main
@__Lwh23 LOD __nnk19_main
P_LOD __nnk19_main
LDI one
F_MLT __nns17_main
STI one
INC __nnk19_main
LES 0
JIZ __Lwh23
@__Lwh23end LOD_V one 0
F_MLT 1000.0
F2I
OUT 0
LOD 0
SET __nnb21_main
LOD_V s 0
SET __nnm22_main
LOD 1
SET __nnk23_main
@__Lwh25 LOD __nnk23_main
LDI s
F_LES __nnm22_main
JIZ __Lif4else
LOD __nnk23_main
LDI s
SET __nnm22_main
LOD __nnk23_main
SET __nnb21_main
@__Lif4else INC __nnk23_main
LES 4
JIZ __Lwh25
@__Lwh25end LOD __nnb21_main
OUT 0
LOD 0
SET __nnb24_main
LOD_V iv 0
SET __nnm25_main
LOD 1
SET __nnk26_main
@__Lwh26 LOD __nnk26_main
LDI iv
LES __nnm25_main
JIZ __Lif5else
LOD __nnk26_main
LDI iv
SET __nnm25_main
LOD __nnk26_main
SET __nnb24_main
@__Lif5else INC __nnk26_main
LES 3
JIZ __Lwh26
@__Lwh26end LOD __nnb24_main
OUT 0
LOD 0
SET __nnb27_main
LOD_V tie 0
SET __nnm28_main
LOD 1
SET __nnk29_main
@__Lwh27 LOD __nnk29_main
LDI tie
F_LES __nnm28_main
JIZ __Lif6else
LOD __nnk29_main
LDI tie
SET __nnm28_main
LOD __nnk29_main
SET __nnb27_main
@__Lif6else INC __nnk29_main
LES 3
JIZ __Lwh27
@__Lwh27end LOD __nnb27_main
OUT 0
LOD 0
SET __nnb30_main
LOD_V one 0
SET __nnm31_main
LOD 1
SET __nnk32_main
@__Lwh28 LOD __nnk32_main
GRE 1
JIZ __Lwh28end
LOD __nnk32_main
LDI one
F_LES __nnm31_main
JIZ __Lif7else
LOD __nnk32_main
LDI one
SET __nnm31_main
LOD __nnk32_main
SET __nnb30_main
@__Lif7else INC __nnk32_main
JMP __Lwh28
@__Lwh28end LOD __nnb30_main
OUT 0
LOD 0
SET __nnb33_main
LOD_V neg 0
SET __nnm34_main
LOD 1
SET __nnk35_main
@__Lwh29 LOD __nnk35_main
LDI neg
F_LES __nnm34_main
JIZ __Lif8else
LOD __nnk35_main
LDI neg
SET __nnm34_main
LOD __nnk35_main
SET __nnb33_main
@__Lif8else INC __nnk35_main
LES 2
JIZ __Lwh29
@__Lwh29end LOD __nnb33_main
OUT 0
LOD 0
SET main_k
@__Lwh9 LOD 0
SET __nnb36_main
LOD_V cnt 0
SET __nnm37_main
LOD 1
SET __nnk38_main
@__Lwh30 LOD __nnk38_main
LDI cnt
LES __nnm37_main
JIZ __Lif9else
LOD __nnk38_main
LDI cnt
SET __nnm37_main
LOD __nnk38_main
SET __nnb36_main
@__Lif9else INC __nnk38_main
LES 2
JIZ __Lwh30
@__Lwh30end LOD __nnb36_main
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
LOD 0
SET __nnb39_main
LOD_V tie 0
SET __nnm40_main
LOD 1
SET __nnk41_main
@__Lwh31 LOD __nnk41_main
LDI tie
F_LES __nnm40_main
JIZ __Lif10else
LOD __nnk41_main
LDI tie
SET __nnm40_main
LOD __nnk41_main
SET __nnb39_main
@__Lif10else INC __nnk41_main
LES 3
JIZ __Lwh31
@__Lwh31end LOD __nnb39_main
MLT 10
P_LOD 0
SET __nnb42_main
LOD_V neg 0
SET __nnm43_main
LOD 1
SET __nnk44_main
@__Lwh32 LOD __nnk44_main
LDI neg
F_LES __nnm43_main
JIZ __Lif11else
LOD __nnk44_main
LDI neg
SET __nnm43_main
LOD __nnk44_main
SET __nnb42_main
@__Lif11else INC __nnk44_main
LES 2
JIZ __Lwh32
@__Lwh32end LOD __nnb42_main
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
LOD 0
SET __nnb45_best
LOD_V best_loc 0
SET __nnm46_best
LOD 1
SET __nnk47_best
@__Lwh33 LOD __nnk47_best
LDI best_loc
F_LES __nnm46_best
JIZ __Lif12else
LOD __nnk47_best
LDI best_loc
SET __nnm46_best
LOD __nnk47_best
SET __nnb45_best
@__Lif12else INC __nnk47_best
LES 2
JIZ __Lwh33
@__Lwh33end LOD __nnb45_best
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
