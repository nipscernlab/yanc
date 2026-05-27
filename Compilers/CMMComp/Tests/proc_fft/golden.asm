NOP
#PRNAME proc_fft
#NUBITS 23
#NDSTAC 5
#SDEPTH 5
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
JMP main
@fft SET fft_N
LOD 1
SET fft_mmax
@Lwh1 LOD fft_N
LES fft_mmax
JIZ Lwh1end
LOD fft_mmax
MLT 2
SET fft_istep
LOD 0
SET fft_m
LOD 0
SET fft_ind
LOD 0
SET fft_sind
@Lwh2 LOD fft_mmax
LES fft_m
JIZ Lwh2end
LOD fft_m
SET fft_k
@Lwh3 LOD fft_N
LES fft_k
JIZ Lwh3end
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
SET_P aux_var 
SET_P aux_var1
SET_P aux_var2
SET   aux_var3
F_MLT aux_var1
P_LOD aux_var 
F_MLT aux_var2
SF_SU2
P_LOD aux_var1
F_MLT aux_var2
P_LOD aux_var 
F_MLT aux_var3
SF_ADD
SET_P fft_temp_i
SET fft_temp
LOD fft_j
P_LOD fft_k
ILI data
P_LOD fft_k
ILI data_i
SET_P aux_var
F_SU1 fft_temp
P_LOD aux_var
F_SU1 fft_temp_i
SET_P aux_var
SET_P aux_var2
SET   aux_var3
P_LOD aux_var2
ISI data
LOD   aux_var3
P_LOD aux_var
ISI data_i
LOD fft_k
P_LOD fft_k
ILI data
P_LOD fft_k
ILI data_i
SET_P aux_var
F_ADD fft_temp
P_LOD aux_var
F_ADD fft_temp_i
SET_P aux_var
SET_P aux_var2
SET   aux_var3
P_LOD aux_var2
ISI data
LOD   aux_var3
P_LOD aux_var
ISI data_i
LOD fft_k
ADD fft_istep
SET fft_k
LOD fft_ind
ADD 1
SET fft_ind
JMP Lwh3
@Lwh3end LOD fft_ind
SET fft_sind
LOD fft_m
ADD 1
SET fft_m
JMP Lwh2
@Lwh2end LOD fft_istep
SET fft_mmax
JMP Lwh1
@Lwh1end RET
@main LOD 0
SET   aux_var
P_LOD 1.000000
STI wpv
LOD   aux_var
P_LOD 0.000000
STI wpv_i
LOD 1
SET   aux_var
P_LOD 0.707107
STI wpv
LOD   aux_var
P_LOD 0.707107
STI wpv_i
LOD 2
SET   aux_var
P_LOD 0.000000
STI wpv
LOD   aux_var
P_LOD 1.000000
STI wpv_i
LOD 3
SET   aux_var
P_LOD -0.707107
STI wpv
LOD   aux_var
P_LOD 0.707107
STI wpv_i
LOD 0
SET   aux_var
P_LOD 1.000000
STI data
LOD   aux_var
P_LOD 0.000000
STI data_i
LOD 1
SET   aux_var
P_LOD 2.000000
STI data
LOD   aux_var
P_LOD 0.000000
STI data_i
LOD 2
SET   aux_var
P_LOD 3.000000
STI data
LOD   aux_var
P_LOD 0.000000
STI data_i
LOD 3
SET   aux_var
P_LOD 4.000000
STI data
LOD   aux_var
P_LOD 0.000000
STI data_i
LOD 4
SET   aux_var
P_LOD 5.000000
STI data
LOD   aux_var
P_LOD 0.000000
STI data_i
LOD 5
SET   aux_var
P_LOD 6.000000
STI data
LOD   aux_var
P_LOD 0.000000
STI data_i
LOD 6
SET   aux_var
P_LOD 7.000000
STI data
LOD   aux_var
P_LOD 0.000000
STI data_i
LOD 7
SET   aux_var
P_LOD 8.000000
STI data
LOD   aux_var
P_LOD 0.000000
STI data_i
LOD 8
CAL fft
LOD 0
ILI data
P_LOD 0
ILI data_i
POP
F_MLT 1000.0
F2I
OUT 0
LOD 0
ILI data
P_LOD 0
ILI data_i
SET_P aux_var
LOD   aux_var
F_MLT 1000.0
F2I
OUT 1
LOD 1
ILI data
P_LOD 1
ILI data_i
POP
F_MLT 1000.0
F2I
OUT 0
LOD 1
ILI data
P_LOD 1
ILI data_i
SET_P aux_var
LOD   aux_var
F_MLT 1000.0
F2I
OUT 1
LOD 2
ILI data
P_LOD 2
ILI data_i
POP
F_MLT 1000.0
F2I
OUT 0
LOD 2
ILI data
P_LOD 2
ILI data_i
SET_P aux_var
LOD   aux_var
F_MLT 1000.0
F2I
OUT 1
LOD 3
ILI data
P_LOD 3
ILI data_i
POP
F_MLT 1000.0
F2I
OUT 0
LOD 3
ILI data
P_LOD 3
ILI data_i
SET_P aux_var
LOD   aux_var
F_MLT 1000.0
F2I
OUT 1
LOD 4
ILI data
P_LOD 4
ILI data_i
POP
F_MLT 1000.0
F2I
OUT 0
LOD 4
ILI data
P_LOD 4
ILI data_i
SET_P aux_var
LOD   aux_var
F_MLT 1000.0
F2I
OUT 1
LOD 5
ILI data
P_LOD 5
ILI data_i
POP
F_MLT 1000.0
F2I
OUT 0
LOD 5
ILI data
P_LOD 5
ILI data_i
SET_P aux_var
LOD   aux_var
F_MLT 1000.0
F2I
OUT 1
LOD 6
ILI data
P_LOD 6
ILI data_i
POP
F_MLT 1000.0
F2I
OUT 0
LOD 6
ILI data
P_LOD 6
ILI data_i
SET_P aux_var
LOD   aux_var
F_MLT 1000.0
F2I
OUT 1
LOD 7
ILI data
P_LOD 7
ILI data_i
POP
F_MLT 1000.0
F2I
OUT 0
LOD 7
ILI data
P_LOD 7
ILI data_i
SET_P aux_var
LOD   aux_var
F_MLT 1000.0
F2I
OUT 1
@fim JMP fim
