NOP
#PRNAME cmm_cov_arr1d
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1
#arrayd vi 1 8 8 1 4 7 10 13 16 19 22
#arrayd vf 2 8 8 50.0 51.0 52.0 53.0 54.0 55.0 56.0 57.0
#arrayd vc 3 8 8 20.0 21.0 22.0 23.0 24.0 25.0 26.0 27.0
#arrayd vc_i 4 8 8 -1.0 -2.0 -3.0 -4.0 -5.0 -6.0 -7.0 -8.0
#SHARE main_n1 main_n0
#SHARE main_f1 main_f0
#SHARE main_c0_i main_c0
#SHARE main_c1 main_c0
#SHARE main_y main_x
#SHARE main_w_i main_x
#SHARE __aux_var main_x
#SHARE main_iv main_w
#SHARE __aux_var2 main_c2_i
#SHARE __aux_var3 main_w
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
LOD 0.0
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
LDI vi
SET main_x
OUT 0
LOD main_n2
ADD main_z0
LDI vi
SET main_x
OUT 0
LOD_V vi 3
SET main_x
OUT 0
F2I_M main_f1
LDI vi
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
LDI vi
SET main_x
OUT 0
F2I_M 3.0
LDI vi
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
LDI vi
SET main_x
OUT 0
F2I_M main_c2
LDI vi
SET main_x
OUT 0
LOD main_n3
LDI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD main_n1
ADD main_z0
LDI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD_V vf 2
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
F2I_M main_f3
LDI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD main_f1
F_ADD main_fz
F2I
LDI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
F2I_M 2.0
LDI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD main_c3
F_MLT main_one
P_LOD main_c3_i
F_MLT main_one_i
SF_SU2
P_LOD main_c3
F_MLT main_one_i
P_LOD main_c3_i
F_MLT main_one
SF_ADD
POP
F2I
LDI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
F2I_M main_c1
LDI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD_V vc 1
P_LOD_V vc_i 1
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
F2I_M main_f2
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD main_f3
F_ADD main_fz
F2I
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
F2I_M 1.0
SET __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
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
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
F2I_M main_c3
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD main_n1
LDI vi
ADD 1000
SET main_x
OUT 0
LOD main_n2
ADD main_z0
LDI vi
ADD 1000
SET main_x
OUT 0
LOD_V vi 3
ADD 1000
SET main_x
OUT 0
F2I_M main_f1
LDI vi
ADD 1000
SET main_x
OUT 0
LOD main_f2
F_ADD main_fz
F2I
LDI vi
ADD 1000
SET main_x
OUT 0
F2I_M 3.0
LDI vi
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
LDI vi
ADD 1000
SET main_x
OUT 0
F2I_M main_c2
LDI vi
ADD 1000
SET main_x
OUT 0
LOD main_n3
LDI vf
F_ADD 1000.0
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD main_n1
ADD main_z0
LDI vf
F_ADD 1000.0
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD_V vf 2
F_ADD 1000.0
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
F2I_M main_f3
LDI vf
F_ADD 1000.0
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD main_f1
F_ADD main_fz
F2I
LDI vf
F_ADD 1000.0
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
F2I_M 2.0
LDI vf
F_ADD 1000.0
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD main_c3
F_MLT main_one
P_LOD main_c3_i
F_MLT main_one_i
SF_SU2
P_LOD main_c3
F_MLT main_one_i
P_LOD main_c3_i
F_MLT main_one
SF_ADD
POP
F2I
LDI vf
F_ADD 1000.0
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
F2I_M main_c1
LDI vf
F_ADD 1000.0
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD_V vc 1
P_LOD_V vc_i 1
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
F2I_M main_f2
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD main_f3
F_ADD main_fz
F2I
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
F2I_M 1.0
SET __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
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
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
F2I_M main_c3
SET   __aux_var
LDI vc
P_LOD __aux_var
LDI vc_i
SET_P __aux_var
F_ADD 100.0
P_LOD 100.0
F_ADD __aux_var
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 300
SET main_iv
LOD 300.5
SET main_fv
LOD 300.0
SET main_cv
LOD 350.0
SET main_cv_i
LOD main_n1
P_LOD main_iv
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 301
SET main_iv
LOD 301.5
SET main_fv
LOD 301.0
SET main_cv
LOD 351.0
SET main_cv_i
LOD main_n2
ADD main_z0
P_LOD main_iv
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 302
SET main_iv
LOD 302.5
SET main_fv
LOD 302.0
SET main_cv
LOD 352.0
SET main_cv_i
LOD main_iv
SET_V vi 3
LOD main_n3
LDI vi
OUT 0
LOD 303
SET main_iv
LOD 303.5
SET main_fv
LOD 303.0
SET main_cv
LOD 353.0
SET main_cv_i
F2I_M main_f1
P_LOD main_iv
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 304
SET main_iv
LOD 304.5
SET main_fv
LOD 304.0
SET main_cv
LOD 354.0
SET main_cv_i
LOD main_f2
F_ADD main_fz
F2I
P_LOD main_iv
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 305
SET main_iv
LOD 305.5
SET main_fv
LOD 305.0
SET main_cv
LOD 355.0
SET main_cv_i
F2I_M 3.0
P_LOD main_iv
STI vi
LOD main_n3
LDI vi
OUT 0
LOD 306
SET main_iv
LOD 306.5
SET main_fv
LOD 306.0
SET main_cv
LOD 356.0
SET main_cv_i
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
P_LOD main_iv
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 307
SET main_iv
LOD 307.5
SET main_fv
LOD 307.0
SET main_cv
LOD 357.0
SET main_cv_i
F2I_M main_c2
P_LOD main_iv
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 308
SET main_iv
LOD 308.5
SET main_fv
LOD 308.0
SET main_cv
LOD 358.0
SET main_cv_i
LOD main_n3
P_LOD main_fv
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 309
SET main_iv
LOD 309.5
SET main_fv
LOD 309.0
SET main_cv
LOD 359.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_fv
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 310
SET main_iv
LOD 310.5
SET main_fv
LOD 310.0
SET main_cv
LOD 360.0
SET main_cv_i
LOD main_fv
SET_V vf 2
LOD main_n2
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 311
SET main_iv
LOD 311.5
SET main_fv
LOD 311.0
SET main_cv
LOD 361.0
SET main_cv_i
F2I_M main_f3
P_LOD main_fv
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 312
SET main_iv
LOD 312.5
SET main_fv
LOD 312.0
SET main_cv
LOD 362.0
SET main_cv_i
LOD main_f1
F_ADD main_fz
F2I
P_LOD main_fv
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 313
SET main_iv
LOD 313.5
SET main_fv
LOD 313.0
SET main_cv
LOD 363.0
SET main_cv_i
F2I_M 2.0
P_LOD main_fv
STI vf
LOD main_n2
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 314
SET main_iv
LOD 314.5
SET main_fv
LOD 314.0
SET main_cv
LOD 364.0
SET main_cv_i
LOD main_c3
F_MLT main_one
P_LOD main_c3_i
F_MLT main_one_i
SF_SU2
P_LOD main_c3
F_MLT main_one_i
P_LOD main_c3_i
F_MLT main_one
SF_ADD
POP
F2I
P_LOD main_fv
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 315
SET main_iv
LOD 315.5
SET main_fv
LOD 315.0
SET main_cv
LOD 365.0
SET main_cv_i
F2I_M main_c1
P_LOD main_fv
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 316
SET main_iv
LOD 316.5
SET main_fv
LOD 316.0
SET main_cv
LOD 366.0
SET main_cv_i
LOD main_n2
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 317
SET main_iv
LOD 317.5
SET main_fv
LOD 317.0
SET main_cv
LOD 367.0
SET main_cv_i
LOD main_n3
ADD main_z0
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 318
SET main_iv
LOD 318.5
SET main_fv
LOD 318.0
SET main_cv
LOD 368.0
SET main_cv_i
LOD main_cv
SET_V vc 1
LOD main_cv_i
SET_V vc_i 1
LOD main_n1
LDI vc
P_LOD main_n1
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 319
SET main_iv
LOD 319.5
SET main_fv
LOD 319.0
SET main_cv
LOD 369.0
SET main_cv_i
F2I_M main_f2
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 320
SET main_iv
LOD 320.5
SET main_fv
LOD 320.0
SET main_cv
LOD 370.0
SET main_cv_i
LOD main_f3
F_ADD main_fz
F2I
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 321
SET main_iv
LOD 321.5
SET main_fv
LOD 321.0
SET main_cv
LOD 371.0
SET main_cv_i
F2I_M 1.0
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n1
LDI vc
P_LOD main_n1
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 322
SET main_iv
LOD 322.5
SET main_fv
LOD 322.0
SET main_cv
LOD 372.0
SET main_cv_i
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
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 323
SET main_iv
LOD 323.5
SET main_fv
LOD 323.0
SET main_cv
LOD 373.0
SET main_cv_i
F2I_M main_c3
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 324
SET main_iv
LOD 324.5
SET main_fv
LOD 324.0
SET main_cv
LOD 374.0
SET main_cv_i
LOD main_n1
P_LOD main_iv
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 325
SET main_iv
LOD 325.5
SET main_fv
LOD 325.0
SET main_cv
LOD 375.0
SET main_cv_i
LOD main_n2
ADD main_z0
P_LOD main_iv
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 326
SET main_iv
LOD 326.5
SET main_fv
LOD 326.0
SET main_cv
LOD 376.0
SET main_cv_i
LOD main_n3
P_LOD main_iv
ADD main_z0
STI vi
LOD main_n3
LDI vi
OUT 0
LOD 327
SET main_iv
LOD 327.5
SET main_fv
LOD 327.0
SET main_cv
LOD 377.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_iv
ADD main_z0
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 328
SET main_iv
LOD 328.5
SET main_fv
LOD 328.0
SET main_cv
LOD 378.0
SET main_cv_i
LOD main_n2
P_LOD 328
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 329
SET main_iv
LOD 329.5
SET main_fv
LOD 329.0
SET main_cv
LOD 379.0
SET main_cv_i
LOD main_n3
ADD main_z0
P_LOD 329
STI vi
LOD main_n3
LDI vi
OUT 0
LOD 330
SET main_iv
LOD 330.5
SET main_fv
LOD 330.0
SET main_cv
LOD 380.0
SET main_cv_i
LOD main_n1
P_F2I_M main_fv
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 331
SET main_iv
LOD 331.5
SET main_fv
LOD 331.0
SET main_cv
LOD 381.0
SET main_cv_i
LOD main_n2
ADD main_z0
P_F2I_M main_fv
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 332
SET main_iv
LOD 332.5
SET main_fv
LOD 332.0
SET main_cv
LOD 382.0
SET main_cv_i
LOD main_n3
P_LOD main_fv
F_ADD main_fz
F2I
STI vi
LOD main_n3
LDI vi
OUT 0
LOD 333
SET main_iv
LOD 333.5
SET main_fv
LOD 333.0
SET main_cv
LOD 383.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_fv
F_ADD main_fz
F2I
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 334
SET main_iv
LOD 334.5
SET main_fv
LOD 334.0
SET main_cv
LOD 384.0
SET main_cv_i
LOD main_n2
P_F2I_M 334.5
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 335
SET main_iv
LOD 335.5
SET main_fv
LOD 335.0
SET main_cv
LOD 385.0
SET main_cv_i
LOD main_n3
ADD main_z0
P_F2I_M 335.5
STI vi
LOD main_n3
LDI vi
OUT 0
LOD 336
SET main_iv
LOD 336.5
SET main_fv
LOD 336.0
SET main_cv
LOD 386.0
SET main_cv_i
LOD main_n1
P_F2I_M main_cv
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 337
SET main_iv
LOD 337.5
SET main_fv
LOD 337.0
SET main_cv
LOD 387.0
SET main_cv_i
LOD main_n2
ADD main_z0
P_F2I_M main_cv
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 338
SET main_iv
LOD 338.5
SET main_fv
LOD 338.0
SET main_cv
LOD 388.0
SET main_cv_i
LOD main_n3
P_LOD main_cv
F_MLT main_one
P_LOD main_cv_i
F_MLT main_one_i
SF_SU2
P_LOD main_cv
F_MLT main_one_i
P_LOD main_cv_i
F_MLT main_one
SF_ADD
POP
F2I
STI vi
LOD main_n3
LDI vi
OUT 0
LOD 339
SET main_iv
LOD 339.5
SET main_fv
LOD 339.0
SET main_cv
LOD 389.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_cv
F_MLT main_one
P_LOD main_cv_i
F_MLT main_one_i
SF_SU2
P_LOD main_cv
F_MLT main_one_i
P_LOD main_cv_i
F_MLT main_one
SF_ADD
POP
F2I
STI vi
LOD main_n1
LDI vi
OUT 0
LOD 340
SET main_iv
LOD 340.5
SET main_fv
LOD 340.0
SET main_cv
LOD 390.0
SET main_cv_i
LOD main_n2
P_F2I_M 340.0
STI vi
LOD main_n2
LDI vi
OUT 0
LOD 341
SET main_iv
LOD 341.5
SET main_fv
LOD 341.0
SET main_cv
LOD 391.0
SET main_cv_i
LOD main_n3
ADD main_z0
P_F2I_M 341.0
STI vi
LOD main_n3
LDI vi
OUT 0
LOD 342
SET main_iv
LOD 342.5
SET main_fv
LOD 342.0
SET main_cv
LOD 392.0
SET main_cv_i
LOD main_n1
P_I2F_M main_iv
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 343
SET main_iv
LOD 343.5
SET main_fv
LOD 343.0
SET main_cv
LOD 393.0
SET main_cv_i
LOD main_n2
ADD main_z0
P_I2F_M main_iv
STI vf
LOD main_n2
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 344
SET main_iv
LOD 344.5
SET main_fv
LOD 344.0
SET main_cv
LOD 394.0
SET main_cv_i
LOD main_n3
P_LOD main_iv
ADD main_z0
I2F
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 345
SET main_iv
LOD 345.5
SET main_fv
LOD 345.0
SET main_cv
LOD 395.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_iv
ADD main_z0
I2F
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 346
SET main_iv
LOD 346.5
SET main_fv
LOD 346.0
SET main_cv
LOD 396.0
SET main_cv_i
LOD main_n2
P_LOD 346.0
STI vf
LOD main_n2
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 347
SET main_iv
LOD 347.5
SET main_fv
LOD 347.0
SET main_cv
LOD 397.0
SET main_cv_i
LOD main_n3
ADD main_z0
P_LOD 347.0
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 348
SET main_iv
LOD 348.5
SET main_fv
LOD 348.0
SET main_cv
LOD 398.0
SET main_cv_i
LOD main_n1
P_LOD main_fv
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 349
SET main_iv
LOD 349.5
SET main_fv
LOD 349.0
SET main_cv
LOD 399.0
SET main_cv_i
LOD main_n2
ADD main_z0
P_LOD main_fv
STI vf
LOD main_n2
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 350
SET main_iv
LOD 350.5
SET main_fv
LOD 350.0
SET main_cv
LOD 400.0
SET main_cv_i
LOD main_n3
P_LOD main_fv
F_ADD main_fz
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 351
SET main_iv
LOD 351.5
SET main_fv
LOD 351.0
SET main_cv
LOD 401.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_fv
F_ADD main_fz
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 352
SET main_iv
LOD 352.5
SET main_fv
LOD 352.0
SET main_cv
LOD 402.0
SET main_cv_i
LOD main_n2
P_LOD 352.5
STI vf
LOD main_n2
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 353
SET main_iv
LOD 353.5
SET main_fv
LOD 353.0
SET main_cv
LOD 403.0
SET main_cv_i
LOD main_n3
ADD main_z0
P_LOD 353.5
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 354
SET main_iv
LOD 354.5
SET main_fv
LOD 354.0
SET main_cv
LOD 404.0
SET main_cv_i
LOD main_n1
P_LOD main_cv
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 355
SET main_iv
LOD 355.5
SET main_fv
LOD 355.0
SET main_cv
LOD 405.0
SET main_cv_i
LOD main_n2
ADD main_z0
P_LOD main_cv
STI vf
LOD main_n2
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 356
SET main_iv
LOD 356.5
SET main_fv
LOD 356.0
SET main_cv
LOD 406.0
SET main_cv_i
LOD main_n3
P_LOD main_cv
F_MLT main_one
P_LOD main_cv_i
F_MLT main_one_i
SF_SU2
P_LOD main_cv
F_MLT main_one_i
P_LOD main_cv_i
F_MLT main_one
SF_ADD
POP
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 357
SET main_iv
LOD 357.5
SET main_fv
LOD 357.0
SET main_cv
LOD 407.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_cv
F_MLT main_one
P_LOD main_cv_i
F_MLT main_one_i
SF_SU2
P_LOD main_cv
F_MLT main_one_i
P_LOD main_cv_i
F_MLT main_one
SF_ADD
POP
STI vf
LOD main_n1
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 358
SET main_iv
LOD 358.5
SET main_fv
LOD 358.0
SET main_cv
LOD 408.0
SET main_cv_i
LOD main_n2
P_LOD 358.0
STI vf
LOD main_n2
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 359
SET main_iv
LOD 359.5
SET main_fv
LOD 359.0
SET main_cv
LOD 409.0
SET main_cv_i
LOD main_n3
ADD main_z0
P_LOD 359.0
STI vf
LOD main_n3
LDI vf
F_MLT 2.0
F2I
OUT 0
LOD 360
SET main_iv
LOD 360.5
SET main_fv
LOD 360.0
SET main_cv
LOD 410.0
SET main_cv_i
LOD main_n1
SET __aux_var
P_I2F_M main_iv
STI vc
LOD __aux_var
P_LOD 0.0
STI vc_i
LOD main_n1
LDI vc
P_LOD main_n1
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 361
SET main_iv
LOD 361.5
SET main_fv
LOD 361.0
SET main_cv
LOD 411.0
SET main_cv_i
LOD main_n2
ADD main_z0
SET __aux_var
P_I2F_M main_iv
STI vc
LOD __aux_var
P_LOD 0.0
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 362
SET main_iv
LOD 362.5
SET main_fv
LOD 362.0
SET main_cv
LOD 412.0
SET main_cv_i
LOD main_n3
P_LOD main_iv
ADD main_z0
I2F
SET_P __aux_var
SET   __aux_var2
P_LOD __aux_var
STI vc
LOD   __aux_var2
P_LOD 0.0
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 363
SET main_iv
LOD 363.5
SET main_fv
LOD 363.0
SET main_cv
LOD 413.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_iv
ADD main_z0
I2F
SET_P __aux_var
SET   __aux_var2
P_LOD __aux_var
STI vc
LOD   __aux_var2
P_LOD 0.0
STI vc_i
LOD main_n1
LDI vc
P_LOD main_n1
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 364
SET main_iv
LOD 364.5
SET main_fv
LOD 364.0
SET main_cv
LOD 414.0
SET main_cv_i
LOD main_n2
SET __aux_var
P_LOD 364.0
STI vc
LOD __aux_var
P_LOD 0.0
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 365
SET main_iv
LOD 365.5
SET main_fv
LOD 365.0
SET main_cv
LOD 415.0
SET main_cv_i
LOD main_n3
ADD main_z0
SET __aux_var
P_LOD 365.0
STI vc
LOD __aux_var
P_LOD 0.0
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 366
SET main_iv
LOD 366.5
SET main_fv
LOD 366.0
SET main_cv
LOD 416.0
SET main_cv_i
LOD main_n1
SET   __aux_var
P_LOD main_fv
STI vc
LOD   __aux_var
P_LOD 0.0
STI vc_i
LOD main_n1
LDI vc
P_LOD main_n1
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 367
SET main_iv
LOD 367.5
SET main_fv
LOD 367.0
SET main_cv
LOD 417.0
SET main_cv_i
LOD main_n2
ADD main_z0
SET   __aux_var
P_LOD main_fv
STI vc
LOD   __aux_var
P_LOD 0.0
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 368
SET main_iv
LOD 368.5
SET main_fv
LOD 368.0
SET main_cv
LOD 418.0
SET main_cv_i
LOD main_n3
P_LOD main_fv
F_ADD main_fz
SET_P __aux_var
SET   __aux_var2
P_LOD __aux_var
STI vc
LOD   __aux_var2
P_LOD 0.0
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 369
SET main_iv
LOD 369.5
SET main_fv
LOD 369.0
SET main_cv
LOD 419.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_fv
F_ADD main_fz
SET_P __aux_var
SET   __aux_var2
P_LOD __aux_var
STI vc
LOD   __aux_var2
P_LOD 0.0
STI vc_i
LOD main_n1
LDI vc
P_LOD main_n1
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 370
SET main_iv
LOD 370.5
SET main_fv
LOD 370.0
SET main_cv
LOD 420.0
SET main_cv_i
LOD main_n2
SET   __aux_var
P_LOD 370.5
STI vc
LOD   __aux_var
P_LOD 0.0
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 371
SET main_iv
LOD 371.5
SET main_fv
LOD 371.0
SET main_cv
LOD 421.0
SET main_cv_i
LOD main_n3
ADD main_z0
SET   __aux_var
P_LOD 371.5
STI vc
LOD   __aux_var
P_LOD 0.0
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 372
SET main_iv
LOD 372.5
SET main_fv
LOD 372.0
SET main_cv
LOD 422.0
SET main_cv_i
LOD main_n1
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n1
LDI vc
P_LOD main_n1
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 373
SET main_iv
LOD 373.5
SET main_fv
LOD 373.0
SET main_cv
LOD 423.0
SET main_cv_i
LOD main_n2
ADD main_z0
SET   __aux_var
P_LOD main_cv
STI vc
LOD   __aux_var
P_LOD main_cv_i
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 374
SET main_iv
LOD 374.5
SET main_fv
LOD 374.0
SET main_cv
LOD 424.0
SET main_cv_i
LOD main_n3
P_LOD main_cv
F_MLT main_one
P_LOD main_cv_i
F_MLT main_one_i
SF_SU2
P_LOD main_cv
F_MLT main_one_i
P_LOD main_cv_i
F_MLT main_one
SF_ADD
SET_P __aux_var
SET_P __aux_var2
SET   __aux_var3
P_LOD __aux_var2
STI vc
LOD   __aux_var3
P_LOD __aux_var
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 375
SET main_iv
LOD 375.5
SET main_fv
LOD 375.0
SET main_cv
LOD 425.0
SET main_cv_i
LOD main_n1
ADD main_z0
P_LOD main_cv
F_MLT main_one
P_LOD main_cv_i
F_MLT main_one_i
SF_SU2
P_LOD main_cv
F_MLT main_one_i
P_LOD main_cv_i
F_MLT main_one
SF_ADD
SET_P __aux_var
SET_P __aux_var2
SET   __aux_var3
P_LOD __aux_var2
STI vc
LOD   __aux_var3
P_LOD __aux_var
STI vc_i
LOD main_n1
LDI vc
P_LOD main_n1
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 376
SET main_iv
LOD 376.5
SET main_fv
LOD 376.0
SET main_cv
LOD 426.0
SET main_cv_i
LOD main_n2
SET   __aux_var
P_LOD 376.0
STI vc
LOD   __aux_var
P_LOD 426.0
STI vc_i
LOD main_n2
LDI vc
P_LOD main_n2
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 377
SET main_iv
LOD 377.5
SET main_fv
LOD 377.0
SET main_cv
LOD 427.0
SET main_cv_i
LOD main_n3
ADD main_z0
SET   __aux_var
P_LOD 377.0
STI vc
LOD   __aux_var
P_LOD 427.0
STI vc_i
LOD main_n3
LDI vc
P_LOD main_n3
LDI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD main_n1
PSH
LDI vi
ADD 1
STI vi
LOD_V vi 1
OUT 0
LOD main_n2
ADD main_z0
PSH
LDI vi
ADD 1
STI vi
LOD_V vi 2
OUT 0
LOD 3
PSH
LDI vi
ADD 1
STI vi
LOD_V vi 3
OUT 0
F2I_M main_f1
PSH
LDI vi
ADD 1
STI vi
LOD_V vi 1
OUT 0
LOD main_f2
F_ADD main_fz
F2I
PSH
LDI vi
ADD 1
STI vi
LOD_V vi 2
OUT 0
F2I_M 3.0
PSH
LDI vi
ADD 1
STI vi
LOD_V vi 3
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
PSH
LDI vi
ADD 1
STI vi
LOD_V vi 1
OUT 0
F2I_M main_c2
PSH
LDI vi
ADD 1
STI vi
LOD_V vi 2
OUT 0
LOD main_n3
PSH
LDI vf
F_ADD 1.0
STI vf
LOD_V vf 3
F_MLT 2.0
F2I
OUT 0
LOD main_n1
ADD main_z0
PSH
LDI vf
F_ADD 1.0
STI vf
LOD_V vf 1
F_MLT 2.0
F2I
OUT 0
LOD 2
PSH
LDI vf
F_ADD 1.0
STI vf
LOD_V vf 2
F_MLT 2.0
F2I
OUT 0
F2I_M main_f3
PSH
LDI vf
F_ADD 1.0
STI vf
LOD_V vf 3
F_MLT 2.0
F2I
OUT 0
LOD main_f1
F_ADD main_fz
F2I
PSH
LDI vf
F_ADD 1.0
STI vf
LOD_V vf 1
F_MLT 2.0
F2I
OUT 0
F2I_M 2.0
PSH
LDI vf
F_ADD 1.0
STI vf
LOD_V vf 2
F_MLT 2.0
F2I
OUT 0
LOD main_c3
F_MLT main_one
P_LOD main_c3_i
F_MLT main_one_i
SF_SU2
P_LOD main_c3
F_MLT main_one_i
P_LOD main_c3_i
F_MLT main_one
SF_ADD
POP
F2I
PSH
LDI vf
F_ADD 1.0
STI vf
LOD_V vf 3
F_MLT 2.0
F2I
OUT 0
F2I_M main_c1
PSH
LDI vf
F_ADD 1.0
STI vf
LOD_V vf 1
F_MLT 2.0
F2I
OUT 0
LOD main_n1
ILI vi
SET main_x
OUT 0
LOD 378
SET main_iv
LOD 378.5
SET main_fv
LOD 378.0
SET main_cv
LOD 428.0
SET main_cv_i
LOD main_n1
P_LOD main_iv
ISI vi
LOD_V vi 4
OUT 0
LOD main_n3
ADD main_z0
ILI vi
SET main_x
OUT 0
LOD 379
SET main_iv
LOD 379.5
SET main_fv
LOD 379.0
SET main_cv
LOD 429.0
SET main_cv_i
LOD main_n3
ADD main_z0
P_LOD main_iv
ISI vi
LOD_V vi 6
OUT 0
LOD main_n1
ILI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD 380
SET main_iv
LOD 380.5
SET main_fv
LOD 380.0
SET main_cv
LOD 430.0
SET main_cv_i
LOD main_n1
P_LOD main_fv
ISI vf
LOD_V vf 4
F_MLT 2.0
F2I
OUT 0
LOD main_n3
ADD main_z0
ILI vf
SET main_y
LOD 2.0
F_MLT main_y
F2I
OUT 0
LOD 381
SET main_iv
LOD 381.5
SET main_fv
LOD 381.0
SET main_cv
LOD 431.0
SET main_cv_i
LOD main_n3
ADD main_z0
P_LOD main_fv
ISI vf
LOD_V vf 6
F_MLT 2.0
F2I
OUT 0
LOD main_n1
ILI vc
P_LOD main_n1
ILI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 382
SET main_iv
LOD 382.5
SET main_fv
LOD 382.0
SET main_cv
LOD 432.0
SET main_cv_i
LOD main_n1
SET   __aux_var
P_LOD main_cv
ISI vc
LOD   __aux_var
P_LOD main_cv_i
ISI vc_i
LOD_V vc 4
P_LOD_V vc_i 4
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD main_n3
ADD main_z0
SET   __aux_var
ILI vc
P_LOD __aux_var
ILI vc_i
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
LOD 383
SET main_iv
LOD 383.5
SET main_fv
LOD 383.0
SET main_cv
LOD 433.0
SET main_cv_i
LOD main_n3
ADD main_z0
SET   __aux_var
P_LOD main_cv
ISI vc
LOD   __aux_var
P_LOD main_cv_i
ISI vc_i
LOD_V vc 6
P_LOD_V vc_i 6
SET_P main_w_i
SET main_w
LOD 2.0
F_MLT main_w
F2I
OUT 0
LOD 2.0
F_MLT main_w_i
F2I
OUT 0
@fim JMP fim
