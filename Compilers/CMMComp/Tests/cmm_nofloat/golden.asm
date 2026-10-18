NOP
#PRNAME cmm_nofloat
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
@main LOD 30000
SET main_x
LOD 0
SET main_k
@__Lwh1 LOD main_x
ADD 5000
SET main_x
OUT 0
LOD main_k
ADD 1
SET main_k
LES 3
JIZ __Lwh1
@__Lwh1end LOD 3
SHR main_x
OUT 0
LOD main_x
MLT 3
OUT 0
@fim JMP fim
