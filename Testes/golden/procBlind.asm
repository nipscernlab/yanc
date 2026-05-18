NOP
#PRNAME procBlind
#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NDSTAC 10
#SDEPTH 10
#NUIOIN 1
#NUIOOU 4
#arrays y 2 1006 "y.txt"
#array x_hat 2 1000
#array h_hat 2 7
#array s_z 2 1000
#array s_x_prev 2 1000
#array s_grad_x 2 1000
#array s_conv 2 1006
#array s_h_cand 2 7
#array s_grad_h 2 7
#arrays lambda_grid 2 12 "lambda_mult.txt"
JMP main
@project_h_hat SET project_h_hat_dummy
LOD 0
SET project_h_hat_ii
@Lwh1 LOD 7
LES project_h_hat_ii
JIZ Lwh1end
LOD project_h_hat_ii
LDI h_hat
P_LOD 0.0
SF_LES
JIZ Lif1else
LOD project_h_hat_ii
P_LOD 0.0
STI h_hat
@Lif1else LOD project_h_hat_ii
ADD 1
SET project_h_hat_ii
JMP Lwh1
@Lwh1end LOD 3
P_LOD 1.0
STI h_hat
RET
@project_h_cand SET project_h_cand_dummy
LOD 0
SET project_h_cand_ii
@Lwh2 LOD 7
LES project_h_cand_ii
JIZ Lwh2end
LOD project_h_cand_ii
LDI s_h_cand
P_LOD 0.0
SF_LES
JIZ Lif2else
LOD project_h_cand_ii
P_LOD 0.0
STI s_h_cand
@Lif2else LOD project_h_cand_ii
ADD 1
SET project_h_cand_ii
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
JIZ Lif3else
LOD 0.000000000001
SET lipschitz_x_L
@Lif3else LOD lipschitz_x_L
RET
@conv_zh SET conv_zh_dummy
LOD 0
SET conv_zh_n
@Lwh4 LOD 1006
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
@Lwh5 LOD 1000
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
@Lwh7 LOD 1006
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
@Lwh8 LOD 1000
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
@Lwh10 LOD 1006
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
@Lwh11 LOD 1000
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
@Lwh13 LOD 1006
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
@Lwh14 LOD 1006
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
@Lwh15 LOD 1000
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
@Lwh18 LOD 1000
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
@fista_x SET fista_x_lam
LOD 0
SET fista_x_ii
@Lwh19 LOD 1000
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
F_MLT fista_x_lam
SET fista_x_step_lam
LOD 0
SET fista_x_have_prev
LOD 0.0
SET fista_x_prev_obj
LOD 0
SET fista_x_it
@Lwh20 LOD 600
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
@Lwh21 LOD 1000
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
JIZ Lif4else
LOD 0.0
SET fista_x_v
@Lif4else LOD fista_x_ii
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
@Lwh22 LOD 1000
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
@Lwh22end LOD 0
CAL conv_xh
LOD 0
CAL residual_sub_y
LOD 0
CAL sum_residual_sq
SET fista_x_rss2
LOD 0.5
F_MLT fista_x_rss2
SET fista_x_data
LOD 0.0
SET fista_x_sx
LOD 0
SET fista_x_ii
@Lwh23 LOD 1000
LES fista_x_ii
JIZ Lwh23end
LOD fista_x_ii
LDI x_hat
F_ADD fista_x_sx
SET fista_x_sx
LOD fista_x_ii
ADD 1
SET fista_x_ii
JMP Lwh23
@Lwh23end LOD fista_x_lam
F_MLT fista_x_sx
F_ADD fista_x_data
SET fista_x_obj
LOD 1
EQU fista_x_have_prev
JIZ Lif5else
F_ABS_M fista_x_prev_obj
SET fista_x_denom
P_LOD 1.0
SF_LES
JIZ Lif6else
LOD 1.0
SET fista_x_denom
@Lif6else LOD fista_x_prev_obj
F_SU1 fista_x_obj
F_ABS
SET fista_x_diff
LOD fista_x_denom
F_DIV fista_x_diff
P_LOD 0.00001
SF_LES
JIZ Lif7else
RET
@Lif7else @Lif5else LOD fista_x_obj
SET fista_x_prev_obj
LOD 1
SET fista_x_have_prev
LOD fista_x_tn
SET fista_x_t
LOD fista_x_it
ADD 1
SET fista_x_it
JMP Lwh20
@Lwh20end RET
@update_h SET update_h_dummy
LOD 0
CAL project_h_hat
LOD 0.00001
SET update_h_step
LOD 0
SET update_h_it
@Lwh24 LOD 80
LES update_h_it
JIZ Lwh24end
LOD 0
CAL conv_xh
LOD 0
CAL residual_sub_y
LOD 0
CAL sum_residual_sq
SET update_h_rss2
LOD 0.5
F_MLT update_h_rss2
SET update_h_dt
LOD 0
CAL correlate_residual_x
LOD 3
P_LOD 0.0
STI s_grad_h
LOD update_h_step
SET update_h_trial
LOD 0
SET update_h_accepted
LOD 0
SET update_h_bt
@Lwh25 LOD 30
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
SET update_h_crss2
LOD 0.5
F_MLT update_h_crss2
SET update_h_cdt
P_LOD update_h_dt
SF_GRE
LIN
JIZ Lif8else
LOD 0
SET update_h_ii
@Lwh27 LOD 7
LES update_h_ii
JIZ Lwh27end
LOD update_h_ii
P_LOD update_h_ii
LDI s_h_cand
STI h_hat
LOD update_h_ii
ADD 1
SET update_h_ii
JMP Lwh27
@Lwh27end LOD update_h_trial
F_MLT 1.05
SET update_h_step
P_LOD 0.00001
SF_GRE
JIZ Lif9else
LOD 0.00001
SET update_h_step
@Lif9else LOD 1
SET update_h_accepted
JMP Lwh25end
@Lif8else LOD update_h_trial
F_MLT 0.5
SET update_h_trial
LOD update_h_bt
ADD 1
SET update_h_bt
JMP Lwh25
@Lwh25end LOD 0
EQU update_h_accepted
JIZ Lif10else
JMP Lwh24end
@Lif10else LOD update_h_it
ADD 1
SET update_h_it
JMP Lwh24
@Lwh24end LOD 0
CAL project_h_hat
RET
@fit_one SET fit_one_lam
LOD 0
SET fit_one_ii
@Lwh28 LOD 1000
LES fit_one_ii
JIZ Lwh28end
LOD fit_one_ii
P_LOD 0.0
STI x_hat
LOD fit_one_ii
ADD 1
SET fit_one_ii
JMP Lwh28
@Lwh28end LOD 0
SET fit_one_ii
@Lwh29 LOD 7
LES fit_one_ii
JIZ Lwh29end
LOD fit_one_ii
P_LOD 0.0
STI h_hat
LOD fit_one_ii
ADD 1
SET fit_one_ii
JMP Lwh29
@Lwh29end LOD 3
P_LOD 1.0
STI h_hat
LOD 0
SET fit_one_have_prev
LOD 0.0
SET fit_one_prev_obj
LOD 0.0
SET fit_one_dt
LOD 0
SET fit_one_it
@Lwh30 LOD 120
LES fit_one_it
JIZ Lwh30end
LOD fit_one_lam
CAL fista_x
LOD 0
CAL update_h
LOD 0
CAL conv_xh
LOD 0
CAL residual_sub_y
LOD 0
CAL sum_residual_sq
SET fit_one_rss2
LOD 0.5
F_MLT fit_one_rss2
SET fit_one_dt
LOD 0.0
SET fit_one_sx
LOD 0
SET fit_one_ii
@Lwh31 LOD 1000
LES fit_one_ii
JIZ Lwh31end
LOD fit_one_ii
LDI x_hat
F_ADD fit_one_sx
SET fit_one_sx
LOD fit_one_ii
ADD 1
SET fit_one_ii
JMP Lwh31
@Lwh31end LOD fit_one_lam
F_MLT fit_one_sx
F_ADD fit_one_dt
SET fit_one_obj
LOD 1
EQU fit_one_have_prev
JIZ Lif11else
F_ABS_M fit_one_prev_obj
SET fit_one_denom
P_LOD 1.0
SF_LES
JIZ Lif12else
LOD 1.0
SET fit_one_denom
@Lif12else LOD fit_one_prev_obj
F_SU1 fit_one_obj
F_ABS
SET fit_one_diff
LOD fit_one_denom
F_DIV fit_one_diff
P_LOD 0.000001
SF_LES
JIZ Lif13else
LOD fit_one_dt
RET
@Lif13else @Lif11else LOD fit_one_obj
SET fit_one_prev_obj
LOD 1
SET fit_one_have_prev
LOD fit_one_it
ADD 1
SET fit_one_it
JMP Lwh30
@Lwh30end LOD fit_one_dt
RET
@main LOD 1.0
SET main_sigma
F_MLT main_sigma
SET main_noise_var
LOD 6.913737
SET main_log_N
LOD 0
SET main_have_best
LOD 0.0
SET main_best_bic
LOD 0
LDI lambda_grid
F_MLT main_sigma
SET main_best_lam
LOD 0
SET main_li
@Lwh32 LOD 12
LES main_li
JIZ Lwh32end
LOD main_li
LDI lambda_grid
F_MLT main_sigma
SET main_lam
CAL fit_one
SET main_dt
LOD 2.0
F_MLT main_dt
SET main_rss
LOD 0
SET main_active
LOD 0
SET main_k
@Lwh33 LOD 1000
LES main_k
JIZ Lwh33end
LOD main_k
LDI x_hat
P_LOD 0.001
SF_GRE
JIZ Lif14else
LOD main_active
ADD 1
SET main_active
@Lif14else LOD main_k
ADD 1
SET main_k
JMP Lwh33
@Lwh33end I2F_M main_active
F_ADD 6.0
SET main_num_params
LOD main_noise_var
F_DIV main_rss
P_LOD main_num_params
F_MLT main_log_N
SF_ADD
SET main_bic
LOD 0
EQU main_have_best
JIZ Lif15else
LOD main_bic
SET main_best_bic
LOD main_lam
SET main_best_lam
LOD 1
SET main_have_best
JMP Lif15end
@Lif15else LOD main_bic
P_LOD main_best_bic
SF_LES
JIZ Lif16else
LOD main_bic
SET main_best_bic
LOD main_lam
SET main_best_lam
@Lif16else @Lif15end LOD main_li
ADD 1
SET main_li
JMP Lwh32
@Lwh32end LOD main_best_lam
CAL fit_one
LOD main_best_lam
SET selected_lambda
F2I_M selected_lambda
OUT 0
LOD 0
SET main_j
@Lwh34 LOD 7
LES main_j
JIZ Lwh34end
LOD main_j
LDI h_hat
F2I
OUT 1
LOD main_j
ADD 1
SET main_j
JMP Lwh34
@Lwh34end LOD 0
SET main_j
@Lwh35 LOD 1000
LES main_j
JIZ Lwh35end
LOD main_j
LDI x_hat
F2I
OUT 2
LOD main_j
ADD 1
SET main_j
JMP Lwh35
@Lwh35end @fim JMP fim

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
