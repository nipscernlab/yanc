NOP
#PRNAME fifo_w
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main LOD 1
OUT 0
LOD 2
OUT 0
LOD 3
OUT 0
LOD 4
OUT 0
LOD 5
SET main_k
@__Lwh1 LOD main_k
GRE 9
JIZ __Lwh1end
LOD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
JMP __Lwh1
@__Lwh1end LOD 11
SET main_k
@__Lwh2 LOD main_k
GRE 15
JIZ __Lwh2end
LOD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
JMP __Lwh2
@__Lwh2end @__Lwh3 LOD 0
SET main_k
JMP __Lwh3
@__Lwh3end @fim JMP fim
