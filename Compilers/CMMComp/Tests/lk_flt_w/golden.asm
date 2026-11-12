NOP
#PRNAME lk_flt_w
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_n
@__Lwh1top I2F_M main_n
F_MLT 1.5
TOM 0
INC main_n
LES 15
JIZ __Lwh1top
@__Lwh1end @fim JMP fim
