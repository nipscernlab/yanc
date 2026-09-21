NOP
#PRNAME cmm_bigin
#NUBITS 32
#NDSTAC 16
#SDEPTH 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_k
@Lwh1 LOD 6
LES main_k
JIZ Lwh1end
INN 0
SET main_x
OUT 0
LOD main_k
ADD 1
SET main_k
JMP Lwh1
@Lwh1end @fim JMP fim
