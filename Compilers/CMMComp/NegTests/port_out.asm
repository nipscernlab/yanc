// Negative fixture for asmcomp (regress step 4b2): a port of OUT is below
// #NUIOOU, as cmmcomp checks for out() (#177).
#PRNAME port_out
#NUBITS 16
#NUIOIN 1
#NUIOOU 2
@main INN 0
OUT 2
@fim JMP fim
