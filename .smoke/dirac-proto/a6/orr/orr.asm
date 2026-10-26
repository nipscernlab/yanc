NOP
#PRNAME orr
#NUIOIN 1
#NUIOOU 1
#array a 1 4
@main LOD 1
SET main_c
LOD 2
SET main_d
LOD 8
SET main_e
LOD 4
SET_V a 0
LOD main_d
ORR main_c
P_LOD main_e
S_ORR
OUT 0
LOD main_d
ORR main_c
P_LOD_V a 0
S_ORR
OUT 0
INN 0
ORR main_c
OUT 0
@fim JMP fim
