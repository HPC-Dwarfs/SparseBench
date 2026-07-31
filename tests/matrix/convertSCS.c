// DL 2025.04.04
// Single rank test to convert MM to SCS format

#include "../../src/matrix.h"
#include "../common.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_convertSCS(void *args, const char *dataDir)
{

  int rank = 0;
  int size = 1;
  int validFileCount = 0;
  int skippedFiles   = 0;

  /* The reported/ output directory is git-ignored, create it if missing */
  char *pathToReported = malloc(strlen(dataDir) + strlen("reported/") + 1);
  strcpy(pathToReported, dataDir);
  strcat(pathToReported, "reported/");
  if (ensureDir(pathToReported) != 0) {
    free(pathToReported);
    return 1;
  }
  free(pathToReported);

  // Open the directory
  char *pathToMatrices = malloc(strlen(dataDir) + strlen("testMatrices/") + 1);
  strcpy(pathToMatrices, dataDir);
  strcat(pathToMatrices, "testMatrices/");

  DIR *dir = opendir(pathToMatrices);
  if (dir == NULL) {
    perror("Error opening directory");
    return 1;
  }

  // Read the directory entries
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strstr(entry->d_name, ".mtx") != NULL) {
      char *pathToMatrix = malloc(strlen(pathToMatrices) + strlen(entry->d_name) + 1);
      strcpy(pathToMatrix, pathToMatrices);
      strcat(pathToMatrix, entry->d_name);

      Matrix A; // thsi is the crs/sell matrix
      Args *arguments = (Args *)args;
      A.C             = arguments->C;
      A.sigma         = arguments->sigma;

      // String preprocessing
      char C_str[STR_LEN];
      char sigma_str[STR_LEN];
      FORMAT_AND_STRIP_MATRIX_FILE(A, entry, C_str, sigma_str)

      // This is the external file to check against
      char *pathToExpectedData = malloc(STR_LEN);
      BUILD_MATRIX_FILE_PATH(
          entry, "expected/", ".in", C_str, sigma_str, pathToExpectedData);

      // Validate against expected data, if it exists
      FILE *fptr = fopen(pathToExpectedData, "r");
      if (!fptr) {
        // No reference data for this matrix/configuration: skip it instead of
        // crashing on fclose(NULL) as before.
        skippedFiles++;
        free(pathToExpectedData);
        free(pathToMatrix);
        continue;
      }
      ++validFileCount;
      {

        MMMatrix m;
        MMMatrixRead(&m, pathToMatrix);

        GMatrix gm;
        matrixConvertfromMM(&m, &gm);

        // Set single rank defaults for MmMatrix
        // m.startRow = 0;
        // m.stopRow = m.nr;
        // m.totalNr = m.nr;
        // m.totalNnz = m.nnz;

        convertMatrix(&A, &gm);

        // Dump to this external file
        char *pathToReportedData = malloc(STR_LEN);
        BUILD_MATRIX_FILE_PATH(
            entry, "reported/", ".out", C_str, sigma_str, pathToReportedData);
        FILE *reportedData = xfopen(pathToReportedData, "w");
        if (reportedData == NULL) {
          free(pathToReportedData);
          fclose(fptr);
          free(pathToExpectedData);
          free(pathToMatrix);
          free(pathToMatrices);
          closedir(dir);
          return 1;
        }

        dumpMatrix_impl(&A, reportedData);
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
      fclose(fptr);
      free(pathToExpectedData);
      free(pathToMatrix);
    }
  }

  if (!validFileCount) {
    fprintf(stderr,
        "No valid files found in %s (%d skipped)\n",
        pathToMatrices,
        skippedFiles);
    free(pathToMatrices);
    closedir(dir);
    return 1;
  }

  free(pathToMatrices);
  closedir(dir);

  return 0;
}