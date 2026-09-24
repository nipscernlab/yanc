NOP
#PRNAME cmm_forbottom
#NUBITS 32
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_s
LOD 0
SET main_k
@Lwh1 LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
LES 9
JIZ Lwh1
@Lwh1end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 3
SET main_k
@Lwh2 LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
LES 7
JIZ Lwh2
@Lwh2end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 9
SET main_k
@Lwh3 LOD main_s
ADD main_k
SET main_s
NEG_M 1
ADD main_k
SET main_k
GRE 2
JIZ Lwh3
@Lwh3end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 9
SET main_k
@Lwh4 LOD main_s
ADD main_k
SET main_s
NEG_M 1
ADD main_k
SET main_k
GRE 3
JIZ Lwh4
@Lwh4end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 0
SET main_k
@Lwh5 LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
EQU 6
JIZ Lwh5
@Lwh5end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 0
SET main_k
@Lwh6 LOD main_s
ADD 7
SET main_s
LOD main_k
ADD 1
SET main_k
XOR 0
JIZ Lwh6
@Lwh6end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 5
SET main_k
@Lwh7 LOD 3
LES main_k
JIZ Lwh7end
LOD main_s
ADD 100
SET main_s
LOD main_k
ADD 1
SET main_k
JMP Lwh7
@Lwh7end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 1
SET main_k
@Lwh8 LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
LES 10
JIZ Lwh8
@Lwh8end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 0
SET main_k
@Lwh9 LOD 3
MOD main_k
P_LOD 0
S_XOR
JIZ Lwh9cont
LOD main_s
ADD main_k
SET main_s
@Lwh9cont LOD main_k
ADD 1
SET main_k
LES 9
JIZ Lwh9
@Lwh9end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 0
SET main_k
@Lwh10 LOD 6
XOR main_k
JIZ Lwh10end
LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
LES 9
JIZ Lwh10
@Lwh10end LOD main_s
OUT 0
@fim JMP fim
