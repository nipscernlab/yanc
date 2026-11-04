NOP
#PRNAME cmm_tailcall
#NUIOOU 1
@main LOD 1
SET acc
CAL twice
LOD acc
OUT 0
LOD 4
CAL sq_plus
OUT 0
LOD 2
CAL deep
ADD 1
OUT 0
LOD 1
CAL maybe
NEG_M 1
CAL maybe
LOD acc
OUT 0
@fim JMP fim
@add3 LOD acc
ADD 3
SET acc
RET
@twice CAL add3
JMP add3
@sq SET sq_a
MLT sq_a
RET
@sq_plus ADD 1
JMP sq
@deep JMP sq_plus
@maybe LES 0
JIZ __Lif1else
CAL add3
@__Lif1else RET
