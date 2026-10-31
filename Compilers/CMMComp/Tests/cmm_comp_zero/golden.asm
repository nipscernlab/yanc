NOP
#PRNAME cmm_comp_zero
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE __aux_var main_z_i
#SHARE __aux_var1 main_z
@main LOD 3.0
SET main_w
LOD 2.0
SET main_w_i
LOD 1.0
SET main_one
LOD 0.0
SET main_one_i
LOD main_w_i
F_MLT -2.0
P_LOD main_w
F_MLT 2.0
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD main_w_i
F_MLT -2.0
P_LOD main_w
F_MLT 2.0
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD main_w
F_MLT main_one
P_LOD main_w_i
F_MLT main_one_i
SF_SU2
P_LOD main_w
F_MLT main_one_i
P_LOD main_w_i
F_MLT main_one
SF_ADD
F_MLT -2.0
SET_P __aux_var
F_MLT 2.0
SET __aux_var1
LOD __aux_var
P_LOD __aux_var1
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD main_w_i
F_MLT 0.5
P_LOD main_w
F_MLT -0.5
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD 3.0
F_MLT main_w
P_LOD 3.0
F_MLT main_w_i
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD 3.0
F_ADD main_w
P_LOD main_w_i
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD 1.0
F_SU2 main_w
P_LOD main_w_i
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD 0.5
F_MLT main_w
P_LOD 0.5
F_MLT main_w_i
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD main_w
P_LOD main_w_i
F_ADD -2.0
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD main_w
F_MLT main_one
P_LOD main_w_i
F_MLT main_one_i
SF_SU2
P_LOD main_w
F_MLT main_one_i
P_LOD main_w_i
F_MLT main_one
SF_ADD
SET_P __aux_var
P_LOD __aux_var
F_ADD -0.5
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
LOD main_w
F_ADD -1.0
P_LOD main_w_i
F_ADD -1.0
SET_P main_z_i
SET main_z
LOD 4.0
F_MLT main_z
F2I
OUT 0
LOD 4.0
F_MLT main_z_i
F2I
OUT 0
@fim JMP fim
