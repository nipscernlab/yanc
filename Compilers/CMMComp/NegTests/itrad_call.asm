// Negative fixture for asmcomp (regress step 4b2): the itr pulse empties both
// stacks (#188), so the #ITRAD point cannot be inside a routine a CAL enters,
// where the call stack holds the routine's return address. Hand-written:
// cmmcomp refuses a #PRACA outside main on its own (praca_not_main.cmm).
#PRNAME itrad_call
#NUBITS 16
#NUIOIN 1
#NUIOOU 1
@main CAL f
@fim JMP fim
@f LOD 1
#ITRAD
OUT 0
RET
