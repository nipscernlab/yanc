// Negative fixture for asmcomp (regress step 4b2): 31 + 8 + 1 = 40 adds up,
// but a SAPHO word is at most 32 bits (the words are built in a C int, TODO
// item 6). An .asm: cmmcomp stops at 40 bits on its own int limit.
#PRNAME fpfmt_wide
#NUBITS 40
#NBMANT 31
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
@main LOD 1.5
F_MLT 2.0
F2I_M 3.0
@fim JMP fim
