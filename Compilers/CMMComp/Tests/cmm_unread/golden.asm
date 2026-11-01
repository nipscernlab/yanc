NOP
#PRNAME cmm_unread
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE main_v_i add_unread_b_i
#SHARE main_v add_unread_b_i
#SHARE main_n add_unread_b
#SHARE main_k add_unread_a_i
#SHARE main_re add_unread_b_i
#SHARE g add_unread_b_i
#SHARE main_w_i main_z
#SHARE main_w main_z
JMP main
@add_unread SET_P add_unread_b_i
SET_P add_unread_b
SET_P add_unread_a_i
SET add_unread_a
F_MLT add_unread_b
P_LOD add_unread_a_i
F_MLT add_unread_b_i
SF_SU2
P_LOD add_unread_a
F_MLT add_unread_b_i
P_LOD add_unread_a_i
F_MLT add_unread_b
SF_ADD
POP
LOD add_unread_a
F_ADD add_unread_b
P_LOD add_unread_a_i
F_ADD add_unread_b_i
RET
@one LOD 7
RET
@main LOD 1.0
SET main_z
LOD 2.0
SET main_z_i
LOD main_z
F_MLT 3.0
P_LOD main_z_i
SF_SU2
P_LOD main_z
P_LOD main_z_i
F_MLT 3.0
SF_ADD
POP
LOD main_z
F_MLT main_z
P_LOD main_z_i
F_MLT main_z_i
SF_SU2
P_LOD main_z
F_MLT main_z_i
P_LOD main_z_i
F_MLT main_z
SF_ADD
SET_P main_v_i
SET main_v
LOD 0
SET main_n
SET main_k
@__Lwh1 LOD main_k
GRE 3
JIZ __Lwh1end
LOD main_n
ADD main_k
SET main_n
LOD main_k
ADD 1
SET main_k
JMP __Lwh1
@__Lwh1end LOD main_n
OUT 0
LOD main_v
SET main_re
F2I_M main_re
OUT 0
LOD main_z
P_LOD main_z_i
P_LOD 2.0
P_LOD 1.0
CAL add_unread
SET_P main_z_i
SET main_z
F2I_M main_z
OUT 0
F2I_M main_z_i
OUT 0
CAL one
OUT 0
CAL one
OUT 0
CAL one
CAL one
OUT 0
LOD 2.5
SET g
LOD 2.0
F_MLT main_z
P_LOD 2.0
F_MLT main_z_i
SET_P main_w_i
SET main_w
@fim JMP fim
