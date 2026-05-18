NOP
#PRNAME ProcDTW
#NUBITS 32
#NDSTAC 5
#SDEPTH 5
#NUIOIN 3
#NUIOOU 3
#NBMANT 23
#NBEXPO 8
#array dtw 1 5625
LOD 75
SET dtw_arr_size
#array data_ref 1 75
#array data 1 75
@main LOD 75
SET len_ref
LOD 0
SET cont
@Lwh1 LOD 75
LES cont
JIZ Lwh1end
LOD cont
P_LOD 0
STI data_ref
LOD cont
ADD 1
SET cont
JMP Lwh1
@Lwh1end LOD 10
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
@Lwh2 LOD 75
LES cont
JIZ Lwh2end
LOD cont
GRE len_data
JIZ Lif1else
LOD cont
P_INN 1
STI data
JMP Lif1end
@Lif1else LOD cont
P_LOD 0
STI data
@Lif1end LOD cont
ADD 1
SET cont
JMP Lwh2
@Lwh2end NEG_M 124
OUT 2
LOD k
OUT 2
LOD len_data
OUT 2
@Lwh3 LOD len_data
LES k
JIZ Lwh3end
LOD 0
SET j
NEG_M 125
OUT 2
@Lwh4 LOD len_ref
LES j
JIZ Lwh4end
LOD 0
EQU k
P_LOD 0
EQU j
S_LAN
JIZ Lif2else
LOD  k
MLT  dtw_arr_size
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
MLT dtw_arr_size
ADD j
LDI dtw
OUT 2
JMP Lif2end
@Lif2else LOD 0
EQU k
P_LOD 0
GRE j
S_LAN
JIZ Lif3else
LOD  k
MLT  dtw_arr_size
ADD  j
P_LOD k
LDI data
P_LOD j
LDI data_ref
NEG
S_ADD
ABS
P_NEG_M 1
ADD j
P_LOD k
MLT   dtw_arr_size
S_ADD
LDI   dtw
S_ADD
STI dtw
LOD k
MLT dtw_arr_size
ADD j
LDI dtw
OUT 2
@Lif3else @Lif2end LOD 0
GRE k
P_LOD 0
EQU j
S_LAN
JIZ Lif4else
LOD  k
MLT  dtw_arr_size
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
MLT dtw_arr_size
ADD j
LDI dtw
S_ADD
STI dtw
LOD k
MLT dtw_arr_size
ADD j
LDI dtw
OUT 2
JMP Lif4end
@Lif4else LOD 0
GRE k
P_LOD 0
GRE j
S_LAN
JIZ Lif5else
NEG_M 1
ADD k
MLT dtw_arr_size
ADD j
LDI dtw
SET menor
OUT 2
NEG_M 1
ADD j
P_LOD k
MLT   dtw_arr_size
S_ADD
LDI   dtw
P_LOD menor
S_LES
JIZ Lif6else
NEG_M 1
ADD j
P_LOD k
MLT   dtw_arr_size
S_ADD
LDI   dtw
SET menor
OUT 2
@Lif6else NEG_M 1
ADD k
P_NEG_M 1
ADD j
SET_P aux_var
MLT   dtw_arr_size
ADD   aux_var
LDI   dtw
OUT 2
NEG_M 1
ADD k
P_NEG_M 1
ADD j
SET_P aux_var
MLT   dtw_arr_size
ADD   aux_var
LDI   dtw
P_LOD menor
S_LES
JIZ Lif7else
NEG_M 1
ADD k
P_NEG_M 1
ADD j
SET_P aux_var
MLT   dtw_arr_size
ADD   aux_var
LDI   dtw
SET menor
OUT 2
@Lif7else LOD  k
MLT  dtw_arr_size
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
MLT dtw_arr_size
ADD j
LDI dtw
OUT 2
@Lif5else @Lif4end LOD j
ADD 1
SET j
JMP Lwh4
@Lwh4end LOD k
ADD 1
SET k
JMP Lwh3
@Lwh3end NEG_M 1
ADD k
P_NEG_M 1
ADD j
SET_P aux_var
MLT   dtw_arr_size
ADD   aux_var
LDI   dtw
SET val_final
NEG_M 1
ADD k
P_NEG_M 1
ADD j
SET_P aux_var
MLT   dtw_arr_size
ADD   aux_var
LDI   dtw
P_LOD 36000
S_GRE
JIZ Lif8else
LOD 0
SET cont
LOD len_data
SET len_ref
@Lwh5 LOD 75
LES cont
JIZ Lwh5end
LOD cont
P_LOD cont
LDI data
STI data_ref
LOD cont
ADD 1
SET cont
JMP Lwh5
@Lwh5end @Lif8else LOD val_final
OUT 1
LOD 36000
OUT 2
LOD 1
OUT 0
@fim JMP fim
