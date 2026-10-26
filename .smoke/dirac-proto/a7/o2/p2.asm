NOP
#PRNAME p2
#NUIOIN 1
#NUIOOU 1
#array b 1 2
#array x 1 2
#SHARE __aux_var main_a
@main LOD 1
SET main_a
LOD 2
SET main_c
LOD main_a
ADD main_c
SET __aux_var
LOD_V b 0
MLT __aux_var
SET_V x 0
LOD_V b 1
MLT __aux_var
SET_V x 1
@fim JMP fim
