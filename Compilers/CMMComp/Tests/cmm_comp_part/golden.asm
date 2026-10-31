NOP
#PRNAME cmm_comp_part
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
@main #arrayd main_zv 3 3 3 1.0 3.0 5.0
#arrayd main_zv_i 4 3 3 2.0 -4.0 6.0
#arrayd main_zm 3 4 4 1.0 2.0 3.0 4.0
#arrayd main_zm_i 4 4 4 1.0 2.0 3.0 4.0
#SHARE __aux_var main_k
#SHARE __aux_var1 main_a
#SHARE __aux_var2 main_a_i
LOD 1
SET main_k
LOD 1.0
SET main_a
LOD 2.0
SET main_a_i
LOD 3.0
SET main_b
LOD -2.0
SET main_b_i
LOD main_k
LDI main_zv
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD main_k
LDI main_zv_i
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V main_zv 2
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V main_zv_i 2
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD main_k
ADD 1
LDI main_zv_i
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD main_k
LDI main_zv_i
F_ADD 1.0
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD main_k
MLT 2
LDI main_zm
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD 1
MLT 2
ADD main_k
LDI main_zm_i
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V main_zm_i 1
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD main_a
F_MLT main_b
P_LOD main_a_i
F_MLT main_b_i
SF_SU2
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD main_a
F_MLT main_b_i
P_LOD main_a_i
F_MLT main_b
SF_ADD
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V main_zv 0
P_LOD_V main_zv_i 0
P_LOD_V main_zv 1
P_LOD_V main_zv_i 1
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
POP
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
LOD_V main_zv 0
P_LOD_V main_zv_i 0
P_LOD_V main_zv 1
P_LOD_V main_zv_i 1
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var 
SET_P __aux_var
LOD   __aux_var
SET main_y
LOD 4.0
F_MLT main_y
F2I
OUT 0
@fim JMP fim
