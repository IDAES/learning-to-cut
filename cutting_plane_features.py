def feat_names():
    feat_names_dict = {}

    feat_names_dict[0] = "A_mean"
    feat_names_dict[1] = "A_max"
    feat_names_dict[2] = "A_min"
    feat_names_dict[3] = "A_std"

    feat_names_dict[4] = "c_mean"
    feat_names_dict[5] = "c_max"
    feat_names_dict[6] = "c_min"
    feat_names_dict[7] = "c_std"

    feat_names_dict[8] = "b_mean"
    feat_names_dict[9] = "b_max"
    feat_names_dict[10] = "b_min"
    feat_names_dict[11] = "b_std"

    feat_names_dict[12] = "var_bin_mean"
    feat_names_dict[13] = "var_bin_max"
    feat_names_dict[14] = "var_bin_min"
    feat_names_dict[15] = "var_bin_std"

    feat_names_dict[16] = "var_int_mean"
    feat_names_dict[17] = "var_int_max"
    feat_names_dict[18] = "var_int_min"
    feat_names_dict[19] = "var_int_std"

    feat_names_dict[20] = "var_implint_mean"
    feat_names_dict[21] = "var_implint_max"
    feat_names_dict[22] = "var_implint_min"
    feat_names_dict[23] = "var_implint_std"

    feat_names_dict[24] = "var_cts_mean"
    feat_names_dict[25] = "var_cts_max"
    feat_names_dict[26] = "var_cts_min"
    feat_names_dict[27] = "var_cts_std"
    
    ## Var basis status
    feat_names_dict[28] = "var_lower_mean"
    feat_names_dict[29] = "var_lower_max"
    feat_names_dict[30] = "var_lower_min"
    feat_names_dict[31] = "var_lower_std"

    feat_names_dict[32] = "var_basic_mean"
    feat_names_dict[33] = "var_basic_max"
    feat_names_dict[34] = "var_basic_min"
    feat_names_dict[35] = "var_basic_std"

    feat_names_dict[36] = "var_upper_mean"
    feat_names_dict[37] = "var_upper_max"
    feat_names_dict[38] = "var_upper_min"
    feat_names_dict[39] = "var_upper_std"

    feat_names_dict[40] = "var_zero_mean"
    feat_names_dict[41] = "var_zero_max"
    feat_names_dict[42] = "var_zero_min"
    feat_names_dict[43] = "var_zero_std"

    feat_names_dict[44] = "var_has_lb_mean"
    feat_names_dict[45] = "var_has_lb_max"
    feat_names_dict[46] = "var_has_lb_min"
    feat_names_dict[47] = "var_has_lb_std"

    feat_names_dict[48] = "var_has_ub_mean"
    feat_names_dict[49] = "var_has_ub_max"
    feat_names_dict[50] = "var_has_ub_min"
    feat_names_dict[51] = "var_has_ub_std"  

    feat_names_dict[52] = "var_has_rc_mean"
    feat_names_dict[53] = "var_has_rc_max"
    feat_names_dict[54] = "var_has_rc_min"
    feat_names_dict[55] = "var_has_rc_std"

    feat_names_dict[56] = "var_sol_mean"
    feat_names_dict[57] = "var_sol_max"
    feat_names_dict[58] = "var_sol_min"
    feat_names_dict[59] = "var_sol_std"

    feat_names_dict[60] = "var_sol_frac_mean"
    feat_names_dict[61] = "var_sol_frac_max"
    feat_names_dict[62] = "var_sol_frac_min"
    feat_names_dict[63] = "var_sol_frac_std"

    feat_names_dict[64] = "var_at_lb_mean"
    feat_names_dict[65] = "var_at_lb_max"
    feat_names_dict[66] = "var_at_lb_min"
    feat_names_dict[67] = "var_at_lb_std"

    feat_names_dict[68] = "var_at_ub_mean"
    feat_names_dict[69] = "var_at_ub_max"
    feat_names_dict[70] = "var_at_ub_min"
    feat_names_dict[71] = "var_at_ub_std"

    ## Constraint features (11 * 4)

    feat_names_dict[72] = "cons_eff_mean"
    feat_names_dict[73] = "cons_eff_max"
    feat_names_dict[74] = "cons_eff_min"
    feat_names_dict[75] = "cons_eff_std"

    feat_names_dict[76] = "cons_rank_mean"
    feat_names_dict[77] = "cons_rank_max"
    feat_names_dict[78] = "cons_rank_min"
    feat_names_dict[79] = "cons_rank_std" 

    feat_names_dict[80] = "cons_int_sup_mean"
    feat_names_dict[81] = "cons_int_sup_max"
    feat_names_dict[82] = "cons_int_sup_min"
    feat_names_dict[83] = "cons_int_sup_std" 

    feat_names_dict[84] = "cons_sup_mean"
    feat_names_dict[85] = "cons_sup_max"
    feat_names_dict[86] = "cons_sup_min"
    feat_names_dict[87] = "cons_sup_std" 

    feat_names_dict[88] = "cons_par_mean"
    feat_names_dict[89] = "cons_par_max"
    feat_names_dict[90] = "cons_par_min"
    feat_names_dict[91] = "cons_par_std"

    feat_names_dict[92] = "cons_exp_imp_mean"
    feat_names_dict[93] = "cons_exp_imp_max"
    feat_names_dict[94] = "cons_exp_imp_min"
    feat_names_dict[95] = "cons_exp_imp_std" 

    feat_names_dict[96] = "cons_viol_mean"
    feat_names_dict[97] = "cons_viol_max"
    feat_names_dict[98] = "cons_viol_min"
    feat_names_dict[99] = "cons_viol_std"

    feat_names_dict[100] = "cons_relative_viol_mean"
    feat_names_dict[101] = "cons_relative_viol_max"
    feat_names_dict[102] = "cons_relative_viol_min"
    feat_names_dict[103] = "cons_relative_viol_std"  

    feat_names_dict[104] = "cons_removable_mean"
    feat_names_dict[105] = "cons_removable_max"
    feat_names_dict[106] = "cons_removable_min"
    feat_names_dict[107] = "cons_removable_std" 

    feat_names_dict[108] = "cons_integ_mean"
    feat_names_dict[109] = "cons_integ_max"
    feat_names_dict[110] = "cons_integ_min"
    feat_names_dict[111] = "cons_integ_std"   

    feat_names_dict[112] = "cons_dual_mean"
    feat_names_dict[113] = "cons_dual_max"
    feat_names_dict[114] = "cons_dual_min"
    feat_names_dict[115] = "cons_dual_std"
    
    feat_names_dict[116] = "cut_rank"
    feat_names_dict[117] = "cut_sup_score"
    feat_names_dict[118] = "cut_int_sup_score"
    feat_names_dict[119] = "cut_bias"
    feat_names_dict[120] = "cut_activity"
    feat_names_dict[121] = "cut_integral"
    feat_names_dict[122] = "cut_abs_viol"
    feat_names_dict[123] = "cut_rel_viol"
    feat_names_dict[124] = "cut_eff"
    feat_names_dict[125] = "cut_obj_paral"
    feat_names_dict[126] = "cut_exp_imp"

    feat_names_dict[127] = "cut_coefs_mean"
    feat_names_dict[128] = "cut_coefs_max"
    feat_names_dict[129] = "cut_coefs_min"
    feat_names_dict[130] = "cut_coefs_std"

    feat_names_dict[131] = "cut_cons_paral_mean"
    feat_names_dict[132] = "cut_cons_paral_max"
    feat_names_dict[133] = "cut_cons_paral_min"
    feat_names_dict[134] = "cut_cons_paral_std"

    return feat_names_dict
