NOP
#PRNAME cmm_nubits8
#NUBITS 8
#NUIOIN 1
#NUIOOU 1
@main LOD 100
SET main_a
LOD 0
SET main_n
LOD 3
SET main_s
#array main_big 1 200
NEG_M 56
OUT 0
LOD main_a
ADD main_a
OUT 0
NEG_M 112
OUT 0
NEG_M 80
ADD main_a
P_NEG_M 80
ADD main_a
S_MLT
OUT 0
NEG_M 128
OUT 0
NEG_M 36
ADD main_a
P_NEG_M 36
ADD main_a
S_ADD
OUT 0
NEG_M 128
OUT 0
LOD 0
SET main_k
@__Lwh1 NEG_M 56
LES main_k
JIZ __Lwh1end
LOD main_n
ADD 1
SET main_n
LOD main_k
ADD 1
SET main_k
JMP __Lwh1
@__Lwh1end LOD main_n
OUT 0
LOD 0
SET main_n
SET main_k
@__Lwh2 LOD main_a
ADD main_a
LES main_k
JIZ __Lwh2end
LOD main_n
ADD 1
SET main_n
LOD main_k
ADD 1
SET main_k
JMP __Lwh2
@__Lwh2end LOD main_n
OUT 0
LOD 0
SET main_n
SET main_k
@__Lwh3 NEG_M 56
LES main_k
JIZ __Lwh3end
LOD main_n
ADD 1
SET main_n
LOD main_k
ADD 1
SET main_k
JMP __Lwh3
@__Lwh3end LOD main_n
OUT 0
LOD main_s
ADD -120
JIZ __sw_body_1_1
ADD 117
JIZ __sw_body_1_2
JMP __sw_body_1_3
@__sw_body_1_1 LOD 11
OUT 0
JMP __switch_end_1
@__sw_body_1_2 LOD 22
OUT 0
JMP __switch_end_1
@__sw_body_1_3 LOD 33
OUT 0
@__switch_end_1 LOD 5
SET_V main_big 100
LOD_V main_big 100
OUT 0
@fim JMP fim
