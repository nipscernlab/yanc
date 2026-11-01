NOP
#PRNAME ProcDTW
#NUBITS 32
#NUIOIN 3
#NUIOOU 3
#NBMANT 23
#NBEXPO 8
#array dtw 1 5625
#array data_ref 1 75
#array data 1 75
#SHARE menor cont
@main LOD 75
SET len_ref
LOD 0
SET cont
@__Lwh1 LOD cont
GRE 75
JIZ __Lwh1end
LOD cont
P_LOD 0
STI data_ref
LOD cont
ADD 1
SET cont
JMP __Lwh1
@__Lwh1end LOD 10
OUT 2
NEG_M 122
OUT 2
@fim JMP fim
#ITRAD
NEG_M 123
OUT 2
LOD 0
SET k
INN 2
SET len_data
LOD 0
SET cont
@__Lwh2 LOD cont
GRE 75
JIZ __Lwh2end
LOD cont
GRE len_data
JIZ __Lif1else
LOD cont
P_INN 1
STI data
JMP __Lif1end
@__Lif1else LOD cont
P_LOD 0
STI data
@__Lif1end LOD cont
ADD 1
SET cont
JMP __Lwh2
@__Lwh2end NEG_M 124
OUT 2
LOD k
OUT 2
LOD len_data
OUT 2
@__Lwh3 LOD len_data
LES k
JIZ __Lwh3end
LOD 0
SET j
NEG_M 125
OUT 2
@__Lwh4 LOD len_ref
LES j
JIZ __Lwh4end
LOD k
EQU 0
P_LOD j
EQU 0
S_LAN
JIZ __Lif2else
LOD  k
MLT  75
ADD  j
P_LOD k
LDI data
P_LOD j
LDI data_ref
NEG
S_ADD
ABS
STI dtw
LOD k
MLT 75
ADD j
LDI dtw
OUT 2
JMP __Lif2end
@__Lif2else LOD k
EQU 0
P_LOD j
LES 0
S_LAN
JIZ __Lif3else
LOD  k
MLT  75
ADD  j
P_LOD k
LDI data
P_LOD j
LDI data_ref
NEG
S_ADD
ABS
P_LOD k
MLT 75
P_NEG_M 1
ADD j
S_ADD
LDI dtw
S_ADD
STI dtw
LOD k
MLT 75
ADD j
LDI dtw
OUT 2
@__Lif3else @__Lif2end LOD k
LES 0
P_LOD j
EQU 0
S_LAN
JIZ __Lif4else
LOD  k
MLT  75
ADD  j
P_LOD k
LDI data
P_LOD j
LDI data_ref
NEG
S_ADD
ABS
P_NEG_M 1
ADD k
MLT 75
ADD j
LDI dtw
S_ADD
STI dtw
LOD k
MLT 75
ADD j
LDI dtw
OUT 2
JMP __Lif4end
@__Lif4else LOD k
LES 0
P_LOD j
LES 0
S_LAN
JIZ __Lif5else
NEG_M 1
ADD k
MLT 75
ADD j
LDI dtw
SET menor
OUT 2
LOD k
MLT 75
P_NEG_M 1
ADD j
S_ADD
LDI dtw
GRE menor
JIZ __Lif6else
LOD k
MLT 75
P_NEG_M 1
ADD j
S_ADD
LDI dtw
SET menor
OUT 2
@__Lif6else NEG_M 1
ADD k
MLT 75
P_NEG_M 1
ADD j
S_ADD
LDI dtw
OUT 2
NEG_M 1
ADD k
MLT 75
P_NEG_M 1
ADD j
S_ADD
LDI dtw
GRE menor
JIZ __Lif7else
NEG_M 1
ADD k
MLT 75
P_NEG_M 1
ADD j
S_ADD
LDI dtw
SET menor
OUT 2
@__Lif7else LOD  k
MLT  75
ADD  j
P_LOD k
LDI data
P_LOD j
LDI data_ref
NEG
S_ADD
ABS
ADD menor
STI dtw
LOD k
MLT 75
ADD j
LDI dtw
OUT 2
@__Lif5else @__Lif4end LOD j
ADD 1
SET j
JMP __Lwh4
@__Lwh4end LOD k
ADD 1
SET k
JMP __Lwh3
@__Lwh3end NEG_M 1
ADD k
MLT 75
P_NEG_M 1
ADD j
S_ADD
LDI dtw
SET val_final
NEG_M 1
ADD k
MLT 75
P_NEG_M 1
ADD j
S_ADD
LDI dtw
LES 36000
JIZ __Lif8else
LOD 0
SET cont
LOD len_data
SET len_ref
@__Lwh5 LOD cont
GRE 75
JIZ __Lwh5end
LOD cont
P_LOD cont
LDI data
STI data_ref
LOD cont
ADD 1
SET cont
JMP __Lwh5
@__Lwh5end @__Lif8else LOD val_final
OUT 1
LOD 36000
OUT 2
LOD 1
OUT 0
@fim JMP fim
