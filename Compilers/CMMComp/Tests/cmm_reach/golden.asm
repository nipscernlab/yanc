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
#SHARE live1_u rare_u
#SHARE main_y rare_u
JMP main
@rare SET rare_u
NEG_M 7
ADD rare_u
RET
@live1 SET live1_u
SET_V buf 1
LOD_V buf 1
ADD 2
RET
@main @Lwh1 INN 0
SET main_x
CAL live1
SET main_y
LOD 12345
EQU main_x
JIZ Lif1else
LOD main_y
CAL rare
SET main_y
@Lif1else LOD main_y
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
