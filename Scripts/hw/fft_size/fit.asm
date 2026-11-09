NOP
#NUBITS 8
#NUIOIN 1
#NUIOOU 1
#FFTSIZ 3
@main #arrays v 1 8 "v.txt"
LOD 8
SET n
@lp INN 0
ILI v
OUT 0
LOD n
ADD -1
SET n
JNZ lp
@fim JMP fim
