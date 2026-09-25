NOP
#PRNAME for_loop
#NUBITS 16
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
#SHARE main_cnt main_idx
#SHARE main_kk main_idx
#SHARE main_outer main_idx
@main @Lwh1 LOD 0
SET main_idx
@Lwh2 LOD 10
ADD main_idx
OUT 0
LOD main_idx
ADD 1
SET main_idx
LES 2
JIZ Lwh2
@Lwh2end LOD 0
SET main_cnt
@Lwh3 LOD 20
ADD main_cnt
OUT 0
LOD main_cnt
ADD 1
SET main_cnt
LES 2
JIZ Lwh3
@Lwh3end LOD 0
SET main_kk
@Lwh4 LOD 30
ADD main_kk
OUT 0
LOD main_kk
ADD 1
SET main_kk
LES 2
JIZ Lwh4
@Lwh4end LOD 0
SET main_outer
@Lwh5 LOD 0
SET main_inner
@Lwh6 LOD main_outer
MLT 10
ADD 40
ADD main_inner
OUT 0
LOD main_inner
ADD 1
SET main_inner
LES 1
JIZ Lwh6
@Lwh6end LOD main_outer
ADD 1
SET main_outer
LES 1
JIZ Lwh5
@Lwh5end LOD 0
SET main_idx
@Lwh7 LOD 2
LES main_idx
JIZ Lwh7end
LOD 60
ADD main_idx
OUT 0
LOD main_idx
ADD 1
SET main_idx
JMP Lwh7
@Lwh7end LOD 0
SET main_idx
@Lwh8 LOD 2
LES main_idx
JIZ Lwh8end
LOD 70
ADD main_idx
OUT 0
LOD main_idx
ADD 1
SET main_idx
JMP Lwh8
@Lwh8end LOD 99
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
