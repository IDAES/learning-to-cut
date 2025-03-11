#include <assert.h>

#include "cutsel_features.h"

#include "helper_functions.h"

#include "scip/struct_lp.h"

#define CUTSEL_NAME              "features"
#define CUTSEL_DESC              "cut selector features"
#define CUTSEL_PRIORITY           10000

struct SCIP_CutselData
{
   SCIP_Real* stats_A;
   SCIP_Real* stats_c;
   SCIP_Real* stats_b;
   SCIP_Real* stats_vars;
   SCIP_Real* stats_conss;

   SCIP_Real* stats_cut_parallelism;
   SCIP_Real* stats_cut_coefs;
   
};

#define cutselCopyFeatures NULL

static
SCIP_DECL_CUTSELFREE(cutselFreeFeatures)
{
   SCIP_CUTSELDATA* cutseldata;

   cutseldata = SCIPcutselGetData(cutsel);

   SCIPfreeBlockMemory(scip, &cutseldata);

   SCIPcutselSetData(cutsel, NULL);

   return SCIP_OKAY;
}

#define cutselInitFeatures NULL

static
SCIP_DECL_CUTSELEXIT(cutselExitFeatures)
{
   SCIP_CUTSELDATA* cutseldata;

   /* initialize branching rule data */
   cutseldata = SCIPcutselGetData(cutsel);
   assert(cutseldata != NULL);

   /* free candidate arrays if any */
   if( cutseldata->stats_A != NULL )
   {
      SCIPfreeBlockMemoryArrayNull(scip, &cutseldata->stats_A, 4);
   }
   if( cutseldata->stats_c != NULL )
   {
      SCIPfreeBlockMemoryArrayNull(scip, &cutseldata->stats_c, 4);
   }
   if( cutseldata->stats_b != NULL )
   {
      SCIPfreeBlockMemoryArrayNull(scip, &cutseldata->stats_b, 4);
   }
   if( cutseldata->stats_vars != NULL )
   {
      SCIPfreeBlockMemoryArrayNull(scip, &cutseldata->stats_vars, 60);
   }
   if( cutseldata->stats_conss != NULL )
   {
      SCIPfreeBlockMemoryArrayNull(scip, &cutseldata->stats_conss, 44);
   }
   if( cutseldata->stats_cut_coefs != NULL )
   {
      SCIPfreeBlockMemoryArrayNull(scip, &cutseldata->stats_cut_coefs, 4);
   }
   if( cutseldata->stats_cut_parallelism != NULL )
   {
      SCIPfreeBlockMemoryArrayNull(scip, &cutseldata->stats_cut_parallelism, 4);
   }
   //+11 for cuts

   return SCIP_OKAY;
}

#define cutselInitsolFeatures NULL

#define cutselExitsolFeatures NULL

static
SCIP_DECL_CUTSELSELECT(cutselSelectFeatures)
{

   SCIP_CUTSELDATA* cutseldata;

   cutseldata = SCIPcutselGetData(cutsel);

   assert(cutseldata != NULL);

   *result = SCIP_SUCCESS;
   *nselectedcuts = 0;

   int best_cut_id = -1;
   SCIP_Real best_cut_score = -1 * SCIPinfinity(scip);

   if ( ncuts > ncuts_alloc)
   {
      SCIP_CALL( SCIPreallocBlockMemoryArray(scip, &features, n_features * ncuts_alloc, n_features * ncuts) );

      SCIP_CALL( SCIPreallocBlockMemoryArray(scip, &cut_bound_imp, ncuts_alloc, ncuts) );

      ncuts_alloc = ncuts;
   }

   if( cutseldata->stats_A == NULL )
   {
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &cutseldata->stats_A, 4) );
   }
   if( cutseldata->stats_c == NULL )
   {
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &cutseldata->stats_c, 4) );
   }
   if( cutseldata->stats_b == NULL )
   {
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &cutseldata->stats_b, 4) );
   }
   if( cutseldata->stats_vars == NULL )
   {
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &cutseldata->stats_vars, 60) );
   }
   if( cutseldata->stats_conss == NULL )
   {
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &cutseldata->stats_conss, 44) );
   }
   if( cutseldata->stats_cut_coefs == NULL )
   {
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &cutseldata->stats_cut_coefs, 4) );
   }
   if( cutseldata->stats_cut_parallelism == NULL )
   {
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &cutseldata->stats_cut_parallelism, 4) );
   }

   cutsel_counter ++;

   int i;

   SCIP_ROW* cut = NULL;

   /* Extract node features */

   int j;

   statsAmatrix(scip, cutseldata->stats_A);

   statsc(scip, cutseldata->stats_c);

   statsb(scip, cutseldata->stats_b);

   stats_vars(scip, cutseldata->stats_vars);

   stats_constraints(scip, cutseldata->stats_conss);

   SCIP_Real degeneracy, varconsratio;

   SCIP_CALL( SCIPgetLPDualDegeneracy(scip, &degeneracy, &varconsratio) );

   SCIP_Real rhs, lhs, row_norm, activity;

   /* Extract cut features */

   for (i = 0; i < ncuts; i++) {

      // Copy node features to each cut
      for (j = 0; j < 4; j++) {

         features[i * n_features + j] = cutseldata->stats_A[j];
         features[i * n_features + j + 4] = cutseldata->stats_c[j];
         features[i * n_features + j + 8] = cutseldata->stats_b[j];
      }
      for (j = 0; j < 60; j++) {
         features[i * n_features + j + 12] = cutseldata->stats_vars[j]; //60 feats
      }

      for (j = 0; j < 44; j++) {
         features[i * n_features + 72 + j] = cutseldata->stats_conss[j]; //44 feats
      }
      //116 feats
      
      cut = cuts[i];

      features[i * n_features + 116] = (SCIP_Real) SCIProwGetRank(cut);
      features[i * n_features + 117] = safe_div(SCIProwGetNNonz(cut), SCIPgetNLPCols(scip)); // support score
      features[i * n_features + 118] = safe_div(SCIPgetRowNumIntCols(scip, cut), SCIProwGetNNonz(cut)); // integer support score

      rhs = SCIProwGetRhs(cut);
      lhs = SCIProwGetLhs(cut);
      
      if ( SCIProwGetNorm(cut) == 0.0 )
      {
         for( j = 0; j < cut->len; ++j )
            if( cut->cols[j]->lppos >= 0 )
               cut->sqrnorm += SQR(cut->vals[j]);
         assert(SCIProwGetNorm(cut) != 0.0);
      }

      row_norm = SCIProwGetNorm(cut);

      activity = SCIPgetRowActivity(scip, cut);

      assert( (!SCIPisInfinity(scip, fabs(rhs)) && SCIPisInfinity(scip, fabs(lhs))) || (!SCIPisInfinity(scip, fabs(lhs)) && SCIPisInfinity(scip, fabs(rhs))) );

      assert( !SCIPisInfinity(scip, fabs(rhs)) && SCIPisInfinity(scip, fabs(lhs)) );

      features[i * n_features + 119] = safe_div(rhs, row_norm);

      if ( SCIPisFeasEQ(scip, activity, rhs) )
         features[i * n_features + 120] = 1.0;
      else
         features[i * n_features + 120] = 0.0;


      features[i * n_features + 121] = (SCIP_Real) SCIProwIsIntegral(cut);

      features[i * n_features + 122] = -1 * SCIPgetRowLPFeasibility(scip, cut); //absolute violation
      
      features[i * n_features + 123] = rhs == 0 ? -1 * SCIPgetRowLPFeasibility(scip, cut) : -1 * SCIPgetRowLPFeasibility(scip, cut) / rhs; // relative violation

      features[i * n_features + 124] = -1 * safe_div(SCIPgetRowLPFeasibility(scip, cut), row_norm); //efficacy

      features[i * n_features + 125] = SCIPgetRowObjParallelism(scip, cut); // obj parallellism

      features[i * n_features + 126] = SCIPgetObjNorm(scip) * features[i * n_features + 125] * features[i * n_features + 124]; // expected improvement

      assert( SCIProwGetNNonz(cut) == cut->len );

      stats_cut_coefs(scip, cut, cutseldata->stats_cut_coefs);

      for(j = 0; j < 4; j++)
         features[i * n_features + 127 + j] = cutseldata->stats_cut_coefs[j];
      
      stats_cut_parallelism(scip, cut, cutseldata->stats_cut_parallelism);
      
      for(j = 0; j < 4; j++)
         features[i * n_features + 131 + j] = cutseldata->stats_cut_parallelism[j];


      #ifdef COLLECT_DATA_MODE
      SCIP_Bool lperror;
      SCIP_Bool cutoff;

      SCIP_Real lp_obj, diving_lp_obj;

      lp_obj = SCIPgetLPObjval(scip);
      // Collect bound imp data
      SCIP_CALL( SCIPstartDive(scip) );
      SCIP_CALL( SCIPaddRowDive(scip, cut) );

      // SCIPsolveDiveLP is not idempotent, make idempotent version
      SCIP_CALL( SCIPsolveDiveLP(scip, -1, &lperror, &cutoff) );
      
      diving_lp_obj = SCIPgetLPObjval(scip);

      SCIP_CALL( SCIPendDive(scip) );

      cut_bound_imp[i] = MAX(diving_lp_obj - lp_obj, 0);
      
      if (cut_bound_imp[i] > best_cut_score)
      {
         best_cut_score = cut_bound_imp[i];
         best_cut_id = i;
      }
      #endif
   }


   #ifdef COLLECT_DATA_MODE
   char node_id[MAXCHAR];
   sprintf(node_id, "%lld_%lld_sepr_%d", SCIPgetNNodes(scip), SCIPgetNLPs(scip), SCIPgetNSepaRounds(scip));

   char feat_data[MAXCHAR];
   strcpy(feat_data, data_dir);
   strcat(feat_data, node_id);
   strcat(feat_data, "_features.txt");

   FILE *f = fopen(feat_data, "w");

   if (f != NULL) {
            
      for(i = 0; i < ncuts; i++) {
         for(j = 0; j < n_features - 1; j++) {
            fprintf(f, "%.16f,", features[i * n_features + j]);
         }  
         fprintf(f, "%.16f\n", features[i * n_features + n_features - 1]);
      }
      
      fclose(f);
   }

   //Store scores

   char scores_data[MAXCHAR];
   strcpy(scores_data, data_dir);
   strcat(scores_data, node_id);
   strcat(scores_data, "_scores.txt");

   FILE *fs = fopen(scores_data, "w");

   if (fs != NULL) {
            
      for(i = 0; i < ncuts; i++) {
         fprintf(fs, "%.16f\n", cut_bound_imp[i]);
      }
      fclose(fs);
   }

   #else // DEPLOY MODE

   for (i = 0; i < ncuts; i++)
      cut_bound_imp[i] = 0.0;

   // Do regression, do not skip the intercept to get exact predictions
   for(j = 0; j < reg_n_params; j++) {

      //Single term
      if( reg_model_int[j * 5] == 1 )
      {
         // Intercept
         if ( reg_model_int[j * 5 + 1] == -2 ) {
            for(i = 0; i < ncuts; i++) {
               cut_bound_imp[i] += reg_coef[j];
            }

         }
         // power = 1
         else if ( reg_model_int[j * 5 + 2] == 1 ) {
            for(i = 0; i < ncuts; i++) {
               cut_bound_imp[i] += ((features[i * n_features + reg_model_int[j * 5 + 1]]) * reg_coef[j]);
            }
         }
         // power = 2
         else if ( reg_model_int[j * 5 + 2] == 2 )
         {
            for(i = 0; i < ncuts; i++) {
               cut_bound_imp[i] += ((square(features[i * n_features + reg_model_int[j * 5 + 1]])) * reg_coef[j]);

            }
         }
      }

      //Two terms
      else {
         for(i = 0; i < ncuts; i++) {
            cut_bound_imp[i] += ((features[i * n_features + reg_model_int[j * 5 + 1]] * features[i * n_features + reg_model_int[j * 5 + 3]]) * reg_coef[j]);
         }
      }
   }


   for (i = 0; i < ncuts; i++)
   {  
      if (cut_bound_imp[i] > best_cut_score)
      {
         best_cut_score = cut_bound_imp[i];
         best_cut_id = i;
      }
   }


   assert(best_cut_id >= 0);

   #endif

   SCIPswapPointers((void**) &cuts[best_cut_id], (void**) &cuts[0]);

   ++(*nselectedcuts);

   return SCIP_OKAY;

}

SCIP_RETCODE SCIPincludeCutselFeatures(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CUTSELDATA* cutseldata = NULL;
   SCIP_CUTSEL* cutsel = NULL;

   /* create hybrid cut selector data */
   SCIP_CALL( SCIPallocBlockMemory(scip, &cutseldata) );
   
   cutseldata->stats_A = NULL;
   cutseldata->stats_c = NULL;
   cutseldata->stats_b = NULL;
   cutseldata->stats_cut_coefs = NULL;
   cutseldata->stats_cut_parallelism = NULL;

   cutseldata->stats_conss = NULL;

   cutseldata->stats_vars = NULL;

   SCIP_CALL( SCIPincludeCutselBasic(scip, &cutsel, CUTSEL_NAME, CUTSEL_DESC, CUTSEL_PRIORITY, cutselSelectFeatures,
         cutseldata) );

   assert(cutsel != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetCutselCopy(scip, cutsel, cutselCopyFeatures) );
   SCIP_CALL( SCIPsetCutselFree(scip, cutsel, cutselFreeFeatures) );
   SCIP_CALL( SCIPsetCutselInit(scip, cutsel, cutselInitFeatures) );
   SCIP_CALL( SCIPsetCutselExit(scip, cutsel, cutselExitFeatures) );
   SCIP_CALL( SCIPsetCutselInitsol(scip, cutsel, cutselInitsolFeatures) );
   SCIP_CALL( SCIPsetCutselExitsol(scip, cutsel, cutselExitsolFeatures) );

   return SCIP_OKAY;
}
