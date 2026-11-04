// Negative fixture for asmcomp (regress step 4b2): #FFTSIZ reverses 1 to
// 30 low bits of an index; asmcomp checks it as cmmcomp does, since it is
// the gate every front end passes through (#108 A3). An .asm: cmmcomp
// refuses 40 on its own.
#PRNAME fftsiz_wide
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#FFTSIZ 40
@main LOD 1
@fim JMP fim
