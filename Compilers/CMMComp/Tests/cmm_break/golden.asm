NOP
#PRNAME cmm_break
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @__Lwh1 LOD 0
SET main_k
@__Lwh2 LOD 10
LES main_k
JIZ __Lwh2end
LOD 3
XOR main_k
JIZ __Lwh2end
LOD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
JMP __Lwh2
@__Lwh2end LOD 100
OUT 0
LOD 0
SET main_k
@__Lwh3 LOD 2
XOR main_k
JIZ __Lwh3end
LOD 10
ADD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
LES 9
JIZ __Lwh3
@__Lwh3end LOD 101
OUT 0
LOD 0
SET main_k
@__Lwh4 LOD 0
SET main_n
@__Lwh5 LOD 2
XOR main_n
JIZ __Lwh5end
LOD main_k
MLT 10
ADD 20
ADD main_n
OUT 0
LOD main_n
ADD 1
SET main_n
LES 4
JIZ __Lwh5
@__Lwh5end LOD main_k
ADD 1
SET main_k
LES 2
JIZ __Lwh4
@__Lwh4end LOD 102
OUT 0
LOD 1
SET main_k
ADD -1
JIZ __sw_body_1_1
JMP __sw_body_1_2
@__sw_body_1_1 LOD 0
SET main_n
@__Lwh6 LOD 2
XOR main_n
JIZ __Lwh6end
LOD 50
ADD main_n
OUT 0
LOD main_n
ADD 1
SET main_n
LES 4
JIZ __Lwh6
@__Lwh6end LOD 59
OUT 0
JMP __switch_end_1
@__sw_body_1_2 LOD 99
OUT 0
@__switch_end_1 LOD 0
SET main_k
@__Lwh7 LOD main_k
ADD -1
JIZ __sw_body_2_1
JMP __sw_body_2_2
@__sw_body_2_1 LOD 61
OUT 0
JMP __switch_end_2
@__sw_body_2_2 LOD 69
OUT 0
@__switch_end_2 LOD 70
ADD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
LES 2
JIZ __Lwh7
@__Lwh7end JMP __Lwh1
@__Lwh1end @fim JMP fim
