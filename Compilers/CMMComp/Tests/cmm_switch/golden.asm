NOP
#PRNAME cmm_switch
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
SET main_s
SET switch_exp
EQU 1
JIZ sw_disp_1_1
JMP sw_body_1_1
@sw_disp_1_1 LOD switch_exp
EQU 2
JIZ sw_disp_1_2
JMP sw_body_1_2
@sw_disp_1_2 JMP sw_body_1_3
@sw_body_1_1 LOD 11
OUT 0
JMP switch_end_1
@sw_body_1_2 LOD 12
OUT 0
JMP switch_end_1
@sw_body_1_3 LOD 19
OUT 0
@switch_end_1 LOD 2
SET main_s
SET switch_exp
EQU 1
JIZ sw_disp_2_1
JMP sw_body_2_1
@sw_disp_2_1 LOD switch_exp
EQU 2
JIZ sw_disp_2_2
JMP sw_body_2_2
@sw_disp_2_2 JMP sw_body_2_3
@sw_body_2_1 LOD 21
OUT 0
JMP switch_end_2
@sw_body_2_2 LOD 22
OUT 0
JMP switch_end_2
@sw_body_2_3 LOD 29
OUT 0
@switch_end_2 LOD 7
SET main_s
SET switch_exp
EQU 1
JIZ sw_disp_3_1
JMP sw_body_3_1
@sw_disp_3_1 LOD switch_exp
EQU 2
JIZ sw_disp_3_2
JMP sw_body_3_2
@sw_disp_3_2 JMP sw_body_3_3
@sw_body_3_1 LOD 31
OUT 0
JMP switch_end_3
@sw_body_3_2 LOD 32
OUT 0
JMP switch_end_3
@sw_body_3_3 LOD 39
OUT 0
@switch_end_3 LOD 20
SET main_s
SET switch_exp
EQU 10
JIZ sw_disp_4_1
JMP sw_body_4_1
@sw_disp_4_1 LOD switch_exp
EQU 20
JIZ sw_disp_4_2
JMP sw_body_4_2
@sw_disp_4_2 LOD switch_exp
EQU 30
JIZ sw_disp_4_3
JMP sw_body_4_3
@sw_disp_4_3 JMP sw_body_4_4
@sw_body_4_1 LOD 41
OUT 0
JMP switch_end_4
@sw_body_4_2 LOD 42
OUT 0
JMP switch_end_4
@sw_body_4_3 LOD 43
OUT 0
JMP switch_end_4
@sw_body_4_4 LOD 49
OUT 0
@switch_end_4 JMP Lwh1
@Lwh1end @fim JMP fim
