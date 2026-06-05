NOP
#PRNAME cmm_comp_mix
#NUBITS 32
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 3.000000
SET main_c
LOD 4.000000
SET main_c_i
LOD 6.000000
SET main_c2
LOD 8.000000
SET main_c2_i
LOD 2
SET main_ni
LOD 2.0
SET main_nf
I2F_M  2
F_ADD main_c
P_LOD  main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M  2
F_SU2 main_c
P_LOD main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M 2
F_MLT main_c
P_I2F_M 2
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M 2
F_DIV main_c2
P_I2F_M 2
F_DIV main_c2_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M 2
F_ADD main_c
P_LOD  main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M 2
F_SU1 main_c
PF_NEG_M  main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M 2
F_MLT main_c
P_I2F_M 2
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD main_c
F_MLT main_c
P_LOD main_c_i
F_MLT main_c_i
SF_ADD
SET aux_var
I2F_M 25
F_MLT main_c
P_LOD aux_var
SF_DIV
P_I2F_M 25
F_MLT main_c_i
P_LOD aux_var
SF_DIV
F_NEG
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 2.0
F_ADD main_c
P_LOD main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 2.0
F_SU2 main_c
P_LOD main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 2.0
F_MLT main_c
P_LOD 2.0
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 2.0
F_DIV main_c2
P_LOD 2.0
F_DIV main_c2_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 2.0
F_ADD main_c
P_LOD main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 2.0
F_SU1 main_c
PF_NEG_M main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 2.0
F_MLT main_c
P_LOD 2.0
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD main_c
F_MLT main_c
P_LOD main_c_i
F_MLT main_c_i
SF_ADD
SET   aux_var
LOD 25.0
F_MLT main_c
P_LOD aux_var
SF_DIV
P_LOD 25.0
F_MLT main_c_i
P_LOD aux_var
SF_DIV
F_NEG
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M 2
F_ADD 3.000000
P_LOD 4.000000
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M 2
F_MLT 3.000000
P_I2F_M 2
F_MLT 4.000000
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 3.000000
SET main_r
LOD 4.000000
SET main_r_i
LOD main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 3.000000
SET main_r
LOD 4.000000
SET main_r_i
LOD main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
I2F_M  main_ni
F_ADD main_c
P_LOD  main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD main_nf
F_MLT main_c
P_LOD main_nf
F_MLT main_c_i
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
