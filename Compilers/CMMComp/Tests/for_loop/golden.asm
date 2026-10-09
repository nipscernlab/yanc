NOP
#PRNAME for_loop
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
#SHARE main_cnt main_idx
#SHARE main_kk main_idx
#SHARE main_outer main_idx
@main @__Lwh1 LOD 0
SET main_idx
@__Lwh2 LOD 10
ADD main_idx
OUT 0
LOD main_idx
ADD 1
SET main_idx
LES 2
JIZ __Lwh2
@__Lwh2end LOD 0
SET main_cnt
@__Lwh3 LOD 20
ADD main_cnt
OUT 0
LOD main_cnt
ADD 1
SET main_cnt
LES 2
JIZ __Lwh3
@__Lwh3end LOD 0
SET main_kk
@__Lwh4 LOD 30
ADD main_kk
OUT 0
LOD main_kk
ADD 1
SET main_kk
LES 2
JIZ __Lwh4
@__Lwh4end LOD 0
SET main_outer
@__Lwh5 LOD 0
SET main_inner
@__Lwh6 LOD main_outer
MLT 10
ADD 40
ADD main_inner
OUT 0
LOD main_inner
ADD 1
SET main_inner
LES 1
JIZ __Lwh6
@__Lwh6end LOD main_outer
ADD 1
SET main_outer
LES 1
JIZ __Lwh5
@__Lwh5end LOD 0
SET main_idx
@__Lwh7 LOD 2
LES main_idx
JIZ __Lwh7end
LOD 60
ADD main_idx
OUT 0
LOD main_idx
ADD 1
SET main_idx
JMP __Lwh7
@__Lwh7end LOD 0
SET main_idx
@__Lwh8 LOD 2
LES main_idx
JIZ __Lwh8end
LOD 70
ADD main_idx
OUT 0
LOD main_idx
ADD 1
SET main_idx
JMP __Lwh8
@__Lwh8end LOD 99
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
