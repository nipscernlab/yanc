NOP
#PRNAME lk_merge
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main @__Lwh1 CAD 0
SET main_a
CAD 1
SET main_b
LOD main_a
MLT 1000
ADD main_b
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
