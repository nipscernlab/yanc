NOP
#PRNAME cmm_opassign
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#array a 1 5
#array m 1 9
#array g 2 3
#array z 3 2
#array z_i 4 2
#SHARE main_x main_k
#SHARE main_j main_n
#SHARE __aux_var main_k
#SHARE main_c_i main_n
#SHARE main_c main_r
#SHARE main_d_i main_k
#SHARE __aux_var1 main_d
#SHARE __aux_var2 main_n
@main LOD 5
SET main_k
ADD -1
SET main_k
OUT 0
LOD 5
SET main_k
ADD -1
SET main_k
OUT 0
LOD 5
SET main_k
ADD 1
SET main_k
OUT 0
LOD 5
SET main_k
ADD 1
SET main_k
SET main_r
OUT 0
OUT 0
LOD 5
SET main_k
PSH
ADD -1
SET_P main_k
SET main_r
OUT 0
LOD main_k
OUT 0
LOD 6
SET main_k
ADD -1
SET main_k
SET main_r
OUT 0
OUT 0
LOD 10
SET main_n
LOD 3
SET main_k
ADD -1
SET main_k
ADD main_n
SET main_r
OUT 0
LOD main_k
OUT 0
LOD 10
SET main_n
LOD 3
SET main_k
PSH
ADD -1
SET_P main_k
NEG
ADD main_n
SET main_r
OUT 0
LOD main_k
OUT 0
LOD 2.5
SET main_x
F_ADD -1.0
SET main_x
F_MLT 10.0
F2I
OUT 0
LOD 2.5
SET main_x
F_ADD -1.0
SET main_x
F_MLT 10.0
F2I
OUT 0
LOD main_x
F_MLT 10.0
F2I
OUT 0
LOD 2.5
SET main_x
PSH
F_ADD -1.0
SET_P main_x
F_MLT 10.0
F2I
OUT 0
LOD main_x
F_MLT 10.0
F2I
OUT 0
LOD 2.5
SET main_x
F_ADD 1.0
SET main_x
F_MLT 10.0
F2I
OUT 0
LOD 7
SET main_k
ADD 8
SET main_k
OUT 0
LOD 7
SET main_k
ADD -10
SET main_k
OUT 0
LOD 7
SET main_k
MLT 3
SET main_k
OUT 0
LOD 23
SET main_k
LOD 4
DIV main_k
NOP
NOP
NOP
QUO
SET main_k
OUT 0
LOD 23
SET main_k
LOD 4
MOD main_k
NOP
NOP
NOP
REM
SET main_k
OUT 0
LOD 13
SET main_k
LOD 6
AND main_k
SET main_k
OUT 0
LOD 9
SET main_k
LOD 6
ORR main_k
SET main_k
OUT 0
LOD 9
SET main_k
LOD 12
XOR main_k
SET main_k
OUT 0
LOD 3
SET main_k
LOD 2
SHL main_k
SET main_k
OUT 0
LOD 13
SET main_k
LOD 2
SHR main_k
SET main_k
OUT 0
NEG_M 8
SET main_k
LOD 2
SHR main_k
SET main_k
OUT 0
NEG_M 8
SET main_k
LOD 2
SRS main_k
SET main_k
OUT 0
LOD 4
SET main_k
LOD 2
SET main_j
ADD 3
MLT main_k
SET main_k
OUT 0
LOD 4
SET main_k
LOD 2
SET main_j
NEG_M main_j
ADD main_k
ADD 3
SET main_k
OUT 0
LOD 1.5
SET main_x
F_ADD 2.25
SET main_x
F_MLT 4.0
F2I
OUT 0
LOD 1.5
SET main_x
F_SU1 2.25
SET main_x
F_MLT 4.0
F2I
OUT 0
LOD 1.5
SET main_x
F_MLT 2.5
SET main_x
F_MLT 4.0
F2I
OUT 0
LOD 7.5
SET main_x
F_MLT 0.5
SET main_x
F_MLT 4.0
F2I
OUT 0
LOD 1
SET_V a 0
LOD 2
SET_V a 1
LOD 3
SET_V a 2
LOD 4
SET_V a 3
LOD 5
SET_V a 4
LOD_V a 1
ADD 10
SET_V a 1
LOD_V a 1
OUT 0
LOD 2
SET main_j
P_LOD main_j
LDI a
ADD -7
STI a
LOD_V a 2
OUT 0
LOD 1
SET main_j
ADD 2
PSH
LDI a
MLT 3
STI a
LOD_V a 3
OUT 0
LOD main_j
PSH
LDI a
ADD -1
STI a
LOD_V a 1
OUT 0
LOD 4
PSH
LDI a
ADD -1
STI a
LOD_V a 4
OUT 0
LOD 0
PSH
LDI a
ADD 1
STI a
LOD_V a 0
OUT 0
LOD 4
SET main_j
PSH
LDI a
ADD -1
STI a
SET main_r
OUT 0
LOD_V a 4
OUT 0
LOD 4
SET main_j
PSH
LDI a
ADD -1
STI a
ADD 1
SET main_r
OUT 0
LOD_V a 4
OUT 0
LOD 0
SET main_j
PSH
LDI a
ADD 1
STI a
SET main_r
OUT 0
LOD_V a 0
OUT 0
LOD 5
SET_V m 5
LOD_V m 5
ADD 4
SET_V m 5
LOD_V m 5
OUT 0
LOD 1
SET main_j
LOD 2
SET main_k
LOD main_j
MLT 3
ADD main_k
P_LOD main_j
MLT 3
ADD main_k
LDI m
P_LOD 1
S_SHL
STI m
LOD_V m 5
OUT 0
LOD main_j
MLT 3
ADD main_k
PSH
LDI m
ADD -1
STI m
LOD_V m 5
OUT 0
LOD main_j
MLT 3
ADD main_k
PSH
LDI m
ADD -1
STI m
ADD 1
SET main_r
OUT 0
LOD_V m 5
OUT 0
LOD main_j
MLT 3
ADD main_k
PSH
LDI m
ADD -1
STI m
SET main_r
OUT 0
LOD_V m 5
OUT 0
LOD 1.5
SET_V g 0
LOD 0.5
SET_V g 1
LOD_V g 1
F_ADD 2.0
SET_V g 1
LOD_V g 1
F_MLT 4.0
F2I
OUT 0
LOD 0
SET main_j
P_LOD main_j
LDI g
F_MLT 3.0
STI g
LOD_V g 0
F_MLT 4.0
F2I
OUT 0
LOD main_j
PSH
LDI g
F_ADD -1.0
STI g
LOD_V g 0
F_MLT 4.0
F2I
OUT 0
LOD main_j
PSH
LDI g
SET __aux_var
F_ADD -1.0
STI g
LOD __aux_var
F_MLT 4.0
F2I
OUT 0
LOD_V g 0
F_MLT 4.0
F2I
OUT 0
LOD main_j
PSH
LDI g
F_ADD -1.0
STI g
F_MLT 4.0
F2I
OUT 0
LOD_V g 0
F_MLT 4.0
F2I
OUT 0
LOD 1.5
P_LOD 2.0
SET_P main_c_i
SET main_c
LOD 0.5
P_LOD -1.0
SET_P main_d_i
SET main_d
F_ADD main_c
P_LOD main_c_i
F_ADD main_d_i
SET_P main_c_i
SET main_c
F_MLT 4.0
F2I
OUT 0
LOD main_c_i
F_MLT 4.0
F2I
OUT 0
LOD 3.0
P_LOD 0.5
SET_P __aux_var
F_SU2 main_c
P_LOD main_c_i
F_SU1 __aux_var
SET_P main_c_i
SET main_c
F_MLT 4.0
F2I
OUT 0
LOD main_c_i
F_MLT 4.0
F2I
OUT 0
LOD 1.0
P_LOD 2.0
SET_P main_c_i
SET main_c
LOD 3.0
P_LOD -1.0
SET_P __aux_var
SET __aux_var1
F_MLT main_c
P_LOD main_c_i
F_MLT __aux_var
SF_SU2
P_LOD main_c
F_MLT __aux_var
P_LOD main_c_i
F_MLT __aux_var1
SF_ADD
SET_P main_c_i
SET main_c
F2I_M main_c
OUT 0
F2I_M main_c_i
OUT 0
LOD 2.0
P_LOD 1.0
SET_V z_i 1
POP
SET_V z 1
LOD_V z 1
P_LOD_V z_i 1
P_LOD 0.5
P_LOD 0.5
SET_P __aux_var
SET_P __aux_var1
SET_P __aux_var2
F_ADD __aux_var1
P_LOD __aux_var2
F_ADD __aux_var
SET_V z_i 1
POP
SET_V z 1
LOD_V z 1
F_MLT 4.0
F2I
OUT 0
LOD_V z_i 1
F_MLT 4.0
F2I
OUT 0
LOD 0
SET main_r
LOD 10
SET main_k
@__Lwh1 LOD main_r
ADD main_k
SET main_r
LOD main_k
ADD -3
SET main_k
GRE 1
JIZ __Lwh1
@__Lwh1end LOD main_r
OUT 0
LOD main_k
OUT 0
LOD 0
SET main_r
LOD 5
SET main_k
@__Lwh2 LOD main_r
ADD main_k
SET main_r
LOD main_k
ADD -1
SET main_k
GRE 1
JIZ __Lwh2
@__Lwh2end LOD main_r
OUT 0
LOD main_k
OUT 0
LOD 0
SET main_r
LOD 5
SET main_k
@__Lwh3 LOD main_r
ADD main_k
SET main_r
LOD main_k
ADD -1
SET main_k
GRE 1
JIZ __Lwh3
@__Lwh3end LOD main_r
OUT 0
LOD main_k
OUT 0
LOD 0
SET main_r
SET main_k
@__Lwh4 LOD main_r
ADD main_k
SET main_r
INC main_k
LES 4
JIZ __Lwh4
@__Lwh4end LOD main_r
OUT 0
LOD main_k
OUT 0
LOD 1
SET main_r
SET main_k
@__Lwh5 LOD main_r
ADD main_k
SET main_r
LOD main_k
MLT 3
SET main_k
LES 99
JIZ __Lwh5
@__Lwh5end LOD main_r
OUT 0
LOD main_k
OUT 0
LOD 4
SET main_k
LOD 0
SET main_r
@__Lwh6 LOD main_k
PSH
ADD -1
SET_P main_k
LES 0
JIZ __Lwh6end
LOD main_r
ADD 2
SET main_r
JMP __Lwh6
@__Lwh6end LOD main_r
OUT 0
LOD main_k
OUT 0
@fim JMP fim
