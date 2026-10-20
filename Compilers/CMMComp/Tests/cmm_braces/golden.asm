NOP
#PRNAME cmm_braces
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#arrayd a 1 5 3 3 -2 7
#arrayd f 2 3 3 1.5 -0.25 4
#arrayd m 1 6 6 1 2 0 4 5 6
#arrayd q 1 4 3 1 2 3
#arrayd c 3 2 2 1.5 3
#arrayd c_i 4 2 2 -2 0.0
#SHARE main_k loc_s
JMP main
@loc #arrayd loc_l 1 3 2 4 5
LOD_V loc_l 1
P_LOD_V loc_l 2
S_ADD
SET loc_s
LOD 9
SET_V loc_l 2
LOD loc_s
RET
@main LOD 0
SET main_k
@__Lwh1 LOD main_k
LDI a
OUT 0
LOD main_k
ADD 1
SET main_k
LES 4
JIZ __Lwh1
@__Lwh1end LOD 0
SET main_k
@__Lwh2 LOD main_k
LDI f
F_MLT 8.0
F2I
OUT 0
LOD main_k
ADD 1
SET main_k
LES 2
JIZ __Lwh2
@__Lwh2end LOD 0
MLT 3
ADD 1
LDI m
OUT 0
LOD 0
MLT 3
ADD 2
LDI m
OUT 0
LOD 1
MLT 3
ADD 0
LDI m
OUT 0
LOD 1
MLT 3
ADD 2
LDI m
OUT 0
LOD 1
MLT 2
ADD 0
LDI q
OUT 0
LOD 1
MLT 2
ADD 1
LDI q
OUT 0
LOD 0
SET main_k
@__Lwh3 LOD main_k
LDI c
P_LOD main_k
LDI c_i
POP
F_MLT 2.0
F2I
OUT 0
LOD main_k
LDI c
P_LOD main_k
LDI c_i
SET_P __aux_var
LOD   __aux_var
F_MLT 2.0
F2I
OUT 0
LOD main_k
ADD 1
SET main_k
LES 1
JIZ __Lwh3
@__Lwh3end CAL loc
OUT 0
CAL loc
OUT 0
@fim JMP fim
