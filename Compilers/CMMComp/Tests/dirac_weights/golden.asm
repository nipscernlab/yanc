NOP
#PRNAME dirac_weights
#NUIOOU 1
#array M 2 12
#array P 2 12
#array Q 2 12
#array R 2 12
#SHARE __dd10_main main_k
#SHARE __dd12_main __dd11_main
#SHARE __dk3_main __dk2_main
#SHARE __dd13_main __dd11_main
#SHARE __dk4_main __dk2_main
#SHARE __dd14_main __dd11_main
#SHARE __dk5_main __dk2_main
#SHARE main_r main_k
#SHARE main_c __dk0_main
@main LOD 0
SET main_k
@__Lwh1 LOD main_k
MLT 4
ADD main_k
P_LOD 2.0
STI M
LOD main_k
MLT 3
ADD main_k
P_LOD 1.0
STI P
LOD main_k
MLT 3
ADD main_k
P_LOD 3.0
STI Q
LOD main_k
MLT 3
ADD 1
P_LOD 1.0
STI R
INC main_k
LES 2
JIZ __Lwh1
@__Lwh1end #array main_C 2 9
LOD -9
SET __dd10_main
LOD -12
SET __dk0_main
@__Ld0 LDI_V M 12
SET __dhf6_main
LOD __dk0_main
LDI_V M 12
SET __dhf7_main
LOD __dk0_main
LDI_V M 12
SET __dhf8_main
LOD __dk0_main
LDI_V M 12
SET __dhf9_main
LOD -3
SET __dk1_main
@__Ld1 ADD __dd10_main
P_LOD __dk1_main
LDI_V P 3
F_MLT __dhf6_main
P_LOD -9
SET __dd11_main
LOD -3
SET __dk2_main
@__Ld2 ADD __dk0_main
LDI_V M 16
P_LOD __dd11_main
ADD __dk1_main
LDI_V P 15
SF_MLT
SF_ADD
P_LOD __dd11_main
ADD 3
SET __dd11_main
INC __dk2_main
JNZ __Ld2
@__Ld2e POP
P_LOD __dk1_main
LDI_V Q 3
F_MLT __dhf7_main
P_LOD -9
SET __dd12_main
LOD -3
SET __dk3_main
@__Ld3 ADD __dk0_main
LDI_V M 16
P_LOD __dd12_main
ADD __dk1_main
LDI_V Q 15
SF_MLT
SF_ADD
P_LOD __dd12_main
ADD 3
SET __dd12_main
INC __dk3_main
JNZ __Ld3
@__Ld3e POP
SF_ADD
P_LOD __dk1_main
LDI_V R 3
F_MLT __dhf8_main
P_LOD -9
SET __dd13_main
LOD -3
SET __dk4_main
@__Ld4 ADD __dk0_main
LDI_V M 16
P_LOD __dd13_main
ADD __dk1_main
LDI_V R 15
SF_MLT
SF_ADD
P_LOD __dd13_main
ADD 3
SET __dd13_main
INC __dk4_main
JNZ __Ld4
@__Ld4e POP
SF_ADD
P_LOD __dk1_main
LDI_V P 3
F_MLT __dhf9_main
P_LOD -9
SET __dd14_main
LOD -3
SET __dk5_main
@__Ld5 ADD __dk0_main
LDI_V M 16
P_LOD __dd14_main
ADD __dk1_main
LDI_V P 15
SF_MLT
SF_ADD
P_LOD __dd14_main
ADD 3
SET __dd14_main
INC __dk5_main
JNZ __Ld5
@__Ld5e POP
SF_ADD
STI_V main_C 12
INC __dk1_main
JNZ __Ld1
@__Ld1e LOD __dd10_main
ADD 3
SET __dd10_main
LOD __dk0_main
ADD 4
SET __dk0_main
JNZ __Ld0
@__Ld0e LOD 0
SET main_r
@__Lwh2 LOD 0
SET main_c
@__Lwh3 LOD main_r
MLT 3
ADD main_c
LDI main_C
F2I
OUT 0
INC main_c
LES 2
JIZ __Lwh3
@__Lwh3end INC main_r
LES 2
JIZ __Lwh2
@__Lwh2end @fim JMP fim
