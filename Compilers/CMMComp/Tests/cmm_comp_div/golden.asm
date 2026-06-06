NOP
#PRNAME cmm_comp_div
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
LOD 4.000000
SET main_x
LOD 2.000000
SET main_x_i
LOD 1.000000
SET main_y
LOD 1.000000
SET main_y_i
F_MLT main_y_i
P_LOD main_y
F_MLT main_y
SF_ADD
SET   aux_var
LOD main_x
F_MLT main_y
P_LOD main_x_i
F_MLT main_y_i
SF_ADD
P_LOD aux_var
SF_DIV
P_LOD main_x_i
F_MLT main_y
P_LOD main_x
F_MLT main_y_i
SF_SU2
P_LOD aux_var
SF_DIV
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 8.000000
SET main_x
LOD 6.000000
SET main_x_i
LOD 1.000000
SET main_y
LOD -1.000000
SET main_y_i
F_MLT main_y_i
P_LOD main_y
F_MLT main_y
SF_ADD
SET   aux_var
LOD main_x
F_MLT main_y
P_LOD main_x_i
F_MLT main_y_i
SF_ADD
P_LOD aux_var
SF_DIV
P_LOD main_x_i
F_MLT main_y
P_LOD main_x
F_MLT main_y_i
SF_SU2
P_LOD aux_var
SF_DIV
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 6.000000
SET main_x
LOD 8.000000
SET main_x_i
LOD 2.000000
SET main_y
LOD 0.000000
SET main_y_i
F_MLT main_y_i
P_LOD main_y
F_MLT main_y
SF_ADD
SET   aux_var
LOD main_x
F_MLT main_y
P_LOD main_x_i
F_MLT main_y_i
SF_ADD
P_LOD aux_var
SF_DIV
P_LOD main_x_i
F_MLT main_y
P_LOD main_x
F_MLT main_y_i
SF_SU2
P_LOD aux_var
SF_DIV
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
