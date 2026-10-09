NOP
#PRNAME cmm_comp_sqrt
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE main_r_i __csqrt_im
#SHARE main_w __csqrt_b
#SHARE main_w_i __csqrt_a
#SHARE __sqrt_num __csqrt_im
@main @__Lwh1 LOD 3.000000
SET main_z
LOD 4.000000
SET main_z_i
LOD main_z
F_MLT main_z
P_LOD main_z_i
F_MLT main_z_i
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD main_z
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 main_z
F_MLT 0.5
CAL __float_sqrt
F_SGN main_z_i
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
LOD -3.000000
F_MLT -3.000000
P_LOD 4.000000
F_MLT 4.000000
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD -3.000000
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 -3.000000
F_MLT 0.5
CAL __float_sqrt
F_SGN 4.000000
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
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
SET_P __csqrt_b
SET   __csqrt_a
F_MLT __csqrt_a
P_LOD __csqrt_b
F_MLT __csqrt_b
SF_ADD
CAL __float_sqrt
SET __csqrt_r
F_ADD __csqrt_a
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 __csqrt_a
F_MLT 0.5
CAL __float_sqrt
F_SGN __csqrt_b
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
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
CAL __float_sqrt
SET __csqrt_r
F_ADD main_z
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 main_z
F_MLT 0.5
CAL __float_sqrt
F_SGN main_z_i
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
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
CAL __float_sqrt
SET __csqrt_r
F_ADD main_z
F_MLT 0.5
CAL __float_sqrt
SET __csqrt_re
LOD __csqrt_r
F_SU1 main_z
F_MLT 0.5
CAL __float_sqrt
F_SGN main_z_i
SET __csqrt_im
LOD __csqrt_re
P_LOD __csqrt_im
SET_P main_r_i
SET main_r
F2I_M main_r
OUT 0
F2I_M main_r_i
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim

// sqrt function --------------------------------------------------------------

@__float_sqrt     SET __sqrt_num       // get the parameter
              F_ROT                // first estimate (nearest power of 2)

                PSH                // update x
              F_DIV __sqrt_num       // iteration 1
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 2
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 3
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV __sqrt_num       // iteration 4
NOP
F_QUO
             SF_ADD
              F_MLT 0.5

                RET
