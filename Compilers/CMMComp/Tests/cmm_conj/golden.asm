NOP
#PRNAME cmm_conj
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE main_z main_c_i
#SHARE main_z_i main_c
@main @__Lwh1 LOD 3.0
PF_NEG_M 4.0
SET_P main_c_i
SET main_c
F2I_M main_c
OUT 0
F2I_M main_c_i
OUT 0
LOD 1.0
PF_NEG_M -2.0
SET_P main_c_i
SET main_c
F2I_M main_c
OUT 0
F2I_M main_c_i
OUT 0
LOD 2.0
SET main_z
LOD 5.0
SET main_z_i
LOD main_z
PF_NEG_M main_z_i
SET_P main_c_i
SET main_c
F2I_M main_c
OUT 0
F2I_M main_c_i
OUT 0
LOD 1.0
SET main_z
LOD 1.0
SET main_z_i
LOD 1.0
SET main_w
LOD 2.0
SET main_w_i
LOD main_z
F_ADD main_w
P_LOD main_z_i
F_ADD main_w_i
F_NEG
SET_P main_c_i
SET main_c
F2I_M main_c
OUT 0
F2I_M main_c_i
OUT 0
LOD 7.0
P_LOD 0.0
SET_P main_c_i
SET main_c
F2I_M main_c
OUT 0
F2I_M main_c_i
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
