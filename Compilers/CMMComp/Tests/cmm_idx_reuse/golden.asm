NOP
#PRNAME cmm_idx_reuse
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#array vi 1 6
#array vf 2 6
#array mi 1 12
#array mf 2 12
#SHARE main_y main_k
@main LOD 1
SET main_a
LOD 2
SET main_b
LOD 0
SET main_k
@__Lwh1 LOD main_k
GRE 6
JIZ __Lwh1end
LOD main_k
P_LOD main_k
STI vi
LOD main_k
P_I2F_M main_k
F_MLT 0.5
STI vf
LOD main_k
ADD 1
SET main_k
JMP __Lwh1
@__Lwh1end LOD main_a
ADD main_b
PSH
LDI vi
ADD 10
STI vi
LOD_V vi 3
OUT 0
LOD main_a
ADD main_b
PSH
LDI vf
F_MLT 4.0
STI vf
LOD_V vf 3
F2I
OUT 0
LOD 5
SET_V mi 7
LOD main_a
MLT 4
P_LOD main_b
ADD 1
S_ADD
PSH
LDI mi
ADD 7
STI mi
LOD_V mi 7
OUT 0
LOD 2.5
SET_V mf 7
LOD main_a
MLT 4
P_LOD main_b
ADD 1
S_ADD
PSH
LDI mf
F_MLT 2.0
STI mf
LOD_V mf 7
F2I
OUT 0
LOD main_a
ADD main_b
LDI vi
PSH
S_MLT
OUT 0
LOD main_a
ADD main_b
LDI vf
PSH
SF_MLT
SET main_y
F2I_M main_y
OUT 0
LOD main_a
ADD main_b
P_LOD main_a
ADD main_b
P_NEG_M 1
S_ADD
LDI vi
ADD 1
STI vi
LOD_V vi 3
OUT 0
LOD main_a
ADD main_b
PSH
LDI vi
I2F
F_ADD 0.5
STI vf
LOD_V vf 3
F_MLT 2.0
F2I
OUT 0
LOD 0
SET main_k
@__Lwh2 LOD main_k
GRE 3
JIZ __Lwh2end
LOD main_k
ADD 1
PSH
LDI vi
ADD 100
STI vi
LOD main_k
ADD 1
SET main_k
JMP __Lwh2
@__Lwh2end LOD_V vi 1
OUT 0
LOD_V vi 2
OUT 0
LOD_V vi 3
OUT 0
@fim JMP fim
