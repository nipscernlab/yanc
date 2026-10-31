NOP
#PRNAME cmm_div_const
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE main_z_i main_f
#SHARE main_z main_n
#SHARE __aux_var main_w
@main LOD 3.0
SET main_a
LOD 1.0
SET main_f
LOD 10
SET main_n
LOD 1.0
SET main_w
LOD 2.0
SET main_w_i
LOD main_a
F_MLT 0.625
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_f
F_MLT 0.333333333
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD main_a
F_ADD main_f
F_MLT 0.142857143
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
I2F_M main_n
F_MLT 0.333333333
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
LOD 0.333333333
F_MLT main_w
P_LOD 0.333333333
F_MLT main_w_i
SET_P main_z_i
SET main_z
LOD 1000.0
F_MLT main_z
F2I
OUT 0
LOD 1000.0
F_MLT main_z_i
F2I
OUT 0
LOD 2.0
F_MLT main_w
P_LOD 2.0
F_MLT main_w_i
SET_P __aux_var
F_ADD 4.0
P_LOD __aux_var
F_ADD 4.0
SET_P __aux_var
F_MLT 0.142857143
P_LOD __aux_var
F_MLT 0.142857143
SET_P main_z_i
SET main_z
LOD 1000.0
F_MLT main_z
F2I
OUT 0
LOD 1000.0
F_MLT main_z_i
F2I
OUT 0
LOD main_a
F_MLT 0.142857143
F_ADD 1000.0
F_SU1 1000.0
SET main_y
LOD 1000.0
F_MLT main_y
F2I
OUT 0
@fim JMP fim
