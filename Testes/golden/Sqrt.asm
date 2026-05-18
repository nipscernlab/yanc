NOP
#PRNAME Sqrt
#NUBITS 32
#NDSTAC 5
#SDEPTH 5
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
JMP main
@my_sqrt SET my_sqrt_num
LOD my_sqrt_num
P_LOD 0.0
S_EQU
JIZ Lif1else
LOD 0.0
RET
@Lif1else LOD 1
SHL my_sqrt_num
P_LOD 24
S_SRS
ADD 22
P_LOD 1
S_SRS
SET my_sqrt_v
NEG_M 22
ADD my_sqrt_v
P_LOD 23
S_SHL
P_LOD 22
SHL 1
S_ADD
P_LOD 1
S_SHL
P_LOD 1
S_SHR
SET my_sqrt_v
SET my_sqrt_x
F_DIV my_sqrt_num
F_ADD my_sqrt_x
F_MLT 0.5
SET my_sqrt_x
F_DIV my_sqrt_num
F_ADD my_sqrt_x
F_MLT 0.5
SET my_sqrt_x
F_DIV my_sqrt_num
F_ADD my_sqrt_x
F_MLT 0.5
SET my_sqrt_x
F_DIV my_sqrt_num
F_ADD my_sqrt_x
F_MLT 0.5
SET my_sqrt_x
RET
@main #arrays main_x 2 1000 "sqrt_x.txt"
#arrays main_a 2 1000 "sqrt_y.txt"
LOD 0
SET main_j
@Lwh1 LOD 1000
LES main_j
JIZ Lwh1end
LOD main_j
LDI main_x
CAL float_sqrt
SET main_y
LOD main_j
LDI main_a
SET main_t
F_SU1 main_y
SET main_e
LOD main_j
ADD 1
SET main_j
JMP Lwh1
@Lwh1end LOD 0
SET main_j
@Lwh2 LOD 1000
LES main_j
JIZ Lwh2end
LOD main_j
LDI main_x
CAL my_sqrt
SET main_y
LOD main_j
LDI main_a
SET main_t
F_SU1 main_y
SET main_e
LOD main_j
ADD 1
SET main_j
JMP Lwh2
@Lwh2end @fim JMP fim

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
