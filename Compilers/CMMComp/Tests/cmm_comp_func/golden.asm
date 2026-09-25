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
#SHARE addc_b_i echo_c_i
#SHARE addc_b echo_c
#SHARE mk_im echo_c_i
#SHARE mk_re echo_c
#SHARE sumri_c_i echo_c_i
#SHARE sumri_c echo_c
#SHARE takesint_x echo_c_i
#SHARE midcomp_b echo_c_i
#SHARE midcomp_c_i echo_c
#SHARE midcomp_c addc_a_i
#SHARE midcomp_a addc_a
#SHARE main_e_i echo_c_i
#SHARE main_e echo_c
#SHARE main_s_i echo_c_i
#SHARE main_s echo_c
#SHARE main_m_i echo_c_i
#SHARE main_m echo_c
#SHARE main_ci_i echo_c_i
#SHARE main_ci echo_c
#SHARE main_cf_i echo_c_i
#SHARE main_cf echo_c
#SHARE main_nest_i echo_c_i
#SHARE main_nest echo_c
JMP main
@echo SET_P echo_c_i
SET echo_c
P_LOD echo_c_i
RET
@addc SET_P addc_b_i
SET_P addc_b
SET_P addc_a_i
SET addc_a
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
F_ADD sumri_c_i
F2I
RET
@takesint SET takesint_x
RET
@midcomp SET_P midcomp_b
SET_P midcomp_c_i
SET_P midcomp_c
SET midcomp_a
OUT 0
F2I_M midcomp_c
OUT 0
F2I_M midcomp_c_i
OUT 0
LOD midcomp_b
RET
@main @Lwh1 LOD 3.000000
P_LOD 4.000000
CAL echo
SET_P main_e_i
SET main_e
F2I_M main_e
OUT 0
F2I_M main_e_i
OUT 0
LOD 1.000000
P_LOD 2.000000
P_LOD 3.000000
P_LOD 4.000000
CAL addc
SET_P main_s_i
SET main_s
F2I_M main_s
OUT 0
F2I_M main_s_i
OUT 0
LOD 5
P_LOD 9
CAL mk
SET_P main_m_i
SET main_m
F2I_M main_m
OUT 0
F2I_M main_m_i
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
F2I_M main_ci
OUT 0
F2I_M main_ci_i
OUT 0
LOD 5.0
P_LOD 0.0
CAL echo
SET_P main_cf_i
SET main_cf
F2I_M main_cf
OUT 0
F2I_M main_cf_i
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
F2I_M main_nest
OUT 0
F2I_M main_nest_i
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
