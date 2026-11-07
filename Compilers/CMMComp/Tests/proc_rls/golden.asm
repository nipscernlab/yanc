NOP
#PRNAME proc_rls
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 2
#NUIOOU 2
#array x 2 4
#array w 2 4
#array P 2 16
#SHARE main_j __dk1_main
#SHARE __dk0_rls_update __dk0_main
#SHARE __dk1_rls_update rls_update_d
#SHARE rls_update_g rls_update_d
#SHARE __dd3_rls_update rls_update_e
#SHARE __dhf2_rls_update __dd2_rls_update
@main LOD -16
SET __dk0_main
@__Ld0 P_LOD 0.0
STI_V P 16
LOD __dk0_main
ADD 1
SET __dk0_main
JNZ __Ld0
@__Ld0e LOD -20
SET __dk1_main
@__Ld1 P_LOD 1000.0
STI_V P 20
LOD __dk1_main
ADD 5
SET __dk1_main
JNZ __Ld1
@__Ld1e LOD -4
SET __dk0_main
@__Ld2 P_LOD 0.0
STI_V w 4
LOD __dk0_main
ADD 1
SET __dk0_main
JNZ __Ld2
@__Ld2e LOD 0
SET main_j
@__Lwh1top LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
F_INN 0
F_MLT 0.001
SET x
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
F_INN 0
F_MLT 0.001
SET x
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
F_INN 0
F_MLT 0.001
SET x
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
F_INN 0
F_MLT 0.001
SET x
INN 1
I2F
F_MLT 0.001
CAL rls_update
LOD -4
SET __dk0_main
@__Ld3 LDI_V w 4
F_MLT 1000.0
F2I
OUT 0
LOD __dk0_main
ADD 1
SET __dk0_main
JNZ __Ld3
@__Ld3e LOD main_j
ADD 1
SET main_j
LES 19
JIZ __Lwh1top
@__Lwh1end @fim JMP fim
@rls_update SET rls_update_d
LOD w
F_MLT x
P_LOD -3
SET __dk0_rls_update
@__Ld4 LDI_V w 4
P_LOD __dk0_rls_update
LDI_V x 4
SF_MLT
SF_ADD
P_LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JNZ __Ld4
@__Ld4e POP
F_SU2 rls_update_d
SET rls_update_e
#array rls_update_Px 2 4
LOD -4
SET __dd2_rls_update
LOD -16
SET __dk0_rls_update
@__Ld5 LOD __dd2_rls_update
P_LOD __dk0_rls_update
LDI_V P 16
F_MLT x
P_LOD -3
SET __dk1_rls_update
@__Ld6 ADD __dk0_rls_update
LDI_V P 20
P_LOD __dk1_rls_update
LDI_V x 4
SF_MLT
SF_ADD
P_LOD __dk1_rls_update
ADD 1
SET __dk1_rls_update
JNZ __Ld6
@__Ld6e POP
STI_V rls_update_Px 4
LOD __dd2_rls_update
ADD 1
SET __dd2_rls_update
LOD __dk0_rls_update
ADD 4
SET __dk0_rls_update
JNZ __Ld5
@__Ld5e LOD x
F_MLT rls_update_Px
P_LOD -3
SET __dk0_rls_update
@__Ld7 LDI_V x 4
P_LOD __dk0_rls_update
LDI_V rls_update_Px 4
SF_MLT
SF_ADD
P_LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JNZ __Ld7
@__Ld7e POP
F_ADD 0.99
F_DIV 1.0
NOP
NOP
F_QUO
SET rls_update_g
#array rls_update_K 2 4
LOD -4
SET __dk0_rls_update
@__Ld8 PSH
LDI_V rls_update_Px 4
F_MLT rls_update_g
STI_V rls_update_K 4
LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JNZ __Ld8
@__Ld8e LOD -4
SET __dk0_rls_update
@__Ld9 PSH
LDI_V w 4
P_LOD __dk0_rls_update
LDI_V rls_update_K 4
F_MLT rls_update_e
SF_ADD
STI_V w 4
LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JNZ __Ld9
@__Ld9e LOD -4
SET __dd3_rls_update
LOD -16
SET __dk0_rls_update
@__Ld10 LOD __dd3_rls_update
LDI_V rls_update_K 4
SET __dhf2_rls_update
LOD -4
SET __dk1_rls_update
@__Ld11 ADD __dk0_rls_update
PSH
LDI_V P 20
P_LOD __dk1_rls_update
LDI_V rls_update_Px 4
F_MLT __dhf2_rls_update
SF_SU2
STI_V P 20
LOD __dk1_rls_update
ADD 1
SET __dk1_rls_update
JNZ __Ld11
@__Ld11e LOD __dd3_rls_update
ADD 1
SET __dd3_rls_update
LOD __dk0_rls_update
ADD 4
SET __dk0_rls_update
JNZ __Ld10
@__Ld10e LOD -16
SET __dk0_rls_update
@__Ld12 PSH
LDI_V P 16
F_MLT 1.010101
STI_V P 16
LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JNZ __Ld12
@__Ld12e RET
