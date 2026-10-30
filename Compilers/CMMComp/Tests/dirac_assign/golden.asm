NOP
#PRNAME dirac_assign
#array __dirac_tmp 2 4
#NUBITS 23
#NUIOIN 1
#NUIOOU 1
#NBMANT 16
#NBEXPO 6
#NUGAIN 128
#array A 2 16
#array B 2 16
#array a 2 4
#array b 2 4
#SHARE __dk4 __dk0
@main LOD -16
SET __dk0
@__Ld0 P_LOD 0.0
STI_V A 16
LOD __dk0
ADD 1
SET __dk0
JIZ __Ld0e
JMP __Ld0
@__Ld0e LOD -20
SET __dk1
@__Ld1 P_LOD 1.0
STI_V A 20
LOD __dk1
ADD 5
SET __dk1
JIZ __Ld1e
JMP __Ld1
@__Ld1e LOD -16
SET __dk0
@__Ld2 P_LOD 0.0
STI_V B 16
LOD __dk0
ADD 1
SET __dk0
JIZ __Ld2e
JMP __Ld2
@__Ld2e LOD -20
SET __dk1
@__Ld3 P_LOD 1.0
STI_V B 20
LOD __dk1
ADD 5
SET __dk1
JIZ __Ld3e
JMP __Ld3
@__Ld3e LOD -4
SET __dk0
@__Ld4 P_LOD 0.0
STI_V a 4
LOD __dk0
ADD 1
SET __dk0
JIZ __Ld4e
JMP __Ld4
@__Ld4e LOD -4
SET __dk0
@__Ld5 P_LOD 0.0
STI_V b 4
LOD __dk0
ADD 1
SET __dk0
JIZ __Ld5e
JMP __Ld5
@__Ld5e @__Lwh1 LOD_V a 0
SET __dhf2
LOD -4
SET __dd3
LOD -16
SET __dk0
@__Ld6 LOD __dd3
P_LOD __dk0
LDI_V B 16
F_MLT __dhf2
P_LOD -3
SET __dk1
@__Ld7 ADD __dk0
LDI_V B 20
P_LOD __dk1
LDI_V a 4
SF_MLT
SF_ADD
P_LOD __dk1
ADD 1
SET __dk1
JIZ __Ld7e
JMP __Ld7
@__Ld7e POP
STI_V __dirac_tmp 4
LOD __dd3
ADD 1
SET __dd3
LOD __dk0
ADD 4
SET __dk0
JIZ __Ld6e
JMP __Ld6
@__Ld6e LOD -4
SET __dk4
@__Ld8 PSH
LDI_V __dirac_tmp 4
STI_V a 4
LOD __dk4
ADD 1
SET __dk4
JIZ __Ld8e
JMP __Ld8
@__Ld8e LOD -4
SET __dk0
@__Ld9 PSH
LDI_V b 4
F_MLT 2.0
STI_V a 4
LOD __dk0
ADD 1
SET __dk0
JIZ __Ld9e
JMP __Ld9
@__Ld9e LOD -16
SET __dd3
LOD -4
SET __dk0
@__Ld10 LDI_V a 4
SET __dhf2
LOD -4
SET __dk1
@__Ld11 ADD __dd3
P_LOD __dk1
LDI_V b 4
F_MLT __dhf2
STI_V A 20
LOD __dk1
ADD 1
SET __dk1
JIZ __Ld11e
JMP __Ld11
@__Ld11e LOD __dd3
ADD 4
SET __dd3
LOD __dk0
ADD 1
SET __dk0
JIZ __Ld10e
JMP __Ld10
@__Ld10e LOD -4
SET __dk0
@__Ld12 PF_INN 0
F_MLT 0.001
STI_V a 4
LOD __dk0
ADD 1
SET __dk0
JIZ __Ld12e
JMP __Ld12
@__Ld12e LOD 1
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
