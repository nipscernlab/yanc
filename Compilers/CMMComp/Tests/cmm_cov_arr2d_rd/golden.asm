NOP
#PRNAME cmm_cov_arr2d_rd
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#arrayd mi 1 12 12 1 2 3 4 11 12 13 14 21 22 23 24
#arrayd mf 2 12 12 100.0 101.0 102.0 103.0 110.0 111.0 112.0 113.0 120.0 121.0 122.0 123.0
#arrayd mc 3 12 12 0.0 1.0 2.0 3.0 10.0 11.0 12.0 13.0 20.0 21.0 22.0 23.0
#arrayd mc_i 4 12 12 -1.0 -2.0 -3.0 -4.0 -11.0 -12.0 -13.0 -14.0 -21.0 -22.0 -23.0 -24.0
#SHARE main_n1 main_n0
#SHARE main_f1 main_f0
#SHARE main_c0_i main_c0
#SHARE main_c1 main_c0
#SHARE main_one main_c3_i
#SHARE main_y main_x
#SHARE __aux_var main_x
#SHARE main_w_i main_x
@main LOD 0
SET main_n0
LOD 1
SET main_n1
LOD 2
SET main_n2
LOD 3
SET main_n3
LOD 0
SET main_z0
LOD 0.7
SET main_f0
LOD 1.7
SET main_f1
LOD 2.7
SET main_f2
LOD 3.7
SET main_f3
LOD 0.0
SET main_fz
SET main_c0
LOD 3.0
SET main_c0_i
LOD 1.0
SET main_c1
LOD 4.0
SET main_c1_i
LOD 2.0
SET main_c2
LOD 5.0
SET main_c2_i
LOD 3.0
SET main_c3
LOD 6.0
SET main_c3_i
LOD 1.0
SET main_one
LOD 0.0
SET main_one_i
LOD main_n1
MLT 4
ADD main_n2
LDI mi
SET main_x
OUT 0
LOD main_n1
MLT 4
ADD main_n2
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD   main_n1
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
LDI mi
SET main_x
OUT 0
LOD main_n2
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
SET   __aux_var
LDI mc
P_LOD __aux_var
LDI mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
MLT 4
ADD 1
LDI mi
SET main_x
OUT 0
LOD main_n2
MLT 4
ADD 1
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD   main_n2
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD main_n1
MLT   4
S_ADD
LDI   mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD main_n1
MLT   4
S_ADD
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD main_n1
MLT   4
S_ADD
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     main_n2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
SET main_x
OUT 0
LOD     main_n2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD     main_n2
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   main_n2
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   main_n2
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   main_n2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
ADD main_z0
MLT 4
ADD main_n2
LDI mi
SET main_x
OUT 0
LOD main_n1
ADD main_z0
MLT 4
ADD main_n2
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n1
ADD main_z0
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
ADD main_z0
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
LDI mi
SET main_x
OUT 0
LOD main_n2
ADD main_z0
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
ADD main_z0
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
SET   __aux_var
LDI mc
P_LOD __aux_var
LDI mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
ADD main_z0
MLT 4
ADD 1
LDI mi
SET main_x
OUT 0
LOD main_n2
ADD main_z0
MLT 4
ADD 1
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
ADD main_z0
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
ADD main_z0
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_n1
ADD main_z0
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n1
ADD main_z0
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
ADD main_z0
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_n2
ADD main_z0
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
ADD main_z0
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
ADD main_z0
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_n2
ADD main_z0
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
ADD main_z0
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD 1
MLT 4
ADD main_n2
LDI mi
SET main_x
OUT 0
LOD 1
MLT 4
ADD main_n2
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD   1
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n3
ADD main_z0
ADD 8
LDI mi
SET main_x
OUT 0
LOD main_n3
ADD main_z0
ADD 8
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n3
ADD main_z0
ADD 8
SET   __aux_var
LDI mc
P_LOD __aux_var
LDI mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD_V mi 9
SET main_x
OUT 0
LOD_V mf 9
SET main_y
F2I_M main_y
OUT 0
LOD_V mc 9
P_LOD_V mc_i 9
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD     1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD     1
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD 1
MLT   4
S_ADD
LDI   mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD 1
MLT   4
S_ADD
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD 1
MLT   4
S_ADD
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
SET main_x
OUT 0
LOD     2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD     2
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   2
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   2
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD     1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD     1
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_f1
MLT 4
ADD main_n2
LDI mi
SET main_x
OUT 0
F2I_M main_f1
MLT 4
ADD main_n2
LDI mf
SET main_y
F2I_M main_y
OUT 0
F2I_M    main_f1
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
F2I_M main_f2
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
F2I_M main_f2
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
F2I_M main_f2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_f2
MLT 4
ADD 1
LDI mi
SET main_x
OUT 0
F2I_M main_f2
MLT 4
ADD 1
LDI mf
SET main_y
F2I_M main_y
OUT 0
F2I_M    main_f2
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI      mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M main_f1
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M main_f1
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M main_f1
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     main_f2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M     main_f2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
F2I_M main_f2
MLT 4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_f2 
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_f2 
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_f2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT 4
ADD main_n2
LDI mi
SET main_x
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT 4
ADD main_n2
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_n3
ADD main_z0
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_n3
ADD main_z0
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_n3
ADD main_z0
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT 4
ADD 1
LDI mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT 4
ADD 1
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f1
F_ADD main_fz
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_f1
F_ADD main_fz
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f1
F_ADD main_fz
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT 4
P_F2I_M 3.0
S_ADD
LDI mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT 4
P_F2I_M 3.0
S_ADD
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M 1.0
MLT 4
ADD main_n2
LDI mi
SET main_x
OUT 0
F2I_M 1.0
MLT 4
ADD main_n2
LDI mf
SET main_y
F2I_M main_y
OUT 0
F2I_M   1.0
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n3
ADD main_z0
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_n3
ADD main_z0
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n3
ADD main_z0
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M 2.0
MLT 4
ADD 1
LDI mi
SET main_x
OUT 0
F2I_M 2.0
MLT 4
ADD 1
LDI mf
SET main_y
F2I_M main_y
OUT 0
F2I_M   2.0
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M 1.0
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M 1.0
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M 1.0
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M 2.0
MLT 4
P_F2I_M 3.0
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M 2.0
MLT 4
P_F2I_M 3.0
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
F2I_M 2.0
MLT 4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
F2I_M      1.0
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT 4
ADD main_n2
LDI mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT 4
ADD main_n2
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_n3
ADD main_z0
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_n3
ADD main_z0
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_n3
ADD main_z0
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
POP
F2I
MLT 4
ADD 1
LDI mi
SET main_x
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
POP
F2I
MLT 4
ADD 1
LDI mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P   __aux_var
F2I
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
POP
F2I
MLT  4
ADD  __aux_var
LDI  mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
POP
F2I
MLT  4
ADD  __aux_var
LDI  mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI    mc
P_LOD __aux_var
LDI    mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
SET_P   __aux_var
F2I
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P   __aux_var
F2I
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_c1
MLT 4
ADD main_n2
LDI mi
SET main_x
OUT 0
F2I_M main_c1
MLT 4
ADD main_n2
LDI mf
SET main_y
F2I_M main_y
OUT 0
F2I_M    main_c1
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n3
ADD main_z0
P_F2I_M main_c2
MLT     4
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_n3
ADD main_z0
P_F2I_M main_c2
MLT     4
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_n3
ADD main_z0
P_F2I_M main_c2
MLT     4
S_ADD
SET     __aux_var
LDI      mc
P_LOD   __aux_var
LDI      mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_c2
MLT 4
ADD 1
LDI mi
SET main_x
OUT 0
F2I_M main_c2
MLT 4
ADD 1
LDI mf
SET main_y
F2I_M main_y
OUT 0
F2I_M    main_c2
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_c1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M main_c1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
F2I_M     main_c1
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_F2I_M main_c1
MLT     4
S_ADD
LDI     mi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_F2I_M main_c1
MLT     4
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_F2I_M main_c1
MLT     4
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     main_c2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M     main_c2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
F2I_M     main_c2
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_c2
MLT   4
ADD   __aux_var
LDI   mi
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_c2
MLT   4
ADD   __aux_var
LDI   mf
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_c2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M      main_c1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
SET main_x
OUT 0
F2I_M      main_c1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
SET main_y
F2I_M main_y
OUT 0
 F2I_M     main_c1
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
 LDI     mc
P_LOD   __aux_var
 LDI     mc_i
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
MLT 4
ADD main_n2
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_n1
MLT 4
ADD main_n2
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD   main_n1
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_n2
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
SET   __aux_var
LDI mc
P_LOD __aux_var
LDI mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
MLT 4
ADD 1
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_n2
MLT 4
ADD 1
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD   main_n2
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD main_n1
MLT   4
S_ADD
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD main_n1
MLT   4
S_ADD
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD main_n1
MLT   4
S_ADD
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     main_n2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD     main_n2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD     main_n2
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   main_n2
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   main_n2
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   main_n2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD     main_n1
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
ADD main_z0
MLT 4
ADD main_n2
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_n1
ADD main_z0
MLT 4
ADD main_n2
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n1
ADD main_z0
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
ADD main_z0
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_n2
ADD main_z0
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
ADD main_z0
MLT 4
P_LOD main_n3
ADD main_z0
S_ADD
SET   __aux_var
LDI mc
P_LOD __aux_var
LDI mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
ADD main_z0
MLT 4
ADD 1
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_n2
ADD main_z0
MLT 4
ADD 1
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
ADD main_z0
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
ADD main_z0
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_n1
ADD main_z0
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n1
ADD main_z0
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
ADD main_z0
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_n2
ADD main_z0
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
ADD main_z0
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n2
ADD main_z0
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_n2
ADD main_z0
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n2
ADD main_z0
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n1
ADD main_z0
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD 1
MLT 4
ADD main_n2
LDI mi
ADD 1000
SET main_x
OUT 0
LOD 1
MLT 4
ADD main_n2
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD   1
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n3
ADD main_z0
ADD 8
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_n3
ADD main_z0
ADD 8
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n3
ADD main_z0
ADD 8
SET   __aux_var
LDI mc
P_LOD __aux_var
LDI mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD_V mi 9
ADD 1000
SET main_x
OUT 0
LOD_V mf 9
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD_V mc 9
P_LOD_V mc_i 9
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD     1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD     1
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD 1
MLT   4
S_ADD
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD 1
MLT   4
S_ADD
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_LOD 1
MLT   4
S_ADD
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD     2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD     2
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   2
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   2
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
LOD   2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD     1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD     1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD     1
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_f1
MLT 4
ADD main_n2
LDI mi
ADD 1000
SET main_x
OUT 0
F2I_M main_f1
MLT 4
ADD main_n2
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M    main_f1
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
F2I_M main_f2
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
F2I_M main_f2
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
F2I_M main_f2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_f2
MLT 4
ADD 1
LDI mi
ADD 1000
SET main_x
OUT 0
F2I_M main_f2
MLT 4
ADD 1
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M    main_f2
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI      mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M main_f1
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M main_f1
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M main_f1
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     main_f2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M     main_f2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M main_f2
MLT 4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_f2 
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_f2 
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_f2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M     main_f1
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT 4
ADD main_n2
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT 4
ADD main_n2
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_n3
ADD main_z0
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_n3
ADD main_z0
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_n3
ADD main_z0
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT 4
ADD 1
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT 4
ADD 1
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f1
F_ADD main_fz
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_f1
F_ADD main_fz
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f1
F_ADD main_fz
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT 4
P_F2I_M 3.0
S_ADD
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT 4
P_F2I_M 3.0
S_ADD
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f1
F_ADD main_fz
F2I
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M 1.0
MLT 4
ADD main_n2
LDI mi
ADD 1000
SET main_x
OUT 0
F2I_M 1.0
MLT 4
ADD main_n2
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M   1.0
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n3
ADD main_z0
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_n3
ADD main_z0
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n3
ADD main_z0
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M 2.0
MLT 4
ADD 1
LDI mi
ADD 1000
SET main_x
OUT 0
F2I_M 2.0
MLT 4
ADD 1
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M   2.0
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M 1.0
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M 1.0
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
SET   __aux_var
F2I_M 1.0
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M 2.0
MLT 4
P_F2I_M 3.0
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M 2.0
MLT 4
P_F2I_M 3.0
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M 2.0
MLT 4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M 2.0
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M     1.0
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M      1.0
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT 4
ADD main_n2
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT 4
ADD main_n2
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_n3
ADD main_z0
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_n3
ADD main_z0
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_n3
ADD main_z0
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
POP
F2I
MLT 4
ADD 1
LDI mi
ADD 1000
SET main_x
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
POP
F2I
MLT 4
ADD 1
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P   __aux_var
F2I
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
POP
F2I
MLT  4
ADD  __aux_var
LDI  mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
POP
F2I
MLT  4
ADD  __aux_var
LDI  mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
P_LOD main_f2
F_ADD main_fz
F2I
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI    mc
P_LOD __aux_var
LDI    mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
SET_P   __aux_var
F2I
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c2
F_MLT main_one
P_LOD main_c2_i
F_MLT main_one_i
SF_SU2
P_LOD main_c2
F_MLT main_one_i
P_LOD main_c2_i
F_MLT main_one
SF_ADD
P_LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET_P __aux_var
POP
F2I
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
POP
F2I
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P   __aux_var
F2I
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_c1
MLT 4
ADD main_n2
LDI mi
ADD 1000
SET main_x
OUT 0
F2I_M main_c1
MLT 4
ADD main_n2
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M    main_c1
MLT   4
ADD   main_n2
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_n3
ADD main_z0
P_F2I_M main_c2
MLT     4
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_n3
ADD main_z0
P_F2I_M main_c2
MLT     4
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_n3
ADD main_z0
P_F2I_M main_c2
MLT     4
S_ADD
SET     __aux_var
LDI      mc
P_LOD   __aux_var
LDI      mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_c2
MLT 4
ADD 1
LDI mi
ADD 1000
SET main_x
OUT 0
F2I_M main_c2
MLT 4
ADD 1
LDI mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M    main_c2
MLT   4
ADD   1
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI   mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M main_c1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M main_c1
MLT     4
P_F2I_M main_f3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M     main_c1
MLT     4
P_F2I_M main_f3
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_F2I_M main_c1
MLT     4
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_F2I_M main_c1
MLT     4
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_f2
F_ADD main_fz
F2I
P_F2I_M main_c1
MLT     4
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M     main_c2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M     main_c2
MLT     4
P_F2I_M 3.0
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
F2I_M     main_c2
MLT     4
P_F2I_M 3.0
S_ADD
SET     __aux_var
LDI     mc
P_LOD   __aux_var
LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_c2
MLT   4
ADD   __aux_var
LDI   mi
ADD 1000
SET main_x
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_c2
MLT   4
ADD   __aux_var
LDI   mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
LOD main_c1
F_MLT main_one
P_LOD main_c1_i
F_MLT main_one_i
SF_SU2
P_LOD main_c1
F_MLT main_one_i
P_LOD main_c1_i
F_MLT main_one
SF_ADD
SET_P __aux_var
F2I
SET   __aux_var
F2I_M main_c2
MLT   4
ADD   __aux_var
SET   __aux_var
LDI   mc
P_LOD __aux_var
LDI mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
F2I_M      main_c1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mi
ADD 1000
SET main_x
OUT 0
F2I_M      main_c1
MLT     4
P_F2I_M main_c3
S_ADD
LDI     mf
F_ADD 1000.0
SET main_y
F2I_M main_y
OUT 0
 F2I_M     main_c1
MLT     4
P_F2I_M main_c3
S_ADD
SET     __aux_var
 LDI     mc
P_LOD   __aux_var
 LDI     mc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
F2I_M main_w
OUT 0
F2I_M main_w_i
OUT 0
@fim JMP fim
