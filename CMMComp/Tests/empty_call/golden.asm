NOP
#PRNAME empty_call
#NUBITS 16
#NDSTAC 4
#SDEPTH 4
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
JMP main
@answer LOD 42
RET
@emit_one LOD 1
OUT 0
RET
@main @Lwh1 LOD 1
JIZ Lwh1end
CAL answer
SET main_y
CAL emit_one
LOD main_y
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
