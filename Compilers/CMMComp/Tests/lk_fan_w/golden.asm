NOP
#PRNAME lk_fan_w
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_n
@__Lwh1top LOD main_n
TOM 0
ADD 100
TOM 1
INC main_n
LES 15
JIZ __Lwh1top
@__Lwh1end @fim JMP fim
