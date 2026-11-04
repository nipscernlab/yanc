NOP
#PRNAME cmm_cov_acc
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
@main LOD 7
SET main_a
LOD 3
SET main_b
LOD 17
SET main_p
LOD 5
SET main_q
LOD 2.5
SET main_f
LOD main_a
MLT main_b
P_LOD main_q
DIV main_p
NOP
NOP
NOP
QUO
S_ADD
OUT 0
LOD main_a
MLT main_b
P_LOD main_q
MOD main_p
NOP
NOP
NOP
REM
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD 7
SET main_a
LOD 3
SET main_b
NEG_M 17
SET main_p
LOD 5
SET main_q
LOD 9.75
SET main_f
LOD main_a
MLT main_b
P_LOD main_q
DIV main_p
NOP
NOP
NOP
QUO
S_ADD
OUT 0
LOD main_a
MLT main_b
P_LOD main_q
MOD main_p
NOP
NOP
NOP
REM
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD 4
SET main_a
LOD 2
SET main_b
LOD 3
SET main_p
LOD 2
SET main_q
LOD 3.0
SET main_f
LOD main_a
MLT main_b
P_LOD main_q
DIV main_p
NOP
NOP
NOP
QUO
S_ADD
OUT 0
LOD main_a
MLT main_b
P_LOD main_q
MOD main_p
NOP
NOP
NOP
REM
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_GRE main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
F_LES main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
LOD main_a
MLT main_b
P_I2F_M main_p
EQU main_f
LIN
S_ADD
OUT 0
@fim JMP fim
