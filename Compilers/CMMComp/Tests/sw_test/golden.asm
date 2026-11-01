NOP
#PRNAME sw_test
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @__Lwh1 INN 0
ADD -1
JIZ __sw_body_1_1
ADD -1
JIZ __sw_body_1_2
JMP __sw_body_1_3
@__sw_body_1_1 LOD 10
SET main_y
JMP __switch_end_1
@__sw_body_1_2 LOD 20
SET main_y
JMP __switch_end_1
@__sw_body_1_3 LOD 0
SET main_y
@__switch_end_1 LOD main_y
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
