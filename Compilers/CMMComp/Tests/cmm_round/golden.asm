NOP
#PRNAME cmm_round
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
LOD 2.7
SET floor_x
F2I
I2F
SET floor_t
F_LES floor_x
JIZ Lflr1end
LOD floor_t
F_ADD -1.0
SET floor_t
@Lflr1end LOD floor_t
SET main_r
F2I_M main_r
OUT 0
F_NEG_M 2.3
SET floor_x
F2I
I2F
SET floor_t
F_LES floor_x
JIZ Lflr2end
LOD floor_t
F_ADD -1.0
SET floor_t
@Lflr2end LOD floor_t
SET main_r
F2I_M main_r
OUT 0
LOD 2.3
SET ceil_x
F2I
I2F
SET ceil_t
LOD ceil_x
F_LES ceil_t
JIZ Lcel1end
LOD ceil_t
F_ADD 1.0
SET ceil_t
@Lcel1end LOD ceil_t
SET main_r
F2I_M main_r
OUT 0
F_NEG_M 2.3
SET ceil_x
F2I
I2F
SET ceil_t
LOD ceil_x
F_LES ceil_t
JIZ Lcel2end
LOD ceil_t
F_ADD 1.0
SET ceil_t
@Lcel2end LOD ceil_t
SET main_r
F2I_M main_r
OUT 0
LOD 2.5
SET round_x
LOD 0.5
F_SGN round_x
F_ADD round_x
F2I
I2F
SET main_r
F2I_M main_r
OUT 0
F_NEG_M 2.5
SET round_x
LOD 0.5
F_SGN round_x
F_ADD round_x
F2I
I2F
SET main_r
F2I_M main_r
OUT 0
LOD 2.4
SET round_x
LOD 0.5
F_SGN round_x
F_ADD round_x
F2I
I2F
SET main_r
F2I_M main_r
OUT 0
LOD 5.0
SET floor_x
F2I
I2F
SET floor_t
F_LES floor_x
JIZ Lflr3end
LOD floor_t
F_ADD -1.0
SET floor_t
@Lflr3end LOD floor_t
SET main_r
F2I_M main_r
OUT 0
F_NEG_M 5.0
SET ceil_x
F2I
I2F
SET ceil_t
LOD ceil_x
F_LES ceil_t
JIZ Lcel3end
LOD ceil_t
F_ADD 1.0
SET ceil_t
@Lcel3end LOD ceil_t
SET main_r
F2I_M main_r
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
