NOP
#PRNAME cmm_comp_mix
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE __aux_var main_r_i
@main @__Lwh1 LOD 3.0
SET main_c
LOD 4.0
SET main_c_i
LOD 6.0
SET main_c2
LOD 8.0
SET main_c2_i
LOD 2
SET main_ni
LOD 2.0
SET main_nf
LOD 2.0
F_ADD main_c
P_LOD  main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_SU2 main_c
P_LOD main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_MLT main_c
P_LOD 2.0
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_DIV main_c2
NOP
NOP
F_QUO
P_LOD 2.0
F_DIV main_c2_i
NOP
NOP
F_QUO
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_ADD main_c
P_LOD  main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_SU1 main_c
PF_NEG_M  main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_MLT main_c
P_LOD 2.0
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD main_c
F_MLT main_c
P_LOD main_c_i
F_MLT main_c_i
SF_ADD
SET __aux_var
LOD 25.0
F_MLT main_c
P_LOD __aux_var
SF_DIV
NOP
NOP
F_QUO
P_LOD 25.0
F_MLT main_c_i
P_LOD __aux_var
SF_DIV
NOP
NOP
F_QUO
F_NEG
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_ADD main_c
P_LOD main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_SU2 main_c
P_LOD main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_MLT main_c
P_LOD 2.0
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_DIV main_c2
NOP
NOP
F_QUO
P_LOD 2.0
F_DIV main_c2_i
NOP
NOP
F_QUO
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_ADD main_c
P_LOD main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_SU1 main_c
PF_NEG_M main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_MLT main_c
P_LOD 2.0
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD main_c
F_MLT main_c
P_LOD main_c_i
F_MLT main_c_i
SF_ADD
SET   __aux_var
LOD 25.0
F_MLT main_c
P_LOD __aux_var
SF_DIV
NOP
NOP
F_QUO
P_LOD 25.0
F_MLT main_c_i
P_LOD __aux_var
SF_DIV
NOP
NOP
F_QUO
F_NEG
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_ADD 3.0
P_LOD 4.0
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 2.0
F_MLT 3.0
P_LOD 2.0
F_MLT 4.0
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 3.0
SET main_r
LOD 4.0
SET main_r_i
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD 3.0
SET main_r
LOD 4.0
SET main_r_i
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
I2F_M  main_ni
F_ADD main_c
P_LOD  main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD main_nf
F_MLT main_c
P_LOD main_nf
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
