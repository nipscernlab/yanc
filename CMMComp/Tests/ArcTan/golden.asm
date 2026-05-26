NOP
#PRNAME ArcTan
#NUBITS 32
#NDSTAC 5
#SDEPTH 5
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
JMP main
@arctan_LUT SET arctan_LUT_x
#arrays arctan_LUT_atan_lut 2 49 "Arctan_LUT.txt"
LOD arctan_LUT_x
SET arctan_LUT_signo
F_ABS_M arctan_LUT_x
SET arctan_LUT_x
LOD 0.0
SET arctan_LUT_v0
LOD arctan_LUT_x
P_LOD 1.0
SF_GRE
JIZ Lif1else
I2F_M 1
P_LOD arctan_LUT_x
SF_DIV
SET arctan_LUT_x
LOD 1.57079632679
SET arctan_LUT_v0
@Lif1else LOD 47.0
F_MLT arctan_LUT_x
SET arctan_LUT_idxf
F2I_M arctan_LUT_idxf
SET arctan_LUT_idx
LDI arctan_LUT_atan_lut
SET arctan_LUT_x
LOD arctan_LUT_idx
ADD 1
LDI arctan_LUT_atan_lut
F_SU1 arctan_LUT_x
P_I2F_M arctan_LUT_idx
F_SU2 arctan_LUT_idxf
SF_MLT
F_ADD arctan_LUT_x
F_SU2 arctan_LUT_v0
F_SGN arctan_LUT_signo
RET
@main #arrays main_x 2 1000 "atan_x.txt"
#arrays main_a 2 1000 "atan_y.txt"
LOD 0
SET main_j
@Lwh1 LOD 1000
LES main_j
JIZ Lwh1end
LOD main_j
LDI main_x
CAL float_atan
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
CAL arctan_LUT
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

// Arctangent function --------------------------------------------------------

#arrays atan_lut 2 49 "$Arctan_LUT.txt"

@float_atan SET   atan_x

         PF_ABS_M atan_x          // test whether to use x or 1/x
          F_LES   1.0
            JIZ   L_atan

          F_ABS_M atan_x          // 1/x branch
          F_DIV   47.0
            SET   atan_idxf       // compute position in x

            F2I
            SET   atan_idx        // take the first index

            LDI   atan_lut
            SET   atan_x          // take the first y value from the table

            LOD   atan_idx
            ADD   1
            LDI   atan_lut        // take the second y value from the table

          F_SU1   atan_x          // perform the linear interpolation
          P_I2F_M atan_idx
          F_SU2   atan_idxf
         SF_MLT
          F_ADD   atan_x

          F_ADD  -1.57079632679   // offset for the 1/x branch

         SF_SGN
            RET

@L_atan   F_ABS_M atan_x
          F_MLT   47.0
            SET   atan_idxf

            F2I
            SET   atan_idx

            LDI   atan_lut
            SET   atan_x

            LOD   atan_idx
            ADD   1
            LDI   atan_lut
            
          F_SU1   atan_x
          P_I2F_M atan_idx
          F_SU2   atan_idxf
         SF_MLT
          F_ADD   atan_x

         SF_SGN
            RET