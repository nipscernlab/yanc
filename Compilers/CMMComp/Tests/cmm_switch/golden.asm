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
@sw_case_1_1 LOD switch_exp
EQU 1
JIZ sw_case_1_2
LOD 11
OUT 0
JMP switch_end_1
@sw_case_1_2 LOD switch_exp
EQU 2
JIZ sw_case_1_3
LOD 12
OUT 0
JMP switch_end_1
@sw_case_1_3 LOD 19
OUT 0
@sw_case_1_4 @switch_end_1 LOD 2
SET main_s
SET switch_exp
@sw_case_2_1 LOD switch_exp
EQU 1
JIZ sw_case_2_2
LOD 21
OUT 0
JMP switch_end_2
@sw_case_2_2 LOD switch_exp
EQU 2
JIZ sw_case_2_3
LOD 22
OUT 0
JMP switch_end_2
@sw_case_2_3 LOD 29
OUT 0
@sw_case_2_4 @switch_end_2 LOD 7
SET main_s
SET switch_exp
@sw_case_3_1 LOD switch_exp
EQU 1
JIZ sw_case_3_2
LOD 31
OUT 0
JMP switch_end_3
@sw_case_3_2 LOD switch_exp
EQU 2
JIZ sw_case_3_3
LOD 32
OUT 0
JMP switch_end_3
@sw_case_3_3 LOD 39
OUT 0
@sw_case_3_4 @switch_end_3 LOD 20
SET main_s
SET switch_exp
@sw_case_4_1 LOD switch_exp
EQU 10
JIZ sw_case_4_2
LOD 41
OUT 0
JMP switch_end_4
@sw_case_4_2 LOD switch_exp
EQU 20
JIZ sw_case_4_3
LOD 42
OUT 0
JMP switch_end_4
@sw_case_4_3 LOD switch_exp
EQU 30
JIZ sw_case_4_4
LOD 43
OUT 0
JMP switch_end_4
@sw_case_4_4 LOD 49
OUT 0
@sw_case_4_5 @switch_end_4 JMP Lwh1
@Lwh1end @fim JMP fim
