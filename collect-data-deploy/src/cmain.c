#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>

#include "scip/scip.h"
#include "scip/scipshell.h"
#include "scip/scipdefplugins.h"

#include "cutsel_features.h"
#include "branching_stop.h"
#include "solver_settings.h"
#include "helper_functions.h"

int counter = 0;

int cutsel_counter = 0;

SCIP_Real* features;
SCIP_Real* cut_bound_imp;
int n_features = 135;

int ncuts_alloc = 100;

char data_dir[MAXCHAR];

SCIP_Real* reg_coef;
int* reg_model_int;
int reg_n_params = 0;

static
SCIP_RETCODE runShell(char** argv)
{

   SCIP* scip = NULL;
   char* episode_id;
   char* dataset_group;
   char* score_type;

   char* filename;
   char* solfilename;
   char* token;

   int seed1, seed2, seed3;

   episode_id = argv[1];
   
   filename = argv[2];

   seed1 = atoi(argv[3]);
   seed2 = atoi(argv[4]);  
   seed3 = atoi(argv[5]);

   dataset_group = argv[6];

   score_type = argv[7];
   solfilename = argv[8];

   #ifdef COLLECT_DATA_MODE

   #ifdef TEST
   strcpy(data_dir, "../");
   #else
   strcpy(data_dir, "../../../data/");
   #endif
   
   strcat(data_dir, dataset_group);
   strcat(data_dir, "/ep");
   strcat(data_dir, episode_id);

   char command[MAXCHAR];

   int result = snprintf(command, sizeof(command), "mkdir -p %s", data_dir);
   //printf("Result of snprintf %d\n", result);

   int status = system(command);

   if (status != 0)
      printf("Failed to create directory.\n");

   strcat(data_dir, "/node_");

   #else // DEPLOY MODE

   FILE* result_file;

   char result_dir[MAXCHAR];

   printf("Main seed: %s\n", argv[1]);

   strcpy(result_dir, "../../../results/");
   strcat(result_dir, score_type);

   char command[MAXCHAR];

   int nchars = snprintf(command, sizeof(command), "mkdir -p %s", result_dir);

   if ( (nchars >= MAXCHAR) || (nchars < 0) ) {
      printf("nchars: %d Failed to create the command string.\n", nchars);
      return -1;
   }

   int status = system(command);

   if (status != 0) {
      printf("Failed to create directory %s.\n", result_dir);
      return -1;
   }
       
   char result_file_name[MAXCHAR];
   strcpy(result_file_name, result_dir);
   strcat(result_file_name, "/");
   strcat(result_file_name, argv[1]);
   strcat(result_file_name, "_");
   strcat(result_file_name, basename(filename));
   strcat(result_file_name, ".csv");

   result_file = fopen(result_file_name, "a");

   FILE* reg_model_file;
   char reg_model_file_name[MAXCHAR];

   strcpy(reg_model_file_name, "../../../models_reg/");
   strcat(reg_model_file_name, score_type);
   strcat(reg_model_file_name, "/");
   strcat(reg_model_file_name, argv[1]);
   strcat(reg_model_file_name, "/glmnet_lasso_quadratic_model.txt");

   reg_model_file = fopen(reg_model_file_name, "r");

   if (reg_model_file == NULL) {
      printf("No regression model file found! \n");
      printf("File name: %s\n", reg_model_file_name);
      return -1;
   }

   // Initiliaze arrays for regression model coefs
   char row[MAXCHAR];
   
   int ncoefficients = 0;
   int field_ct;

   fgets(row, MAXCHAR, reg_model_file);

   reg_n_params = atoi(row);

   printf("Number of parameters (read from reg. model file): %d \n", reg_n_params);

   assert(reg_n_params > 0);

   reg_coef = (SCIP_Real*) malloc(reg_n_params * sizeof(SCIP_Real));

   reg_model_int = (int*) malloc(reg_n_params * sizeof(int) * 5);

   while (feof(reg_model_file) != TRUE)
   {
      fgets(row, MAXCHAR, reg_model_file);

      token = strtok(row, ",");
      
      field_ct = 0;
      while(token != NULL)
      {
         if (field_ct == 0) {
            reg_model_int[ncoefficients * 5] = atoi(token); //nterms
         }
         else if (field_ct == 1) {
            reg_model_int[ncoefficients * 5 + 1] = atoi(token); //term 1 id
         }
         else if (field_ct == 2) {
            reg_model_int[ncoefficients * 5 + 2] = atoi(token); //term 1 power
         }
         else if (field_ct == 3) {
            reg_model_int[ncoefficients * 5 + 3] = atoi(token); //term 2 id
         }
         else if (field_ct == 4) {
            reg_model_int[ncoefficients * 5 + 4] = atoi(token); //term 2 power
         }
         else if (field_ct == 5) {
            SCIP_Real val;
            char* ptr;
            val = strtod(token, &ptr);
            reg_coef[ncoefficients] = val;
         }

         field_ct ++;
         token = strtok(NULL, ",");
      }
      ncoefficients ++;
   }
   printf("Number of coefficients in the reg. model: %d \n", ncoefficients);
   fclose(reg_model_file);

   if( reg_n_params != ncoefficients) {
      printf("Number of parameters read does not equal the number of coefficients!\n");
      free(reg_coef);
      free(reg_model_int);
      return -1;
   }

   #endif

   SCIP_CALL( SCIPcreate(&scip) );

   SCIP_CALL( SCIPincludeCutselFeatures(scip) );

   SCIP_CALL( SCIPincludeBranchruleStop(scip) );

   SCIP_CALL( SCIPincludeDefaultPlugins(scip) );

   SCIP_CALL( set_solver_settings(scip, seed1, seed2, seed3) );

   SCIP_CALL( SCIPreadProb(scip, filename, NULL) );

   SCIP_CALL( SCIPreadSol(scip, solfilename) );

   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &features, n_features * ncuts_alloc) );
   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &cut_bound_imp, ncuts_alloc) );


   #ifndef COLLECT_DATA_MODE
   struct timeval walltime_begin, walltime_end;
   struct tms cputime_begin, cputime_end;

   clock_t processtime_begin = clock();

   gettimeofday(&walltime_begin, NULL); //record walltime
   (void)times(&cputime_begin); //record user & system cpu time
   #endif

   SCIP_CALL( SCIPsolve(scip) );

   #ifndef COLLECT_DATA_MODE
   clock_t processtime_end = clock();
   gettimeofday(&walltime_end, NULL);
   (void)times(&cputime_end);

   SCIP_Real processtime = (SCIP_Real)(processtime_end - processtime_begin) / CLOCKS_PER_SEC;
   SCIP_Real walltime = (SCIP_Real) ((walltime_end.tv_sec - walltime_begin.tv_sec) + (walltime_end.tv_usec - walltime_begin.tv_usec) * 1e-6);

   SCIP_Real cpu_user_time = (SCIP_Real) (cputime_end.tms_utime - cputime_begin.tms_utime) / sysconf(_SC_CLK_TCK);
   SCIP_Real cpu_system_time = (SCIP_Real) (cputime_end.tms_stime - cputime_begin.tms_stime) / sysconf(_SC_CLK_TCK);
   #endif

   SCIPfreeBlockMemoryArray(scip, &features, n_features * ncuts_alloc);
   SCIPfreeBlockMemoryArray(scip, &cut_bound_imp, ncuts_alloc);

   #ifndef COLLECT_DATA_MODE
   char status_str[50];
   switch( SCIPgetStatus(scip) )
   {
   case SCIP_STATUS_UNKNOWN:
      strcpy(status_str, "unknown");
      break;
   case SCIP_STATUS_USERINTERRUPT:
      strcpy(status_str, "userinterrupt");
      break;
   case SCIP_STATUS_NODELIMIT:
      strcpy(status_str, "nodelimit");
      break;
   case SCIP_STATUS_TOTALNODELIMIT:
      strcpy(status_str, "totalnodelimit");
      break;
   case SCIP_STATUS_STALLNODELIMIT:
      strcpy(status_str, "stallnodelimit");
      break;
   case SCIP_STATUS_TIMELIMIT:
      strcpy(status_str, "timelimit");
      break;
   case SCIP_STATUS_MEMLIMIT:
      strcpy(status_str, "memlimit");
      break;
   case SCIP_STATUS_GAPLIMIT:
      strcpy(status_str, "gaplimit");
      break;
   case SCIP_STATUS_SOLLIMIT:
      strcpy(status_str, "sollimit");
      break;
   case SCIP_STATUS_BESTSOLLIMIT:
      strcpy(status_str, "bestsollimit");
      break;
   case SCIP_STATUS_RESTARTLIMIT:
      strcpy(status_str, "restartlimit");
      break;
   case SCIP_STATUS_OPTIMAL:
      strcpy(status_str, "optimal");
      break;
   case SCIP_STATUS_INFEASIBLE:
      strcpy(status_str, "infeasible");
      break;
   case SCIP_STATUS_UNBOUNDED:
      strcpy(status_str, "unbounded");
      break;
   case SCIP_STATUS_INFORUNBD:
      strcpy(status_str, "inforunbd");
      break;
   case SCIP_STATUS_TERMINATE:
      strcpy(status_str, "terminate");
      break;
   default:
      return SCIP_INVALIDDATA;
   }

   SCIP_Real igc;
   SCIP_Real primal_bound, dual_bound, first_dual_bound;
   
   first_dual_bound = SCIPgetFirstLPDualboundRoot(scip);

   primal_bound = SCIPgetPrimalbound(scip);
   dual_bound = SCIPgetDualbound(scip);

   igc = safe_div(dual_bound - first_dual_bound, primal_bound - first_dual_bound);

   fprintf(result_file, "%s", "policy,seed,instance,nnodes,nlps,nlpiterations,nnodelps,ndivinglps,nsbs,stime,gap,status,mem_used,mem_total,walltime,proctime,cpu_user_time,cpu_system_time,primal_bound,dual_bound,first_dual_bound,igc\n");

   fprintf(result_file, "%s,", score_type);
   fprintf(result_file, "%s,", argv[1]);
   fprintf(result_file, "%s,", basename(filename) );
   fprintf(result_file, "%d,", (int) SCIPgetNNodes(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNLPs(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNLPIterations(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNNodeLPs(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNDivingLPs(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNStrongbranchs(scip));
   fprintf(result_file, "%f,", (SCIP_Real) SCIPgetSolvingTime(scip));
   fprintf(result_file, "%f,", (SCIP_Real) SCIPgetGap(scip));
   fprintf(result_file, "%s,", status_str);
   fprintf(result_file, "%f,", (SCIP_Real) SCIPgetMemUsed(scip) / 1048576.0 );
   fprintf(result_file, "%f,", (SCIP_Real) SCIPgetMemTotal(scip) / 1048576.0 );
   fprintf(result_file, "%f,", walltime);
   fprintf(result_file, "%f,", processtime);
   fprintf(result_file, "%f,", cpu_user_time);
   fprintf(result_file, "%f,", cpu_system_time);
   fprintf(result_file, "%f,", primal_bound);
   fprintf(result_file, "%f,", dual_bound);
   fprintf(result_file, "%f,", first_dual_bound);
   fprintf(result_file, "%f\n", igc);

   fclose(result_file);

   free(reg_coef);
   free(reg_model_int);
   #endif


   SCIP_CALL( SCIPfree(&scip) );

   BMScheckEmptyMemory();

   return SCIP_OKAY;
}

int main(int argc, char** argv)
{
   if (argc != 9)
      return -1;

   SCIP_RETCODE retcode;

   retcode = runShell(argv);

   if( retcode != SCIP_OKAY )
   {
      SCIPprintError(retcode);
      return -1;
   }

   return 0;
}

