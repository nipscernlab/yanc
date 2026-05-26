NOP
#PRNAME proc_sim
#NUBITS 23
#NDSTAC 10
#SDEPTH 10
#NUIOIN 1
#NUIOOU 2
#NBMANT 16
#NBEXPO 6
#FFTSIZ 3
#NUGAIN 128
@main #arrays main_ene 2 512 "Valores_Interpolados_Canal_A12-L_512bins.txt"
LOD 0.0
SET main_r1
LOD 0.0
SET main_r2
LOD 0.0
SET main_r3
LOD 0.0
SET main_r4
LOD 0.0
SET main_r5
LOD 0.0
SET main_r6
LOD 0.0
SET main_r7
@Lwh1 LOD 1
JIZ Lwh1end
INN 0
P_LOD 300
S_LES
JIZ Lif1else
INN 0
LDI main_ene
SET main_x
JMP Lif1end
@Lif1else LOD 0.0
SET main_x
@Lif1end LOD main_x
F_MLT 1000.0
F2I
OUT 0
LOD 0.1267
F_MLT main_r2
F_ADD main_x
P_LOD 0.4181
F_MLT main_r3
SF_ADD
P_LOD 0.4352
F_MLT main_r4
SF_ADD
P_LOD 0.1524
F_MLT main_r6
SF_ADD
P_LOD 0.0056
F_MLT main_r1
P_LOD 0.0709
F_MLT main_r5
SF_ADD
P_LOD 0.0643
F_MLT main_r7
SF_ADD
SF_SU2
SET main_xl
LOD 0.0222
F_MLT main_xl
P_LOD 0.1068
F_MLT main_r1
SF_ADD
P_LOD 0.1894
F_MLT main_r2
SF_ADD
P_LOD 0.1037
F_MLT main_r3
SF_ADD
P_LOD 0.1063
F_MLT main_r4
P_LOD 0.1890
F_MLT main_r5
SF_ADD
P_LOD 0.1053
F_MLT main_r6
SF_ADD
P_LOD 0.0215
F_MLT main_r7
SF_ADD
SF_SU2
SET main_y
LOD main_r6
SET main_r7
LOD main_r5
SET main_r6
LOD main_r4
SET main_r5
LOD main_r3
SET main_r4
LOD main_r2
SET main_r3
LOD main_r1
SET main_r2
LOD main_xl
SET main_r1
LOD main_y
F_MLT 1000.0
F2I
OUT 1
JMP Lwh1
@Lwh1end @fim JMP fim
