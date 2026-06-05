NOP
#PRNAME cmm_switch_fall
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
SET main_x
SET switch_exp
EQU 1
JIZ sw_disp_1_1
JMP sw_body_1_1
@sw_disp_1_1 LOD switch_exp
EQU 2
JIZ sw_disp_1_2
JMP sw_body_1_2
@sw_disp_1_2 LOD switch_exp
EQU 3
JIZ sw_disp_1_3
JMP sw_body_1_3
@sw_disp_1_3 JMP sw_body_1_4
@sw_body_1_1 LOD 1
OUT 0
@sw_body_1_2 LOD 2
OUT 0
@sw_body_1_3 LOD 3
OUT 0
@sw_body_1_4 LOD 9
OUT 0
@switch_end_1 LOD 2
SET main_x
SET switch_exp
EQU 1
JIZ sw_disp_2_1
JMP sw_body_2_1
@sw_disp_2_1 LOD switch_exp
EQU 2
JIZ sw_disp_2_2
JMP sw_body_2_2
@sw_disp_2_2 LOD switch_exp
EQU 3
JIZ sw_disp_2_3
JMP sw_body_2_3
@sw_disp_2_3 JMP sw_body_2_4
@sw_body_2_1 @sw_body_2_2 @sw_body_2_3 LOD 33
OUT 0
JMP switch_end_2
@sw_body_2_4 LOD 99
OUT 0
@switch_end_2 LOD 2
SET main_x
SET switch_exp
EQU 1
JIZ sw_disp_3_1
JMP sw_body_3_1
@sw_disp_3_1 LOD switch_exp
EQU 2
JIZ sw_disp_3_2
JMP sw_body_3_2
@sw_disp_3_2 LOD switch_exp
EQU 3
JIZ sw_disp_3_3
JMP sw_body_3_3
@sw_disp_3_3 JMP sw_body_3_4
@sw_body_3_1 LOD 11
OUT 0
JMP switch_end_3
@sw_body_3_2 LOD 22
OUT 0
@sw_body_3_3 LOD 33
OUT 0
JMP switch_end_3
@sw_body_3_4 LOD 44
OUT 0
@switch_end_3 JMP Lwh1
@Lwh1end @fim JMP fim
