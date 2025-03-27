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

#include "branching_stop.h"
#include "solver_settings.h"
#include "cutsel_boundimp.h"

int cutsel_counter = 0;

#define TOL 0.1

SCIP_Real safe_div(SCIP_Real x, SCIP_Real y) {
   if (fabs(y) <= TOL)
      return 0;
   else
      return x / y ;
}

static
SCIP_RETCODE runShell(char** argv)
{

   SCIP* scip = NULL;
   char* episode_id;
   char* dataset_group;
   char* cutselector;

   char* filename;
   char* solfilename;

   int seed1, seed2, seed3;

   SCIP_Real igc;
   SCIP_Real primal_bound, dual_bound, first_dual_bound;
   
   episode_id = argv[1];
   
   filename = argv[2];

   seed1 = atoi(argv[3]);
   seed2 = atoi(argv[4]);  
   seed3 = atoi(argv[5]);

   dataset_group = argv[6];

   FILE* result_file;

   char result_dir[MAXCHAR];
   cutselector = argv[7];

   solfilename = argv[8];
   
   printf("Main seed: %s\n", argv[1]);
   #ifdef TEST
   strcpy(result_dir, "../");
   #else
   if ( strcmp(solfilename, "-") == 0 )
      strcpy(result_dir, "../../results-no-sol/");
   else
      strcpy(result_dir, "../../results/");
   #endif
   
   strcat(result_dir, cutselector);

   char command[MAXCHAR];
   int nchars, status;
   
   nchars = snprintf(command, sizeof(command), "mkdir -p %s", result_dir);

   if ( (nchars >= MAXCHAR) || (nchars < 0) ) {
      printf("nchars: %d Failed to create the command string.\n", nchars);
      return -1;
   }

   status = system(command);

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

   SCIP_CALL( SCIPcreate(&scip) );

   SCIP_CALL( SCIPincludeCutselBoundimp(scip) );

   SCIP_CALL( SCIPincludeBranchruleStop(scip) );

   SCIP_CALL( SCIPincludeDefaultPlugins(scip) );

   SCIP_CALL( set_solver_settings(scip, seed1, seed2, seed3) );

   if ( strcmp(cutselector, "hybrid") == 0 )
   {
      SCIP_CALL( SCIPsetIntParam(scip, "cutselection/hybrid/priority", 25000) ); //max 8K
   }
   else if ( strcmp(cutselector, "boundimp") == 0 )
   {
      SCIP_CALL( SCIPsetIntParam(scip, "cutselection/boundimp/priority", 25000) );
   }

   SCIP_CALL( SCIPreadProb(scip, filename, NULL) );

   if ( strcmp(solfilename, "-") == 0 )
      printf("Solution file not provided\n");
   else
      SCIP_CALL( SCIPreadSol(scip, solfilename) );

   struct timeval walltime_begin, walltime_end;
   struct tms cputime_begin, cputime_end;

   clock_t processtime_begin = clock();

   gettimeofday(&walltime_begin, NULL); //record walltime
   (void)times(&cputime_begin); //record user & system cpu time

   SCIP_CALL( SCIPsolve(scip) );

   clock_t processtime_end = clock();
   gettimeofday(&walltime_end, NULL);
   (void)times(&cputime_end);

   double processtime = (double)(processtime_end - processtime_begin) / CLOCKS_PER_SEC;
   double walltime = (double) ((walltime_end.tv_sec - walltime_begin.tv_sec) + (walltime_end.tv_usec - walltime_begin.tv_usec) * 1e-6);

   double cpu_user_time = (double) (cputime_end.tms_utime - cputime_begin.tms_utime) / sysconf(_SC_CLK_TCK);
   double cpu_system_time = (double) (cputime_end.tms_stime - cputime_begin.tms_stime) / sysconf(_SC_CLK_TCK);

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

   first_dual_bound = SCIPgetFirstLPDualboundRoot(scip);

   primal_bound = SCIPgetPrimalbound(scip);
   dual_bound = SCIPgetDualbound(scip);

   igc = safe_div(dual_bound - first_dual_bound, primal_bound - first_dual_bound);


   fprintf(result_file, "%s", "policy,seed,instance,nnodes,nlps,nlpiterations,nnodelps,ndivinglps,nsbs,stime,gap,status,mem_used,mem_total,walltime,proctime,cpu_user_time,cpu_system_time,primal_bound,dual_bound,first_dual_bound,igc\n");

   fprintf(result_file, "%s,", cutselector);
   fprintf(result_file, "%s,", argv[1]);
   fprintf(result_file, "%s,", basename(filename) );
   fprintf(result_file, "%d,", (int) SCIPgetNNodes(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNLPs(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNLPIterations(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNNodeLPs(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNDivingLPs(scip));
   fprintf(result_file, "%d,", (int) SCIPgetNStrongbranchs(scip));
   fprintf(result_file, "%f,", (double) SCIPgetSolvingTime(scip));
   fprintf(result_file, "%f,", (double) SCIPgetGap(scip));
   fprintf(result_file, "%s,", status_str);
   fprintf(result_file, "%f,", (double) SCIPgetMemUsed(scip) / 1048576.0 );
   fprintf(result_file, "%f,", (double) SCIPgetMemTotal(scip) / 1048576.0 );
   fprintf(result_file, "%f,", walltime);
   fprintf(result_file, "%f,", processtime);
   fprintf(result_file, "%f,", cpu_user_time);
   fprintf(result_file, "%f,", cpu_system_time);
   fprintf(result_file, "%f,", primal_bound);
   fprintf(result_file, "%f,", dual_bound);
   fprintf(result_file, "%f,", first_dual_bound);
   fprintf(result_file, "%f\n", igc);

   fclose(result_file);

   SCIP_CALL( SCIPfree(&scip) );

   BMScheckEmptyMemory();

   return SCIP_OKAY;
}

int main(int argc, char** argv)
{
   if (argc != 9)
   {
      printf("You entered %d args, 9 needed\n", argc);
      return -1;
   }
      

   SCIP_RETCODE retcode;

   retcode = runShell(argv);

   if( retcode != SCIP_OKAY )
   {
      SCIPprintError(retcode);
      return -1;
   }

   return 0;
}

