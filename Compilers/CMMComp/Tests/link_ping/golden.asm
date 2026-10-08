NOP
#PRNAME link_ping
#NUBITS 32
#NUIOIN 1
#NUIOOU 1
#NBMANT 23
#NBEXPO 8
#NUGAIN 128
@main LOD 0
SET main_n
@Lwh1 LOD 16
LES main_n
JIZ Lwh1end
LOD main_n
@Ltom1 TOM Ltom1
@Lcad2 CAD Lcad2
SET main_r
OUT 0
LOD main_n
ADD 1
SET main_n
JMP Lwh1
@Lwh1end @fim JMP fim
