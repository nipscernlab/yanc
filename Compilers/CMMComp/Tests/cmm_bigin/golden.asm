NOP
#PRNAME cmm_bigin
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_k
@__Lwh1 LOD main_k
GRE 6
JIZ fim
INN 0
SET main_x
OUT 0
LOD main_k
ADD 1
SET main_k
JMP __Lwh1
@__Lwh1end @fim JMP fim
