NOP
#PRNAME func_combos
#NUBITS 16
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
JMP main
@noargs_void LOD 7
OUT 0
RET
@noargs_int LOD 42
RET
@one_int SET one_int_a
LOD one_int_a
ADD 1
RET
@one_float SET one_float_b
LOD one_float_b
F_ADD 0.5
RET
@one_comp SET_P one_comp_c_i
SET one_comp_c
LOD one_comp_c
P_LOD one_comp_c_i
RET
@three_mixed SET_P three_mixed_r_i
SET_P three_mixed_r
SET_P three_mixed_q
SET three_mixed_p
LOD three_mixed_p
OUT 0
F2I_M three_mixed_q
OUT 0
LOD three_mixed_r
F2I
OUT 0
LOD three_mixed_p
RET
@main @Lwh1 LOD 1
JIZ Lwh1end
CAL noargs_void
CAL noargs_int
SET main_v1
LOD 5
CAL one_int
SET main_v2
LOD 2.5
CAL one_float
SET main_v3
LOD 1.000000
P_LOD 2.000000
CAL one_comp
SET_P main_v4_i
SET main_v4
LOD 7
P_LOD 3.5
P_LOD 6.000000
P_LOD 7.000000
CAL three_mixed
SET main_v5
LOD main_v1
OUT 0
LOD main_v2
OUT 0
F2I_M main_v3
OUT 0
LOD main_v4
F2I
OUT 0
LOD main_v5
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
