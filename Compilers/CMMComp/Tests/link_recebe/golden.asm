NOP
#PRNAME link_recebe
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE main_v main_k
@main LOD 0
SET main_n
@__Lwh1 LOD 16
LES main_n
JIZ __Lwh1end
LOD 0
SET main_k
@__Lwh2 NEG_M main_n
ADD 16
LES main_k
JIZ __Lwh2end
LOD main_k
ADD 1
SET main_k
JMP __Lwh2
@__Lwh2end CAD 0
SET main_v
OUT 0
LOD main_n
ADD 1
SET main_n
JMP __Lwh1
@__Lwh1end @fim JMP fim
