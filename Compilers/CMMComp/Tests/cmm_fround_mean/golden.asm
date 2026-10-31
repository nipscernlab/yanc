NOP
#PRNAME cmm_fround_mean
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#FROUND 2
#SHARE main_d main_n
#SHARE main_e main_n
@main LOD 1048576.0
SET main_base
LOD 0.00390625
SET main_s8
@__Lwh1 LOD 1
SET main_r
LOD 0
SET main_sum
LOD main_base
SET main_acc
LOD 0
SET main_k
@__Lwh2 LOD 0
SET main_n
@__Lwh3 LOD 7
SHL main_r
AND 65535
XOR main_r
SET main_r
LOD 9
SHR main_r
XOR main_r
SET main_r
LOD 8
SHL main_r
AND 65535
XOR main_r
SET main_r
I2F_M main_r
F_MLT main_s8
F_ADD main_acc
SET main_acc
LOD main_sum
ADD main_r
SET main_sum
LOD main_n
ADD 1
SET main_n
LES 31
JIZ __Lwh3
@__Lwh3end LOD main_acc
F_SU1 main_base
P_LOD 16
SHR main_sum
I2F
F_MLT 256.0
SF_SU2
SET main_d
LOD 65535
AND main_sum
I2F
F_MLT main_s8
F_SU2 main_d
F_MLT 256.0
SET main_e
F2I_M main_e
OUT 0
LOD main_k
ADD 1
SET main_k
LES 31
JIZ __Lwh2
@__Lwh2end JMP __Lwh1
@__Lwh1end @fim JMP fim
