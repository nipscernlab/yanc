NOP
#NUBITS 8
#NUIOIN 1
#NUIOOU 1
// The processor of Scripts/hw/tb_toaqui_return.sh (#189): a loop that calls
// f (f puts 2 on port 0 and returns) with the #TOAQUI point right after the
// CAL, so the program reaches it only through f's RET.
@main LOD 0
SET k
@loop CAL f
#TOAQUI
LOD k
ADD 1
SET k
JMP loop
@f LOD 2
OUT 0
RET
