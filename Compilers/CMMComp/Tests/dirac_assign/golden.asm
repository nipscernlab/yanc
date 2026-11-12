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
#SHARE __dk4_main __dk0_main
@main LOD -16
SET __dk0_main
@__Ld0 P_LOD 0.0
STI_V A 16
INC __dk0_main
JNZ __Ld0
@__Ld0e LOD -20
SET __dk1_main
@__Ld1 P_LOD 1.0
STI_V A 20
LOD __dk1_main
ADD 5
SET __dk1_main
JNZ __Ld1
@__Ld1e LOD -16
SET __dk0_main
@__Ld2 P_LOD 0.0
STI_V B 16
INC __dk0_main
JNZ __Ld2
@__Ld2e LOD -20
SET __dk1_main
@__Ld3 P_LOD 1.0
STI_V B 20
LOD __dk1_main
ADD 5
SET __dk1_main
JNZ __Ld3
@__Ld3e LOD -4
SET __dk0_main
@__Ld4 P_LOD 0.0
STI_V a 4
INC __dk0_main
JNZ __Ld4
@__Ld4e LOD -4
SET __dk0_main
@__Ld5 P_LOD 0.0
STI_V b 4
INC __dk0_main
JNZ __Ld5
@__Ld5e @__Lwh1 LOD_V a 0
SET __dhf2_main
LOD -4
SET __dd3_main
LOD -16
SET __dk0_main
@__Ld6 LOD __dd3_main
P_LOD __dk0_main
LDI_V B 16
F_MLT __dhf2_main
P_LOD -3
SET __dk1_main
@__Ld7 ADD __dk0_main
LDI_V B 20
P_LOD __dk1_main
LDI_V a 4
SF_MLT
SF_ADD
P_INC __dk1_main
JNZ __Ld7
@__Ld7e POP
STI_V __dirac_tmp 4
INC __dd3_main
LOD __dk0_main
ADD 4
SET __dk0_main
JNZ __Ld6
@__Ld6e LOD -4
SET __dk4_main
@__Ld8 PSH
LDI_V __dirac_tmp 4
STI_V a 4
INC __dk4_main
JNZ __Ld8
@__Ld8e LOD -4
SET __dk0_main
@__Ld9 PSH
LDI_V b 4
F_MLT 2.0
STI_V a 4
INC __dk0_main
JNZ __Ld9
@__Ld9e LOD -16
SET __dd3_main
LOD -4
SET __dk0_main
@__Ld10 LDI_V a 4
SET __dhf2_main
LOD -4
SET __dk1_main
@__Ld11 ADD __dd3_main
P_LOD __dk1_main
LDI_V b 4
F_MLT __dhf2_main
STI_V A 20
INC __dk1_main
JNZ __Ld11
@__Ld11e LOD __dd3_main
ADD 4
SET __dd3_main
INC __dk0_main
JNZ __Ld10
@__Ld10e LOD -4
SET __dk0_main
@__Ld12 PF_INN 0
F_MLT 0.001
STI_V a 4
INC __dk0_main
JNZ __Ld12
@__Ld12e LOD 1
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
