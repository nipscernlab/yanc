NOP
#PRNAME cmm_decl_list
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE main_x soma_n
#SHARE main_w soma_s
#SHARE gf ga
#SHARE gw_i gg
#SHARE gw gz
#SHARE main_a gg
#SHARE main_y main_b
#SHARE main_z_i soma_n
#SHARE main_z soma_s
LOD 3
SET ga
ADD 4
SET gc
LOD 2.5
SET gg
LOD 1.0
SET gz
LOD 2.0
SET gz_i
JMP main
@soma LOD 0
SET soma_n
SET soma_s
@__Lwh1 LOD 3
LES soma_n
JIZ __Lwh1end
LOD soma_n
ADD 1
SET soma_n
LOD soma_s
ADD soma_n
SET soma_s
JMP __Lwh1
@__Lwh1end LOD soma_s
RET
@main LOD 5
SET main_b
MLT 2
SET main_c
LOD 0.5
SET main_x
LOD 3.0
SET main_w
LOD -1.0
SET main_w_i
LOD 1
SET gb
LOD ga
OUT 0
LOD gb
OUT 0
LOD gc
OUT 0
LOD gg
F_MLT 2.0
SET gf
F_MLT 4.0
F2I
OUT 0
LOD gg
F_MLT 4.0
F2I
OUT 0
LOD 2.0
F_MLT gz
P_LOD 2.0
F_MLT gz_i
SET_P gw_i
SET gw
F2I_M gw
OUT 0
F2I_M gw_i
OUT 0
NEG_M main_b
ADD main_c
SET main_a
OUT 0
LOD main_b
OUT 0
LOD main_c
OUT 0
LOD main_x
F_ADD 1.0
SET main_y
LOD main_x
F_MLT 4.0
F2I
OUT 0
LOD main_y
F_MLT 4.0
F2I
OUT 0
LOD 1.0
F_ADD main_w
P_LOD main_w_i
SET_P main_z_i
SET main_z
F2I_M main_z
OUT 0
F2I_M main_z_i
OUT 0
CAL soma
OUT 0
CAL soma
OUT 0
@fim JMP fim
