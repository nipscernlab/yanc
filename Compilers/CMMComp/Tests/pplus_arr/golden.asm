NOP
#PRNAME pplus_arr
#NUBITS 16
#NDSTAC 4
#SDEPTH 4
#NUIOIN 1
#NUIOOU 1
#NBMANT 10
#NBEXPO 5
#NUGAIN 128
@main #array main_a 1 4
#array main_b 1 16
LOD 4
SET main_b_arr_size
@Lwh1 LOD 1
JIZ Lwh1end
INN 0
SET main_idx
LDI main_a
ADD 1
LOD main_idx
STI main_a
LOD main_idx
MLT main_b_arr_size
ADD main_idx
LDI main_b
ADD 1
LOD  main_idx
MLT  main_b_arr_size
ADD  main_idx
STI main_b
LOD main_idx
LDI main_a
OUT 0
JMP Lwh1
@Lwh1end @fim JMP fim
