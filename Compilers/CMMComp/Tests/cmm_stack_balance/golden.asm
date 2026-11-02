NOP
#PRNAME cmm_stack_balance
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
@main LOD 3.0
SET main_a
LOD 4.0
SET main_b
LOD 0
SET main_k
@__Lwh1top LOD main_k
ADD 1
ADD 1
OUT 0
LOD main_a
F_MLT main_b
F_ADD 1.0
SET main_y
F2I_M main_y
OUT 0
LOD main_y
CAL g
POP
LOD main_k
ADD 1
SET main_k
LES 39
JIZ __Lwh1top
@__Lwh1end LOD main_k
OUT 0
@fim JMP fim
@g P_LOD 2.0
RET
