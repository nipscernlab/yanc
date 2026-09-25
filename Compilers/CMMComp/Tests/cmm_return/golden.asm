NOP
#PRNAME cmm_return
#NUBITS 16
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
#SHARE firstHit_limit add5_a
#SHARE nestedFind_target add5_a
#SHARE nestedFind_a firstHit_k
#SHARE classify_x add5_a
#SHARE maybe_x add5_a
JMP main
@add5 SET add5_a
ADD 5
RET
@firstHit SET firstHit_limit
LOD 0
SET firstHit_k
@Lwh1 LOD firstHit_limit
EQU firstHit_k
JIZ Lif1else
LOD firstHit_k
ADD firstHit_k
RET
@Lif1else LOD firstHit_k
ADD 1
SET firstHit_k
LES 99
JIZ Lwh1
@Lwh1end LOD 0
RET
@nestedFind SET nestedFind_target
LOD 0
SET nestedFind_a
@Lwh2 LOD 0
SET nestedFind_b
@Lwh3 LOD nestedFind_a
MLT 10
ADD nestedFind_b
EQU nestedFind_target
JIZ Lif2else
LOD nestedFind_a
MLT 100
ADD nestedFind_b
RET
@Lif2else LOD nestedFind_b
ADD 1
SET nestedFind_b
LES 4
JIZ Lwh3
@Lwh3end LOD nestedFind_a
ADD 1
SET nestedFind_a
LES 4
JIZ Lwh2
@Lwh2end LOD 0
RET
@classify SET classify_x
ADD -1
JIZ sw_body_1_1
ADD -1
JIZ sw_body_1_2
JMP sw_body_1_3
@sw_body_1_1 LOD 11
RET
@sw_body_1_2 LOD 22
RET
@sw_body_1_3 LOD 99
RET
@switch_end_1 LOD 0
RET
@maybe SET maybe_x
LOD 0
EQU maybe_x
JIZ Lif3else
RET
@Lif3else LOD 500
ADD maybe_x
OUT 0
RET
@main @Lwh4 LOD 10
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
JMP Lwh4
@Lwh4end @fim JMP fim
