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
@Lwh1 LOD main_um
MLT main_um
PSH
@Lcad1 CAD Lcad1
S_ADD
@Ltom2 TOM Ltom2
JMP Lwh1
@Lwh1end @fim JMP fim
