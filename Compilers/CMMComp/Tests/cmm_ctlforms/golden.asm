NOP
#PRNAME cmm_ctlforms
#NUBITS 32
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE sw2_v sw_v
#SHARE sw2_r sw_r
#SHARE sw3_v sw_v
#SHARE sw3_r sw_r
#SHARE main_k sw_v
#SHARE main_s sw_v
#SHARE main_n sw_r
JMP main
@sw SET sw_v
LOD 0
SET sw_r
LOD sw_v
ADD -7
JIZ sw_body_1_1
ADD 5
JIZ sw_body_1_2
ADD 2
JIZ sw_body_1_3
ADD -1000
JIZ sw_body_1_4
JMP sw_body_1_5
@sw_body_1_1 LOD 70
SET sw_r
JMP switch_end_1
@sw_body_1_2 LOD 20
SET sw_r
JMP switch_end_1
@sw_body_1_3 LOD 1
SET sw_r
JMP switch_end_1
@sw_body_1_4 LOD 1000
SET sw_r
JMP switch_end_1
@sw_body_1_5 NEG_M 1
SET sw_r
@switch_end_1 LOD sw_r
RET
@sw2 SET sw2_v
LOD 0
SET sw2_r
LOD sw2_v
ADD -5
JIZ sw_body_2_1
JMP sw_body_2_2
@sw_body_2_1 LOD 5
XOR sw2_v
JIZ switch_end_2
LOD 55
SET sw2_r
JMP switch_end_2
@sw_body_2_2 LOD 9
SET sw2_r
@switch_end_2 LOD sw2_r
RET
@sw3 SET sw3_v
LOD 0
SET sw3_r
LOD sw3_v
ADD -2147483647
JIZ sw_body_3_1
ADD 2147483647
JIZ sw_body_3_2
JMP sw_body_3_3
@sw_body_3_1 LOD 1
SET sw3_r
JMP switch_end_3
@sw_body_3_2 LOD 2
SET sw3_r
JMP switch_end_3
@sw_body_3_3 LOD 3
SET sw3_r
@switch_end_3 LOD sw3_r
RET
@main LOD 0
SET main_k
@Lwh1 LOD main_k
ADD 1
SET main_k
LOD 5
XOR main_k
JIZ Lwh1end
JMP Lwh1
@Lwh1end LOD main_k
OUT 0
@Lwh2 LOD main_k
ADD 1
SET main_k
LOD 7
EQU main_k
JIZ Lwh2cont
JMP Lwh2end
@Lwh2cont JMP Lwh2
@Lwh2end LOD main_k
OUT 0
LOD 0
SET main_s
LOD 0
SET main_n
@Lwh3 LOD 2
XOR main_n
JIZ Lwh3cont
LOD main_s
ADD main_n
SET main_s
@Lwh3cont LOD main_n
ADD 1
SET main_n
LES 5
JIZ Lwh3
@Lwh3end LOD main_s
OUT 0
LOD 0
CAL sw
OUT 0
LOD 2
CAL sw
OUT 0
LOD 7
CAL sw
OUT 0
LOD 1000
CAL sw
OUT 0
LOD 3
CAL sw
OUT 0
LOD 5
CAL sw2
OUT 0
LOD 6
CAL sw2
OUT 0
LOD 2147483647
CAL sw3
OUT 0
LOD 0
CAL sw3
OUT 0
NEG_M 5
CAL sw3
OUT 0
@fim JMP fim
