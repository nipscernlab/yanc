NOP
#PRNAME cmm_cnum_expr
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#arrayd v 3 4 4 1.0 -3.0 0.0 -5.0
#arrayd v_i 4 4 4 2.0 -4.0 2.0 6.0
#SHARE main_y put_z_i
#SHARE main_z_i put_z_i
#SHARE main_z put_z
#SHARE __aux_var put_z_i
JMP main
@put SET_P put_z_i
SET put_z
F2I_M put_z
OUT 0
F2I_M put_z_i
OUT 0
RET
@main LOD 2.0
SET main_y
LOD 4
SET main_n
LOD 1.0
SET main_w
SET main_w_i
LOD 2.0
SET main_x
LOD main_y
F_MLT 3.0
P_LOD 2.0
SET_P main_z_i
SET main_z
P_LOD main_z_i
CAL put
NEG_M 1
ADD main_n
I2F
P_LOD 2.0
SET_P main_z_i
SET main_z
P_LOD main_z_i
CAL put
LOD 3.0
F_SU2 main_w
P_LOD main_w_i
SET_P __aux_var
P_LOD __aux_var
F_ADD 4.0
SET_P main_z_i
SET main_z
P_LOD main_z_i
CAL put
LOD 3.0
SET main_z
LOD -2.0
SET main_z_i
LOD main_z
P_LOD main_z_i
CAL put
LOD -3.0
SET main_z
LOD 2.0
SET main_z_i
LOD main_z
P_LOD main_z_i
CAL put
LOD 0.0
SET main_z
LOD -2.0
SET main_z_i
LOD main_z
P_LOD main_z_i
CAL put
LOD -1.0
SET main_z
LOD -5.0
SET main_z_i
LOD main_z
P_LOD main_z_i
CAL put
LOD main_x
P_LOD 1.0
SET_P main_z_i
SET main_z
P_LOD main_z_i
CAL put
F_NEG_M main_x
P_LOD 1.0
SET_P main_z_i
SET main_z
P_LOD main_z_i
CAL put
LOD 0.0
F_MLT 0.0
P_LOD 2.0
F_MLT 2.0
SF_SU2
P_LOD 0.0
F_MLT 2.0
P_LOD 2.0
F_MLT 0.0
SF_ADD
SET_P main_z_i
SET main_z
P_LOD main_z_i
CAL put
LOD 0
SET main_k
@__Lwh1 LOD main_k
LDI v
P_LOD main_k
LDI v_i
CAL put
LOD main_k
ADD 1
SET main_k
LES 3
JIZ __Lwh1
@__Lwh1end @fim JMP fim
