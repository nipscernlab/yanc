NOP
#PRNAME cmm_int_reassoc
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE v0 main_x
#SHARE v33 main_y
JMP main
@g POP
MLT 2
RET
@main LOD 6
SET main_x
LOD 4
SET main_y
LOD 3
SET main_k
LOD 2
SET main_j
LOD main_x
MLT main_y
ADD main_k
ADD main_j
OUT 0
LOD main_x
MLT main_y
ADD main_k
P_NEG_M main_j
S_ADD
OUT 0
LOD main_x
MLT main_y
P_NEG_M main_k
S_ADD
P_NEG_M main_j
S_ADD
OUT 0
LOD main_x
MLT main_y
P_NEG_M main_k
S_ADD
ADD main_j
OUT 0
LOD main_x
MLT main_y
P_NEG_M main_k
S_ADD
ADD 1
OUT 0
LOD main_x
MLT main_y
ADD main_j
ADD -7
OUT 0
LOD main_x
MLT main_y
ADD main_k
ADD -1
OUT 0
LOD main_x
MLT main_y
NEG
ADD main_k
ADD main_j
OUT 0
NEG_M main_y
ADD main_x
ADD -1
OUT 0
NEG_M main_j
ADD main_k
ADD 4
OUT 0
LOD main_x
ADD 1
P_LOD main_y
CAL g
OUT 0
LOD 9
SET v0
LOD 9
SET v33
LOD v0
ADD v33
OUT 0
LOD 9
OUT 0
@fim JMP fim
