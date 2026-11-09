NOP
#NUBITS 8
#NUIOIN 1
#NUIOOU 1
// The second processor of Scripts/hw/tb_praca_restart.sh (#188): it offers
// n + 1 on link 0 and waits there, since the testbench never takes it. A
// #PRACA pulse during that wait must make the simulation warn.
@main LOD 0
SET n
#ITRAD
@loop LOD n
ADD 1
SET n
TOM 0
JMP loop
