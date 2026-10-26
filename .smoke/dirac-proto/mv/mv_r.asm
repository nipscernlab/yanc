NOP
#PRNAME mv
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array M 2 256
#array v 2 16
#array y 2 16
#SHARE main_t main_j
@main LOD 0
SET main_k
@__Lwh1 LOD 0
SET main_j
@__Lwh2 LOD  main_k
MLT  16
ADD  main_j
PF_INN 0
F_MLT 0.001
STI M
LOD main_j
ADD 1
SET main_j
LES 15
JIZ __Lwh2
@__Lwh2end LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh1
@__Lwh1end LOD 0
SET main_k
@__Lwh3 LOD main_k
PF_INN 0
F_MLT 0.001
STI v
LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh3
@__Lwh3end LOD 0
SET main_t
@__Lwh4 LOD 3
LES main_t
JIZ __Lwh4end
LOD 0
SET main_rn
NEG_M 16
SET main_r
LOD main_rn
@__Lr LDI 0
F_MLT_V v 0
P_LOD main_rn
LDI 1
F_MLT_V v 1
SF_ADD
P_LOD main_rn
LDI 2
F_MLT_V v 2
SF_ADD
P_LOD main_rn
LDI 3
F_MLT_V v 3
SF_ADD
P_LOD main_rn
LDI 4
F_MLT_V v 4
SF_ADD
P_LOD main_rn
LDI 5
F_MLT_V v 5
SF_ADD
P_LOD main_rn
LDI 6
F_MLT_V v 6
SF_ADD
P_LOD main_rn
LDI 7
F_MLT_V v 7
SF_ADD
P_LOD main_rn
LDI 8
F_MLT_V v 8
SF_ADD
P_LOD main_rn
LDI 9
F_MLT_V v 9
SF_ADD
P_LOD main_rn
LDI 10
F_MLT_V v 10
SF_ADD
P_LOD main_rn
LDI 11
F_MLT_V v 11
SF_ADD
P_LOD main_rn
LDI 12
F_MLT_V v 12
SF_ADD
P_LOD main_rn
LDI 13
F_MLT_V v 13
SF_ADD
P_LOD main_rn
LDI 14
F_MLT_V v 14
SF_ADD
P_LOD main_rn
LDI 15
F_MLT_V v 15
SF_ADD
SET main_s
LOD main_r
P_LOD main_s
STI 288
LOD main_r
ADD 1
SET main_r
JIZ __Lrend
LOD main_rn
ADD 16
SET main_rn
JMP __Lr
@__Lrend LOD 0
SET main_k
@__Lwh5 LOD main_k
LDI y
SET main_ib
P_LOD main_ib
OUT 0
LOD main_k
ADD 1
SET main_k
LES 15
JIZ __Lwh5
@__Lwh5end LOD main_t
P_LOD main_t
LDI y
STI v
LOD main_t
ADD 1
SET main_t
JMP __Lwh4
@__Lwh4end @fim JMP fim
