#ifndef __SOLVER_SETTINGS_H__
#define __SOLVER_SETTINGS_H__

#include "scip/scip.h"

SCIP_RETCODE set_solver_settings(SCIP* scip, int seed1, int seed2, int seed3);

#endif