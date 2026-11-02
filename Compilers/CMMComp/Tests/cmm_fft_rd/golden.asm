NOP
#PRNAME cmm_fft_rd
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#FFTSIZ 3
#NUGAIN 128
#array x 1 8
@main LOD 0
SET main_k
@__Lwh1top LOD main_k
P_LOD main_k
STI x
LOD main_k
ADD 1
SET main_k
LES 7
JIZ __Lwh1top
@__Lwh1end LOD 0
SET main_k
@__Lwh2top LOD main_k
ILI x
OUT 0
LOD main_k
ADD 1
SET main_k
LES 7
JIZ __Lwh2top
@__Lwh2end @fim JMP fim
