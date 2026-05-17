NOP
#PRNAME Seno
#NUBITS 32
#NDSTAC 5
#SDEPTH 5
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
JMP main
@seno_LUT SET seno_LUT_x
#arrays seno_LUT_Seno_LUT 2 152 "Seno_LUT.txt"
@Lwh1 F_ABS_M seno_LUT_x
P_LOD 3.141592653589793
SF_GRE
JIZ Lwh1end
LOD 6.283185307
F_SGN seno_LUT_x
F_SU2 seno_LUT_x
SET seno_LUT_x
JMP Lwh1
@Lwh1end LOD seno_LUT_x
F_MLT 47.746482927568
F_ABS
SET seno_LUT_idxf
F2I_M seno_LUT_idxf
SET seno_LUT_idx
LOD seno_LUT_idx
LDI seno_LUT_Seno_LUT
SET seno_LUT_v
LOD seno_LUT_idx
ADD 1
LDI seno_LUT_Seno_LUT
F_SU1 seno_LUT_v
P_I2F_M seno_LUT_idx
F_SU2 seno_LUT_idxf
SF_MLT
F_ADD seno_LUT_v
F_SGN seno_LUT_x
RET
@main #arrays main_x 2 1000 "sin_x.txt"
#arrays main_a 2 1000 "sin_y.txt"
LOD 0
SET main_j
@Lwh2 LOD 1000
LES main_j
JIZ Lwh2end
LOD main_j
LDI main_x
F_ADD 6.283185307
CAL float_sin
SET main_y
LOD main_j
LDI main_a
SET main_t
LOD main_t
F_SU1 main_y
SET main_e
LOD main_j
ADD 1
SET main_j
JMP Lwh2
@Lwh2end LOD 0
SET main_j
@Lwh3 LOD 1000
LES main_j
JIZ Lwh3end
LOD main_j
LDI main_x
CAL seno_LUT
SET main_y
LOD main_j
LDI main_a
SET main_t
LOD main_t
F_SU1 main_y
SET main_e
LOD main_j
ADD 1
SET main_j
JMP Lwh3
@Lwh3end @fim JMP fim

// Sine function --------------------------------------------------------------

#arrays sin_LUT 2 152 "$Sin_LUT.txt"

@float_sin      SET   sin_x                 // save x

@L_sin        F_ABS_M sin_x                 // check whether x < pi
              F_LES   3.141592653589793     // compare on magnitude for float
                JIZ   L_sin_end

                LOD   6.283185307           // otherwise, keep subtracting 2pi
              F_SGN   sin_x
              F_SU2   sin_x
                SET   sin_x
                JMP   L_sin

@L_sin_end    F_ABS_M sin_x                 // method starts here

              F_MLT   47.746482927568       // multiply by 150.0/pi to find the position in x
                SET   sin_idxf              // save into idxf

                F2I                         // round the index down
                SET   sin_idx               // save into idx

                LDI   sin_LUT               // fetch the matching data in the table
                SET   sin_v                 // save into v

                LOD   sin_idx               // fetch the next index
                ADD   1                     // could be INC_M sin_idx

                LDI   sin_LUT               // fetch the matching data in the table

              F_SU1   sin_v                 // subtract from v

              P_I2F_M sin_idx               // get the slope of the line
              F_SU2   sin_idxf

             SF_MLT                         // compute the value on the line

              F_ADD   sin_v                 // add the offset v

              F_SGN   sin_x                 // apply the sign and return
                RET
