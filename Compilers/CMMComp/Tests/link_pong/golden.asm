NOP
#PRNAME link_pong
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 1
SET main_um
@__Lwh1 LOD main_um
MLT main_um
PSH
@__Lcad1 CAD __Lcad1
S_ADD
@__Ltom2 TOM __Ltom2
JMP __Lwh1
@__Lwh1end @fim JMP fim
