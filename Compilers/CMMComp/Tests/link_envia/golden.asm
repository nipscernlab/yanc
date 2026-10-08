NOP
#PRNAME link_envia
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_n
@Lwh1 LOD 16
LES main_n
JIZ Lwh1end
LOD 0
SET main_k
@Lwh2 LOD main_n
LES main_k
JIZ Lwh2end
LOD main_k
ADD 1
SET main_k
JMP Lwh2
@Lwh2end LOD main_n
MLT 7
ADD 3
@Ltom1 TOM Ltom1
LOD main_n
ADD 1
SET main_n
JMP Lwh1
@Lwh1end @fim JMP fim
