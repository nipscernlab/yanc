NOP
#PRNAME multiparam
#NUBITS 16
#NDSTAC 4
#SDEPTH 4
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
JMP main
@add SET_P add_b
SET add_a
LOD add_a
ADD add_b
RET
@triple_sum SET_P triple_sum_z
SET_P triple_sum_y
SET triple_sum_x
LOD triple_sum_x
ADD triple_sum_y
ADD triple_sum_z
RET
@main @Lwh1 LOD 1
JIZ Lwh1end
INN 0
SET main_v
P_LOD 1
CAL add
SET main_two
LOD main_v
P_LOD 1
P_LOD 2
CAL triple_sum
SET main_three
LOD main_two
OUT 0
LOD main_three
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
