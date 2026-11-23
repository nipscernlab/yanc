// Negative fixture for asmcomp (regress step 4b2): the operand of INN is
// the port number, not a name (#177). asmcomp read a name as port 0.
#PRNAME port_name
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
@main INN x
OUT 0
@fim JMP fim
