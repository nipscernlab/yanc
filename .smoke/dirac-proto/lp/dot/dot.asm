NOP
#PRNAME dot
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array w 2 16
#array x 2 16
#SHARE main_ib main_k
@main LOD 0
SET main_k
@__Lwh1 LOD main_k
PF_INN 0
F_MLT 0.001
STI w
LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh1
@__Lwh1end LOD 0
SET main_k
@__Lwh2 LOD main_k
PF_INN 0
F_MLT 0.001
STI x
LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh2
@__Lwh2end LOD 0
SET main_t
@__Lwh3 LOD 6
LES main_t
JIZ __Lwh3end
LOD_V w 0
P_LOD_V x 0
SF_MLT
SET main_s
LOD 1
SET main_k
@__Lwh4 LOD main_k
LDI w
P_LOD main_k
LDI x
SF_MLT
F_ADD main_s
SET main_s
LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh4
@__Lwh4end LOD main_s
SET main_ib
OUT 0
LOD main_t
P_LOD main_s
STI w
LOD main_t
ADD 1
SET main_t
JMP __Lwh3
@__Lwh3end @fim JMP fim
