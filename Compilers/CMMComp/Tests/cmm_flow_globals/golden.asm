NOP
#PRNAME cmm_flow_globals
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE h_x k
LOD 3
SET a
LOD 5
SET g
LOD 1
CAL h
SET k
LOD 7
SET vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
JMP main
@f LES 0
JIZ __Lif1else
LOD 1
RET
@__Lif1else LOD 2
RET
@__Lif1end @h SET h_x
@__Lwh1 LOD h_x
LES 3
JIZ __Lif2else
LOD h_x
RET
@__Lif2else LOD h_x
ADD 1
SET h_x
JMP __Lwh1
@__Lwh1end @s ADD -1
JIZ __sw_body_1_1
JMP __sw_body_1_2
@__sw_body_1_1 LOD 10
RET
@__sw_body_1_2 LOD 20
RET
@__switch_end_1 @main LOD a
OUT 0
LOD g
OUT 0
LOD k
OUT 0
LOD 1
CAL f
OUT 0
LOD 0
CAL f
OUT 0
LOD 1
CAL s
OUT 0
LOD 7
CAL s
OUT 0
LOD vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
OUT 0
@fim JMP fim
