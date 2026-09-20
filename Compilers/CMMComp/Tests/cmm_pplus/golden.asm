NOP
#PRNAME cmm_pplus
#NUBITS 32
#NDSTAC 16
#SDEPTH 16
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#array a 1 4
#array b 1 9
LOD 3
SET b_arr_size
#array f 2 2
#array hist 1 4
#array s 1 6
#array buf 1 4
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 5
SET main_k
ADD 1
SET main_k
OUT 0
LOD 1.5
SET main_x
F_ADD 1.0
SET main_x
F_MLT 10.0
F2I
OUT 0
LOD 2.5
SET main_x
PSH
F_ADD 1.0
SET_P main_x
F2I
SET main_r
OUT 0
LOD main_x
F_MLT 10.0
F2I
OUT 0
LOD 5
SET main_n
PSH
ADD 1
SET_P main_n
SET main_r
OUT 0
LOD main_n
OUT 0
LOD 0
SET_V a 0
LOD 0
SET_V a 1
LOD 0
SET_V a 2
LOD 0
SET_V a 3
LOD 2
SET main_j
PSH
LDI a
ADD 1
STI a
LOD main_j
PSH
LDI a
ADD 1
STI a
LOD_V a 2
OUT 0
LOD_V a 0
P_LOD_V a 1
S_ADD
P_LOD_V a 3
S_ADD
OUT 0
LOD  1
MLT  b_arr_size
ADD  2
P_LOD 5
STI b
LOD  1
MLT  b_arr_size
ADD  2
PSH
LDI b
ADD 1
STI b
LOD 1
MLT b_arr_size
ADD 2
LDI b
OUT 0
LOD 1
SET main_j
MLT  b_arr_size
ADD  main_j
PSH
LDI b
ADD 1
STI b
LOD  main_j
MLT  b_arr_size
ADD  main_j
PSH
LDI b
ADD 1
STI b
LOD 1
MLT b_arr_size
ADD 1
LDI b
OUT 0
LOD 1
P_LOD 2.5
STI f
LOD 1
PSH
LDI f
F_ADD 1.0
STI f
LOD 1
LDI f
F_MLT 10.0
F2I
OUT 0
LOD 3
SET main_j
PSH
LDI a
ADD 1
STI a
ADD -1
SET main_r
OUT 0
LOD_V a 3
OUT 0
LOD main_j
PSH
LDI a
ADD 1
STI a
ADD -1
PSH
LOD main_j
PSH
LDI a
ADD 1
STI a
ADD -1
S_ADD
SET main_r
OUT 0
LOD_V a 3
OUT 0
LOD 0
P_LOD 1.5
STI f
LOD 0
PSH
LDI f
SET aux_var
F_ADD 1.0
STI f
LOD aux_var
SET main_y
F_MLT 10.0
F2I
OUT 0
LOD 0
LDI f
F_MLT 10.0
F2I
OUT 0
LOD  1
MLT  b_arr_size
ADD  1
PSH
LDI b
ADD 1
STI b
ADD -1
SET main_r
OUT 0
LOD 1
MLT b_arr_size
ADD 1
LDI b
OUT 0
LOD 10
SET main_k
LOD main_j
PSH
LDI a
ADD 1
STI a
ADD -1
ADD main_k
SET main_r
OUT 0
LOD_V a 3
OUT 0
LOD 0
SET main_n
LOD 0
SET_V buf 0
LOD 0
SET_V buf 1
LOD 0
SET_V buf 2
LOD main_n
PSH
ADD 1
SET_P main_n
P_LOD 7
STI buf
LOD main_n
PSH
ADD 1
SET_P main_n
P_LOD 9
STI buf
LOD main_n
OUT 0
LOD_V buf 0
OUT 0
LOD_V buf 1
OUT 0
LOD_V buf 2
OUT 0
LOD 0
SET main_k
LOD 0
SET main_r
@Lwh2 LOD main_k
PSH
ADD 1
SET_P main_k
P_LOD 3
S_LES
JIZ Lwh2end
LOD main_r
ADD 1
SET main_r
JMP Lwh2
@Lwh2end LOD main_r
OUT 0
LOD main_k
OUT 0
LOD 0
SET_V hist 0
LOD 0
SET_V hist 1
LOD 0
SET_V hist 2
LOD 0
SET_V hist 3
LOD 1
SET_V s 0
LOD 3
SET_V s 1
LOD 1
SET_V s 2
LOD 0
SET_V s 3
LOD 1
SET_V s 4
LOD 3
SET_V s 5
LOD 0
SET main_j
@Lwh3 LOD 6
LES main_j
JIZ Lwh3end
LOD main_j
LDI s
PSH
LDI hist
ADD 1
STI hist
LOD main_j
ADD 1
SET main_j
JMP Lwh3
@Lwh3end LOD_V hist 0
OUT 0
LOD_V hist 1
OUT 0
LOD_V hist 2
OUT 0
LOD_V hist 3
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
