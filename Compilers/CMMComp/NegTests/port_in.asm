// Negative fixture for asmcomp (regress step 4b2): a port of INN is below
// #NUIOIN, as cmmcomp checks for in() (#177). Hand-written: asmcomp took
// INN 5 with two input ports, and the hardware, which keeps one bit of the
// port number for two ports, would have read port 1.
#PRNAME port_in
#NUBITS 16
#NUIOIN 2
#NUIOOU 1
@main INN 5
OUT 0
@fim JMP fim
