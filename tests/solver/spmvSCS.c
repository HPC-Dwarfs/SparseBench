// DL 2025.04.07
// Single rank SpMV test

// We force the matrix format to be Sell-C-sigma on a single rank
#ifdef _MPI
#undef _MPI
#endif
#ifdef CRS
#undef CRS
#endif
#ifdef CCRS
#undef CCRS
#endif

#define SCS

#include "../../src/allocate.h"
#include "../../src/comm.h"
#include "../../src/debugger.h"
#include "../../src/matrix.h"
#include "../../src/solver.h"
#include "../common.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include "../../src/affinity.h"
#include <omp.h>
#endif

int test_spmvSCS(void* args, const char* dataDir)
{

  int rank           = 0;
  int size           = 1;
  int validFileCount = 0;

  // Open the directory
  char* pathToMatrices = malloc(strlen(dataDir) + strlen("testMatrices/") + 1);
  strcpy(pathToMatrices, dataDir);
  strcat(pathToMatrices, "testMatrices/");
  DIR* dir = opendir(pathToMatrices);
  if (dir == NULL) {
    perror("Error opening directory");
    return 1;
  }

  // Read the directory entries
  struct dirent* entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strstr(entry->d_name, ".mtx") != NULL) {
      char* pathToMatrix = malloc(
          strlen(pathToMatrices) + strlen(entry->d_name) + 1);
      strcpy(pathToMatrix, pathToMatrices);
      strcat(pathToMatrix, entry->d_name);

      MMMatrix M;
      Args* arguments = (Args*)args;

      char C_str[STR_LEN];
      char sigma_str[STR_LEN];
      sprintf(C_str, "%d", arguments->C);
      sprintf(sigma_str, "%d", arguments->sigma);

      // String preprocessing
      FORMAT_AND_STRIP_VECTOR_FILE(entry)

      // This is the external file to check against
      char* pathToExpectedData = malloc(STR_LEN);
      BUILD_VECTOR_FILE_PATH(entry,
          "expected/",
          "_spmv_x_1.in",
          pathToExpectedData);

      if (fopen(pathToExpectedData, "r")) {
        ++validFileCount;

        MMMatrix M;
        MMMatrixRead(&M, pathToMatrix);

        // Dump to this external file
        char* pathToReportedData = malloc(STR_LEN);
        BUILD_MATRIX_FILE_PATH(entry,
            "reported/",
            "_spmv_x_1.out",
            C_str,
            sigma_str,
            pathToReportedData);
        FILE* reportedData = fopen(pathToReportedData, "w");
        if (reportedData == NULL) {
          perror("fopen failed for reportedData");
          exit(EXIT_FAILURE);
        }

        GMatrix m;
        Matrix A;
        Comm c;

        // Set single rank defaults
        M.startRow = 0;
        M.stopRow  = M.nr;
        M.totalNr  = M.nr;
        M.totalNnz = M.nnz;
        c.rank     = 0;
        c.size     = 1;
        c.logFile  = reportedData;

        matrixConvertfromMM(&M, &m);
        convertMatrix(&A, &m);

        CG_FLOAT* x = (CG_FLOAT*)allocate(ARRAY_ALIGNMENT,
            A.nrPadded * sizeof(CG_FLOAT));
        CG_FLOAT* y = (CG_FLOAT*)allocate(ARRAY_ALIGNMENT,
            A.nrPadded * sizeof(CG_FLOAT));

        // Fix x = 1 for now
        for (int i = 0; i < A.nrPadded; ++i) {
          x[i] = (CG_FLOAT)1.0;
          y[i] = (CG_FLOAT)0.0;
        }

        spMVM(&A, x, y);

        commVectorDump(&c, y, A.nr, pathToReportedData);
        fclose(reportedData);

        // If the expect and reported data differ in some way
        if (diff_files(pathToExpectedData, pathToReportedData)) {
          free(pathToReportedData);
          free(pathToExpectedData);
          free(pathToMatrix);

          closedir(dir);
          return 1;
        }
      }
      free(pathToExpectedData);
      free(pathToMatrix);
    }
  }

  closedir(dir);

  if (!validFileCount) {
    fprintf(stderr, "No valid files found in %s\n", pathToMatrices);
    free(pathToMatrices);
    return 1;
  } else {
    free(pathToMatrices);
    return 0;
  }
}