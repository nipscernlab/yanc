NOP
#PRNAME cmm_comp_func
#NUBITS 16
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
JMP main
@echo SET_P echo_c_i
SET echo_c
LOD echo_c
P_LOD echo_c_i
RET
@addc SET_P addc_b_i
SET_P addc_b
SET_P addc_a_i
SET addc_a
LOD addc_a
F_ADD addc_b
P_LOD addc_a_i
F_ADD addc_b_i
RET
@mk SET_P mk_im
SET mk_re
I2F_M mk_re
P_I2F_M mk_im
RET
@sumri SET_P sumri_c_i
SET sumri_c
LOD sumri_c
P_LOD sumri_c_i
SF_ADD
F2I
RET
@takesint SET takesint_x
LOD takesint_x
RET
@midcomp SET_P midcomp_b
SET_P midcomp_c_i
SET_P midcomp_c
SET midcomp_a
LOD midcomp_a
OUT 0
LOD midcomp_c
F2I
OUT 0
LOD midcomp_c_i
F2I
OUT 0
LOD midcomp_b
RET
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 3.000000
P_LOD 4.000000
CAL echo
SET_P main_e_i
SET main_e
F2I
OUT 0
LOD main_e_i
F2I
OUT 0
LOD 1.000000
P_LOD 2.000000
P_LOD 3.000000
P_LOD 4.000000
CAL addc
SET_P main_s_i
SET main_s
F2I
OUT 0
LOD main_s_i
F2I
OUT 0
LOD 5
P_LOD 9
CAL mk
SET_P main_m_i
SET main_m
F2I
OUT 0
LOD main_m_i
F2I
OUT 0
LOD 10.000000
P_LOD 5.000000
CAL sumri
OUT 0
I2F_M 8
P_LOD 0.0
CAL echo
SET_P main_ci_i
SET main_ci
F2I
OUT 0
LOD main_ci_i
F2I
OUT 0
LOD 5.0
P_LOD 0.0
CAL echo
SET_P main_cf_i
SET main_cf
F2I
OUT 0
LOD main_cf_i
F2I
OUT 0
LOD 7.000000
F2I
CAL takesint
OUT 0
LOD 1
P_LOD 11.000000
P_LOD 22.000000
P_LOD 2
CAL midcomp
OUT 0
LOD 6.000000
P_LOD 7.000000
CAL echo
CAL echo
SET_P main_nest_i
SET main_nest
F2I
OUT 0
LOD main_nest_i
F2I
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
