#include "solver_settings.h"

SCIP_RETCODE set_solver_settings(SCIP* scip, int seed1, int seed2, int seed3) {

    SCIP_CALL( SCIPsetIntParam(scip, "cutselection/features/priority", 22000) );
    SCIP_CALL( SCIPsetIntParam(scip, "separating/maxroundsroot", 10) );
    SCIP_CALL( SCIPsetIntParam(scip, "separating/maxrounds", 0) );
    SCIP_CALL( SCIPsetIntParam(scip, "separating/maxcutsroot", 1) );
    SCIP_CALL( SCIPsetIntParam(scip, "separating/maxcuts", 0) );
    SCIP_CALL( SCIPsetLongintParam(scip, "limits/nodes", 1) );
    SCIP_CALL( SCIPsetIntParam(scip, "presolving/maxrestarts", 0) );
    SCIP_CALL( SCIPsetBoolParam(scip, "conflict/enable", FALSE) );
    SCIP_CALL( SCIPsetRealParam(scip, "separating/minefficacyroot", 0.0) );
    SCIP_CALL( SCIPsetRealParam(scip, "separating/minefficacy", 0.0) );
    SCIP_CALL( SCIPsetHeuristics(scip, SCIP_PARAMSETTING_OFF, TRUE) );
    SCIP_CALL( SCIPsetIntParam(scip, "randomization/permutationseed", seed1) );
    SCIP_CALL( SCIPsetIntParam(scip, "randomization/randomseedshift", seed2) );
    SCIP_CALL( SCIPsetIntParam(scip, "randomization/lpseed", seed3) );

    #ifndef COLLECT_DATA_MODE
    SCIP_CALL( SCIPsetIntParam(scip, "display/verblevel", 0) );
    SCIP_CALL( SCIPsetIntParam(scip, "timing/clocktype", 2) ); //1: CPU time, 2: Wallclock time
    #endif

    return SCIP_OKAY;

}