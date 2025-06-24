// DL 2025.04.04
// Single rank test to convert MM to SCS format

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

#include "../../src/comm.h"
#include "../../src/matrix.h"
#include "../common.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_convertSCS(void* args, const char* dataDir)
{

  int rank = 0;
  int size = 1;

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

      // String preprocessing
      char C_str[STR_LEN];
      char sigma_str[STR_LEN];
      FORMAT_AND_STRIP_MATRIX_FILE(M, entry, C_str, sigma_str, arguments)

      // This is the external file to check against
      char* pathToExpectedData = malloc(STR_LEN);
      BUILD_MATRIX_FILE_PATH(entry,
          "expected/",
          ".in",
          C_str,
          sigma_str,
          pathToExpectedData);

      // Validate against expected data, if it exists
      if (fopen(pathToExpectedData, "r")) {

        // Dump to this external file
        char* pathToReportedData = malloc(STR_LEN);
        BUILD_MATRIX_FILE_PATH(entry,
            "reported/",
            ".out",
            C_str,
            sigma_str,
            pathToReportedData);
        FILE* reportedData = fopen(pathToReportedData, "w");
        if (reportedData == NULL) {
          perror("fopen failed for reportedData");
          exit(EXIT_FAILURE);
        }

        MMMatrixRead(&M, pathToMatrix);

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

        A.C     = arguments->C;
        A.sigma = arguments->sigma;

        convertMatrix(&A, &m);
        commMatrixDump(&c, &A);
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

  free(pathToMatrices);
  closedir(dir);

  return 0;
}