NOP
#PRNAME cmm_conj
#NUBITS 32
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 3.000000
PF_NEG_M 4.000000
SET_P main_c_i
SET main_c
F2I
OUT 0
LOD main_c_i
F2I
OUT 0
LOD 1.000000
PF_NEG_M -2.000000
SET_P main_c_i
SET main_c
F2I
OUT 0
LOD main_c_i
F2I
OUT 0
LOD 2.000000
SET main_z
LOD 5.000000
SET main_z_i
LOD main_z
PF_NEG_M main_z_i
SET_P main_c_i
SET main_c
F2I
OUT 0
LOD main_c_i
F2I
OUT 0
LOD 1.000000
SET main_z
LOD 1.000000
SET main_z_i
LOD 1.000000
SET main_w
LOD 2.000000
SET main_w_i
LOD main_z
F_ADD main_w
P_LOD main_z_i
F_ADD main_w_i
F_NEG
SET_P main_c_i
SET main_c
F2I
OUT 0
LOD main_c_i
F2I
OUT 0
LOD 7.0
P_LOD 0.0
SET_P main_c_i
SET main_c
F2I
OUT 0
LOD main_c_i
F2I
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
