NOP
#PRNAME cmm_fft_wr
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
@Lwh1 LOD 8
LES main_k
JIZ Lwh1end
LOD main_k
P_LOD main_k
ISI x
LOD main_k
ADD 1
SET main_k
JMP Lwh1
@Lwh1end LOD 0
SET main_k
@Lwh2 LOD 8
LES main_k
JIZ Lwh2end
LOD main_k
LDI x
SET main_v
OUT 0
LOD main_k
ADD 1
SET main_k
JMP Lwh2
@Lwh2end @fim JMP fim
