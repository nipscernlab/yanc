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
#SHARE __dk1_rls_update rls_update_d
#SHARE rls_update_g rls_update_d
#SHARE __dd3_rls_update rls_update_e
#SHARE __dhf2_rls_update __dd2_rls_update
#SHARE __dk0_main __dk0_rls_update
#SHARE main_j __dk1_main
#SHARE __aux_var __dk0_rls_update
JMP main
@rls_update SET rls_update_d
LOD w
F_MLT x
P_LOD -3
SET __dk0_rls_update
@__Ld0 LDI_V w 4
P_LOD __dk0_rls_update
LDI_V x 4
SF_MLT
SF_ADD
P_LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JIZ __Ld0e
JMP __Ld0
@__Ld0e POP
F_SU2 rls_update_d
SET rls_update_e
#array rls_update_Px 2 4
LOD -4
SET __dd2_rls_update
LOD -16
SET __dk0_rls_update
@__Ld1 LOD __dd2_rls_update
P_LOD __dk0_rls_update
LDI_V P 16
F_MLT x
P_LOD -3
SET __dk1_rls_update
@__Ld2 ADD __dk0_rls_update
LDI_V P 20
P_LOD __dk1_rls_update
LDI_V x 4
SF_MLT
SF_ADD
P_LOD __dk1_rls_update
ADD 1
SET __dk1_rls_update
JIZ __Ld2e
JMP __Ld2
@__Ld2e POP
STI_V rls_update_Px 4
LOD __dd2_rls_update
ADD 1
SET __dd2_rls_update
LOD __dk0_rls_update
ADD 4
SET __dk0_rls_update
JIZ __Ld1e
JMP __Ld1
@__Ld1e LOD x
F_MLT rls_update_Px
P_LOD -3
SET __dk0_rls_update
@__Ld3 LDI_V x 4
P_LOD __dk0_rls_update
LDI_V rls_update_Px 4
SF_MLT
SF_ADD
P_LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JIZ __Ld3e
JMP __Ld3
@__Ld3e POP
F_ADD 0.99
F_DIV 1.0
NOP
NOP
F_QUO
SET rls_update_g
#array rls_update_K 2 4
LOD -4
SET __dk0_rls_update
@__Ld4 PSH
LDI_V rls_update_Px 4
F_MLT rls_update_g
STI_V rls_update_K 4
LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JIZ __Ld4e
JMP __Ld4
@__Ld4e LOD -4
SET __dk0_rls_update
@__Ld5 PSH
LDI_V w 4
P_LOD __dk0_rls_update
LDI_V rls_update_K 4
F_MLT rls_update_e
SF_ADD
STI_V w 4
LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JIZ __Ld5e
JMP __Ld5
@__Ld5e LOD -4
SET __dd3_rls_update
LOD -16
SET __dk0_rls_update
@__Ld6 LOD __dd3_rls_update
LDI_V rls_update_K 4
SET __dhf2_rls_update
LOD -4
SET __dk1_rls_update
@__Ld7 ADD __dk0_rls_update
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
JIZ __Ld7e
JMP __Ld7
@__Ld7e LOD __dd3_rls_update
ADD 1
SET __dd3_rls_update
LOD __dk0_rls_update
ADD 4
SET __dk0_rls_update
JIZ __Ld6e
JMP __Ld6
@__Ld6e LOD -16
SET __dk0_rls_update
@__Ld8 PSH
LDI_V P 16
F_MLT 1.010101
STI_V P 16
LOD __dk0_rls_update
ADD 1
SET __dk0_rls_update
JIZ __Ld8e
JMP __Ld8
@__Ld8e RET
@main LOD -16
SET __dk0_main
@__Ld9 P_LOD 0.0
STI_V P 16
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld9e
JMP __Ld9
@__Ld9e LOD -20
SET __dk1_main
@__Ld10 P_LOD 1000.0
STI_V P 20
LOD __dk1_main
ADD 5
SET __dk1_main
JIZ __Ld10e
JMP __Ld10
@__Ld10e LOD -4
SET __dk0_main
@__Ld11 P_LOD 0.0
STI_V w 4
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld11e
JMP __Ld11
@__Ld11e LOD 0
SET main_j
@__Lwh1 LOD 20
LES main_j
JIZ fim
F_INN 0
F_MLT 0.001
SET __aux_var
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
LOD __aux_var
SET x
F_INN 0
F_MLT 0.001
SET __aux_var
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
LOD __aux_var
SET x
F_INN 0
F_MLT 0.001
SET __aux_var
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
LOD __aux_var
SET x
F_INN 0
F_MLT 0.001
SET __aux_var
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
LOD __aux_var
SET x
INN 1
I2F
F_MLT 0.001
CAL rls_update
LOD -4
SET __dk0_main
@__Ld12 LDI_V w 4
F_MLT 1000.0
F2I
OUT 0
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld12e
JMP __Ld12
@__Ld12e LOD main_j
ADD 1
SET main_j
JMP __Lwh1
@__Lwh1end @fim JMP fim
