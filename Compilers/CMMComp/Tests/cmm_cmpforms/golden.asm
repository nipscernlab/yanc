NOP
#PRNAME cmm_cmpforms
#NUBITS 32
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
JMP main
@b1 SET_P b1_f
SET b1_k
LOD 0
SET b1_s
I2F_M b1_k
F_GRE b1_f
JIZ Lif1else
LOD b1_s
ADD 1
SET b1_s
@Lif1else I2F_M b1_k
F_LES b1_f
JIZ Lif2else
LOD b1_s
ADD 2
SET b1_s
@Lif2else I2F_M b1_k
EQU b1_f
JIZ Lif3else
LOD b1_s
ADD 4
SET b1_s
@Lif3else LOD b1_s
RET
@b2 SET_P b2_m
SET b2_k
LOD 0
SET b2_s
LOD b2_k
ADD 1
GRE b2_m
JIZ Lif4else
LOD b2_s
ADD 1
SET b2_s
@Lif4else LOD b2_k
ADD 1
LES b2_m
JIZ Lif5else
LOD b2_s
ADD 2
SET b2_s
@Lif5else LOD b2_k
ADD 1
EQU b2_m
JIZ Lif6else
LOD b2_s
ADD 4
SET b2_s
@Lif6else LOD b2_s
RET
@b3 SET_P b3_f
SET b3_k
LOD 0
SET b3_s
LOD b3_k
ADD 1
I2F
F_GRE b3_f
JIZ Lif7else
LOD b3_s
ADD 1
SET b3_s
@Lif7else LOD b3_k
ADD 1
I2F
F_LES b3_f
JIZ Lif8else
LOD b3_s
ADD 2
SET b3_s
@Lif8else LOD b3_k
ADD 1
I2F
EQU b3_f
JIZ Lif9else
LOD b3_s
ADD 4
SET b3_s
@Lif9else LOD b3_s
RET
@b4 SET_P b4_k
SET b4_f
LOD 0
SET b4_s
I2F_M b4_k
F_LES b4_f
JIZ Lif10else
LOD b4_s
ADD 1
SET b4_s
@Lif10else I2F_M b4_k
F_GRE b4_f
JIZ Lif11else
LOD b4_s
ADD 2
SET b4_s
@Lif11else I2F_M b4_k
EQU b4_f
JIZ Lif12else
LOD b4_s
ADD 4
SET b4_s
@Lif12else LOD b4_s
RET
@b5 SET_P b5_k
SET b5_f
LOD 0
SET b5_s
LOD b5_k
ADD 1
I2F
F_LES b5_f
JIZ Lif13else
LOD b5_s
ADD 1
SET b5_s
@Lif13else LOD b5_k
ADD 1
I2F
F_GRE b5_f
JIZ Lif14else
LOD b5_s
ADD 2
SET b5_s
@Lif14else LOD b5_k
ADD 1
I2F
EQU b5_f
JIZ Lif15else
LOD b5_s
ADD 4
SET b5_s
@Lif15else LOD b5_s
RET
@b6 SET_P b6_g
SET b6_f
LOD 0
SET b6_s
LOD b6_f
F_GRE b6_g
JIZ Lif16else
LOD b6_s
ADD 1
SET b6_s
@Lif16else LOD b6_f
F_LES b6_g
JIZ Lif17else
LOD b6_s
ADD 2
SET b6_s
@Lif17else LOD b6_f
EQU b6_g
JIZ Lif18else
LOD b6_s
ADD 4
SET b6_s
@Lif18else LOD b6_s
RET
@b7 SET_P b7_g
SET b7_f
LOD 0
SET b7_s
LOD b7_g
F_ADD 0.5
F_LES b7_f
JIZ Lif19else
LOD b7_s
ADD 1
SET b7_s
@Lif19else LOD b7_g
F_ADD 0.5
F_GRE b7_f
JIZ Lif20else
LOD b7_s
ADD 2
SET b7_s
@Lif20else LOD b7_g
F_ADD 0.5
EQU b7_f
JIZ Lif21else
LOD b7_s
ADD 4
SET b7_s
@Lif21else LOD b7_s
RET
@b8 SET_P b8_g
SET b8_f
LOD 0
SET b8_s
LOD b8_f
F_ADD 0.5
F_GRE b8_g
JIZ Lif22else
LOD b8_s
ADD 1
SET b8_s
@Lif22else LOD b8_f
F_ADD 0.5
F_LES b8_g
JIZ Lif23else
LOD b8_s
ADD 2
SET b8_s
@Lif23else LOD b8_f
F_ADD 0.5
EQU b8_g
JIZ Lif24else
LOD b8_s
ADD 4
SET b8_s
@Lif24else LOD b8_s
RET
@main LOD 2
P_LOD 3.0
CAL b1
OUT 0
LOD 2
P_LOD 2.0
CAL b1
OUT 0
LOD 2
P_LOD 1.0
CAL b1
OUT 0
LOD 2
P_LOD 4
CAL b2
OUT 0
LOD 2
P_LOD 3
CAL b2
OUT 0
LOD 2
P_LOD 2
CAL b2
OUT 0
LOD 2
P_LOD 4.0
CAL b3
OUT 0
LOD 2
P_LOD 3.0
CAL b3
OUT 0
LOD 2
P_LOD 2.0
CAL b3
OUT 0
LOD 1.0
P_LOD 2
CAL b4
OUT 0
LOD 2.0
P_LOD 2
CAL b4
OUT 0
LOD 3.0
P_LOD 2
CAL b4
OUT 0
LOD 2.0
P_LOD 2
CAL b5
OUT 0
LOD 3.0
P_LOD 2
CAL b5
OUT 0
LOD 4.0
P_LOD 2
CAL b5
OUT 0
LOD 1.0
P_LOD 2.0
CAL b6
OUT 0
LOD 2.0
P_LOD 2.0
CAL b6
OUT 0
LOD 3.0
P_LOD 2.0
CAL b6
OUT 0
LOD 1.0
P_LOD 1.0
CAL b7
OUT 0
LOD 1.5
P_LOD 1.0
CAL b7
OUT 0
LOD 2.0
P_LOD 1.0
CAL b7
OUT 0
LOD 1.0
P_LOD 2.0
CAL b8
OUT 0
LOD 1.5
P_LOD 2.0
CAL b8
OUT 0
LOD 2.0
P_LOD 2.0
CAL b8
OUT 0
@fim JMP fim
