NOP
#PRNAME lk_count7
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_n
@__Lwh1 LOD 16
LES main_n
JIZ __Lwh1end
LOD main_n
MLT 7
TOM 0
LOD main_n
ADD 1
SET main_n
JMP __Lwh1
@__Lwh1end @fim JMP fim
