NOP
#PRNAME cmm_round
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
#SHARE main_r __floor_x
#SHARE __ceil_x __floor_x
#SHARE __ceil_t __floor_t
#SHARE __round_x __floor_x
@main @__Lwh1 LOD 2.7
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
SET main_r
F2I_M main_r
OUT 0
LOD -2.3
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr2end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr2end LOD __floor_t
SET main_r
F2I_M main_r
OUT 0
LOD 2.3
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel1end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel1end LOD __ceil_t
SET main_r
F2I_M main_r
OUT 0
LOD -2.3
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel2end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel2end LOD __ceil_t
SET main_r
F2I_M main_r
OUT 0
LOD 2.5
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_r
F2I_M main_r
OUT 0
LOD -2.5
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_r
F2I_M main_r
OUT 0
LOD 2.4
SET __round_x
LOD 0.5
F_SGN __round_x
F_ADD __round_x
F2I
I2F
SET main_r
F2I_M main_r
OUT 0
LOD 5.0
SET __floor_x
F2I
I2F
SET __floor_t
F_LES __floor_x
JIZ __Lflr3end
LOD __floor_t
F_ADD -1.0
SET __floor_t
@__Lflr3end LOD __floor_t
SET main_r
F2I_M main_r
OUT 0
LOD -5.0
SET __ceil_x
F2I
I2F
SET __ceil_t
LOD __ceil_x
F_LES __ceil_t
JIZ __Lcel3end
LOD __ceil_t
F_ADD 1.0
SET __ceil_t
@__Lcel3end LOD __ceil_t
SET main_r
F2I_M main_r
OUT 0
JMP __Lwh1
@__Lwh1end @fim JMP fim
