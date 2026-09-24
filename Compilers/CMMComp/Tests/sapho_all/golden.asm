NOP
#PRNAME sapho_all
#NUBITS 32
#NDSTAC 16
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#FFTSIZ 3
#FROUND 2
#array ia 1 8
#array fa 2 8
JMP main
@twice SET twice_v
ADD twice_v
RET
@diff SET_P diff_b
SET diff_a
NEG_M diff_b
ADD diff_a
RET
@main #ITRAD
@Lwh1 LOD 0
SET s
LOD 0
SET main_w
INN 0
SET main_x
F_INN 0
SET main_fx
NEG_M 4
ADD main_x
SET main_y
NEG_M main_x
ADD 0
SET main_z
LOD main_fx
F_MLT 0.5
SET main_fy
LOD main_fx
F_SU1 10.0
SET main_fz
LOD main_x
ADD main_y
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
MLT main_y
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
DIV main_z
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
MOD main_z
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
AND main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ORR main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
XOR main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
LAN main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_w
LOR main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
SHL main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
SHR main_z
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
SRS main_z
SET main_t
LOD s
ADD main_t
SET s
LOD main_z
SGN main_y
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
LES main_x
JIZ Lif1else
LOD s
ADD 1
SET s
@Lif1else LOD main_y
GRE main_x
JIZ Lif2else
LOD s
ADD 2
SET s
@Lif2else LOD main_y
EQU main_x
JIZ Lif3else
LOD s
ADD 4
SET s
@Lif3else LOD main_x
ADD 1
P_LOD main_y
MLT 2
S_ADD
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD 1
P_LOD main_y
ADD 2
S_MLT
SET main_t
LOD s
ADD main_t
SET s
NEG_M 1
ADD main_z
P_LOD main_y
ADD 1
S_DIV
SET main_t
LOD s
ADD main_t
SET s
NEG_M 1
ADD main_z
P_LOD main_y
ADD 1
S_MOD
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD 1
P_LOD main_y
ADD 2
S_AND
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD 1
P_LOD main_y
ADD 2
S_ORR
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD 1
P_LOD main_y
ADD 2
S_XOR
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD 1
P_NEG_M 3
ADD main_y
S_LAN
SET main_t
LOD s
ADD main_t
SET s
NEG_M 7
ADD main_x
P_LOD main_y
ADD 2
S_LOR
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD 1
P_NEG_M 1
ADD main_y
S_SHL
SET main_t
LOD s
ADD main_t
SET s
NEG_M 1
ADD main_z
P_NEG_M 1
ADD main_y
S_SHR
SET main_t
LOD s
ADD main_t
SET s
NEG_M 1
ADD main_z
P_NEG_M 1
ADD main_y
S_SRS
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_NEG_M 1
ADD main_z
S_SGN
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD 1
P_LOD main_y
ADD 2
S_LES
JIZ Lif4else
LOD s
ADD 8
SET s
@Lif4else LOD main_x
ADD 1
P_LOD main_y
ADD 2
S_GRE
JIZ Lif5else
LOD s
ADD 16
SET s
@Lif5else LOD main_x
ADD 1
P_LOD main_y
ADD 5
S_EQU
JIZ Lif6else
LOD s
ADD 32
SET s
@Lif6else LOD main_x
ADD main_y
NEG
SET main_t
LOD s
ADD main_t
SET s
NEG_M main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_NEG_M main_x
NEG
S_ADD
SET main_t
LOD s
ADD main_t
SET s
NEG_M 1
ADD main_z
ABS
SET main_t
LOD s
ADD main_t
SET s
ABS_M main_z
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_ABS_M main_z
NEG
S_ADD
SET main_t
LOD s
ADD main_t
SET s
NEG_M 1
ADD main_z
PST
SET main_t
LOD s
ADD main_t
SET s
PST_M main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_PST_M main_z
NEG
S_ADD
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
MLT 1000
NRM
SET main_t
LOD s
ADD main_t
SET s
NRM_M main_z
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_NRM_M main_x
NEG
S_ADD
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD main_y
INV
SET main_t
LOD s
ADD main_t
SET s
INV_M main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_INV_M main_x
NEG
S_ADD
SET main_t
LOD s
ADD main_t
SET s
NEG_M 7
ADD main_x
LIN
SET main_t
LOD s
ADD main_t
SET s
LIN_M main_x
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_LIN_M main_x
NEG
S_ADD
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_ADD main_fy
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_MLT main_fy
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fy
F_DIV main_fx
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_SU1 main_fy
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fz
F_SGN main_fy
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_ADD 1.0
P_LOD main_fy
F_MLT 2.0
SF_ADD
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_ADD 1.0
P_LOD main_fy
F_ADD 2.0
SF_MLT
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_ADD 1.0
P_LOD main_fy
F_ADD 2.0
SF_DIV
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_ADD 1.0
P_LOD main_fy
F_ADD 2.0
SF_SU2
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_MLT 2.0
F_SU2 main_fy
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fy
F_ADD 1.0
P_LOD main_fz
F_SU1 1.0
SF_SGN
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_GRE main_fy
JIZ Lif7else
LOD s
ADD 64
SET s
@Lif7else LOD main_fx
F_LES main_fy
JIZ Lif8else
LOD s
ADD 128
SET s
@Lif8else LOD main_fx
F_ADD 1.0
P_LOD main_fy
F_ADD 2.0
SF_LES
JIZ Lif9else
LOD s
ADD 256
SET s
@Lif9else LOD main_fx
F_ADD 1.0
P_LOD main_fy
F_ADD 2.0
SF_GRE
JIZ Lif10else
LOD s
ADD 512
SET s
@Lif10else LOD main_fx
F_ADD main_fy
F_NEG
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
F_NEG_M main_fx
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fy
F_ADD 1.0
PF_NEG_M main_fx
SF_SU2
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fz
F_SU1 1.0
F_ABS
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
F_ABS_M main_fz
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fy
F_ADD 1.0
PF_ABS_M main_fz
SF_SU2
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fz
F_SU1 1.0
F_PST
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
F_PST_M main_fx
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fy
F_ADD 1.0
PF_PST_M main_fz
SF_SU2
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
ADD main_y
I2F
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
I2F_M main_x
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fy
F_ADD 1.0
P_I2F_M main_x
SF_SU2
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fy
F_ADD 1.0
P_I2F_M main_x
SF_SU1
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_fx
F_ADD main_fy
F2I
SET main_t
LOD s
ADD main_t
SET s
F2I_M main_fy
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
I2F
F_SU1 main_fy
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_F2I_M main_fy
LIN
NEG
S_ADD
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
ADD 1
P_INN 0
NEG
S_ADD
SET main_t
LOD s
ADD main_t
SET s
LOD main_fy
F_ADD 1.0
PF_INN 0
SF_SU2
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
P_LOD main_x
STI ia
LOD main_y
LDI ia
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
P_LOD main_z
ISI ia
LOD main_y
ILI ia
SET main_t
LOD s
ADD main_t
SET s
LOD main_y
P_LOD main_fx
STI fa
LOD main_y
LDI fa
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
CAL twice
SET main_t
LOD s
ADD main_t
SET s
LOD main_x
P_LOD main_y
CAL diff
SET main_t
LOD s
ADD main_t
SET s
LOD 2.0
P_LOD 1.0
P_LOD 1.0
P_LOD 0.0
SET_P aux_var 
SET_P aux_var1
SET_P aux_var2
F_ADD aux_var1
P_LOD aux_var2
F_ADD aux_var 
POP
F2I
LDI ia
SET main_t
LOD s
ADD main_t
SET s
LOD 4.0
CAL float_sqrt
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD 0.0
CAL float_exp
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD 1.0
CAL float_log
SET main_fw
F_MLT 4194304.0
F2I
SET main_t
LOD s
ADD main_t
SET s
LOD 16
SHR s
XOR s
SET main_t
LOD 8
SHR main_t
XOR main_t
SET main_t
LOD 4
SHR main_t
XOR main_t
SET main_t
LOD 15
AND main_t
OUT 0
#TOAQUI
JMP Lwh1
@Lwh1end @fim JMP fim

// sqrt function --------------------------------------------------------------

@float_sqrt     SET sqrt_num       // get the parameter
              F_ROT                // first estimate (nearest power of 2)

                PSH                // update x
              F_DIV sqrt_num       // iteration 1
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 2
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 3
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 4
             SF_ADD
              F_MLT 0.5

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

// Natural logarithm ----------------------------------------------------------
// log(x) = e*ln2 + log(m),  e = XPO(x) = floor(log2 x),  m = x*2^-e in [1,2).
// O(1): e via XPO, m via a single F_SCL (no halve/double loop). log(m) is the
// atanh series 2u(1 + u^2/3 + u^4/5 + u^6/7 + u^8/9), u = (m-1)/(m+1).
// x <= 0 returns 0.

@float_log  SET   log_x                 // save x (acc still = x)
            F_LES 0.0                    // (x > 0) ?   [F_LES true when acc > X]
            JIZ   L_log_zero             // x <= 0 -> bail out with 0

            LOD   log_x
            XPO                          // e = floor(log2 x)   (int)
            SET   log_e
            NEG                          // -e
            SET   log_ne
            LOD   log_x
            F_SCL log_ne                 // m = x * 2^(-e)  -> [1,2)
            SET   log_m

            F_ADD 1.0                    // m+1   (acc still = m)
            SET   log_t2
            LOD   log_m
            F_SU1 1.0                    // m-1   (acc - X)
            SET   log_t1
            LOD   log_t2
            F_DIV log_t1                 // u = (m-1)/(m+1)   [F_DIV X = X/acc]
            SET   log_u
            F_MLT log_u
            SET   log_w                  // w = u^2

            LOD   0.1111111111           // Horner in w (1/9, 1/7, 1/5, 1/3, 1)
            F_MLT log_w
            F_ADD 0.1428571429
            F_MLT log_w
            F_ADD 0.2
            F_MLT log_w
            F_ADD 0.3333333333
            F_MLT log_w
            F_ADD 1.0
            F_MLT log_u
            F_MLT 2.0                     // log(m)
            SET   log_lm

            LOD   log_e
            I2F                           // float(e)
            F_MLT 0.6931471806            // e * ln2
            F_ADD log_lm                  // + log(m)  -> log(x)
            RET

@L_log_zero LOD   0.0
            RET
