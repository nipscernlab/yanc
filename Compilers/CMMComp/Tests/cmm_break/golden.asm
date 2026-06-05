NOP
#PRNAME cmm_break
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
LOD 0
SET main_k
@Lwh2 LOD 10
LES main_k
JIZ Lwh2end
LOD 3
EQU main_k
JIZ Lif1else
JMP Lwh2end
@Lif1else LOD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
JMP Lwh2
@Lwh2end LOD 100
OUT 0
LOD 0
SET main_k
@Lwh3 LOD 10
LES main_k
JIZ Lwh3end
LOD 2
EQU main_k
JIZ Lif2else
JMP Lwh3end
@Lif2else LOD 10
ADD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
JMP Lwh3
@Lwh3end LOD 101
OUT 0
LOD 0
SET main_k
@Lwh4 LOD 3
LES main_k
JIZ Lwh4end
LOD 0
SET main_n
@Lwh5 LOD 5
LES main_n
JIZ Lwh5end
LOD 2
EQU main_n
JIZ Lif3else
JMP Lwh5end
@Lif3else LOD main_k
MLT 10
ADD 20
ADD main_n
OUT 0
LOD main_n
ADD 1
SET main_n
JMP Lwh5
@Lwh5end LOD main_k
ADD 1
SET main_k
JMP Lwh4
@Lwh4end LOD 102
OUT 0
LOD 1
SET main_k
SET switch_exp
EQU 1
JIZ sw_disp_1_1
JMP sw_body_1_1
@sw_disp_1_1 JMP sw_body_1_2
@sw_body_1_1 LOD 0
SET main_n
@Lwh6 LOD 5
LES main_n
JIZ Lwh6end
LOD 2
EQU main_n
JIZ Lif4else
JMP Lwh6end
@Lif4else LOD 50
ADD main_n
OUT 0
LOD main_n
ADD 1
SET main_n
JMP Lwh6
@Lwh6end LOD 59
OUT 0
JMP switch_end_1
@sw_body_1_2 LOD 99
OUT 0
@switch_end_1 LOD 0
SET main_k
@Lwh7 LOD 3
LES main_k
JIZ Lwh7end
LOD main_k
SET switch_exp
EQU 1
JIZ sw_disp_2_1
JMP sw_body_2_1
@sw_disp_2_1 JMP sw_body_2_2
@sw_body_2_1 LOD 61
OUT 0
JMP switch_end_2
@sw_body_2_2 LOD 69
OUT 0
@switch_end_2 LOD 70
ADD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
JMP Lwh7
@Lwh7end JMP Lwh1
@Lwh1end @fim JMP fim
