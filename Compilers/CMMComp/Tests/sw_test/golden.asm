NOP
#PRNAME sw_test
#NUBITS 16
#NDSTAC 4
#SDEPTH 4
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
#SHARE main_y main_x
@main @Lwh1 INN 0
SET main_x
ADD -1
JIZ sw_body_1_1
ADD -1
JIZ sw_body_1_2
JMP sw_body_1_3
@sw_body_1_1 LOD 10
SET main_y
JMP switch_end_1
@sw_body_1_2 LOD 20
SET main_y
JMP switch_end_1
@sw_body_1_3 LOD 0
SET main_y
@switch_end_1 LOD main_y
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
