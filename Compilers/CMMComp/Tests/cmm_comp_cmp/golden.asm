NOP
#PRNAME cmm_comp_cmp
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE __aux_var2 __aux_var
@main LOD 3.0
SET main_z
LOD 4.0
SET main_z_i
LOD 3.0
SET main_w
LOD 4.0
SET main_w_i
SET main_u
LOD 3.0
SET main_u_i
LOD 1.0
SET main_e
LOD 0.0
SET main_e_i
LOD 3.0
SET main_r
LOD 0.0
SET main_r_i
LOD 3.0
SET main_x
LOD 3
SET main_n
LOD main_z
EQU main_w
P_LOD main_z_i
EQU main_w_i
S_AND
OUT 0
LOD main_z
EQU main_u
P_LOD main_z_i
EQU main_u_i
S_AND
OUT 0
LOD main_z
EQU main_u
P_LOD main_z_i
EQU main_u_i
S_AND
LIN
OUT 0
LOD main_z
EQU main_w
P_LOD main_z_i
EQU main_w_i
S_AND
LIN
OUT 0
LOD main_r
EQU main_x
P_LOD main_r_i
EQU 0.0
S_AND
OUT 0
LOD main_x
EQU main_r
P_LOD 0.0
EQU main_r_i
S_AND
OUT 0
I2F_M main_n
EQU main_r
P_LOD 0.0
EQU main_r_i
S_AND
OUT 0
I2F_M main_n
EQU main_r
P_LOD 0.0
EQU main_r_i
S_AND
OUT 0
LOD main_z
EQU main_x
P_LOD main_z_i
EQU 0.0
S_AND
OUT 0
I2F_M main_n
EQU main_z
P_LOD 0.0
EQU main_z_i
S_AND
OUT 0
LOD main_r
EQU 3.0
P_LOD main_r_i
EQU 0.0
S_AND
OUT 0
LOD main_r
EQU 3.0
P_LOD main_r_i
EQU 0.0
S_AND
OUT 0
LOD main_z
EQU 3.0
P_LOD main_z_i
EQU 4.0
S_AND
OUT 0
LOD 3.0
EQU main_z
P_LOD 4.0
EQU main_z_i
S_AND
OUT 0
LOD main_z
EQU 4.0
P_LOD main_z_i
EQU 3.0
S_AND
OUT 0
LOD main_z
F_MLT main_e
P_LOD main_z_i
F_MLT main_e_i
SF_SU2
P_LOD main_z
F_MLT main_e_i
P_LOD main_z_i
F_MLT main_e
SF_ADD
EQU main_w_i
SET_P __aux_var
EQU main_w
AND __aux_var
OUT 0
LOD main_z
F_MLT main_e
P_LOD main_z_i
F_MLT main_e_i
SF_SU2
P_LOD main_z
F_MLT main_e_i
P_LOD main_z_i
F_MLT main_e
SF_ADD
EQU main_w_i
SET_P __aux_var
EQU main_w
AND __aux_var
OUT 0
LOD main_z
F_MLT main_e
P_LOD main_z_i
F_MLT main_e_i
SF_SU2
P_LOD main_z
F_MLT main_e_i
P_LOD main_z_i
F_MLT main_e
SF_ADD
P_LOD main_u
F_MLT main_e
P_LOD main_u_i
F_MLT main_e_i
SF_SU2
P_LOD main_u
F_MLT main_e_i
P_LOD main_u_i
F_MLT main_e
SF_ADD
SET_P __aux_var
SET_P __aux_var1
EQU __aux_var
SET_P __aux_var2
EQU __aux_var1
AND __aux_var2
OUT 0
LOD main_z
F_MLT main_e
P_LOD main_z_i
F_MLT main_e_i
SF_SU2
P_LOD main_z
F_MLT main_e_i
P_LOD main_z_i
F_MLT main_e
SF_ADD
P_LOD main_w
F_MLT main_e
P_LOD main_w_i
F_MLT main_e_i
SF_SU2
P_LOD main_w
F_MLT main_e_i
P_LOD main_w_i
F_MLT main_e
SF_ADD
SET_P __aux_var
SET_P __aux_var1
EQU __aux_var
SET_P __aux_var2
EQU __aux_var1
AND __aux_var2
OUT 0
LOD main_z
F_MLT main_e
P_LOD main_z_i
F_MLT main_e_i
SF_SU2
P_LOD main_z
F_MLT main_e_i
P_LOD main_z_i
F_MLT main_e
SF_ADD
EQU 4.0
SET_P __aux_var
EQU 3.0
AND __aux_var
OUT 0
LOD main_r
F_MLT main_e
P_LOD main_r_i
F_MLT main_e_i
SF_SU2
P_LOD main_r
F_MLT main_e_i
P_LOD main_r_i
F_MLT main_e
SF_ADD
EQU 0.0
SET_P __aux_var
EQU main_x
AND __aux_var
OUT 0
LOD main_r
F_MLT main_e
P_LOD main_r_i
F_MLT main_e_i
SF_SU2
P_LOD main_r
F_MLT main_e_i
P_LOD main_r_i
F_MLT main_e
SF_ADD
EQU 0.0
SET_P __aux_var
P_I2F_M main_n
S_EQU
AND __aux_var
OUT 0
LOD main_z
F_MLT main_e
P_LOD main_z_i
F_MLT main_e_i
SF_SU2
P_LOD main_z
F_MLT main_e_i
P_LOD main_z_i
F_MLT main_e
SF_ADD
EQU 0.0
SET_P __aux_var
P_I2F_M main_n
S_EQU
AND __aux_var
OUT 0
LOD main_r
F_MLT main_e
P_LOD main_r_i
F_MLT main_e_i
SF_SU2
P_LOD main_r
F_MLT main_e_i
P_LOD main_r_i
F_MLT main_e
SF_ADD
P_LOD main_x
F_MLT 2.0
F_SU1 3.0
SET_P __aux_var
EQU 0.0
SET_P __aux_var1
EQU __aux_var
AND __aux_var1
OUT 0
LOD main_x
F_MLT 2.0
F_SU1 3.0
P_LOD main_r
F_MLT main_e
P_LOD main_r_i
F_MLT main_e_i
SF_SU2
P_LOD main_r
F_MLT main_e_i
P_LOD main_r_i
F_MLT main_e
SF_ADD
EQU 0.0
SET_P __aux_var
S_EQU
AND __aux_var
OUT 0
LOD main_r
F_MLT main_e
P_LOD main_r_i
F_MLT main_e_i
SF_SU2
P_LOD main_r
F_MLT main_e_i
P_LOD main_r_i
F_MLT main_e
SF_ADD
P_LOD main_n
MLT 2
ADD -3
I2F
SET_P __aux_var
EQU 0.0
SET_P __aux_var1
EQU __aux_var
AND __aux_var1
OUT 0
LOD main_n
MLT 2
ADD -3
P_LOD main_r
F_MLT main_e
P_LOD main_r_i
F_MLT main_e_i
SF_SU2
P_LOD main_r
F_MLT main_e_i
P_LOD main_r_i
F_MLT main_e
SF_ADD
EQU 0.0
SET_P __aux_var
SET_P __aux_var1
I2F
EQU __aux_var1
AND __aux_var
OUT 0
LOD main_r
F_MLT main_e
P_LOD main_r_i
F_MLT main_e_i
SF_SU2
P_LOD main_r
F_MLT main_e_i
P_LOD main_r_i
F_MLT main_e
SF_ADD
P_LOD main_n
MLT 2
ADD -2
I2F
SET_P __aux_var
EQU 0.0
SET_P __aux_var1
EQU __aux_var
AND __aux_var1
OUT 0
LOD main_n
MLT 2
ADD -3
I2F
EQU main_r
P_LOD main_r_i
EQU 0.0
S_AND
OUT 0
LOD main_n
MLT 2
ADD -3
I2F
EQU main_r
P_LOD main_r_i
EQU 0.0
S_AND
OUT 0
LOD main_x
F_MLT 2.0
F_SU1 3.0
EQU main_r
P_LOD main_r_i
EQU 0.0
S_AND
OUT 0
LOD main_x
F_MLT 2.0
F_SU1 2.0
EQU main_r
P_LOD main_r_i
EQU 0.0
S_AND
OUT 0
LOD main_z
F_MLT main_e
P_LOD main_z_i
F_MLT main_e_i
SF_SU2
P_LOD main_z
F_MLT main_e_i
P_LOD main_z_i
F_MLT main_e
SF_ADD
P_LOD main_u
F_MLT main_e
P_LOD main_u_i
F_MLT main_e_i
SF_SU2
P_LOD main_u
F_MLT main_e_i
P_LOD main_u_i
F_MLT main_e
SF_ADD
SET_P __aux_var
SET_P __aux_var1
EQU __aux_var
SET_P __aux_var2
EQU __aux_var1
AND __aux_var2
LIN
OUT 0
LOD main_z
F_MLT main_e
P_LOD main_z_i
F_MLT main_e_i
SF_SU2
P_LOD main_z
F_MLT main_e_i
P_LOD main_z_i
F_MLT main_e
SF_ADD
P_LOD main_w
F_MLT main_e
P_LOD main_w_i
F_MLT main_e_i
SF_SU2
P_LOD main_w
F_MLT main_e_i
P_LOD main_w_i
F_MLT main_e
SF_ADD
SET_P __aux_var
SET_P __aux_var1
EQU __aux_var
SET_P __aux_var2
EQU __aux_var1
AND __aux_var2
LIN
OUT 0
LOD 1.0
EQU 0.0
P_LOD 0.0
EQU 1.0
S_AND
OUT 0
NEG_M 5
I2F
EQU 3.0
P_LOD 4.0
EQU 0.0
S_AND
OUT 0
F2I_M 5.0
OUT 0
F2I_M -7.0
OUT 0
@fim JMP fim
