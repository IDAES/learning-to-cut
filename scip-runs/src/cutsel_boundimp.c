#include <assert.h>

#include "cutsel_boundimp.h"

#include "scip/struct_lp.h"

#define CUTSEL_NAME              "boundimp"
#define CUTSEL_DESC              "cut selector boundimp"
#define CUTSEL_PRIORITY           10000

struct SCIP_CutselData
{
   SCIP_Real* cutscores;

};

#define cutselCopyBoundimp NULL

static
SCIP_DECL_CUTSELFREE(cutselFreeBoundimp)
{ 
   SCIP_CUTSELDATA* cutseldata;

   cutseldata = SCIPcutselGetData(cutsel);

   SCIPfreeBlockMemory(scip, &cutseldata);

   SCIPcutselSetData(cutsel, NULL);

   return SCIP_OKAY;
}

#define cutselInitBoundimp NULL

static
SCIP_DECL_CUTSELEXIT(cutselExitBoundimp)
{
   SCIP_CUTSELDATA* cutseldata;

   cutseldata = SCIPcutselGetData(cutsel);
   assert(cutseldata != NULL);

   return SCIP_OKAY;
}

#define cutselInitsolBoundimp NULL

#define cutselExitsolBoundimp NULL

static
SCIP_DECL_CUTSELSELECT(cutselSelectBoundimp)
{

   SCIP_CUTSELDATA* cutseldata;

   cutseldata = SCIPcutselGetData(cutsel);

   assert(cutseldata != NULL);

   *nselectedcuts = 0;
   
   *result = SCIP_SUCCESS;

   if ( ncuts == 1) {
      *nselectedcuts ++;
      return SCIP_OKAY;
   }

   SCIP_Bool lperror, cutoff;
   SCIP_Real lp_obj, diving_lp_obj;

   lp_obj = SCIPgetLPObjval(scip);

   SCIP_ROW* cut = NULL;

   cutsel_counter ++;

   int i;

   SCIP_Real best_score = -1 * SCIPinfinity(scip);
   int best_cut_id = -1;

   for(i = 0; i < ncuts; i++) {

      cut = cuts[i];
      
      SCIP_CALL( SCIPstartDive(scip) );
      SCIP_CALL( SCIPaddRowDive(scip, cut) );

      // SCIPsolveDiveLP is not idempotent, make idempotent version
      SCIP_CALL( SCIPsolveDiveLP(scip, -1, &lperror, &cutoff) );
      
      diving_lp_obj = SCIPgetLPObjval(scip);

      SCIP_CALL( SCIPendDive(scip) );

      if (diving_lp_obj - lp_obj > best_score)
      {
         best_cut_id = i;
         best_score = diving_lp_obj - lp_obj;
      }

   }

   SCIPswapPointers((void**) &cuts[best_cut_id], (void**) &cuts[0]);

   ++(*nselectedcuts);

   return SCIP_OKAY;

}

SCIP_RETCODE SCIPincludeCutselBoundimp(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CUTSELDATA* cutseldata = NULL;
   SCIP_CUTSEL* cutsel = NULL;

   SCIP_CALL( SCIPallocBlockMemory(scip, &cutseldata) );
   
   SCIP_CALL( SCIPincludeCutselBasic(scip, &cutsel, CUTSEL_NAME, CUTSEL_DESC, CUTSEL_PRIORITY, cutselSelectBoundimp,
         cutseldata) );

   assert(cutsel != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetCutselCopy(scip, cutsel, cutselCopyBoundimp) );
   SCIP_CALL( SCIPsetCutselFree(scip, cutsel, cutselFreeBoundimp) );
   SCIP_CALL( SCIPsetCutselInit(scip, cutsel, cutselInitBoundimp) );
   SCIP_CALL( SCIPsetCutselExit(scip, cutsel, cutselExitBoundimp) );
   SCIP_CALL( SCIPsetCutselInitsol(scip, cutsel, cutselInitsolBoundimp) );
   SCIP_CALL( SCIPsetCutselExitsol(scip, cutsel, cutselExitsolBoundimp) );

   return SCIP_OKAY;
}
