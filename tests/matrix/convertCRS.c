// DL 2025.06.25
// Unit test: Converts a Matrix Market (MM) matrix to internal CRS format
//            and compares against expected output from disk.
// Assumes a single MPI rank (no parallel communication).

#include "../../src/comm.h"
#include "../../src/matrix.h"
#include "../common.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Runs conversion tests for all `.mtx` files in `dataDir/testMatrices/`.
 *
 * For each matrix:
 *  1. Builds full path to the file
 *  2. Loads MM matrix and converts it to GMatrix, and then CRS
 *  3. Dumps the converted result to a file
 *  4. Compares the dump against expected result (if present)
 *
 * Returns 0 on success, 1 if any test fails.
 */
int test_convertCRS(void* args, const char* dataDir)
{
  // Compose path to directory containing test matrices
  char matricesPath[STR_LEN];
  snprintf(matricesPath, STR_LEN, "%s%s", dataDir, "testMatrices/");

  // Open the directory and check for errors
  DIR* dir = opendir(matricesPath);
  if (dir == NULL) {
    perror("Error opening directory");
    return 1;
  }

  // Read the directory entries
  struct dirent* entry;

  // Iterate through files in the directory
  while ((entry = readdir(dir)) != NULL) {
    // Only process files with ".mtx" extension
    if (strstr(entry->d_name, ".mtx") != NULL) {

      // Build full path to matrix file
      char matrixPath[STR_LEN];
      snprintf(matrixPath, STR_LEN, "%s%s", matricesPath, entry->d_name);

      MMMatrix M;
      Args* arguments = (Args*)args;

      STRIP_MATRIX_FILE(entry) // Removes .mtx from file name

      // Path to expected output (".in" file) for this matrix
      char pathToExpectedData[STR_LEN];
      snprintf(pathToExpectedData,
          STR_LEN,
          "data/expected/%s_CRS.in",
          entry->d_name);

      // Only test if expected file exists
      if (fopen(pathToExpectedData, "r")) {

        // Path to write reported output (".out" file)
        char pathToReportedData[STR_LEN];
        snprintf(pathToReportedData,
            STR_LEN,
            "data/reported/%s_CRS.out",
            entry->d_name);

        // Open output file for writing results
        FILE* reportedData = fopen(pathToReportedData, "w");
        if (reportedData == NULL) {
          perror("fopen failed for reportedData");
          exit(EXIT_FAILURE); // Crash fast on I/O error
        }

        // Load matrix from Matrix Market file
        MMMatrixRead(&M, matrixPath);

        // Declare graph and algebraic matrices
        GMatrix m;
        Matrix A;
        Comm c;

        // Set single-rank defaults (no parallel distribution)
        M.startRow = 0;
        M.stopRow  = M.nr;
        M.totalNr  = M.nr;
        M.totalNnz = M.nnz;
        c.rank     = 0;
        c.size     = 1;
        c.logFile  = reportedData;

        // Convert to internal formats
        matrixConvertfromMM(&M, &m); // MM → GMatrix
        convertMatrix(&A, &m);       // GMatrix → Matrix
        commMatrixDump(&c, &A);      // Output formatted result
        fclose(reportedData);

        // Diff against expected file — if mismatch, fail the test
        if (diff_files(pathToExpectedData, pathToReportedData, 0)) {
          closedir(dir);
          return 1;
        }
      }
    }
  }

  // Clean up and return success
  closedir(dir);
  return 0;
}
