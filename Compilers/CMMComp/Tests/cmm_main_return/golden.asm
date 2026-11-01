NOP
#PRNAME cmm_main_return
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE main_n early_p
JMP main
@early SET early_p
JIZ __Lif1else
LOD 10
OUT 0
RET
@__Lif1else LOD 20
OUT 0
RET
@main LOD 1
CAL early
LOD 0
CAL early
LOD 2
SET main_n
EQU 2
JIZ __Lif2else
LOD 5
OUT 0
JMP fim
@__Lif2else LOD 6
OUT 0
@fim JMP fim
