NOP
#PRNAME dirac_shapes
#array __dirac_tmp 2 9
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#arrayd a 2 3 3 1.0 2.0 3.0
#arrayd e 2 3 3 4.0 5.0 6.0
#arrayd b 2 2 2 10.0 30.0
#arrayd B 2 6 6 1.0 2.0 3.0 4.0 5.0 7.0
#arrayd M 2 9 9 1.0 2.0 0.0 0.0 1.0 3.0 4.0 0.0 1.0
#arrayd P 2 9 9 0.0 1.0 0.0 0.0 0.0 1.0 1.0 0.0 0.0
#arrayd ia 1 3 3 1 2 3
#arrayd IM 1 9 9 2 0 1 1 3 0 0 1 2
#array A 2 6
#array C 2 9
#array v 2 3
#array y 2 2
#array iv 1 3
#array w 2 3
#SHARE k __dk0_main
#SHARE __dk4_main __dk1_main
#SHARE __dk2_main __dhf2_main
#SHARE x __dk1_main
#SHARE __dd9_main __dhf2_main
#SHARE __dk7_main __dk1_main
#SHARE __dk8_main __dk0_main
#SHARE __dhf13_main __dk0_main
#SHARE __dk10_main __dhf2_main
#SHARE __dd14_main __dhf2_main
#SHARE __dk11_main __dk1_main
#SHARE __dk12_main __dd3_main
#SHARE __dd2_main __dhf2_main
#SHARE __dhf1_main __dd3_main
#SHARE __dd6_main __dhf2_main
#SHARE __dk5_main __dk0_main
#SHARE __dhf9_main __dd3_main
#SHARE __dd10_main __dhf2_main
#SHARE __dhf3_main __dd3_main
#SHARE n __dhf2_main
#SHARE __aux_var __dk1_main
#SHARE __dd12_main __dk0_main
#SHARE __dhf18_main __dk1_main
#SHARE __dd19_main __dk0_main
#SHARE __dk13_main __dhf2_main
#SHARE __dk14_main __dk0_main
#SHARE __dd20_main __dk0_main
#SHARE __dk15_main __dhf2_main
#SHARE __dd21_main __dd3_main
#SHARE __dk16_main __dd4_main
#SHARE __dk17_main __dd3_main
#SHARE __dhf4_main __dhf2_main
#SHARE __dd7_main __dd3_main
#SHARE __dk6_main __dd4_main
#SHARE __dhf10_main __dd3_main
#SHARE __dd11_main __dd4_main
#SHARE tw_s __dk0_main
#SHARE __dhf1_tw __dhf2_main
#SHARE __dk0_tw __dd3_main
#SHARE __dd6_tw __dhf2_main
#SHARE __dk4_tw __dd3_main
#SHARE __dk5_tw __dd4_main
#SHARE __dhf9_tw __dhf2_main
#SHARE __dd10_tw __dd3_main
#SHARE __dk7_tw __dd4_main
#SHARE __dk8_tw __dk9_main
@main LOD -6
SET __dd3_main
LOD -3
SET __dk0_main
@__Ld0 LDI_V a 3
SET __dhf2_main
LOD -2
SET __dk1_main
@__Ld1 ADD __dd3_main
P_LOD __dk1_main
LDI_V b 2
F_MLT __dhf2_main
STI_V A 8
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld1e
JMP __Ld1
@__Ld1e LOD __dd3_main
ADD 2
SET __dd3_main
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld0e
JMP __Ld0
@__Ld0e LOD 0
SET k
@__Lwh1 LOD k
MLT 2
LDI A
F2I
OUT 0
LOD k
MLT 2
ADD 1
LDI A
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh1
@__Lwh1end LOD -3
SET __dd3_main
LOD -6
SET __dk0_main
@__Ld2 LOD __dd3_main
LDI_V a 3
SET __dhf2_main
LOD -2
SET __dk1_main
@__Ld3 ADD __dk0_main
PSH
LDI_V B 8
P_LOD __dk1_main
LDI_V b 2
F_MLT __dhf2_main
SF_SU2
STI_V A 8
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld3e
JMP __Ld3
@__Ld3e LOD __dd3_main
ADD 1
SET __dd3_main
LOD __dk0_main
ADD 2
SET __dk0_main
JIZ __Ld2e
JMP __Ld2
@__Ld2e LOD 0
SET k
@__Lwh2 LOD k
MLT 2
LDI A
F2I
OUT 0
LOD k
MLT 2
ADD 1
LDI A
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh2
@__Lwh2end LOD -3
SET __dd3_main
LOD -6
SET __dk0_main
@__Ld4 LOD __dd3_main
LDI_V a 3
F_MLT 2.0
SET __dhf2_main
LOD -2
SET __dk1_main
@__Ld5 ADD __dk0_main
PSH
LDI_V B 8
P_LOD __dk1_main
LDI_V b 2
F_MLT __dhf2_main
SF_ADD
STI_V A 8
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld5e
JMP __Ld5
@__Ld5e LOD __dd3_main
ADD 1
SET __dd3_main
LOD __dk0_main
ADD 2
SET __dk0_main
JIZ __Ld4e
JMP __Ld4
@__Ld4e LOD 0
SET k
@__Lwh3 LOD k
MLT 2
LDI A
F2I
OUT 0
LOD k
MLT 2
ADD 1
LDI A
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh3
@__Lwh3end LOD -3
SET __dk0_main
@__Ld6 PSH
LDI_V a 3
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld6e
JMP __Ld6
@__Ld6e LOD_V v 0
SET __dhf2_main
LOD -3
SET __dd3_main
LOD -9
SET __dk0_main
@__Ld7 LOD __dd3_main
P_LOD __dk0_main
LDI_V P 9
F_MLT __dhf2_main
P_LOD -2
SET __dk1_main
@__Ld8 ADD __dk0_main
LDI_V P 12
P_LOD __dk1_main
LDI_V v 3
SF_MLT
SF_ADD
P_LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld8e
JMP __Ld8
@__Ld8e POP
STI_V __dirac_tmp 3
LOD __dd3_main
ADD 1
SET __dd3_main
LOD __dk0_main
ADD 3
SET __dk0_main
JIZ __Ld7e
JMP __Ld7
@__Ld7e LOD -3
SET __dk4_main
@__Ld9 PSH
LDI_V __dirac_tmp 3
STI_V v 3
LOD __dk4_main
ADD 1
SET __dk4_main
JIZ __Ld9e
JMP __Ld9
@__Ld9e LOD 0
SET k
@__Lwh4 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh4
@__Lwh4end LOD -3
SET __dk0_main
@__Ld10 PSH
LDI_V a 3
P_LOD __dk0_main
LDI_V e 3
SF_ADD
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld10e
JMP __Ld10
@__Ld10e LOD 0
SET k
@__Lwh5 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh5
@__Lwh5end LOD -3
SET __dk0_main
@__Ld11 PSH
LDI_V a 3
P_LOD __dk0_main
LDI_V e 3
SF_SU2
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld11e
JMP __Ld11
@__Ld11e LOD 0
SET k
@__Lwh6 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh6
@__Lwh6end LOD -3
SET __dk0_main
@__Ld12 PSH
LDI_V a 3
F_NEG
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld12e
JMP __Ld12
@__Ld12e LOD 0
SET k
@__Lwh7 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh7
@__Lwh7end LOD -3
SET __dk0_main
@__Ld13 PSH
LDI_V a 3
P_LOD __dk0_main
LDI_V e 3
F_MLT 2.0
SF_SU2
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld13e
JMP __Ld13
@__Ld13e LOD 0
SET k
@__Lwh8 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh8
@__Lwh8end LOD -3
SET __dk0_main
@__Ld14 PSH
LDI_V a 3
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld14e
JMP __Ld14
@__Ld14e LOD -3
SET __dk0_main
@__Ld15 PSH
LDI_V v 3
P_LOD __dk0_main
LDI_V e 3
F_MLT 2.0
SF_ADD
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld15e
JMP __Ld15
@__Ld15e LOD 0
SET k
@__Lwh9 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh9
@__Lwh9end LOD M
F_MLT e
P_LOD -2
SET __dk0_main
@__Ld16 LDI_V M 3
P_LOD __dk0_main
LDI_V e 3
SF_MLT
SF_ADD
P_LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld16e
JMP __Ld16
@__Ld16e POP
F_MLT a
P_LOD -2
SET __dd3_main
LOD -6
SET __dk1_main
@__Ld17 LOD __dd3_main
LDI_V a 3
P_LOD __dk1_main
LDI_V M 9
F_MLT e
P_LOD -2
SET __dk2_main
@__Ld18 ADD __dk1_main
LDI_V M 12
P_LOD __dk2_main
LDI_V e 3
SF_MLT
SF_ADD
P_LOD __dk2_main
ADD 1
SET __dk2_main
JIZ __Ld18e
JMP __Ld18
@__Ld18e POP
SF_MLT
SF_ADD
P_LOD __dd3_main
ADD 1
SET __dd3_main
LOD __dk1_main
ADD 3
SET __dk1_main
JIZ __Ld17e
JMP __Ld17
@__Ld17e POP
SET x
F2I_M x
OUT 0
LOD a
F_MLT e
P_LOD -2
SET __dk0_main
@__Ld19 LDI_V a 3
P_LOD __dk0_main
LDI_V e 3
SF_MLT
SF_ADD
P_LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld19e
JMP __Ld19
@__Ld19e POP
F_MLT 2.0
F_ADD 1.0
SET x
F2I_M x
OUT 0
LOD -3
SET __dd9_main
LOD -9
SET __dk7_main
@__Ld20 LOD __dd9_main
P_LOD __dk7_main
LDI_V M 9
F_MLT e
P_LOD -2
SET __dk8_main
@__Ld21 ADD __dk7_main
LDI_V M 12
P_LOD __dk8_main
LDI_V e 3
SF_MLT
SF_ADD
P_LOD __dk8_main
ADD 1
SET __dk8_main
JIZ __Ld21e
JMP __Ld21
@__Ld21e POP
STI_V __dirac_tmp 3
LOD __dd9_main
ADD 1
SET __dd9_main
LOD __dk7_main
ADD 3
SET __dk7_main
JIZ __Ld20e
JMP __Ld20
@__Ld20e LOD_V __dirac_tmp 0
SET __dhf13_main
LOD_V __dirac_tmp 0
F_MLT M
P_LOD -2
SET __dk10_main
@__Ld22 LDI_V M 3
P_LOD __dk10_main
LDI_V __dirac_tmp 3
SF_MLT
SF_ADD
P_LOD __dk10_main
ADD 1
SET __dk10_main
JIZ __Ld22e
JMP __Ld22
@__Ld22e POP
F_MLT a
P_LOD -2
SET __dd14_main
LOD -6
SET __dk11_main
@__Ld23 LOD __dd14_main
LDI_V a 3
P_LOD __dk11_main
LDI_V M 9
F_MLT __dhf13_main
P_LOD -2
SET __dk12_main
@__Ld24 ADD __dk11_main
LDI_V M 12
P_LOD __dk12_main
LDI_V __dirac_tmp 3
SF_MLT
SF_ADD
P_LOD __dk12_main
ADD 1
SET __dk12_main
JIZ __Ld24e
JMP __Ld24
@__Ld24e POP
SF_MLT
SF_ADD
P_LOD __dd14_main
ADD 1
SET __dd14_main
LOD __dk11_main
ADD 3
SET __dk11_main
JIZ __Ld23e
JMP __Ld23
@__Ld23e POP
SET x
F2I_M x
OUT 0
LOD -2
SET __dk0_main
@__Ld25 PSH
LDI_V B 2
F_MLT a
P_LOD -4
SET __dd2_main
LOD -2
SET __dk1_main
@__Ld26 LDI_V a 3
P_LOD __dd2_main
ADD __dk0_main
LDI_V B 8
SF_MLT
SF_ADD
P_LOD __dd2_main
ADD 2
SET __dd2_main
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld26e
JMP __Ld26
@__Ld26e POP
STI_V y 2
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld25e
JMP __Ld25
@__Ld25e LOD_V y 0
F2I
OUT 0
LOD_V y 1
F2I
OUT 0
LOD -3
SET __dk0_main
@__Ld27 PSH
LDI_V M 3
F_MLT a
P_LOD -6
SET __dd2_main
LOD -2
SET __dk1_main
@__Ld28 LDI_V a 3
P_LOD __dd2_main
ADD __dk0_main
LDI_V M 12
SF_MLT
SF_ADD
P_LOD __dd2_main
ADD 3
SET __dd2_main
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld28e
JMP __Ld28
@__Ld28e POP
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld27e
JMP __Ld27
@__Ld27e LOD 0
SET k
@__Lwh10 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh10
@__Lwh10end LOD_V b 1
SET __dhf1_main
LOD -3
SET __dd2_main
LOD -6
SET __dk0_main
@__Ld29 LOD __dd2_main
P_LOD __dk0_main
LDI_V B 6
F_MLT b
P_LOD __dk0_main
LDI_V B 7
F_MLT __dhf1_main
SF_ADD
STI_V v 3
LOD __dd2_main
ADD 1
SET __dd2_main
LOD __dk0_main
ADD 2
SET __dk0_main
JIZ __Ld29e
JMP __Ld29
@__Ld29e LOD 0
SET k
@__Lwh11 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh11
@__Lwh11end LOD -3
SET __dd2_main
LOD -9
SET __dk0_main
@__Ld30 LOD __dd2_main
P_LOD __dk0_main
LDI_V M 9
F_MLT a
P_LOD -2
SET __dk1_main
@__Ld31 ADD __dk0_main
LDI_V M 12
P_LOD __dk1_main
LDI_V a 3
SF_MLT
SF_ADD
P_LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld31e
JMP __Ld31
@__Ld31e POP
P_LOD __dd2_main
LDI_V e 3
SF_ADD
STI_V v 3
LOD __dd2_main
ADD 1
SET __dd2_main
LOD __dk0_main
ADD 3
SET __dk0_main
JIZ __Ld30e
JMP __Ld30
@__Ld30e LOD 0
SET k
@__Lwh12 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh12
@__Lwh12end LOD -3
SET __dd6_main
LOD -9
SET __dk4_main
@__Ld32 LOD __dd6_main
P_LOD __dk4_main
LDI_V M 9
F_MLT a
P_LOD -2
SET __dk5_main
@__Ld33 ADD __dk4_main
LDI_V M 12
P_LOD __dk5_main
LDI_V a 3
SF_MLT
SF_ADD
P_LOD __dk5_main
ADD 1
SET __dk5_main
JIZ __Ld33e
JMP __Ld33
@__Ld33e POP
STI_V __dirac_tmp 3
LOD __dd6_main
ADD 1
SET __dd6_main
LOD __dk4_main
ADD 3
SET __dk4_main
JIZ __Ld32e
JMP __Ld32
@__Ld32e LOD_V __dirac_tmp 0
SET __dhf9_main
LOD -3
SET __dd10_main
LOD -9
SET __dk7_main
@__Ld34 LOD __dd10_main
P_LOD __dk7_main
LDI_V M 9
F_MLT __dhf9_main
P_LOD -2
SET __dk8_main
@__Ld35 ADD __dk7_main
LDI_V M 12
P_LOD __dk8_main
LDI_V __dirac_tmp 3
SF_MLT
SF_ADD
P_LOD __dk8_main
ADD 1
SET __dk8_main
JIZ __Ld35e
JMP __Ld35
@__Ld35e POP
STI_V v 3
LOD __dd10_main
ADD 1
SET __dd10_main
LOD __dk7_main
ADD 3
SET __dk7_main
JIZ __Ld34e
JMP __Ld34
@__Ld34e LOD 0
SET k
@__Lwh13 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh13
@__Lwh13end LOD -3
SET __dd6_main
LOD -9
SET __dk4_main
@__Ld36 LOD __dd6_main
P_LOD __dk4_main
LDI_V P 9
F_MLT a
P_LOD -2
SET __dk5_main
@__Ld37 ADD __dk4_main
LDI_V P 12
P_LOD __dk5_main
LDI_V a 3
SF_MLT
SF_ADD
P_LOD __dk5_main
ADD 1
SET __dk5_main
JIZ __Ld37e
JMP __Ld37
@__Ld37e POP
STI_V __dirac_tmp 3
LOD __dd6_main
ADD 1
SET __dd6_main
LOD __dk4_main
ADD 3
SET __dk4_main
JIZ __Ld36e
JMP __Ld36
@__Ld36e LOD_V __dirac_tmp 0
SET __dhf9_main
LOD -3
SET __dd10_main
LOD -9
SET __dk7_main
@__Ld38 LOD __dd10_main
P_LOD __dk7_main
LDI_V M 9
F_MLT __dhf9_main
P_LOD -2
SET __dk8_main
@__Ld39 ADD __dk7_main
LDI_V M 12
P_LOD __dk8_main
LDI_V __dirac_tmp 3
SF_MLT
SF_ADD
P_LOD __dk8_main
ADD 1
SET __dk8_main
JIZ __Ld39e
JMP __Ld39
@__Ld39e POP
STI_V v 3
LOD __dd10_main
ADD 1
SET __dd10_main
LOD __dk7_main
ADD 3
SET __dk7_main
JIZ __Ld38e
JMP __Ld38
@__Ld38e LOD 0
SET k
@__Lwh14 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh14
@__Lwh14end LOD -9
SET __dk0_main
@__Ld40 LDI_V M 9
SET __dhf3_main
LOD -3
SET __dk1_main
@__Ld41 ADD __dk0_main
P_LOD __dk1_main
LDI_V P 3
F_MLT __dhf3_main
P_LOD -6
SET __dd4_main
LOD -2
SET __dk2_main
@__Ld42 ADD __dk0_main
LDI_V M 12
P_LOD __dd4_main
ADD __dk1_main
LDI_V P 12
SF_MLT
SF_ADD
P_LOD __dd4_main
ADD 3
SET __dd4_main
LOD __dk2_main
ADD 1
SET __dk2_main
JIZ __Ld42e
JMP __Ld42
@__Ld42e POP
STI_V C 12
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld41e
JMP __Ld41
@__Ld41e LOD __dk0_main
ADD 3
SET __dk0_main
JIZ __Ld40e
JMP __Ld40
@__Ld40e LOD 0
SET k
@__Lwh15 LOD k
MLT 3
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 1
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 2
LDI C
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh15
@__Lwh15end LOD -9
SET __dk0_main
@__Ld43 PSH
LDI_V M 9
P_LOD __dk0_main
LDI_V P 9
F_MLT 2.0
SF_SU2
STI_V C 9
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld43e
JMP __Ld43
@__Ld43e LOD 0
SET k
@__Lwh16 LOD k
MLT 3
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 1
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 2
LDI C
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh16
@__Lwh16end LOD -9
SET __dk0_main
@__Ld44 P_LOD 0.0
STI_V C 9
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld44e
JMP __Ld44
@__Ld44e LOD -12
SET __dk1_main
@__Ld45 P_LOD 3.0
STI_V C 12
LOD __dk1_main
ADD 4
SET __dk1_main
JIZ __Ld45e
JMP __Ld45
@__Ld45e LOD 0
SET k
@__Lwh17 LOD k
MLT 3
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 1
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 2
LDI C
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh17
@__Lwh17end LOD -6
SET __dk0_main
@__Ld46 P_LOD 0.0
STI_V A 6
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld46e
JMP __Ld46
@__Ld46e LOD -6
SET __dk1_main
@__Ld47 P_LOD 2.0
STI_V A 6
LOD __dk1_main
ADD 3
SET __dk1_main
JIZ __Ld47e
JMP __Ld47
@__Ld47e LOD 0
SET k
@__Lwh18 LOD k
MLT 2
LDI A
F2I
OUT 0
LOD k
MLT 2
ADD 1
LDI A
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh18
@__Lwh18end LOD -3
SET __dd3_main
LOD -9
SET __dk0_main
@__Ld48 LOD __dd3_main
LDI_V a 3
SET __dhf2_main
LOD -3
SET __dk1_main
@__Ld49 ADD __dk0_main
P_LOD __dk1_main
LDI_V e 3
F_MLT __dhf2_main
P_LOD __dk1_main
ADD __dk0_main
LDI_V M 12
SF_ADD
STI_V C 12
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld49e
JMP __Ld49
@__Ld49e LOD __dd3_main
ADD 1
SET __dd3_main
LOD __dk0_main
ADD 3
SET __dk0_main
JIZ __Ld48e
JMP __Ld48
@__Ld48e LOD 0
SET k
@__Lwh19 LOD k
MLT 3
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 1
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 2
LDI C
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh19
@__Lwh19end LOD -3
SET __dd2_main
LOD -9
SET __dk0_main
@__Ld50 LOD __dd2_main
P_LOD __dk0_main
LDI_V IM 9
MLT ia
P_LOD -2
SET __dk1_main
@__Ld51 ADD __dk0_main
LDI_V IM 12
P_LOD __dk1_main
LDI_V ia 3
S_MLT
S_ADD
P_LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld51e
JMP __Ld51
@__Ld51e POP
STI_V iv 3
LOD __dd2_main
ADD 1
SET __dd2_main
LOD __dk0_main
ADD 3
SET __dk0_main
JIZ __Ld50e
JMP __Ld50
@__Ld50e LOD 0
SET k
@__Lwh20 LOD k
LDI iv
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh20
@__Lwh20end LOD 5
SET n
LOD -3
SET __dk0_main
@__Ld52 PSH
LDI_V ia 3
MLT n
I2F
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld52e
JMP __Ld52
@__Ld52e LOD 0
SET k
@__Lwh21 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh21
@__Lwh21end I2F_M n
SET __dhf1_main
LOD -3
SET __dk0_main
@__Ld53 PSH
LDI_V a 3
F_MLT __dhf1_main
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld53e
JMP __Ld53
@__Ld53e LOD 0
SET k
@__Lwh22 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh22
@__Lwh22end I2F_M ia
F_MLT e
P_LOD -2
SET __dk0_main
@__Ld54 LDI_V ia 3
P_LOD __dk0_main
LDI_V e 3
SET_P __aux_var
I2F
F_MLT __aux_var
SF_ADD
P_LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld54e
JMP __Ld54
@__Ld54e POP
SET x
F2I_M x
OUT 0
LOD -3
SET __dk0_main
@__Ld55 PSH
LDI_V ia 3
STI_V iv 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld55e
JMP __Ld55
@__Ld55e LOD_V iv 1
SET_V iv 2
LOD_V iv 0
SET_V iv 1
LOD n
ADD 1
SET iv
LOD 0
SET k
@__Lwh23 LOD k
LDI iv
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh23
@__Lwh23end LOD -3
SET __dk0_main
@__Ld56 PF_INN 0
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld56e
JMP __Ld56
@__Ld56e LOD 0
SET k
@__Lwh24 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh24
@__Lwh24end LOD -3
SET __dk0_main
@__Ld57 PF_INN 0
F_MLT 2.0
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld57e
JMP __Ld57
@__Ld57e LOD 0
SET k
@__Lwh25 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh25
@__Lwh25end LOD -3
SET __dk0_main
@__Ld58 P_LOD 0.0
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld58e
JMP __Ld58
@__Ld58e LOD 0
SET k
@__Lwh26 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh26
@__Lwh26end LOD -9
SET __dk0_main
@__Ld59 PSH
LDI_V P 9
STI_V C 9
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld59e
JMP __Ld59
@__Ld59e LOD -9
SET __dk0_main
@__Ld60 LDI_V M 9
SET __dhf3_main
LOD -3
SET __dk1_main
@__Ld61 ADD __dk0_main
P_LOD __dk1_main
LDI_V C 3
F_MLT __dhf3_main
P_LOD -6
SET __dd4_main
LOD -2
SET __dk2_main
@__Ld62 ADD __dk0_main
LDI_V M 12
P_LOD __dd4_main
ADD __dk1_main
LDI_V C 12
SF_MLT
SF_ADD
P_LOD __dd4_main
ADD 3
SET __dd4_main
LOD __dk2_main
ADD 1
SET __dk2_main
JIZ __Ld62e
JMP __Ld62
@__Ld62e POP
STI_V __dirac_tmp 12
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld61e
JMP __Ld61
@__Ld61e LOD __dk0_main
ADD 3
SET __dk0_main
JIZ __Ld60e
JMP __Ld60
@__Ld60e LOD -9
SET __dk5_main
@__Ld63 PSH
LDI_V __dirac_tmp 9
STI_V C 9
LOD __dk5_main
ADD 1
SET __dk5_main
JIZ __Ld63e
JMP __Ld63
@__Ld63e LOD 0
SET k
@__Lwh27 LOD k
MLT 3
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 1
LDI C
F2I
OUT 0
LOD k
MLT 3
ADD 2
LDI C
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh27
@__Lwh27end LOD -3
SET __dk0_main
@__Ld64 PSH
LDI_V a 3
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld64e
JMP __Ld64
@__Ld64e LOD_V v 0
SET __dhf2_main
LOD -3
SET __dk0_main
@__Ld65 PSH
LDI_V M 3
F_MLT __dhf2_main
P_LOD -6
SET __dd3_main
LOD -2
SET __dk1_main
@__Ld66 LDI_V v 3
P_LOD __dd3_main
ADD __dk0_main
LDI_V M 12
SF_MLT
SF_ADD
P_LOD __dd3_main
ADD 3
SET __dd3_main
LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld66e
JMP __Ld66
@__Ld66e POP
STI_V __dirac_tmp 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld65e
JMP __Ld65
@__Ld65e LOD -3
SET __dk4_main
@__Ld67 PSH
LDI_V __dirac_tmp 3
STI_V v 3
LOD __dk4_main
ADD 1
SET __dk4_main
JIZ __Ld67e
JMP __Ld67
@__Ld67e LOD 0
SET k
@__Lwh28 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh28
@__Lwh28end LOD 2.0
SET x
LOD -3
SET __dk0_main
@__Ld68 PSH
LDI_V ia 3
I2F
F_MLT x
F2I
STI_V iv 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld68e
JMP __Ld68
@__Ld68e LOD 0
SET k
@__Lwh29 LOD k
LDI iv
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh29
@__Lwh29end LOD -3
SET __dk0_main
@__Ld69 LDI_V ia 3
OUT 0
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld69e
JMP __Ld69
@__Ld69e LOD -3
SET __dk0_main
@__Ld70 LDI_V ia 3
OUT 0
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld70e
JMP __Ld70
@__Ld70e LOD -3
SET __dk0_main
@__Ld71 LDI_V a 3
F_MLT 2.0
F2I
OUT 0
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld71e
JMP __Ld71
@__Ld71e LOD -9
SET __dk0_main
@__Ld72 LDI_V IM 9
MLT ia
P_LOD -2
SET __dk1_main
@__Ld73 ADD __dk0_main
LDI_V IM 12
P_LOD __dk1_main
LDI_V ia 3
S_MLT
S_ADD
P_LOD __dk1_main
ADD 1
SET __dk1_main
JIZ __Ld73e
JMP __Ld73
@__Ld73e POP
OUT 0
LOD __dk0_main
ADD 3
SET __dk0_main
JIZ __Ld72e
JMP __Ld72
@__Ld72e LOD -3
SET __dd6_main
LOD -9
SET __dk4_main
@__Ld74 LOD __dd6_main
P_LOD __dk4_main
LDI_V M 9
F_MLT a
P_LOD -2
SET __dk5_main
@__Ld75 ADD __dk4_main
LDI_V M 12
P_LOD __dk5_main
LDI_V a 3
SF_MLT
SF_ADD
P_LOD __dk5_main
ADD 1
SET __dk5_main
JIZ __Ld75e
JMP __Ld75
@__Ld75e POP
STI_V __dirac_tmp 3
LOD __dd6_main
ADD 1
SET __dd6_main
LOD __dk4_main
ADD 3
SET __dk4_main
JIZ __Ld74e
JMP __Ld74
@__Ld74e LOD_V __dirac_tmp 0
SET __dhf9_main
LOD -9
SET __dk7_main
@__Ld76 LDI_V M 9
F_MLT __dhf9_main
P_LOD -2
SET __dk8_main
@__Ld77 ADD __dk7_main
LDI_V M 12
P_LOD __dk8_main
LDI_V __dirac_tmp 3
SF_MLT
SF_ADD
P_LOD __dk8_main
ADD 1
SET __dk8_main
JIZ __Ld77e
JMP __Ld77
@__Ld77e POP
F2I
OUT 0
LOD __dk7_main
ADD 3
SET __dk7_main
JIZ __Ld76e
JMP __Ld76
@__Ld76e LOD 3.0
SET x
F_MLT 3.0
SET __dhf9_main
LOD -3
SET __dk10_main
@__Ld78 PSH
LDI_V M 3
F_MLT a
P_LOD -6
SET __dd12_main
LOD -2
SET __dk11_main
@__Ld79 LDI_V a 3
P_LOD __dd12_main
ADD __dk10_main
LDI_V M 12
SF_MLT
SF_ADD
P_LOD __dd12_main
ADD 3
SET __dd12_main
LOD __dk11_main
ADD 1
SET __dk11_main
JIZ __Ld79e
JMP __Ld79
@__Ld79e POP
STI_V __dirac_tmp 3
LOD __dk10_main
ADD 1
SET __dk10_main
JIZ __Ld78e
JMP __Ld78
@__Ld78e LOD_V __dirac_tmp 0
SET __dhf18_main
LOD __dhf9_main
P_LOD_V __dirac_tmp 0
F_MLT M
P_LOD -6
SET __dd19_main
LOD -2
SET __dk13_main
@__Ld80 LDI_V __dirac_tmp 3
P_LOD __dd19_main
LDI_V M 9
SF_MLT
SF_ADD
P_LOD __dd19_main
ADD 3
SET __dd19_main
LOD __dk13_main
ADD 1
SET __dk13_main
JIZ __Ld80e
JMP __Ld80
@__Ld80e POP
P_LOD M
F_MLT e
P_LOD -2
SET __dk14_main
@__Ld81 LDI_V M 3
P_LOD __dk14_main
LDI_V e 3
SF_MLT
SF_ADD
P_LOD __dk14_main
ADD 1
SET __dk14_main
JIZ __Ld81e
JMP __Ld81
@__Ld81e POP
SF_MLT
P_LOD -6
SET __dd20_main
LOD -2
SET __dk15_main
@__Ld82 LDI_V M 3
F_MLT __dhf18_main
P_LOD -6
SET __dd21_main
LOD -2
SET __dk16_main
@__Ld83 LDI_V __dirac_tmp 3
P_LOD __dd21_main
ADD __dk15_main
LDI_V M 12
SF_MLT
SF_ADD
P_LOD __dd21_main
ADD 3
SET __dd21_main
LOD __dk16_main
ADD 1
SET __dk16_main
JIZ __Ld83e
JMP __Ld83
@__Ld83e POP
P_LOD __dd20_main
LDI_V M 9
F_MLT e
P_LOD -2
SET __dk17_main
@__Ld84 ADD __dd20_main
LDI_V M 12
P_LOD __dk17_main
LDI_V e 3
SF_MLT
SF_ADD
P_LOD __dk17_main
ADD 1
SET __dk17_main
JIZ __Ld84e
JMP __Ld84
@__Ld84e POP
SF_MLT
SF_ADD
P_LOD __dd20_main
ADD 3
SET __dd20_main
LOD __dk15_main
ADD 1
SET __dk15_main
JIZ __Ld82e
JMP __Ld82
@__Ld82e POP
SF_ADD
SET x
F2I_M x
OUT 0
LOD 2.0
CAL tw
SET __dhf4_main
LOD -3
SET __dd7_main
LOD -9
SET __dk5_main
@__Ld85 LOD __dd7_main
P_LOD __dk5_main
LDI_V M 9
F_MLT a
P_LOD -2
SET __dk6_main
@__Ld86 ADD __dk5_main
LDI_V M 12
P_LOD __dk6_main
LDI_V a 3
SF_MLT
SF_ADD
P_LOD __dk6_main
ADD 1
SET __dk6_main
JIZ __Ld86e
JMP __Ld86
@__Ld86e POP
STI_V __dirac_tmp 3
LOD __dd7_main
ADD 1
SET __dd7_main
LOD __dk5_main
ADD 3
SET __dk5_main
JIZ __Ld85e
JMP __Ld85
@__Ld85e LOD_V __dirac_tmp 0
SET __dhf10_main
LOD -3
SET __dd11_main
LOD -9
SET __dk8_main
@__Ld87 LOD __dd11_main
P_LOD __dk8_main
LDI_V M 9
F_MLT __dhf10_main
P_LOD -2
SET __dk9_main
@__Ld88 ADD __dk8_main
LDI_V M 12
P_LOD __dk9_main
LDI_V __dirac_tmp 3
SF_MLT
SF_ADD
P_LOD __dk9_main
ADD 1
SET __dk9_main
JIZ __Ld88e
JMP __Ld88
@__Ld88e POP
P_LOD __dd11_main
LDI_V e 3
F_MLT __dhf4_main
SF_ADD
STI_V v 3
LOD __dd11_main
ADD 1
SET __dd11_main
LOD __dk8_main
ADD 3
SET __dk8_main
JIZ __Ld87e
JMP __Ld87
@__Ld87e LOD 0
SET k
@__Lwh30 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh30
@__Lwh30end LOD 1.0
CAL tw
SET __dhf1_main
LOD x
F_SU1 1.0
SET __dhf2_main
LOD -3
SET __dk0_main
@__Ld89 PSH
LDI_V a 3
F_MLT __dhf1_main
P_LOD __dk0_main
LDI_V e 3
F_MLT __dhf2_main
SF_ADD
STI_V v 3
LOD __dk0_main
ADD 1
SET __dk0_main
JIZ __Ld89e
JMP __Ld89
@__Ld89e LOD 0
SET k
@__Lwh31 LOD k
LDI v
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh31
@__Lwh31end LOD 0
SET k
@__Lwh32 LOD k
LDI w
F2I
OUT 0
LOD k
ADD 1
SET k
LES 2
JIZ __Lwh32
@__Lwh32end @fim JMP fim
@tw SET tw_s
F_ADD 1.0
SET __dhf1_tw
LOD -3
SET __dk0_tw
@__Ld90 PSH
LDI_V e 3
F_MLT __dhf1_tw
STI_V w 3
LOD __dk0_tw
ADD 1
SET __dk0_tw
JIZ __Ld90e
JMP __Ld90
@__Ld90e LOD -3
SET __dd6_tw
LOD -9
SET __dk4_tw
@__Ld91 LOD __dd6_tw
P_LOD __dk4_tw
LDI_V M 9
F_MLT w
P_LOD -2
SET __dk5_tw
@__Ld92 ADD __dk4_tw
LDI_V M 12
P_LOD __dk5_tw
LDI_V w 3
SF_MLT
SF_ADD
P_LOD __dk5_tw
ADD 1
SET __dk5_tw
JIZ __Ld92e
JMP __Ld92
@__Ld92e POP
STI_V __dirac_tmp 3
LOD __dd6_tw
ADD 1
SET __dd6_tw
LOD __dk4_tw
ADD 3
SET __dk4_tw
JIZ __Ld91e
JMP __Ld91
@__Ld91e LOD_V __dirac_tmp 0
SET __dhf9_tw
LOD -3
SET __dd10_tw
LOD -9
SET __dk7_tw
@__Ld93 LOD __dd10_tw
P_LOD __dk7_tw
LDI_V M 9
F_MLT __dhf9_tw
P_LOD -2
SET __dk8_tw
@__Ld94 ADD __dk7_tw
LDI_V M 12
P_LOD __dk8_tw
LDI_V __dirac_tmp 3
SF_MLT
SF_ADD
P_LOD __dk8_tw
ADD 1
SET __dk8_tw
JIZ __Ld94e
JMP __Ld94
@__Ld94e POP
STI_V w 3
LOD __dd10_tw
ADD 1
SET __dd10_tw
LOD __dk7_tw
ADD 3
SET __dk7_tw
JIZ __Ld93e
JMP __Ld93
@__Ld93e LOD tw_s
F_ADD 1.0
RET
