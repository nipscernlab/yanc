NOP
#PRNAME cmm_forbottom
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_s
SET main_k
@__Lwh1 LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
LES 9
JIZ __Lwh1
@__Lwh1end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 3
SET main_k
@__Lwh2 LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
LES 7
JIZ __Lwh2
@__Lwh2end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 9
SET main_k
@__Lwh3 LOD main_s
ADD main_k
SET main_s
NEG_M 1
ADD main_k
SET main_k
GRE 2
JIZ __Lwh3
@__Lwh3end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 9
SET main_k
@__Lwh4 LOD main_s
ADD main_k
SET main_s
NEG_M 1
ADD main_k
SET main_k
GRE 3
JIZ __Lwh4
@__Lwh4end LOD main_s
OUT 0
LOD 0
SET main_s
SET main_k
@__Lwh5 LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
EQU 6
JIZ __Lwh5
@__Lwh5end LOD main_s
OUT 0
LOD 0
SET main_s
SET main_k
@__Lwh6 LOD main_s
ADD 7
SET main_s
LOD main_k
ADD 1
SET main_k
XOR 0
JIZ __Lwh6
@__Lwh6end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 5
SET main_k
@__Lwh7 LOD 3
LES main_k
JIZ __Lwh7end
LOD main_s
ADD 100
SET main_s
LOD main_k
ADD 1
SET main_k
JMP __Lwh7
@__Lwh7end LOD main_s
OUT 0
LOD 0
SET main_s
LOD 1
SET main_k
@__Lwh8 LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
LES 10
JIZ __Lwh8
@__Lwh8end LOD main_s
OUT 0
LOD 0
SET main_s
SET main_k
@__Lwh9 LOD 3
MOD main_k
NOP
NOP
NOP
REM
XOR 0
JIZ __Lwh9cont
LOD main_s
ADD main_k
SET main_s
@__Lwh9cont LOD main_k
ADD 1
SET main_k
LES 9
JIZ __Lwh9
@__Lwh9end LOD main_s
OUT 0
LOD 0
SET main_s
SET main_k
@__Lwh10 LOD 6
XOR main_k
JIZ __Lwh10end
LOD main_s
ADD main_k
SET main_s
LOD main_k
ADD 1
SET main_k
LES 9
JIZ __Lwh10
@__Lwh10end LOD main_s
OUT 0
@fim JMP fim
