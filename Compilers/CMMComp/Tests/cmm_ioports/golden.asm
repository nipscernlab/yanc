NOP
#PRNAME cmm_ioports
#NUBITS 16
#NUIOIN 1
#NUIOOU 32
@main LOD 0
SET main_k
@__Lwh1 INN 0
OUT 31
INN 0
OUT 9
LOD main_k
ADD 1
SET main_k
LES 2
JIZ __Lwh1
@__Lwh1end @fim JMP fim
