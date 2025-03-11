#ifndef __HELPER_FUNCTIONS_H__
#define __HELPER_FUNCTIONS_H__

#define TOL 0.1

int is_prim_sol_at_lb(SCIP* scip, SCIP_COL* col);

int is_prim_sol_at_ub(SCIP* scip, SCIP_COL* col);

SCIP_Real safe_div(SCIP_Real x, SCIP_Real y);

SCIP_Real square(SCIP_Real x);

typedef struct {
    SCIP_Real sum;
    SCIP_Real sum_sq;
    SCIP_Real min;
    SCIP_Real max;
    SCIP_Real mean;
    SCIP_Real std;
    int count;
} Statistics;

void initialize_statistics(SCIP* scip, Statistics *stats);

void update_statistics(Statistics *stats, SCIP_Real value);

void calculate_mean_std(Statistics *stats);

void copy_stats_to_features(Statistics *stats, SCIP_Real* feat_arr);

void var_stats_binomial(int nvars, int ncols, SCIP_Real* var_type);

int getNFixedColumns(SCIP* scip);

int getNFixedColumnsRow(SCIP* scip, SCIP_ROW* row);

void statsAmatrix(SCIP* scip, SCIP_Real* statsA);

void statsb(SCIP* scip, SCIP_Real* stats_b_arr);

void statsc(SCIP* scip, SCIP_Real* stats_c_arr);

void stats_vars(SCIP* scip, SCIP_Real* stats_vars_arr);

void stats_constraints(SCIP* scip, SCIP_Real* stats_cons_arr);

void stats_cut_coefs(SCIP* scip, SCIP_ROW* cut, SCIP_Real* cut_coefs);

void stats_cut_parallelism(SCIP* scip, SCIP_ROW* cut, SCIP_Real* cut_parallelism);

#endif