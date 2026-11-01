NOP
#PRNAME lk_chain_a
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_n
@__Lwh1 LOD main_n
GRE 16
JIZ fim
LOD main_n
MLT 3
ADD 1
TOM 0
LOD main_n
ADD 1
SET main_n
JMP __Lwh1
@__Lwh1end @fim JMP fim
#LKSYNC 0
