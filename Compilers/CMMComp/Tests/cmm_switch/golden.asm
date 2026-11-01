NOP
#PRNAME cmm_switch
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @__Lwh1 LOD 1
ADD -1
JIZ __sw_body_1_1
ADD -1
JIZ __sw_body_1_2
JMP __sw_body_1_3
@__sw_body_1_1 LOD 11
OUT 0
JMP __switch_end_1
@__sw_body_1_2 LOD 12
OUT 0
JMP __switch_end_1
@__sw_body_1_3 LOD 19
OUT 0
@__switch_end_1 LOD 2
ADD -1
JIZ __sw_body_2_1
ADD -1
JIZ __sw_body_2_2
JMP __sw_body_2_3
@__sw_body_2_1 LOD 21
OUT 0
JMP __switch_end_2
@__sw_body_2_2 LOD 22
OUT 0
JMP __switch_end_2
@__sw_body_2_3 LOD 29
OUT 0
@__switch_end_2 LOD 7
ADD -1
JIZ __sw_body_3_1
ADD -1
JIZ __sw_body_3_2
JMP __sw_body_3_3
@__sw_body_3_1 LOD 31
OUT 0
JMP __switch_end_3
@__sw_body_3_2 LOD 32
OUT 0
JMP __switch_end_3
@__sw_body_3_3 LOD 39
OUT 0
@__switch_end_3 LOD 20
ADD -10
JIZ __sw_body_4_1
ADD -10
JIZ __sw_body_4_2
ADD -10
JIZ __sw_body_4_3
JMP __sw_body_4_4
@__sw_body_4_1 LOD 41
OUT 0
JMP __Lwh1
@__sw_body_4_2 LOD 42
OUT 0
JMP __Lwh1
@__sw_body_4_3 LOD 43
OUT 0
JMP __Lwh1
@__sw_body_4_4 LOD 49
OUT 0
@__switch_end_4 JMP __Lwh1
@__Lwh1end @fim JMP fim
