NOP
#PRNAME cmm_ilit8
#NUBITS 8
#NUIOIN 1
#NUIOOU 1
#arrayd c 1 2 2 -1 -128
@main NEG_M 1
OUT 0
NEG_M 128
OUT 0
LOD 127
OUT 0
LOD_V c 0
OUT 0
LOD_V c 1
OUT 0
LOD 3
SET main_x
NEG_M 5
ADD main_x
OUT 0
@__Lwh1 JMP __Lwh1
@__Lwh1end @fim JMP fim
