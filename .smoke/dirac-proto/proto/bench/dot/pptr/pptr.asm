NOP
#PRNAME dot
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array a 2 16
#array b 2 16
#SHARE main_c main_k
#SHARE main_ib main_s
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
@__Lwh2 LOD main_k
PF_INN 0
F_MLT 0.001
STI b
LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh2
@__Lwh2end F_INN 0
F_MLT 0.001
SET main_c
LOD 0
SET main_t
@__Lwh3 LOD 2
LES main_t
JIZ __Lwh3end
LOD_V a 1
LOD_V b 1
LOD_V a 0
F_MLT_V b 0
NOP
P_LOD __pd
F_MLT __pd
SF_ADD
SET main_s
LOD 0
SET main_q
LOD main_s
SET main_ib
OUT 0
LOD 7
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
JMP __Lwh3
@__Lwh3end @fim JMP fim

