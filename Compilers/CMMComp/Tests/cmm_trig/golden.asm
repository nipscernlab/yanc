NOP
#PRNAME cmm_trig
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
LOD 0.0
CAL float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 1.5707963
CAL float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 0.0
F_NEG
F_ADD 1.570796327CAL float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 100.0
CAL float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
LOD 3.1415927
F_NEG
F_ADD 1.570796327CAL float_sin
F_MLT 1000.0
SET main_r
F2I_M main_r
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim

// Sine function --------------------------------------------------------------
// Range reduction is O(1): k = round(x/2pi); x -= k*2pi brings x into [-pi, pi]
// (no subtraction loop). Then the table lookup on |x| in [0, pi] with linear
// interpolation, and the sign is applied at the end (sin is odd on [-pi, pi]).
// NB: F2I truncates toward zero, so round-to-nearest uses a sign-half bias.

#arrays sin_LUT 2 152 "$Sin_LUT.txt"

@float_sin      SET   sin_x                 // save x

              F_MLT   0.1591549431          // q = x / 2pi   (1/2pi)
                SET   sin_q
                LOD   0.5
              F_SGN   sin_q                 // copysign(0.5, q)
              F_ADD   sin_q                 // q + copysign(0.5,q)
                F2I                         // k = round(q)   (F2I truncates)
                I2F                         // float(k)
              F_MLT   6.2831853072          // k * 2pi
              F_SU2   sin_x                 // x - k*2pi  -> [-pi, pi]
                SET   sin_x

              F_ABS_M sin_x                 // table method starts here, on |x|

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
