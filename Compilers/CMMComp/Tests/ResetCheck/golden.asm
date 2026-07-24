NOP
#PRNAME ResetCheck
#NUBITS 16
#NDSTAC 4
#SDEPTH 4
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main LOD 3
SET main_a
LOD 40
SET main_b
LOD main_a
OUT 0
LOD main_a
ADD main_b
SET main_a
OUT 0
NEG_M main_a
ADD main_b
SET main_b
OUT 0
LOD main_a
MLT main_b
OUT 0
@Lwh1 LOD 1
JIZ Lwh1end
LOD main_a
SET main_b
JMP Lwh1
@Lwh1end @fim JMP fim
