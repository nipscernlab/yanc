NOP
#PRNAME cmm_reorder
#NUBITS 16
#NDSTAC 4
#SDEPTH 4
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 7
SET main_a
LOD 3
SET main_b
LOD 5
SET main_c
LOD 11
SET main_d
LOD 2
SET main_e
LOD 13
SET main_f
LOD 4
SET main_g
LOD 6
SET main_h
LOD main_c
ADD main_d
P_LOD main_e
ADD main_f
S_ADD
P_LOD main_a
ADD main_b
S_ADD
OUT 0
LOD main_a
ADD main_b
P_LOD main_c
ADD main_d
S_MLT
P_LOD main_e
ADD main_f
P_LOD main_g
ADD main_h
S_MLT
S_ADD
OUT 0
LOD main_c
MLT main_d
P_LOD main_e
MLT main_f
S_ADD
P_LOD main_a
MLT main_b
S_ADD
OUT 0
LOD main_a
ADD main_b
ADD main_c
ADD main_d
ADD main_e
ADD main_f
ADD main_g
ADD main_h
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
