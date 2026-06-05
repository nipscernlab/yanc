NOP
#PRNAME cmm_dowhile
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
LOD 10
SET main_k
@Lwh2 LOD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
LOD 5
LES main_k
JIZ Lwh2end
JMP Lwh2
@Lwh2end LOD 0
SET main_k
@Lwh3 LOD 20
ADD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
LOD 3
LES main_k
JIZ Lwh3end
JMP Lwh3
@Lwh3end LOD 0
SET main_k
@Lwh4 LOD 2
EQU main_k
JIZ Lif1else
JMP Lwh4end
@Lif1else LOD 30
ADD main_k
OUT 0
LOD main_k
ADD 1
SET main_k
LOD 10
LES main_k
JIZ Lwh4end
JMP Lwh4
@Lwh4end LOD 0
SET main_k
@Lwh5 LOD main_k
ADD 1
SET main_k
LOD 3
EQU main_k
JIZ Lif2else
JMP Lwh5cont
@Lif2else LOD 40
ADD main_k
OUT 0
@Lwh5cont LOD 5
LES main_k
JIZ Lwh5end
JMP Lwh5
@Lwh5end LOD 0
SET main_k
@Lwh6 LOD 0
SET main_n
@Lwh7 LOD 2
EQU main_n
JIZ Lif3else
JMP Lwh7end
@Lif3else LOD main_k
MLT 10
ADD 50
ADD main_n
OUT 0
LOD main_n
ADD 1
SET main_n
LOD 5
LES main_n
JIZ Lwh7end
JMP Lwh7
@Lwh7end LOD main_k
ADD 1
SET main_k
LOD 3
LES main_k
JIZ Lwh6end
JMP Lwh6
@Lwh6end JMP Lwh1
@Lwh1end @fim JMP fim
