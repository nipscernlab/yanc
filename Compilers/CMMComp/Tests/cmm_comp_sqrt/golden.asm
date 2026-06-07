NOP
#PRNAME cmm_comp_sqrt
#NUBITS 32
#NDSTAC 8
#SDEPTH 8
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main @Lwh1 LOD 1
JIZ Lwh1end
LOD 3.000000
SET main_z
LOD 4.000000
SET main_z_i
LOD main_z
F_MLT main_z
P_LOD main_z_i
F_MLT main_z_i
SF_ADD
CAL float_sqrt
SET csqrt_r
F_ADD main_z
F_MLT 0.5
CAL float_sqrt
SET csqrt_re
LOD csqrt_r
F_SU1 main_z
F_MLT 0.5
CAL float_sqrt
F_SGN main_z_i
SET csqrt_im
LOD csqrt_re
P_LOD csqrt_im
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD -3.000000
F_MLT -3.000000
P_LOD 4.000000
F_MLT 4.000000
SF_ADD
CAL float_sqrt
SET csqrt_r
F_ADD -3.000000
F_MLT 0.5
CAL float_sqrt
SET csqrt_re
LOD csqrt_r
F_SU1 -3.000000
F_MLT 0.5
CAL float_sqrt
F_SGN 4.000000
SET csqrt_im
LOD csqrt_re
P_LOD csqrt_im
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 1.000000
SET main_z
LOD 2.000000
SET main_z_i
LOD 2.000000
SET main_w
LOD 2.000000
SET main_w_i
LOD main_z
F_ADD main_w
P_LOD main_z_i
F_ADD main_w_i
SET_P csqrt_b
SET   csqrt_a
F_MLT csqrt_a
P_LOD csqrt_b
F_MLT csqrt_b
SF_ADD
CAL float_sqrt
SET csqrt_r
F_ADD csqrt_a
F_MLT 0.5
CAL float_sqrt
SET csqrt_re
LOD csqrt_r
F_SU1 csqrt_a
F_MLT 0.5
CAL float_sqrt
F_SGN csqrt_b
SET csqrt_im
LOD csqrt_re
P_LOD csqrt_im
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD 4.000000
SET main_z
LOD 0.000000
SET main_z_i
LOD main_z
F_MLT main_z
P_LOD main_z_i
F_MLT main_z_i
SF_ADD
CAL float_sqrt
SET csqrt_r
F_ADD main_z
F_MLT 0.5
CAL float_sqrt
SET csqrt_re
LOD csqrt_r
F_SU1 main_z
F_MLT 0.5
CAL float_sqrt
F_SGN main_z_i
SET csqrt_im
LOD csqrt_re
P_LOD csqrt_im
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
LOD -4.000000
SET main_z
LOD 0.000000
SET main_z_i
LOD main_z
F_MLT main_z
P_LOD main_z_i
F_MLT main_z_i
SF_ADD
CAL float_sqrt
SET csqrt_r
F_ADD main_z
F_MLT 0.5
CAL float_sqrt
SET csqrt_re
LOD csqrt_r
F_SU1 main_z
F_MLT 0.5
CAL float_sqrt
F_SGN main_z_i
SET csqrt_im
LOD csqrt_re
P_LOD csqrt_im
SET_P main_r_i
SET main_r
F2I
OUT 0
LOD main_r_i
F2I
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim

// sqrt function --------------------------------------------------------------

@float_sqrt     SET sqrt_num       // get the parameter
              F_ROT                // first estimate (nearest power of 2)

                PSH                // update x
              F_DIV sqrt_num       // iteration 1
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 2
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 3
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 4
             SF_ADD
              F_MLT 0.5

                RET
