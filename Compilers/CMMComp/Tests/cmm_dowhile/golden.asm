NOP
#PRNAME cmm_dowhile
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @__Lwh1 LOD 10
SET main_k
@__Lwh2 LOD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
LOD 5
LES main_k
JIZ __Lwh2end
JMP __Lwh2
@__Lwh2end LOD 0
SET main_k
@__Lwh3 LOD 20
ADD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
LOD 3
LES main_k
JIZ __Lwh3end
JMP __Lwh3
@__Lwh3end LOD 0
SET main_k
@__Lwh4 LOD 2
XOR main_k
JIZ __Lwh4end
LOD 30
ADD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
LOD 10
LES main_k
JIZ __Lwh4end
JMP __Lwh4
@__Lwh4end LOD 0
SET main_k
@__Lwh5 LOD main_k
ADD 1
SET main_k
LOD 3
XOR main_k
JIZ __Lwh5cont
LOD 40
ADD main_k
OUT 0
@__Lwh5cont LOD 5
LES main_k
JIZ __Lwh5end
JMP __Lwh5
@__Lwh5end LOD 0
SET main_k
@__Lwh6 LOD 0
SET main_n
@__Lwh7 LOD 2
XOR main_n
JIZ __Lwh7end
LOD main_k
MLT 10
ADD 50
ADD main_n
OUT 0
LOD main_n
ADD 1
SET main_n
LOD 5
LES main_n
JIZ __Lwh7end
JMP __Lwh7
@__Lwh7end LOD main_k
ADD 1
SET main_k
LOD 3
LES main_k
JIZ __Lwh6end
JMP __Lwh6
@__Lwh6end JMP __Lwh1
@__Lwh1end @fim JMP fim
