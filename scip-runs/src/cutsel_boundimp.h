#ifndef __SCIP_CUTSEL_BOUNDIMP_H__
#define __SCIP_CUTSEL_BOUNDIMP_H__
#include "scip/scip.h"

#define TOL 0.1
#define MAXCHAR 4095
extern char data_dir[];

extern int cutsel_counter;

#ifdef __cplusplus
extern "C" {
#endif

/** creates the xyz separator and includes it in SCIP
 *
 * @ingroup CutSelectorIncludes
 */
SCIP_EXPORT
SCIP_RETCODE SCIPincludeCutselBoundimp(
   SCIP*                 scip                /**< SCIP data structure */
   );

/**@addtogroup CUTSELECTORS
 *
 * @{
 */

/** perform a cut selection algorithm for the given array of cuts
 *
 *  The input cuts array should be resorted such that the selected cuts come first.
 */
SCIP_EXPORT
SCIP_RETCODE SCIPselectCutsBoundimp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ROW**            cuts,               /**< array with cuts to perform selection algorithm */
   SCIP_ROW**            forcedcuts,         /**< array with forced cuts */
   int                   nforcedcuts,        /**< number of forced cuts */
   int                   maxselectedcuts,    /**< maximal number of cuts from cuts array to select */
   int*                  nselectedcuts       /**< pointer to return number of selected cuts from cuts array */
   );


/** @} */

#ifdef __cplusplus
}
#endif

#endif



