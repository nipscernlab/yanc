NOP
#PRNAME cmm_flit
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#arrayd g 2 3 3 1.0e1 0.5 2.5e-1
#arrayd c 3 1 1 1.0e-1
#arrayd c_i 4 1 1 -2.0e1
#SHARE main_n main_z
#SHARE __aux_var main_z
#SHARE main_w_i main_z
#SHARE main_w main_z_i
@main LOD 1.0e-8
F_MLT 1.0e8
F_ADD 0.5
F2I
OUT 0
LOD 1.0e-8
SET main_z
LOD 2.0e-8
SET main_z_i
LOD main_z
F_MLT 1.0e8
F_ADD 0.5
F2I
OUT 0
LOD main_z_i
F_MLT 1.0e8
F_ADD 0.5
F2I
OUT 0
F2I_M 2.5e3
OUT 0
LOD 0.5
F_MLT 4.0
F2I
OUT 0
F2I_M 5.0
OUT 0
LOD 7.5
F_MLT 2.0
F2I
OUT 0
LOD 1.5
F_MLT 2.0
F2I
OUT 0
LOD -2.5e1
SET main_n
F2I_M main_n
OUT 0
LOD 10.0
P_LOD 10.0
SET_P __aux_var
F_ADD -3.0e0
P_LOD __aux_var
F_ADD -4.0e0
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD_V g 0
F2I
OUT 0
LOD_V g 1
F_MLT 4.0
F2I
OUT 0
LOD_V g 2
F_MLT 8.0
F2I
OUT 0
LOD_V c 0
F_MLT 10.0
F_ADD 0.5
F2I
OUT 0
LOD_V c_i 0
F2I
OUT 0
@__Lwh1 JMP __Lwh1
@__Lwh1end @fim JMP fim
