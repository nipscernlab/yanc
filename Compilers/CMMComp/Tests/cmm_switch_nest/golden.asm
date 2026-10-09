NOP
#PRNAME cmm_switch_nest
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @__Lwh1 LOD 1
SET main_a
LOD 2
SET main_b
LOD main_a
ADD -1
JIZ __sw_body_1_1
ADD -1
JIZ __sw_body_1_2
JMP __sw_body_1_3
@__sw_body_1_1 LOD main_b
ADD -2
JIZ __sw_body_2_1
JMP __sw_body_2_2
@__sw_body_2_1 LOD 12
OUT 0
JMP __switch_end_2
@__sw_body_2_2 LOD 19
OUT 0
@__switch_end_2 LOD 100
OUT 0
JMP __switch_end_1
@__sw_body_1_2 LOD 200
OUT 0
JMP __switch_end_1
@__sw_body_1_3 LOD 99
OUT 0
@__switch_end_1 LOD 5
SET main_a
ADD -5
JIZ __sw_body_3_1
JMP __sw_body_3_2
@__sw_body_3_1 LOD main_a
JIZ __Lif1else
JMP __switch_end_3
@__Lif1else LOD 999
OUT 0
@__sw_body_3_2 LOD 888
OUT 0
@__switch_end_3 LOD 50
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
