#ifndef __SCIP_BRANCH_STOP_H__
#define __SCIP_BRANCH_STOP_H__


#include "scip/scip.h"
#include "scip/struct_lp.h"

/** creates the collect data branching rule and includes it in SCIP */
SCIP_RETCODE SCIPincludeBranchruleStop(
   SCIP*                 scip                /**< SCIP data structure */
   );

#endif
