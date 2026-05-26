NOP
#PRNAME procBlind
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NDSTAC 10
#SDEPTH 10
#NUIOIN 1
#NUIOOU 1
#array y 2 40
#array x_hat 2 34
#array h_hat 2 7
#array s_z 2 34
#array s_x_prev 2 34
#array s_grad_x 2 34
#array s_conv 2 40
#array s_h_cand 2 7
#array s_h_best 2 7
#array s_grad_h 2 7
JMP main
@project_h_hat SET project_h_hat_dummy
LOD 0
SET project_h_hat_k
@Lwh1 LOD 7
LES project_h_hat_k
JIZ Lwh1end
LOD project_h_hat_k
LDI h_hat
P_LOD 0.0
SF_LES
JIZ Lif1else
LOD project_h_hat_k
P_LOD 0.0
STI h_hat
@Lif1else LOD project_h_hat_k
LDI h_hat
P_LOD 1.0
SF_GRE
JIZ Lif2else
LOD project_h_hat_k
P_LOD 1.0
STI h_hat
@Lif2else LOD project_h_hat_k
ADD 1
SET project_h_hat_k
JMP Lwh1
@Lwh1end LOD 3
P_LOD 1.0
STI h_hat
RET
@project_h_cand SET project_h_cand_dummy
LOD 0
SET project_h_cand_k
@Lwh2 LOD 7
LES project_h_cand_k
JIZ Lwh2end
LOD project_h_cand_k
LDI s_h_cand
P_LOD 0.0
SF_LES
JIZ Lif3else
LOD project_h_cand_k
P_LOD 0.0
STI s_h_cand
@Lif3else LOD project_h_cand_k
LDI s_h_cand
P_LOD 1.0
SF_GRE
JIZ Lif4else
LOD project_h_cand_k
P_LOD 1.0
STI s_h_cand
@Lif4else LOD project_h_cand_k
ADD 1
SET project_h_cand_k
JMP Lwh2
@Lwh2end LOD 3
P_LOD 1.0
STI s_h_cand
RET
@lipschitz_x SET lipschitz_x_dummy
LOD 0.0
SET lipschitz_x_s
LOD 0
SET lipschitz_x_k
@Lwh3 LOD 7
LES lipschitz_x_k
JIZ Lwh3end
LOD lipschitz_x_k
LDI h_hat
F_ADD lipschitz_x_s
SET lipschitz_x_s
LOD lipschitz_x_k
ADD 1
SET lipschitz_x_k
JMP Lwh3
@Lwh3end LOD lipschitz_x_s
F_MLT lipschitz_x_s
SET lipschitz_x_L
P_LOD 0.000000000001
SF_LES
JIZ Lif5else
LOD 0.000000000001
SET lipschitz_x_L
@Lif5else LOD lipschitz_x_L
RET
@conv_zh SET conv_zh_dummy
LOD 0
SET conv_zh_n
@Lwh4 LOD 40
LES conv_zh_n
JIZ Lwh4end
LOD conv_zh_n
P_LOD 0.0
STI s_conv
LOD conv_zh_n
ADD 1
SET conv_zh_n
JMP Lwh4
@Lwh4end LOD 0
SET conv_zh_ii
@Lwh5 LOD 34
LES conv_zh_ii
JIZ Lwh5end
LOD conv_zh_ii
LDI s_z
SET conv_zh_zi
LOD 0
SET conv_zh_j
@Lwh6 LOD 7
LES conv_zh_j
JIZ Lwh6end
LOD conv_zh_ii
ADD conv_zh_j
P_LOD conv_zh_ii
ADD conv_zh_j
LDI s_conv
P_LOD conv_zh_j
LDI h_hat
F_MLT conv_zh_zi
SF_ADD
STI s_conv
LOD conv_zh_j
ADD 1
SET conv_zh_j
JMP Lwh6
@Lwh6end LOD conv_zh_ii
ADD 1
SET conv_zh_ii
JMP Lwh5
@Lwh5end RET
@conv_xh SET conv_xh_dummy
LOD 0
SET conv_xh_n
@Lwh7 LOD 40
LES conv_xh_n
JIZ Lwh7end
LOD conv_xh_n
P_LOD 0.0
STI s_conv
LOD conv_xh_n
ADD 1
SET conv_xh_n
JMP Lwh7
@Lwh7end LOD 0
SET conv_xh_ii
@Lwh8 LOD 34
LES conv_xh_ii
JIZ Lwh8end
LOD conv_xh_ii
LDI x_hat
SET conv_xh_xi
LOD 0
SET conv_xh_j
@Lwh9 LOD 7
LES conv_xh_j
JIZ Lwh9end
LOD conv_xh_ii
ADD conv_xh_j
P_LOD conv_xh_ii
ADD conv_xh_j
LDI s_conv
P_LOD conv_xh_j
LDI h_hat
F_MLT conv_xh_xi
SF_ADD
STI s_conv
LOD conv_xh_j
ADD 1
SET conv_xh_j
JMP Lwh9
@Lwh9end LOD conv_xh_ii
ADD 1
SET conv_xh_ii
JMP Lwh8
@Lwh8end RET
@conv_x_hcand SET conv_x_hcand_dummy
LOD 0
SET conv_x_hcand_n
@Lwh10 LOD 40
LES conv_x_hcand_n
JIZ Lwh10end
LOD conv_x_hcand_n
P_LOD 0.0
STI s_conv
LOD conv_x_hcand_n
ADD 1
SET conv_x_hcand_n
JMP Lwh10
@Lwh10end LOD 0
SET conv_x_hcand_ii
@Lwh11 LOD 34
LES conv_x_hcand_ii
JIZ Lwh11end
LOD conv_x_hcand_ii
LDI x_hat
SET conv_x_hcand_xi
LOD 0
SET conv_x_hcand_j
@Lwh12 LOD 7
LES conv_x_hcand_j
JIZ Lwh12end
LOD conv_x_hcand_ii
ADD conv_x_hcand_j
P_LOD conv_x_hcand_ii
ADD conv_x_hcand_j
LDI s_conv
P_LOD conv_x_hcand_j
LDI s_h_cand
F_MLT conv_x_hcand_xi
SF_ADD
STI s_conv
LOD conv_x_hcand_j
ADD 1
SET conv_x_hcand_j
JMP Lwh12
@Lwh12end LOD conv_x_hcand_ii
ADD 1
SET conv_x_hcand_ii
JMP Lwh11
@Lwh11end RET
@residual_sub_y SET residual_sub_y_dummy
LOD 0
SET residual_sub_y_n
@Lwh13 LOD 40
LES residual_sub_y_n
JIZ Lwh13end
LOD residual_sub_y_n
P_LOD residual_sub_y_n
LDI s_conv
P_LOD residual_sub_y_n
LDI y
SF_SU2
STI s_conv
LOD residual_sub_y_n
ADD 1
SET residual_sub_y_n
JMP Lwh13
@Lwh13end RET
@sum_residual_sq SET sum_residual_sq_dummy
LOD 0.0
SET sum_residual_sq_s
LOD 0
SET sum_residual_sq_n
@Lwh14 LOD 40
LES sum_residual_sq_n
JIZ Lwh14end
LOD sum_residual_sq_n
LDI s_conv
P_LOD sum_residual_sq_n
LDI s_conv
SF_MLT
F_ADD sum_residual_sq_s
SET sum_residual_sq_s
LOD sum_residual_sq_n
ADD 1
SET sum_residual_sq_n
JMP Lwh14
@Lwh14end LOD sum_residual_sq_s
RET
@correlate_residual_h SET correlate_residual_h_dummy
LOD 0
SET correlate_residual_h_n
@Lwh15 LOD 34
LES correlate_residual_h_n
JIZ Lwh15end
LOD 0.0
SET correlate_residual_h_s
LOD 0
SET correlate_residual_h_k
@Lwh16 LOD 7
LES correlate_residual_h_k
JIZ Lwh16end
LOD correlate_residual_h_n
ADD correlate_residual_h_k
LDI s_conv
P_LOD correlate_residual_h_k
LDI h_hat
SF_MLT
F_ADD correlate_residual_h_s
SET correlate_residual_h_s
LOD correlate_residual_h_k
ADD 1
SET correlate_residual_h_k
JMP Lwh16
@Lwh16end LOD correlate_residual_h_n
P_LOD correlate_residual_h_s
STI s_grad_x
LOD correlate_residual_h_n
ADD 1
SET correlate_residual_h_n
JMP Lwh15
@Lwh15end RET
@correlate_residual_x SET correlate_residual_x_dummy
LOD 0
SET correlate_residual_x_n
@Lwh17 LOD 7
LES correlate_residual_x_n
JIZ Lwh17end
LOD 0.0
SET correlate_residual_x_s
LOD 0
SET correlate_residual_x_k
@Lwh18 LOD 34
LES correlate_residual_x_k
JIZ Lwh18end
LOD correlate_residual_x_n
ADD correlate_residual_x_k
LDI s_conv
P_LOD correlate_residual_x_k
LDI x_hat
SF_MLT
F_ADD correlate_residual_x_s
SET correlate_residual_x_s
LOD correlate_residual_x_k
ADD 1
SET correlate_residual_x_k
JMP Lwh18
@Lwh18end LOD correlate_residual_x_n
P_LOD correlate_residual_x_s
STI s_grad_h
LOD correlate_residual_x_n
ADD 1
SET correlate_residual_x_n
JMP Lwh17
@Lwh17end RET
@fista_x SET fista_x_dummy
LOD 0
SET fista_x_ii
@Lwh19 LOD 34
LES fista_x_ii
JIZ Lwh19end
LOD fista_x_ii
P_LOD fista_x_ii
LDI x_hat
STI s_z
LOD fista_x_ii
P_LOD fista_x_ii
LDI x_hat
STI s_x_prev
LOD fista_x_ii
ADD 1
SET fista_x_ii
JMP Lwh19
@Lwh19end LOD 1.0
SET fista_x_t
LOD 0
CAL lipschitz_x
SET fista_x_L
F_DIV 1.0
SET fista_x_step
SET fista_x_step_lam
LOD 0
SET fista_x_it
@Lwh20 LOD 20
LES fista_x_it
JIZ Lwh20end
LOD 0
CAL conv_zh
LOD 0
CAL residual_sub_y
LOD 0
CAL correlate_residual_h
LOD 0
SET fista_x_ii
@Lwh21 LOD 34
LES fista_x_ii
JIZ Lwh21end
LOD fista_x_ii
LDI s_z
P_LOD fista_x_ii
LDI s_grad_x
F_MLT fista_x_step
SF_SU2
F_SU1 fista_x_step_lam
SET fista_x_v
P_LOD 0.0
SF_LES
JIZ Lif6else
LOD 0.0
SET fista_x_v
@Lif6else LOD fista_x_ii
P_LOD fista_x_v
STI x_hat
LOD fista_x_ii
ADD 1
SET fista_x_ii
JMP Lwh21
@Lwh21end LOD 4.0
F_MLT fista_x_t
F_MLT fista_x_t
F_ADD 1.0
CAL float_sqrt
F_ADD 1.0
F_MLT 0.5
SET fista_x_tn
LOD fista_x_t
F_SU1 1.0
P_LOD fista_x_tn
SF_DIV
SET fista_x_mom
LOD 0
SET fista_x_ii
@Lwh22 LOD 34
LES fista_x_ii
JIZ Lwh22end
LOD fista_x_ii
LDI x_hat
SET fista_x_xi
LOD fista_x_ii
P_LOD fista_x_ii
LDI s_x_prev
F_SU2 fista_x_xi
F_MLT fista_x_mom
F_ADD fista_x_xi
STI s_z
LOD fista_x_ii
P_LOD fista_x_xi
STI s_x_prev
LOD fista_x_ii
ADD 1
SET fista_x_ii
JMP Lwh22
@Lwh22end LOD fista_x_tn
SET fista_x_t
LOD fista_x_it
ADD 1
SET fista_x_it
JMP Lwh20
@Lwh20end RET
@update_h SET update_h_dummy
LOD 0
CAL project_h_hat
LOD 0
SET update_h_it
@Lwh23 LOD 5
LES update_h_it
JIZ Lwh23end
LOD 0
CAL conv_xh
LOD 0
CAL residual_sub_y
LOD 0
CAL sum_residual_sq
F_MLT 0.5
SET update_h_dt
LOD 0
CAL correlate_residual_x
LOD 3
P_LOD 0.0
STI s_grad_h
LOD update_h_dt
SET update_h_best_dt
LOD 0
SET update_h_ii
@Lwh24 LOD 7
LES update_h_ii
JIZ Lwh24end
LOD update_h_ii
P_LOD update_h_ii
LDI h_hat
STI s_h_best
LOD update_h_ii
ADD 1
SET update_h_ii
JMP Lwh24
@Lwh24end LOD 0.00001
SET update_h_trial
LOD 0
SET update_h_bt
@Lwh25 LOD 15
LES update_h_bt
JIZ Lwh25end
LOD 0
SET update_h_ii
@Lwh26 LOD 7
LES update_h_ii
JIZ Lwh26end
LOD update_h_ii
P_LOD update_h_ii
LDI h_hat
P_LOD update_h_ii
LDI s_grad_h
F_MLT update_h_trial
SF_SU2
STI s_h_cand
LOD update_h_ii
ADD 1
SET update_h_ii
JMP Lwh26
@Lwh26end LOD 0
CAL project_h_cand
LOD 0
CAL conv_x_hcand
LOD 0
CAL residual_sub_y
LOD 0
CAL sum_residual_sq
F_MLT 0.5
SET update_h_cdt
P_LOD update_h_best_dt
SF_LES
JIZ Lif7else
LOD update_h_cdt
SET update_h_best_dt
LOD 0
SET update_h_ii
@Lwh27 LOD 7
LES update_h_ii
JIZ Lwh27end
LOD update_h_ii
P_LOD update_h_ii
LDI s_h_cand
STI s_h_best
LOD update_h_ii
ADD 1
SET update_h_ii
JMP Lwh27
@Lwh27end @Lif7else LOD update_h_trial
F_MLT 0.5
SET update_h_trial
LOD update_h_bt
ADD 1
SET update_h_bt
JMP Lwh25
@Lwh25end LOD 0
SET update_h_ii
@Lwh28 LOD 7
LES update_h_ii
JIZ Lwh28end
LOD update_h_ii
P_LOD update_h_ii
LDI s_h_best
STI h_hat
LOD update_h_ii
ADD 1
SET update_h_ii
JMP Lwh28
@Lwh28end LOD update_h_it
ADD 1
SET update_h_it
JMP Lwh23
@Lwh23end LOD 0
CAL project_h_hat
RET
@fit SET fit_dummy
LOD 0
SET fit_ii
@Lwh29 LOD 34
LES fit_ii
JIZ Lwh29end
LOD fit_ii
P_LOD 0.0
STI x_hat
LOD fit_ii
ADD 1
SET fit_ii
JMP Lwh29
@Lwh29end LOD 0
SET fit_ii
@Lwh30 LOD 7
LES fit_ii
JIZ Lwh30end
LOD fit_ii
P_LOD 0.0
STI h_hat
LOD fit_ii
ADD 1
SET fit_ii
JMP Lwh30
@Lwh30end LOD 2
P_LOD 0.5
STI h_hat
LOD 3
P_LOD 1.0
STI h_hat
LOD 4
P_LOD 0.5
STI h_hat
LOD 0
SET fit_it
@Lwh31 LOD 40
LES fit_it
JIZ Lwh31end
LOD 0
CAL fista_x
LOD 0
CAL update_h
LOD fit_it
ADD 1
SET fit_it
JMP Lwh31
@Lwh31end RET
@main LOD 0
SET main_n
@Lwh32 LOD 40
LES main_n
JIZ Lwh32end
LOD main_n
PF_INN 0
P_LOD 30000.0
SF_DIV
STI y
LOD main_n
ADD 1
SET main_n
JMP Lwh32
@Lwh32end LOD 0
CAL fit
LOD 0
SET main_j
@Lwh33 LOD 7
LES main_j
JIZ Lwh33end
LOD main_j
LDI h_hat
F_MLT 1000000.0
F_ADD 0.5
F2I
SET main_r
OUT 0
LOD main_j
ADD 1
SET main_j
JMP Lwh33
@Lwh33end @fim JMP fim

// sqrt function --------------------------------------------------------------

@float_sqrt     SET sqrt_num       // get the parameter
              F_ROT                // first estimate (nearest power of 2)

                PSH                // update x
              F_DIV sqrt_num       // iteration 1
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 2
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 3
             SF_ADD
              F_MLT 0.5

                PSH                // update x
              F_DIV sqrt_num       // iteration 4
             SF_ADD
              F_MLT 0.5

                RET
