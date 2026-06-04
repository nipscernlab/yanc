NOP
#PRNAME cmm_arridx
#NUBITS 16
#NDSTAC 4
#SDEPTH 4
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main #array main_v 1 8
@Lwh1 LOD 1
JIZ Lwh1end
LOD 7
SET main_a
LOD 3
SET main_b
LOD main_a
SET_V main_v 0
LOD main_b
SET_V main_v 1
LOD main_a
ADD main_b
SET_V main_v 3
LOD main_a
MLT main_b
SET_V main_v 5
LOD main_a
ADD main_b
ADD main_a
SET_V main_v 7
LOD_V main_v 0
OUT 0
LOD_V main_v 1
OUT 0
LOD_V main_v 3
OUT 0
LOD_V main_v 5
OUT 0
LOD_V main_v 7
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
