NOP
#PRNAME proc_fft
#NUBITS 23
#NUIOIN 1
#NUIOOU 2
#NBMANT 16
#NBEXPO 6
#FFTSIZ 3
#NUGAIN 128
#array data 3 8
#array data_i 4 8
#array wpv 3 4
#array wpv_i 4 4
#SHARE fft_temp_i __aux_var1
#SHARE fft_temp __aux_var2
JMP main
@fft SET fft_N
LOD 1
SET fft_mmax
@__Lwh1 LOD fft_N
LES fft_mmax
JIZ __Lwh1end
LOD fft_mmax
MLT 2
SET fft_istep
LOD 0
SET fft_m
SET fft_ind
SET fft_sind
@__Lwh2 LOD fft_mmax
LES fft_m
JIZ __Lwh2end
LOD fft_m
SET fft_k
@__Lwh3 LOD fft_N
LES fft_k
JIZ __Lwh3end
LOD fft_k
ADD fft_mmax
SET fft_j
LOD fft_sind
LDI wpv
P_LOD fft_sind
LDI wpv_i
P_LOD fft_j
ILI data
P_LOD fft_j
ILI data_i
SET_P __aux_var 
SET_P __aux_var1
SET_P __aux_var2
SET   __aux_var3
F_MLT __aux_var1
P_LOD __aux_var 
F_MLT __aux_var2
SF_SU2
P_LOD __aux_var1
F_MLT __aux_var2
P_LOD __aux_var 
F_MLT __aux_var3
SF_ADD
SET_P fft_temp_i
SET fft_temp
LOD fft_j
P_LOD fft_k
ILI data
F_SU1 fft_temp
P_LOD fft_k
ILI data_i
F_SU1 fft_temp_i
SET_P __aux_var
ISI data
LOD fft_j
P_LOD __aux_var
ISI data_i
LOD fft_k
P_LOD fft_k
ILI data
F_ADD fft_temp
P_LOD fft_k
ILI data_i
F_ADD fft_temp_i
SET_P __aux_var
ISI data
LOD fft_k
P_LOD __aux_var
ISI data_i
LOD fft_k
ADD fft_istep
SET fft_k
LOD fft_ind
ADD 1
SET fft_ind
JMP __Lwh3
@__Lwh3end LOD fft_ind
SET fft_sind
LOD fft_m
ADD 1
SET fft_m
JMP __Lwh2
@__Lwh2end LOD fft_istep
SET fft_mmax
JMP __Lwh1
@__Lwh1end RET
@main LOD 1.0
SET_V wpv 0
LOD 0.0
SET_V wpv_i 0
LOD 0.7071067811
SET_V wpv 1
LOD 0.7071067811
SET_V wpv_i 1
LOD 0.0
SET_V wpv 2
LOD 1.0
SET_V wpv_i 2
LOD -0.7071067811
SET_V wpv 3
LOD 0.7071067811
SET_V wpv_i 3
LOD 1.0
SET_V data 0
LOD 0.0
SET_V data_i 0
LOD 2.0
SET_V data 1
LOD 0.0
SET_V data_i 1
LOD 3.0
SET_V data 2
LOD 0.0
SET_V data_i 2
LOD 4.0
SET_V data 3
LOD 0.0
SET_V data_i 3
LOD 5.0
SET_V data 4
LOD 0.0
SET_V data_i 4
LOD 6.0
SET_V data 5
LOD 0.0
SET_V data_i 5
LOD 7.0
SET_V data 6
LOD 0.0
SET_V data_i 6
LOD 8.0
SET_V data 7
LOD 0.0
SET_V data_i 7
LOD 8
CAL fft
LOD 0
ILI data
F_MLT 1000.0
F2I
OUT 0
LOD 0
ILI data_i
F_MLT 1000.0
F2I
OUT 1
LOD 1
ILI data
F_MLT 1000.0
F2I
OUT 0
LOD 1
ILI data_i
F_MLT 1000.0
F2I
OUT 1
LOD 2
ILI data
F_MLT 1000.0
F2I
OUT 0
LOD 2
ILI data_i
F_MLT 1000.0
F2I
OUT 1
LOD 3
ILI data
F_MLT 1000.0
F2I
OUT 0
LOD 3
ILI data_i
F_MLT 1000.0
F2I
OUT 1
LOD 4
ILI data
F_MLT 1000.0
F2I
OUT 0
LOD 4
ILI data_i
F_MLT 1000.0
F2I
OUT 1
LOD 5
ILI data
F_MLT 1000.0
F2I
OUT 0
LOD 5
ILI data_i
F_MLT 1000.0
F2I
OUT 1
LOD 6
ILI data
F_MLT 1000.0
F2I
OUT 0
LOD 6
ILI data_i
F_MLT 1000.0
F2I
OUT 1
LOD 7
ILI data
F_MLT 1000.0
F2I
OUT 0
LOD 7
ILI data_i
F_MLT 1000.0
F2I
OUT 1
@fim JMP fim
