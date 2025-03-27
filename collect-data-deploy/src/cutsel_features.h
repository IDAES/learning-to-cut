#ifndef __SCIP_CUTSEL_FEATURES_H__
#define __SCIP_CUTSEL_FEATURES_H__
#include "scip/scip.h"

#define MAXCHAR 4095

extern int cut_counter;
extern int cutsel_counter;

extern SCIP_Real* features;
extern SCIP_Real* cut_bound_imp;
extern int n_features;
extern int ncuts_alloc;

extern char data_dir[];

extern SCIP_Real* reg_coef;
extern int* reg_model_int;
extern int reg_n_params;

#ifdef __cplusplus
extern "C" {
#endif

SCIP_EXPORT
SCIP_RETCODE SCIPincludeCutselFeatures(
   SCIP*                 scip                /**< SCIP data structure */
   );

SCIP_EXPORT
SCIP_RETCODE SCIPselectCutsFeatures(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ROW**            cuts,               /**< array with cuts to perform selection algorithm */
   SCIP_ROW**            forcedcuts,         /**< array with forced cuts */
   int                   nforcedcuts,        /**< number of forced cuts */
   int                   maxselectedcuts,    /**< maximal number of cuts from cuts array to select */
   int*                  nselectedcuts       /**< pointer to return number of selected cuts from cuts array */
   );

#ifdef __cplusplus
}
#endif

#endif
