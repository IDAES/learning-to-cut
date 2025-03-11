#include <assert.h>
#include <string.h>
#include <math.h>
#include "scip/struct_clock.h"
#include "scip/struct_branch.h"
#include "scip/branch_vanillafullstrong.h"
#include "branching_stop.h"

#define BRANCHRULE_NAME            "stop"
#define BRANCHRULE_DESC            "stop branching rule"
#define BRANCHRULE_PRIORITY        50000
#define BRANCHRULE_MAXDEPTH        -1
#define BRANCHRULE_MAXBOUNDDIST    1.0


static
SCIP_DECL_BRANCHEXECLP(branchExeclpStop)
{  

   SCIP_CALL( SCIPinterruptSolve(scip) );

   return SCIP_OKAY;
}

SCIP_RETCODE SCIPincludeBranchruleStop(
   SCIP*                 scip 
   )
{
   SCIP_BRANCHRULEDATA* branchruledata;
   SCIP_BRANCHRULE* branchrule;

   branchruledata = NULL;
   branchrule = NULL;
   /* include branching rule */
   SCIP_CALL( SCIPincludeBranchruleBasic(scip, &branchrule, BRANCHRULE_NAME, BRANCHRULE_DESC, BRANCHRULE_PRIORITY, BRANCHRULE_MAXDEPTH,
         BRANCHRULE_MAXBOUNDDIST, branchruledata) );
   assert(branchrule != NULL);

   SCIP_CALL( SCIPsetBranchruleExecLp(scip, branchrule, branchExeclpStop) );

   return SCIP_OKAY;
}

