NOP
#PRNAME link_recebe
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_n
@__Lwh1top LOD 0
SET main_k
@__Lwh2 NEG_M main_n
ADD 16
LES main_k
JIZ __Lwh2end
INC main_k
JMP __Lwh2
@__Lwh2end CAD 0
OUT 0
INC main_n
LES 15
JIZ __Lwh1top
@__Lwh1end @fim JMP fim
