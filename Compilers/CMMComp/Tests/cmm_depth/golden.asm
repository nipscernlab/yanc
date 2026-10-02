NOP
#PRNAME cmm_depth
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE top_a leaf_b
#SHARE main_x top_c
JMP main
@leaf SET_P leaf_b
SET leaf_a
ADD leaf_b
RET
@mid SET mid_a
P_LOD 1
CAL leaf
P_LOD mid_a
P_LOD 2
CAL leaf
S_ADD
RET
@top SET_P top_c
SET_P top_b
SET top_a
CAL mid
ADD top_b
ADD top_c
RET
@main @Lwh1 INN 0
SET main_x
P_LOD 2
P_LOD 3
CAL top
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
