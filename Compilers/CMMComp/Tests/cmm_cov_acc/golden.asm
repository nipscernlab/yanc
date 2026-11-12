NOP
#PRNAME cmm_cov_acc
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array iv 1 3
#array fv 2 3
#SHARE main_z main_q
#SHARE main_z_i main_f
#SHARE __aux_var main_r_i
#SHARE __aux_var1 main_r
#SHARE __dhf1_main main_b
#SHARE __dk0_main main_a
#SHARE __dhi1_main main_b
@main LOD 7
SET main_a
LOD 3
SET main_b
LOD 17
SET main_p
LOD 5
SET main_q
LOD 2.5
SET main_f
LOD main_a
MLT main_b
P_LOD main_q
DIV main_p
NOP
NOP
NOP
QUO
S_ADD
OUT 0
LOD main_a
MLT main_b
P_LOD main_q
MOD main_p
NOP
NOP
NOP
REM
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD 7
SET main_a
LOD 3
SET main_b
NEG_M 17
SET main_p
LOD 5
SET main_q
LOD 9.75
SET main_f
LOD main_a
MLT main_b
P_LOD main_q
DIV main_p
NOP
NOP
NOP
QUO
S_ADD
OUT 0
LOD main_a
MLT main_b
P_LOD main_q
MOD main_p
NOP
NOP
NOP
REM
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD 4
SET main_a
LOD 2
SET main_b
LOD 3
SET main_p
LOD 2
SET main_q
LOD 3.0
SET main_f
LOD main_a
MLT main_b
P_LOD main_q
DIV main_p
NOP
NOP
NOP
QUO
S_ADD
OUT 0
LOD main_a
MLT main_b
P_LOD main_q
MOD main_p
NOP
NOP
NOP
REM
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD 1.5
SET main_z
LOD 2.0
SET main_z_i
LOD 1.5
SET main_w
LOD 2.0
SET main_w_i
LOD 40
SET main_p
LOD 2.0
SET main_g
LOD 7
SET main_a
LOD 3
SET main_b
LOD main_z_i
F_MLT -2.0
P_LOD main_z
F_MLT 2.0
SET_P main_r_i
SET main_r
LOD 4.0
F_MLT main_r
F2I
OUT 0
LOD 4.0
F_MLT main_r_i
F2I
OUT 0
LOD main_z
F_ADD main_w
P_LOD main_z_i
F_ADD main_w_i
P_LOD main_z_i
F_MLT -2.0
P_LOD main_z
F_MLT 2.0
SET_P __aux_var
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var
SET_P main_r_i
SET main_r
LOD 4.0
F_MLT main_r
F2I
OUT 0
LOD 4.0
F_MLT main_r_i
F2I
OUT 0
LOD main_z
EQU main_w
P_LOD main_z_i
EQU main_w_i
S_AND
OUT 0
LOD main_a
MLT main_b
P_LOD main_a
MLT main_b
S_ADD
P_LOD main_z
EQU main_w
P_LOD main_z_i
EQU main_w_i
S_AND
S_ADD
OUT 0
I2F_M main_a
EQU main_z
P_LOD 0.0
EQU main_z_i
S_AND
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_a
EQU main_z
P_LOD 0.0
EQU main_z_i
S_AND
S_ADD
OUT 0
F2I_M main_g
SHL main_p
OUT 0
LOD main_a
MLT main_b
P_F2I_M main_g
SHL main_p
S_ADD
OUT 0
LOD 2
JIZ __sw_body_1_1
ADD -2
JIZ __sw_body_1_2
ADD -1
JIZ __sw_body_1_3
JMP __switch_end_1
@__sw_body_1_1 LOD 1
OUT 0
JMP __switch_end_1
@__sw_body_1_2 LOD 2
OUT 0
JMP __switch_end_1
@__sw_body_1_3 LOD 3
OUT 0
@__switch_end_1 LOD 2.5
SET main_f
LOD 2
SET main_q
LOD 0.0
F_MLT main_f
SET __dhf1_main
LOD -3
SET __dk0_main
@__Ld0 P_F2I_M __dhf1_main
STI_V iv 3
INC __dk0_main
JNZ __Ld0
@__Ld0e LOD_V iv 1
OUT 0
LOD 0
MLT main_q
SET __dhi1_main
LOD -3
SET __dk0_main
@__Ld1 P_I2F_M __dhi1_main
STI_V fv 3
INC __dk0_main
JNZ __Ld1
@__Ld1e LOD_V fv 1
F2I
OUT 0
@fim JMP fim
