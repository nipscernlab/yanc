NOP
#PRNAME cmm_order_switch
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array m 1 12
#SHARE __aux_var main_a
#SHARE main_f main_a
#SHARE main_k main_b
#SHARE __switch_exp main_a
#SHARE main_z main_a
#SHARE main_z_i main_b
@main LOD 0
SET main_a
LOD 1
SET main_b
LOD main_a
ADD 1
MLT 4
P_LOD main_b
ADD 1
S_ADD
P_LOD 7
STI m
LOD main_a
ADD 1
P_LOD main_b
ADD 1
SET_P __aux_var
MLT   4
ADD   __aux_var
PSH
LDI m
ADD 1
STI m
LOD_V m 6
OUT 0
LOD_V m 9
OUT 0
LOD 0.5
SET main_f
LOD 0
SET main_k
@__Lwh1 F2I_M main_f
ADD -1
JIZ __sw_body_1_1
ADD -1
JIZ __sw_body_1_2
JMP __sw_body_1_3
@__sw_body_1_1 LOD 10
OUT 0
JMP __switch_end_1
@__sw_body_1_2 LOD 20
OUT 0
JMP __switch_end_1
@__sw_body_1_3 LOD 99
OUT 0
@__switch_end_1 LOD main_f
F_ADD 1.0
SET main_f
LOD main_k
ADD 1
SET main_k
LES 3
JIZ __Lwh1
@__Lwh1end LOD 2.25
SET main_f
F2I_M main_f
SET __switch_exp
I2F_M __switch_exp
EQU 1.5
JIZ __sw_disp_2_1
JMP __sw_body_2_1
@__sw_disp_2_1 I2F_M __switch_exp
EQU 2.0
JIZ __sw_body_2_3
JMP __sw_body_2_2
@__sw_body_2_1 LOD 15
OUT 0
JMP __switch_end_2
@__sw_body_2_2 LOD 2
OUT 0
JMP __switch_end_2
@__sw_body_2_3 LOD 99
OUT 0
@__switch_end_2 LOD 3.5
SET main_z
LOD 1.0
SET main_z_i
F2I_M main_z
ADD -3
JIZ __sw_body_3_1
JMP __sw_body_3_2
@__sw_body_3_1 LOD 3
OUT 0
JMP fim
@__sw_body_3_2 LOD 99
OUT 0
@__switch_end_3 @fim JMP fim
