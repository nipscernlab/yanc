NOP
#PRNAME cmm_empty
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
@main INN 0
SET main_a
LES 2
JIZ __Lif1else
JMP __Lif1end
@__Lif1else LOD 7
OUT 0
@__Lif1end @__Lwh1 LOD main_a
GRE 0
JIZ __Lwh1end
JMP __Lwh1
@__Lwh1end @__Lwh2 LOD main_a
GRE 0
JIZ __Lwh2end
JMP __Lwh2
@__Lwh2end LOD 0
SET main_a
@__Lwh3 LOD main_a
ADD 1
SET main_a
LES 1
JIZ __Lwh3
@__Lwh3end LOD main_a
OUT 0
@fim JMP fim
