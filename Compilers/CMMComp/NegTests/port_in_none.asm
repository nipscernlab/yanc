// Negative fixture for asmcomp (regress step 4b2): with #NUIOIN 0 the
// processor has no input pin, so no INN is right, not even on port 0 (#177).
// asmcomp wrote an address decoder of zero ports and left req_in undeclared.
#PRNAME port_in_none
#NUBITS 16
#NUIOIN 0
#NUIOOU 1
@main INN 0
OUT 0
@fim JMP fim
