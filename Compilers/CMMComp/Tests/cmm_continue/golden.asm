NOP
#PRNAME cmm_continue
#NUBITS 16
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main @Lwh1 LOD 0
SET main_k
@Lwh2 LOD 5
LES main_k
JIZ Lwh2end
LOD main_k
ADD 1
SET main_k
LOD 3
XOR main_k
JIZ Lwh2
LOD main_k
OUT 0
JMP Lwh2
@Lwh2end LOD 0
SET main_k
@Lwh3 LOD 6
LES main_k
JIZ Lwh3end
LOD 2
XOR main_k
JIZ Lwh3cont
LOD 10
ADD main_k
OUT 0
@Lwh3cont LOD main_k
ADD 1
SET main_k
JMP Lwh3
@Lwh3end LOD 0
SET main_k
@Lwh4 LOD 3
LES main_k
JIZ Lwh4end
LOD 0
SET main_n
@Lwh5 LOD 3
LES main_n
JIZ Lwh5end
LOD 1
XOR main_n
JIZ Lwh5cont
LOD main_k
MLT 10
ADD 20
ADD main_n
OUT 0
@Lwh5cont LOD main_n
ADD 1
SET main_n
JMP Lwh5
@Lwh5end LOD main_k
ADD 1
SET main_k
JMP Lwh4
@Lwh4end LOD 0
SET main_k
@Lwh6 LOD 4
LES main_k
JIZ Lwh6end
LOD main_k
ADD -2
JIZ sw_body_1_1
JMP sw_body_1_2
@sw_body_1_1 JMP Lwh6cont
@sw_body_1_2 LOD 50
ADD main_k
OUT 0
@switch_end_1 LOD 60
ADD main_k
OUT 0
@Lwh6cont LOD main_k
ADD 1
SET main_k
JMP Lwh6
@Lwh6end JMP Lwh1
@Lwh1end @fim JMP fim
