#include "scip/scip.h"
#include "scip/struct_lp.h"

#include "helper_functions.h"

int is_prim_sol_at_lb(SCIP* scip, SCIP_COL* col) {
	double lb_val = SCIPcolGetLb(col);
	if (!SCIPisInfinity(scip, fabs(lb_val))) {
		return SCIPisEQ(scip, SCIPcolGetPrimsol(col), lb_val);
	}
	return 0;
}

int is_prim_sol_at_ub(SCIP* scip, SCIP_COL* col) {
	double ub_val = SCIPcolGetUb(col);
	if (!SCIPisInfinity(scip, fabs(ub_val))) {
		return SCIPisEQ(scip, SCIPcolGetPrimsol(col), ub_val);
	}
	return 0;
}


SCIP_Real safe_div(SCIP_Real x, SCIP_Real y) {
   if (fabs(y) <= TOL)
      return 0;
   else
      return x / y ;
}

SCIP_Real square(SCIP_Real x) { 
   return x * x; 
}

void initialize_statistics(SCIP* scip, Statistics *stats) {
    stats->sum = 0.0;
    stats->sum_sq = 0.0;
    stats->min = SCIPinfinity(scip);
    stats->max = -1 * SCIPinfinity(scip);
    stats->count = 0.0;
}

void update_statistics(Statistics *stats, SCIP_Real value) {
    stats->sum += value;
    stats->sum_sq += value * value;
    stats->min = MIN(value, stats->min);
    stats->max = MAX(value, stats->max);
    stats->count++;
}

void calculate_mean_std(Statistics *stats) {

    if ( stats->count == 0){
        stats->mean = 0.0;
        stats->std = 0.0;
        stats->max = 0.0;
        stats->min = 0.0;
    }
    else {
        stats->mean = stats->sum / (SCIP_Real) stats->count;
        stats->std = stats->sum_sq / (SCIP_Real) stats->count - stats->mean * stats->mean;
        stats->std = sqrt(stats->std);
    }
}

void copy_stats_to_features(Statistics *stats, SCIP_Real* feat_arr) {
    
    feat_arr[0] = stats->mean;
    feat_arr[1] = stats->max;
    feat_arr[2] = stats->min;
    feat_arr[3] = stats->std;

}

void var_stats_binomial(int nvars, int ncols, SCIP_Real* var_type)
{

    assert( ncols > 0);
    assert( nvars <= ncols);

    var_type[0] = (SCIP_Real) nvars / (SCIP_Real) ncols;
    var_type[1] = nvars > 0 ? 1 : 0; //max
    var_type[2] = 0; //min
    var_type[3] = (SCIP_Real) nvars * ( 1 - var_type[0]); //n*p*(1-p), variance
    var_type[3] = sqrt(var_type[3]); //std

}


int getNFixedColumns(SCIP* scip) {
   SCIP_COL** cols;
   SCIP_COL* col;
   int ncols, i;
   int nfixed = 0;

   SCIP_Real lb,ub;

   cols = SCIPgetLPCols(scip);
   ncols = SCIPgetNLPCols(scip);

   for(i = 0; i < ncols; i++) {
      col = cols[i];

      lb = SCIPcolGetLb(col);
      ub = SCIPcolGetUb(col);

      if ( SCIPisFeasEQ(scip, lb, ub) )
         nfixed ++;
   }
   return nfixed;
}

int getNFixedColumnsRow(SCIP* scip, SCIP_ROW* row) {

   SCIP_COL* row_col;
   int i;
   int nfixed = 0;
   SCIP_Real lb, ub;

   for(i = 0; i < row->len; i++) {
      row_col = row->cols[i];

      lb = SCIPcolGetLb(row_col);
      ub = SCIPcolGetUb(row_col);

    if ( SCIPisFeasEQ(scip, lb, ub) )
    {
        nfixed ++;
    }
   }
   return nfixed;
}

void statsAmatrix(SCIP* scip, SCIP_Real* statsA) {

    SCIP_ROW** rows;
    SCIP_ROW* row;

    SCIP_Real* row_vals;

    SCIP_Real rhs, lhs;

    int i, j, nrows, ncols;

    rows = SCIPgetLPRows(scip);
    nrows = SCIPgetNLPRows(scip);

    SCIP_Real row_norm;

    Statistics a_stats;

    initialize_statistics(scip, &a_stats);
    
    for(i = 0; i < nrows; i++) {
        row = rows[i];

        row_norm = SCIProwGetNorm(row);

        if ( row_norm == 0.0 )
        {
            for( j = 0; j < row->len; j++ )
                if( row->cols[j]->lppos >= 0 )
                row->sqrnorm += SQR(row->vals[j]);
            assert(SCIProwGetNorm(row) != 0.0);
            row_norm = SCIProwGetNorm(row);
        }

        ncols = SCIProwGetNLPNonz(row);
        assert( ncols == SCIProwGetNNonz(row) );

        row_vals = SCIProwGetVals(row);

        rhs = SCIProwGetRhs(row);
        lhs = SCIProwGetLhs(row);
        
        if ( !SCIPisInfinity(scip, fabs(rhs)) ) {
            for(j = 0; j < ncols; j++) {
                update_statistics(&a_stats, row_vals[j] / row_norm);
            }
        }
        if ( !SCIPisInfinity(scip, fabs(lhs)) ) {
            for(j = 0; j < ncols; j++) {
                update_statistics(&a_stats, -1 * row_vals[j] / row_norm);
            }
        }

        
    }

    calculate_mean_std(&a_stats);

    copy_stats_to_features(&a_stats, statsA);

}

void statsb(SCIP* scip, SCIP_Real * stats_b_arr) {

    SCIP_ROW** rows;
    SCIP_ROW* row;

    int i, j, nrows;

    rows = SCIPgetLPRows(scip);
    nrows = SCIPgetNLPRows(scip);

    SCIP_Real rhs, lhs, row_constant, val, row_norm;

    Statistics b_stats;

    initialize_statistics(scip, &b_stats);

    for(i = 0; i < nrows; i++) {
        row = rows[i];

        row_norm = SCIProwGetNorm(row);

        if ( row_norm == 0.0 )
        {
            for( j = 0; j < row->len; j++ ) {
                if( row->cols[j]->lppos >= 0 )
                    row->sqrnorm += SQR(row->vals[j]);
            }

            assert(SCIProwGetNorm(row) != 0.0);

            row_norm = SCIProwGetNorm(row);
        }

        row_constant = SCIProwGetConstant(row);

        rhs = SCIProwGetRhs(row) - row_constant;
        lhs = SCIProwGetLhs(row) - row_constant;

        if ( !SCIPisInfinity(scip, fabs(rhs)) ) {
            val = (rhs - row_constant) / row_norm;
            update_statistics(&b_stats, val);
        }
        if ( !SCIPisInfinity(scip, fabs(lhs)) ) {
            val = - (lhs - row_constant) / row_norm;
            update_statistics(&b_stats, val);
        }
    }

    calculate_mean_std(&b_stats);

    copy_stats_to_features(&b_stats, stats_b_arr);

}

void statsc(SCIP* scip, SCIP_Real* stats_c_arr) {
    SCIP_COL** lpcols;
    SCIP_COL* lpcol;
    int i, ncols;
    SCIP_Real lb, ub, obj, obj_norm;

    Statistics c_stats;

    initialize_statistics(scip, &c_stats);

    lpcols = SCIPgetLPCols(scip);
    ncols = SCIPgetNLPCols(scip);

    obj_norm = SCIPgetObjNorm(scip);

    for(i = 0; i < ncols; i++) {
        lpcol = lpcols[i];
        lb = SCIPcolGetLb(lpcol);
        ub = SCIPcolGetUb(lpcol);
        obj = safe_div(SCIPcolGetObj(lpcol), obj_norm);

        if( !SCIPisFeasEQ(scip, lb, ub) ) {
            update_statistics(&c_stats, obj);
        }
    }

    calculate_mean_std(&c_stats);

    copy_stats_to_features(&c_stats, stats_c_arr);
}

void stats_vars(SCIP* scip, SCIP_Real* stats_vars_arr) {
    /* 60 feats */
    SCIP_COL** cols;
    SCIP_COL* col;
    SCIP_VAR* var;
    int i, ncols;

    int var_bin_ctr = 0;
    int var_int_ctr = 0;
    int var_implint_ctr = 0;
    int var_cts_ctr = 0;

    int var_lower_ctr = 0;
    int var_basic_ctr = 0;
    int var_upper_ctr = 0;
    int var_zero_ctr = 0;

    SCIP_Real obj_norm, lb, ub, sol;

    obj_norm = SCIPgetObjNorm(scip);

    Statistics lb_stats, ub_stats, redcost_stats, sol_stats, sol_frac_stats, sol_at_lb_stats, sol_at_ub_stats;

    initialize_statistics(scip, &lb_stats);
    initialize_statistics(scip, &ub_stats);
    initialize_statistics(scip, &redcost_stats);
    initialize_statistics(scip, &sol_stats);
    initialize_statistics(scip, &sol_frac_stats);
    initialize_statistics(scip, &sol_at_lb_stats);
    initialize_statistics(scip, &sol_at_ub_stats);

    cols = SCIPgetLPCols(scip);
    ncols = SCIPgetNLPCols(scip);

    for(i = 0; i < ncols; i++) {

        col = cols[i];
        var = SCIPcolGetVar(col);

        switch (SCIPvarGetType(var)) {
	        case SCIP_VARTYPE_BINARY:
                var_bin_ctr ++;
                break;
	        case SCIP_VARTYPE_INTEGER:
                var_int_ctr ++;
                break;
            case SCIP_VARTYPE_IMPLINT:
                var_implint_ctr ++;
                break;
            case SCIP_VARTYPE_CONTINUOUS:
                var_cts_ctr ++;
                break;
            default:
                break;
        }

        switch (SCIPcolGetBasisStatus(col)) {
	        case SCIP_BASESTAT_LOWER:
		        var_lower_ctr ++;
		        break;
            case SCIP_BASESTAT_BASIC:
                var_basic_ctr ++;
                break;
            case SCIP_BASESTAT_UPPER:
                var_upper_ctr ++;
                break;
            case SCIP_BASESTAT_ZERO: // For columns not in LP, and for free variables that are nonbasic and set to 0
                var_zero_ctr ++;
                break; 
            default:
                break;
	    }

        lb = SCIPcolGetLb(col);
        ub = SCIPcolGetUb(col);

        if ( !SCIPisInfinity(scip, lb) )
            update_statistics(&lb_stats, 1.0);
        else 
            update_statistics(&lb_stats, 0.0);

        if ( !SCIPisInfinity(scip, ub) )
            update_statistics(&ub_stats, 1.0);
        else
            update_statistics(&ub_stats, 0.0);

        update_statistics(&redcost_stats, SCIPgetColRedcost(scip, col) / obj_norm);

        sol = SCIPcolGetPrimsol(col);

        update_statistics(&sol_stats, sol);

        update_statistics(&sol_frac_stats, SCIPfeasFrac(scip, sol));

        if (is_prim_sol_at_lb(scip, col)) {
            update_statistics(&sol_at_lb_stats, 1.0);
        }
        else
            update_statistics(&sol_at_lb_stats, 0.0);

        if (is_prim_sol_at_ub(scip, col)) {
            update_statistics(&sol_at_ub_stats, 1.0);
        }
        else
            update_statistics(&sol_at_ub_stats, 0.0);
        
    }

    calculate_mean_std(&lb_stats);
    calculate_mean_std(&ub_stats);
    calculate_mean_std(&redcost_stats);
    calculate_mean_std(&sol_stats);
    calculate_mean_std(&sol_frac_stats);
    calculate_mean_std(&sol_at_lb_stats);
    calculate_mean_std(&sol_at_ub_stats);

    var_stats_binomial(var_bin_ctr, ncols, stats_vars_arr);
    var_stats_binomial(var_int_ctr, ncols, &stats_vars_arr[4]);
    var_stats_binomial(var_implint_ctr, ncols, &stats_vars_arr[8]);
    var_stats_binomial(var_cts_ctr, ncols, &stats_vars_arr[12]);

    var_stats_binomial(var_lower_ctr, ncols, &stats_vars_arr[16]);
    var_stats_binomial(var_basic_ctr, ncols, &stats_vars_arr[20]);
    var_stats_binomial(var_upper_ctr, ncols, &stats_vars_arr[24]);
    var_stats_binomial(var_zero_ctr, ncols, &stats_vars_arr[28]);

    copy_stats_to_features(&lb_stats, &stats_vars_arr[32]);
    copy_stats_to_features(&ub_stats, &stats_vars_arr[36]);
    copy_stats_to_features(&redcost_stats, &stats_vars_arr[40]);
    copy_stats_to_features(&sol_stats, &stats_vars_arr[44]);
    copy_stats_to_features(&sol_frac_stats, &stats_vars_arr[48]);
    copy_stats_to_features(&sol_at_lb_stats, &stats_vars_arr[52]);
    copy_stats_to_features(&sol_at_ub_stats, &stats_vars_arr[56]);


} 

void stats_constraints(SCIP* scip, SCIP_Real* stats_cons_arr) {
    /* Efficacy, rank, integral support, support */
    SCIP_ROW** rows;
    SCIP_ROW* row;

    int i, nrows;
    SCIP_Real obj_norm, eff, rank, int_sup, sup, par, exp_imp, viol, relative_viol, removable, integ, dual;
    SCIP_Real rhs;

    obj_norm = SCIPgetObjNorm(scip);

    rows = SCIPgetLPRows(scip);
    nrows = SCIPgetNLPRows(scip);

    Statistics eff_stats, rank_stats, int_sup_stats, sup_stats, par_stats, exp_imp_stats, viol_stats, relative_viol_stats, removable_stats, integ_stats, dual_stats;

    initialize_statistics(scip, &eff_stats);
    initialize_statistics(scip, &rank_stats);
    initialize_statistics(scip, &int_sup_stats);
    initialize_statistics(scip, &sup_stats);
    initialize_statistics(scip, &par_stats);
    initialize_statistics(scip, &exp_imp_stats);
    initialize_statistics(scip, &viol_stats);
    initialize_statistics(scip, &relative_viol_stats);
    initialize_statistics(scip, &removable_stats);
    initialize_statistics(scip, &integ_stats);
    initialize_statistics(scip, &dual_stats);

    SCIP_Real row_norm;
    int j;

    for(i = 0; i < nrows; i++) {
        row = rows[i];

        row_norm = SCIProwGetNorm(row);

        if ( row_norm == 0.0 )
        {
            for( j = 0; j < row->len; j++ ) {
                if( row->cols[j]->lppos >= 0 )
                    row->sqrnorm += SQR(row->vals[j]);
            }

            assert(SCIProwGetNorm(row) != 0.0);

            row_norm = SCIProwGetNorm(row);
        }

        rhs = SCIProwGetRhs(row);

        eff = SCIPgetCutEfficacy(scip, NULL, row);
        rank = (SCIP_Real) SCIProwGetRank(row);
        int_sup = (SCIP_Real) SCIPgetRowNumIntCols(scip, row) / (SCIP_Real) SCIProwGetNNonz(row);
        sup = (SCIP_Real) SCIProwGetNNonz(row) / (SCIP_Real) SCIPgetNLPCols(scip);
        par = SCIPgetRowObjParallelism(scip, row);
        exp_imp = obj_norm * par * eff;
        viol = -1 * SCIPgetRowLPFeasibility(scip, row);
        relative_viol = rhs == 0 ? -1 * SCIPgetRowLPFeasibility(scip, row) : -1 * SCIPgetRowLPFeasibility(scip, row) / rhs;
        removable = (SCIP_Real) SCIProwIsRemovable(row);
        integ = (SCIP_Real) SCIProwIsIntegral(row);
        dual = SCIProwGetDualsol(row) / obj_norm * row_norm;

        update_statistics(&eff_stats, eff);
        update_statistics(&rank_stats, rank);
        update_statistics(&int_sup_stats, int_sup);
        update_statistics(&sup_stats, sup);
        update_statistics(&par_stats, par);
        update_statistics(&exp_imp_stats, exp_imp);
        update_statistics(&viol_stats, viol);
        update_statistics(&relative_viol_stats, relative_viol);
        update_statistics(&removable_stats, removable);
        update_statistics(&integ_stats, integ);
        update_statistics(&dual_stats, dual);

    }


    calculate_mean_std(&eff_stats);
    calculate_mean_std(&rank_stats);
    calculate_mean_std(&int_sup_stats);
    calculate_mean_std(&sup_stats);
    calculate_mean_std(&par_stats);
    calculate_mean_std(&exp_imp_stats);
    calculate_mean_std(&viol_stats);
    calculate_mean_std(&relative_viol_stats);
    calculate_mean_std(&removable_stats);
    calculate_mean_std(&integ_stats);
    calculate_mean_std(&dual_stats);

    copy_stats_to_features(&eff_stats, stats_cons_arr);
    copy_stats_to_features(&rank_stats, &stats_cons_arr[4]);
    copy_stats_to_features(&int_sup_stats, &stats_cons_arr[8]);
    copy_stats_to_features(&sup_stats, &stats_cons_arr[12]);
    copy_stats_to_features(&par_stats, &stats_cons_arr[16]);
    copy_stats_to_features(&exp_imp_stats, &stats_cons_arr[20]);
    copy_stats_to_features(&viol_stats, &stats_cons_arr[24]);
    copy_stats_to_features(&relative_viol_stats, &stats_cons_arr[28]);
    copy_stats_to_features(&removable_stats, &stats_cons_arr[32]);
    copy_stats_to_features(&integ_stats, &stats_cons_arr[36]);
    copy_stats_to_features(&dual_stats, &stats_cons_arr[40]);

}

void stats_cut_coefs(SCIP* scip, SCIP_ROW* cut, SCIP_Real* cut_coefs) {

    int i;
    SCIP_Real cut_norm = SCIProwGetNorm(cut);

    if ( cut_norm == 0.0 )
    {
        for( i = 0; i < cut->len; i++ )
            if( cut->cols[i]->lppos >= 0 )
               cut->sqrnorm += SQR(cut->vals[i]);
        assert(SCIProwGetNorm(cut) != 0.0);
        cut_norm = SCIProwGetNorm(cut);
    }

    

    Statistics coef_stats;
    initialize_statistics(scip, &coef_stats);

    for (i = 0; i < cut->len; i++) {

        assert( SCIPcolIsInLP(cut->cols[i]) );

        update_statistics(&coef_stats, cut->vals[i] / cut_norm);

    }

    calculate_mean_std(&coef_stats);

    copy_stats_to_features(&coef_stats, cut_coefs);


}

void stats_cut_parallelism(SCIP* scip, SCIP_ROW* cut, SCIP_Real* cut_parallelism) {
    int i, nrows;
    SCIP_ROW** rows;
    SCIP_Real val;

    nrows = SCIPgetNLPRows(scip);
    rows = SCIPgetLPRows(scip);

    Statistics cut_par;
    Statistics* cut_par_ptr = &cut_par;

    initialize_statistics(scip, cut_par_ptr);

    for(i = 0; i < nrows; i++) {
        val = SCIProwGetParallelism(cut, rows[i], 'e');

        update_statistics(cut_par_ptr, val);
    }

    cut_parallelism[0] = cut_par_ptr->sum / (SCIP_Real) nrows;
    cut_parallelism[1] = cut_par_ptr->max;
    cut_parallelism[2] = cut_par_ptr->min;
    cut_parallelism[3] = (cut_par_ptr->sum_sq - cut_parallelism[0] * cut_parallelism[0]) /  (SCIP_Real) nrows;

}
