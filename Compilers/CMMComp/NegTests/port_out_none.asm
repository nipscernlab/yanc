// Negative fixture for asmcomp (regress step 4b2): with #NUIOOU 0 the
// processor has no output pin, so no OUT is right, not even on port 0 (#177).
#PRNAME port_out_none
#NUBITS 16
#NUIOIN 1
#NUIOOU 0
@main INN 0
OUT 0
@fim JMP fim
