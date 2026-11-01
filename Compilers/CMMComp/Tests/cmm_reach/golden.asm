NOP
#PRNAME cmm_reach
#NUBITS 32
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#array buf 1 3
JMP main
@rare ADD -7
RET
@live1 SET_V buf 1
LOD_V buf 1
ADD 2
RET
@main @__Lwh1 INN 0
SET main_x
CAL live1
SET main_y
LOD main_x
EQU 12345
JIZ __Lif1else
LOD main_y
CAL rare
SET main_y
@__Lif1else LOD main_y
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
