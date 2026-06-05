NOP
#PRNAME cmm_switch_nest
#NUBITS 16
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 1
SET main_a
LOD 2
SET main_b
LOD main_a
SET switch_exp
EQU 1
JIZ sw_disp_1_1
JMP sw_body_1_1
@sw_disp_1_1 LOD switch_exp
EQU 2
JIZ sw_disp_1_2
JMP sw_body_1_2
@sw_disp_1_2 JMP sw_body_1_3
@sw_body_1_1 LOD main_b
SET switch_exp
EQU 2
JIZ sw_disp_2_1
JMP sw_body_2_1
@sw_disp_2_1 JMP sw_body_2_2
@sw_body_2_1 LOD 12
OUT 0
JMP switch_end_2
@sw_body_2_2 LOD 19
OUT 0
@switch_end_2 LOD 100
OUT 0
JMP switch_end_1
@sw_body_1_2 LOD 200
OUT 0
JMP switch_end_1
@sw_body_1_3 LOD 99
OUT 0
@switch_end_1 LOD 5
SET main_a
SET switch_exp
EQU 5
JIZ sw_disp_3_1
JMP sw_body_3_1
@sw_disp_3_1 JMP sw_body_3_2
@sw_body_3_1 LOD main_a
JIZ Lif1else
JMP switch_end_3
@Lif1else LOD 999
OUT 0
@sw_body_3_2 LOD 888
OUT 0
@switch_end_3 LOD 50
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
