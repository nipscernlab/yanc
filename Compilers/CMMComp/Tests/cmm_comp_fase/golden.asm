NOP
#PRNAME cmm_comp_fase
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
LOD 1.000000
SET main_c
LOD 2.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa1a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa1b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa1z
@Lfa1b LOD fase_t
F_ADD 3.14159265359
JMP Lfa1z
@Lfa1a LOD 0.0
F_LES main_c
JIZ Lfa1c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa1z
@Lfa1c LOD 0.0
F_LES main_c_i
JIZ Lfa1d
LOD 1.57079632679
JMP Lfa1z
@Lfa1d LOD main_c_i
F_LES 0.0
JIZ Lfa1e
LOD -1.57079632679
JMP Lfa1z
@Lfa1e LOD 0.0
@Lfa1z F_MLT 1000.0
F2I
OUT 0
LOD -1.000000
SET main_c
LOD 2.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa2a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa2b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa2z
@Lfa2b LOD fase_t
F_ADD 3.14159265359
JMP Lfa2z
@Lfa2a LOD 0.0
F_LES main_c
JIZ Lfa2c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa2z
@Lfa2c LOD 0.0
F_LES main_c_i
JIZ Lfa2d
LOD 1.57079632679
JMP Lfa2z
@Lfa2d LOD main_c_i
F_LES 0.0
JIZ Lfa2e
LOD -1.57079632679
JMP Lfa2z
@Lfa2e LOD 0.0
@Lfa2z F_MLT 1000.0
F2I
OUT 0
LOD -1.000000
SET main_c
LOD -2.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa3a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa3b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa3z
@Lfa3b LOD fase_t
F_ADD 3.14159265359
JMP Lfa3z
@Lfa3a LOD 0.0
F_LES main_c
JIZ Lfa3c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa3z
@Lfa3c LOD 0.0
F_LES main_c_i
JIZ Lfa3d
LOD 1.57079632679
JMP Lfa3z
@Lfa3d LOD main_c_i
F_LES 0.0
JIZ Lfa3e
LOD -1.57079632679
JMP Lfa3z
@Lfa3e LOD 0.0
@Lfa3z F_MLT 1000.0
F2I
OUT 0
LOD 1.000000
SET main_c
LOD -2.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa4a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa4b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa4z
@Lfa4b LOD fase_t
F_ADD 3.14159265359
JMP Lfa4z
@Lfa4a LOD 0.0
F_LES main_c
JIZ Lfa4c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa4z
@Lfa4c LOD 0.0
F_LES main_c_i
JIZ Lfa4d
LOD 1.57079632679
JMP Lfa4z
@Lfa4d LOD main_c_i
F_LES 0.0
JIZ Lfa4e
LOD -1.57079632679
JMP Lfa4z
@Lfa4e LOD 0.0
@Lfa4z F_MLT 1000.0
F2I
OUT 0
LOD 2.000000
SET main_c
LOD 0.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa5a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa5b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa5z
@Lfa5b LOD fase_t
F_ADD 3.14159265359
JMP Lfa5z
@Lfa5a LOD 0.0
F_LES main_c
JIZ Lfa5c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa5z
@Lfa5c LOD 0.0
F_LES main_c_i
JIZ Lfa5d
LOD 1.57079632679
JMP Lfa5z
@Lfa5d LOD main_c_i
F_LES 0.0
JIZ Lfa5e
LOD -1.57079632679
JMP Lfa5z
@Lfa5e LOD 0.0
@Lfa5z F_MLT 1000.0
F2I
OUT 0
LOD -2.000000
SET main_c
LOD 0.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa6a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa6b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa6z
@Lfa6b LOD fase_t
F_ADD 3.14159265359
JMP Lfa6z
@Lfa6a LOD 0.0
F_LES main_c
JIZ Lfa6c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa6z
@Lfa6c LOD 0.0
F_LES main_c_i
JIZ Lfa6d
LOD 1.57079632679
JMP Lfa6z
@Lfa6d LOD main_c_i
F_LES 0.0
JIZ Lfa6e
LOD -1.57079632679
JMP Lfa6z
@Lfa6e LOD 0.0
@Lfa6z F_MLT 1000.0
F2I
OUT 0
LOD 0.000000
SET main_c
LOD 2.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa7a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa7b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa7z
@Lfa7b LOD fase_t
F_ADD 3.14159265359
JMP Lfa7z
@Lfa7a LOD 0.0
F_LES main_c
JIZ Lfa7c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa7z
@Lfa7c LOD 0.0
F_LES main_c_i
JIZ Lfa7d
LOD 1.57079632679
JMP Lfa7z
@Lfa7d LOD main_c_i
F_LES 0.0
JIZ Lfa7e
LOD -1.57079632679
JMP Lfa7z
@Lfa7e LOD 0.0
@Lfa7z F_MLT 1000.0
F2I
OUT 0
LOD 0.000000
SET main_c
LOD -2.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa8a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa8b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa8z
@Lfa8b LOD fase_t
F_ADD 3.14159265359
JMP Lfa8z
@Lfa8a LOD 0.0
F_LES main_c
JIZ Lfa8c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa8z
@Lfa8c LOD 0.0
F_LES main_c_i
JIZ Lfa8d
LOD 1.57079632679
JMP Lfa8z
@Lfa8d LOD main_c_i
F_LES 0.0
JIZ Lfa8e
LOD -1.57079632679
JMP Lfa8z
@Lfa8e LOD 0.0
@Lfa8z F_MLT 1000.0
F2I
OUT 0
LOD 0.000000
SET main_c
LOD 0.000000
SET main_c_i
LOD main_c
F_LES 0.0
JIZ Lfa9a
LOD main_c
F_DIV main_c_i
CAL float_atan
SET fase_t
LOD main_c_i
F_LES 0.0
JIZ Lfa9b
LOD fase_t
F_ADD -3.14159265359
JMP Lfa9z
@Lfa9b LOD fase_t
F_ADD 3.14159265359
JMP Lfa9z
@Lfa9a LOD 0.0
F_LES main_c
JIZ Lfa9c
LOD main_c
F_DIV main_c_i
CAL float_atan
JMP Lfa9z
@Lfa9c LOD 0.0
F_LES main_c_i
JIZ Lfa9d
LOD 1.57079632679
JMP Lfa9z
@Lfa9d LOD main_c_i
F_LES 0.0
JIZ Lfa9e
LOD -1.57079632679
JMP Lfa9z
@Lfa9e LOD 0.0
@Lfa9z F_MLT 1000.0
F2I
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim

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