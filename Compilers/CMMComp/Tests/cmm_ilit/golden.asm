NOP
#PRNAME cmm_ilit
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#arrayd a 1 3 3 16 8 3
#arrayd b 1 3 3 -1 -2147483648 1
#SHARE main_s main_m
#SHARE main_y main_m
#SHARE main_x main_m
@main LOD 31
OUT 0
LOD 42
OUT 0
LOD 42
OUT 0
LOD 5
OUT 0
LOD 10
OUT 0
LOD 10
OUT 0
LOD 7
OUT 0
LOD 2147483647
OUT 0
LOD_V a 0
OUT 0
LOD_V a 1
OUT 0
LOD_V a 2
OUT 0
LOD 15
SET main_m
OUT 0
LOD 4
SHL 1
OUT 0
LOD 16
SET main_s
ADD -16
JIZ __sw_body_1_1
ADD 8
JIZ __sw_body_1_2
JMP __switch_end_1
@__sw_body_1_1 LOD 1
OUT 0
JMP __switch_end_1
@__sw_body_1_2 LOD 2
OUT 0
@__switch_end_1 NEG_M 1
OUT 0
NEG_M 2147483647
P_NEG_M 1
S_ADD
OUT 0
LOD_V b 0
OUT 0
LOD_V b 1
OUT 0
LOD_V b 2
OUT 0
NEG_M 65536
AND 305419896
SET main_y
OUT 0
LOD 5
SET main_x
NEG_M 1
ADD main_x
OUT 0
@__Lwh1 JMP __Lwh1
@__Lwh1end @fim JMP fim
