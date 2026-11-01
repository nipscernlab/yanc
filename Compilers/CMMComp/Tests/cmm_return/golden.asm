NOP
#PRNAME cmm_return
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
#SHARE nestedFind_target firstHit_limit
#SHARE nestedFind_a firstHit_k
#SHARE maybe_x firstHit_limit
JMP main
@add5 ADD 5
RET
@firstHit SET firstHit_limit
LOD 0
SET firstHit_k
@__Lwh1 LOD firstHit_limit
EQU firstHit_k
JIZ __Lif1else
LOD firstHit_k
ADD firstHit_k
RET
@__Lif1else LOD firstHit_k
ADD 1
SET firstHit_k
LES 99
JIZ __Lwh1
@__Lwh1end LOD 0
RET
@nestedFind SET nestedFind_target
LOD 0
SET nestedFind_a
@__Lwh2 LOD 0
SET nestedFind_b
@__Lwh3 LOD nestedFind_a
MLT 10
ADD nestedFind_b
EQU nestedFind_target
JIZ __Lif2else
LOD nestedFind_a
MLT 100
ADD nestedFind_b
RET
@__Lif2else LOD nestedFind_b
ADD 1
SET nestedFind_b
LES 4
JIZ __Lwh3
@__Lwh3end LOD nestedFind_a
ADD 1
SET nestedFind_a
LES 4
JIZ __Lwh2
@__Lwh2end LOD 0
RET
@classify ADD -1
JIZ __sw_body_1_1
ADD -1
JIZ __sw_body_1_2
JMP __sw_body_1_3
@__sw_body_1_1 LOD 11
RET
@__sw_body_1_2 LOD 22
RET
@__sw_body_1_3 LOD 99
RET
@maybe SET maybe_x
EQU 0
JIZ __Lif3else
RET
@__Lif3else LOD 500
ADD maybe_x
OUT 0
RET
@main @__Lwh4 LOD 10
CAL add5
OUT 0
LOD 4
CAL firstHit
OUT 0
LOD 23
CAL nestedFind
OUT 0
LOD 1
CAL classify
OUT 0
LOD 2
CAL classify
OUT 0
LOD 7
CAL classify
OUT 0
LOD 0
CAL maybe
LOD 3
CAL maybe
LOD 600
OUT 0
JMP __Lwh4
@__Lwh4end @fim JMP fim
