NOP
#PRNAME cm
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#SHARE main_ip main_p
#SHARE main_iq main_q
@main LOD 0
SET main_n
LOD 0
SET main_bad
@__Lwh1 LOD 500
LES main_n
JIZ __Lwh1end
F_INN 0
F_MLT 0.0013
SET main_x
F_INN 0
F_MLT 0.00071
SET main_y
LOD main_x
F_MLT main_y
SET main_p
LOD main_y
F_MLT main_x
SET main_q
LOD main_p
SET main_ip
LOD main_q
SET main_iq
EQU main_ip
LIN
JIZ __Lif1else
LOD main_bad
ADD 1
SET main_bad
@__Lif1else LOD main_x
F_ADD main_y
SET main_p
LOD main_y
F_ADD main_x
SET main_q
LOD main_p
SET main_ip
LOD main_q
SET main_iq
EQU main_ip
LIN
JIZ __Lif2else
LOD main_bad
ADD 1
SET main_bad
@__Lif2else LOD main_x
F_SU1 main_y
SET main_p
F_NEG_M main_y
F_ADD main_x
SET main_q
LOD main_p
SET main_ip
LOD main_q
SET main_iq
EQU main_ip
LIN
JIZ __Lif3else
LOD main_bad
ADD 1
SET main_bad
@__Lif3else LOD main_n
ADD 1
SET main_n
JMP __Lwh1
@__Lwh1end LOD main_bad
OUT 0
@fim JMP fim
