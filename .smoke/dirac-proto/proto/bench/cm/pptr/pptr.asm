NOP
#PRNAME cm
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array a 2 16
#array P 2 256
#SHARE main_q main_k
@main LOD 0
SET main_k
@__Lwh1 LOD main_k
PF_INN 0
F_MLT 0.001
STI a
LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh1
@__Lwh1end LOD 0
SET main_k
@__Lwh2 LOD 0
SET main_j
@__Lwh3 LOD  main_k
MLT  16
ADD  main_j
PF_INN 0
F_MLT 0.001
STI P
LOD main_j
ADD 1
SET main_j
LES 15
JIZ __Lwh3
@__Lwh3end LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh2
@__Lwh2end F_INN 0
F_MLT 0.001
SET main_c
LOD 0
SET main_t
@__Lwh4 LOD 2
LES main_t
JIZ __Lwh4end
LOD_V P 0
LOD_V P 0
LOD __pd
F_MLT main_c
NOP
LOD __pd
F_MLT main_c
SET __pd
LOD 0
SET main_q
LOD 0
SET main_k
@__Lwh5 LOD 0
SET main_j
@__Lwh6 LOD main_k
MLT 16
ADD main_j
LDI P
SET main_ib
P_LOD main_ib
OUT 0
LOD main_j
ADD 1
SET main_j
LES 15
JIZ __Lwh6
@__Lwh6end LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh5
@__Lwh5end LOD 7
OUT 0
LOD main_c
F_ADD 0.25
SET main_c
LOD main_t
P_LOD main_t
LDI a
F_ADD 0.125
STI a
LOD main_t
ADD 1
SET main_t
JMP __Lwh4
@__Lwh4end @fim JMP fim

