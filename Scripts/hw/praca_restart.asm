NOP
#NUBITS 8
#NUIOIN 1
#NUIOOU 1
// The processor of Scripts/hw/tb_praca_restart.sh (#188): a loop that calls
// a -> b -> c, three return addresses deep, and c holds three words on the
// data stack before it adds them to n (n + 3). asmcomp works out SDEPTH 3 and
// NDSTAC 3, not a power of two, so a pointer that climbed past 3 would read
// outside the stack. The #ITRAD point is the loop's top, where both stacks
// are empty; each pass puts n on port 0.
@main LOD 0
SET n
#ITRAD
@loop CAL a
LOD n
OUT 0
JMP loop
@a CAL b
RET
@b CAL c
RET
@c LOD n
P_LOD 1
P_LOD 1
P_LOD 1
S_ADD
S_ADD
S_ADD
SET n
RET
