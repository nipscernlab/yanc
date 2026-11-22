// Negative fixture for asmcomp (regress step 4b2): a port past 255, in
// a stack form of INN (#177). asmcomp marked the ports used in a table of 256
// entries and wrote past its end; the table now grows, and the port is held
// to #NUIOIN like any other.
#PRNAME port_in_wide
#NUBITS 16
#NUIOIN 2
#NUIOOU 1
@main P_INN 300
OUT 0
@fim JMP fim
