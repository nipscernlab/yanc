NOP
#PRNAME cmm_names
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE main_k switch_exp
#SHARE __aux_var Lwh1
#SHARE main_yy Lwh1
#SHARE __pow_b aux_var
#SHARE __pow_r Lwh1
#SHARE main_y aux_var
#SHARE main_s pow_b
#SHARE __floor_x floor_x
#SHARE __floor_t pow_b
#SHARE main_r floor_x
#SHARE __sqrt_num pow_b
LOD 4
SET Lwh1
LOD 2
SET aux_var
LOD 2
SET pow_b
LOD 0.5
SET floor_x
LOD 5
SET sw_body_1_1
LOD 0
SET switch_exp
LOD 9
SET fim
#array g 2 2
@main LOD 0
SET main_k
@__Lwh1 LOD 3
LES main_k
JIZ __Lwh1end
LOD main_k
ADD 1
SET main_k
JMP __Lwh1
@__Lwh1end LOD Lwh1
ADD main_k
OUT 0
LOD 1.5
SET_V g 0
LOD 0
PSH
LDI g
SET __aux_var
F_ADD 1.0
STI g
LOD __aux_var
SET main_yy
LOD aux_var
OUT 0
LOD 2.0
SET __pow_b
SET __pow_r
LOD __pow_b
F_MLT __pow_b
SET __pow_b
LOD __pow_r
F_MLT __pow_b
SET __pow_r
F2I
SET main_y
ADD pow_b
OUT 0
LOD 16.0
CAL __float_sqrt
SET main_s
F_ADD floor_x
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr1end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr1end LOD __floor_t
F2I
SET main_r
OUT 0
LOD main_k
ADD -3
JIZ __sw_body_1_1
JMP __sw_body_1_2
@__sw_body_1_1 LOD 1
SET switch_exp
JMP __switch_end_1
@__sw_body_1_2 LOD 2
SET switch_exp
@__switch_end_1 LOD sw_body_1_1
OUT 0
LOD fim
OUT 0
@fim JMP fim

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
