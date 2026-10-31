NOP
#PRNAME cmm_continue
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @__Lwh1 LOD 0
SET main_k
@__Lwh2 LOD 5
LES main_k
JIZ __Lwh2end
LOD main_k
ADD 1
SET main_k
LOD 3
XOR main_k
JIZ __Lwh2
LOD main_k
OUT 0
JMP __Lwh2
@__Lwh2end LOD 0
SET main_k
@__Lwh3 LOD 2
XOR main_k
JIZ __Lwh3cont
LOD 10
ADD main_k
OUT 0
@__Lwh3cont LOD main_k
ADD 1
SET main_k
LES 5
JIZ __Lwh3
@__Lwh3end LOD 0
SET main_k
@__Lwh4 LOD 0
SET main_n
@__Lwh5 LOD 1
XOR main_n
JIZ __Lwh5cont
LOD main_k
MLT 10
ADD 20
ADD main_n
OUT 0
@__Lwh5cont LOD main_n
ADD 1
SET main_n
LES 2
JIZ __Lwh5
@__Lwh5end LOD main_k
ADD 1
SET main_k
LES 2
JIZ __Lwh4
@__Lwh4end LOD 0
SET main_k
@__Lwh6 LOD main_k
ADD -2
JIZ __Lwh6cont
@__sw_body_1_2 LOD 50
ADD main_k
OUT 0
@__switch_end_1 LOD 60
ADD main_k
OUT 0
@__Lwh6cont LOD main_k
ADD 1
SET main_k
LES 3
JIZ __Lwh6
@__Lwh6end JMP __Lwh1
@__Lwh1end @fim JMP fim
