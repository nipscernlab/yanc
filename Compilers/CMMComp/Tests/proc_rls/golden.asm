NOP
#PRNAME proc_rls
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NDSTAC 5
#SDEPTH 5
#NUIOIN 2
#NUIOOU 2
#array x 2 4
#array w 2 4
#array P 2 16
LOD 4
SET P_arr_size
JMP main
@rls_update SET rls_update_d
LOD w
F_MLT x
P_LOD_V w 1
F_MLT_V x 1
SF_ADD
P_LOD_V w 2
F_MLT_V x 2
SF_ADD
P_LOD_V w 3
F_MLT_V x 3
SF_ADD
F_SU2 rls_update_d
SET rls_update_e
#array rls_update_Px 2 4
LOD_V P 0
F_MLT x
P_LOD_V P 1
F_MLT_V x 1
SF_ADD
P_LOD_V P 2
F_MLT_V x 2
SF_ADD
P_LOD_V P 3
F_MLT_V x 3
SF_ADD
SET_V rls_update_Px 0
LOD_V P 4
F_MLT x
P_LOD_V P 5
F_MLT_V x 1
SF_ADD
P_LOD_V P 6
F_MLT_V x 2
SF_ADD
P_LOD_V P 7
F_MLT_V x 3
SF_ADD
SET_V rls_update_Px 1
LOD_V P 8
F_MLT x
P_LOD_V P 9
F_MLT_V x 1
SF_ADD
P_LOD_V P 10
F_MLT_V x 2
SF_ADD
P_LOD_V P 11
F_MLT_V x 3
SF_ADD
SET_V rls_update_Px 2
LOD_V P 12
F_MLT x
P_LOD_V P 13
F_MLT_V x 1
SF_ADD
P_LOD_V P 14
F_MLT_V x 2
SF_ADD
P_LOD_V P 15
F_MLT_V x 3
SF_ADD
SET_V rls_update_Px 3
LOD x
F_MLT rls_update_Px
P_LOD_V x 1
F_MLT_V rls_update_Px 1
SF_ADD
P_LOD_V x 2
F_MLT_V rls_update_Px 2
SF_ADD
P_LOD_V x 3
F_MLT_V rls_update_Px 3
SF_ADD
F_ADD 0.99
F_DIV 1.0
SET rls_update_g
#array rls_update_K 2 4
LOD_V rls_update_Px 0
F_MLT rls_update_g
SET_V rls_update_K 0
LOD_V rls_update_Px 1
F_MLT rls_update_g
SET_V rls_update_K 1
LOD_V rls_update_Px 2
F_MLT rls_update_g
SET_V rls_update_K 2
LOD_V rls_update_Px 3
F_MLT rls_update_g
SET_V rls_update_K 3
LOD_V rls_update_K 0
F_MLT rls_update_e
F_ADD_V w 0
SET_V w 0
LOD_V rls_update_K 1
F_MLT rls_update_e
F_ADD_V w 1
SET_V w 1
LOD_V rls_update_K 2
F_MLT rls_update_e
F_ADD_V w 2
SET_V w 2
LOD_V rls_update_K 3
F_MLT rls_update_e
F_ADD_V w 3
SET_V w 3
LOD_V rls_update_K 0
F_MLT_V rls_update_Px 0
F_NEG
F_ADD_V P 0
SET_V P 0
LOD_V rls_update_K 0
F_MLT_V rls_update_Px 1
F_NEG
F_ADD_V P 4
SET_V P 4
LOD_V rls_update_K 0
F_MLT_V rls_update_Px 2
F_NEG
F_ADD_V P 8
SET_V P 8
LOD_V rls_update_K 0
F_MLT_V rls_update_Px 3
F_NEG
F_ADD_V P 12
SET_V P 12
LOD_V rls_update_K 1
F_MLT_V rls_update_Px 0
F_NEG
F_ADD_V P 1
SET_V P 1
LOD_V rls_update_K 1
F_MLT_V rls_update_Px 1
F_NEG
F_ADD_V P 5
SET_V P 5
LOD_V rls_update_K 1
F_MLT_V rls_update_Px 2
F_NEG
F_ADD_V P 9
SET_V P 9
LOD_V rls_update_K 1
F_MLT_V rls_update_Px 3
F_NEG
F_ADD_V P 13
SET_V P 13
LOD_V rls_update_K 2
F_MLT_V rls_update_Px 0
F_NEG
F_ADD_V P 2
SET_V P 2
LOD_V rls_update_K 2
F_MLT_V rls_update_Px 1
F_NEG
F_ADD_V P 6
SET_V P 6
LOD_V rls_update_K 2
F_MLT_V rls_update_Px 2
F_NEG
F_ADD_V P 10
SET_V P 10
LOD_V rls_update_K 2
F_MLT_V rls_update_Px 3
F_NEG
F_ADD_V P 14
SET_V P 14
LOD_V rls_update_K 3
F_MLT_V rls_update_Px 0
F_NEG
F_ADD_V P 3
SET_V P 3
LOD_V rls_update_K 3
F_MLT_V rls_update_Px 1
F_NEG
F_ADD_V P 7
SET_V P 7
LOD_V rls_update_K 3
F_MLT_V rls_update_Px 2
F_NEG
F_ADD_V P 11
SET_V P 11
LOD_V rls_update_K 3
F_MLT_V rls_update_Px 3
F_NEG
F_ADD_V P 15
SET_V P 15
LOD_V P 0
F_MLT 1.010101
SET_V P 0
LOD_V P 1
F_MLT 1.010101
SET_V P 1
LOD_V P 2
F_MLT 1.010101
SET_V P 2
LOD_V P 3
F_MLT 1.010101
SET_V P 3
LOD_V P 4
F_MLT 1.010101
SET_V P 4
LOD_V P 5
F_MLT 1.010101
SET_V P 5
LOD_V P 6
F_MLT 1.010101
SET_V P 6
LOD_V P 7
F_MLT 1.010101
SET_V P 7
LOD_V P 8
F_MLT 1.010101
SET_V P 8
LOD_V P 9
F_MLT 1.010101
SET_V P 9
LOD_V P 10
F_MLT 1.010101
SET_V P 10
LOD_V P 11
F_MLT 1.010101
SET_V P 11
LOD_V P 12
F_MLT 1.010101
SET_V P 12
LOD_V P 13
F_MLT 1.010101
SET_V P 13
LOD_V P 14
F_MLT 1.010101
SET_V P 14
LOD_V P 15
F_MLT 1.010101
SET_V P 15
RET
@main LOD 1000.0
SET_V P 0
SET_V P 5
SET_V P 10
SET_V P 15
LOD 0.0
SET_V P 1
SET_V P 2
SET_V P 3
SET_V P 4
SET_V P 6
SET_V P 7
SET_V P 8
SET_V P 9
SET_V P 11
SET_V P 12
SET_V P 13
SET_V P 14
LOD 0.0
SET_V w 0
SET_V w 1
SET_V w 2
SET_V w 3
LOD 0
SET main_j
@Lwh1 LOD 20
LES main_j
JIZ Lwh1end
F_INN 0
F_MLT 0.001
SET aux_var
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
LOD aux_var
SET x
F_INN 0
F_MLT 0.001
SET aux_var
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
LOD aux_var
SET x
F_INN 0
F_MLT 0.001
SET aux_var
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
LOD aux_var
SET x
F_INN 0
F_MLT 0.001
SET aux_var
LOD_V x 2
SET_V x 3
LOD_V x 1
SET_V x 2
LOD_V x 0
SET_V x 1
LOD aux_var
SET x
INN 1
I2F
F_MLT 0.001
CAL rls_update
LOD_V w 0
F_MLT 1000.0
F2I
OUT 0
LOD_V w 1
F_MLT 1000.0
F2I
OUT 0
LOD_V w 2
F_MLT 1000.0
F2I
OUT 0
LOD_V w 3
F_MLT 1000.0
F2I
OUT 0
LOD main_j
ADD 1
SET main_j
JMP Lwh1
@Lwh1end @fim JMP fim
