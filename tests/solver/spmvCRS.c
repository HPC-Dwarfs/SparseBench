// DL 2025.06.25
// Unit test: Performs SpMV (sparse matrix-vector multiplication) in CRS format
//            and compares against expected output from disk.
// Assumes a single MPI rank (no parallel communication).

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

/**
 * Runs SpMV tests for all `.mtx` files in `dataDir/testMatrices/`.
 *
 * For each matrix:
 *  1. Loads the file as a Matrix Market input
 *  2. Computes y = A * x with x = 1
 *  3. Dumps the result to a file
 *  4. Compares against expected result (if present)
 *
 * Returns 0 on success, 1 if any test fails.
 */

int test_spmvCRS(void* args, const char* dataDir)
{
  // Compose path to directory containing test matrices
  char matricesPath[STR_LEN];
  snprintf(matricesPath, STR_LEN, "%s%s", dataDir, "testMatrices/");

  // Open the matrix directory for reading
  DIR* dir = opendir(matricesPath);
  if (dir == NULL) {
    perror("Error opening directory");
    return 1;
  }

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

      FORMAT_AND_STRIP_VECTOR_FILE(entry);

      // Path to expected output (".in" file) for this matrix
      char pathToExpectedData[STR_LEN];
      snprintf(pathToExpectedData,
          STR_LEN,
          "data/expected/%s_spmv_x_1.in",
          entry->d_name);

      // Only test if expected file exists
      if (fopen(pathToExpectedData, "r")) {

        // Path to write reported output (".out" file)
        char pathToReportedData[STR_LEN];
        snprintf(pathToReportedData,
            STR_LEN,
            "data/reported/%s_CRS_spmv_x_1.out",
            entry->d_name);

        // Open output file for writing results
        FILE* reportedData = fopen(pathToReportedData, "w");
        if (reportedData == NULL) {
          perror("fopen failed for reportedData");
          exit(EXIT_FAILURE);
        }

        // Read matrix from Matrix Market file
        MMMatrixRead(&M, matrixPath);

        // Declare matrix and communication structs
        GMatrix m;
        Matrix A;
        Comm c;

        // Set single-rank defaults
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

        CG_FLOAT* x = (CG_FLOAT*)allocate(ARRAY_ALIGNMENT,
            A.nr * sizeof(CG_FLOAT));
        CG_FLOAT* y = (CG_FLOAT*)allocate(ARRAY_ALIGNMENT,
            A.nr * sizeof(CG_FLOAT));

        for (int i = 0; i < A.nr; ++i) {
          x[i] = (CG_FLOAT)1.0;
          y[i] = (CG_FLOAT)0.0;
        }

        spMVM(&A, x, y);
        // Output formatted result
        commVectorDump(&c, y, A.nr, pathToReportedData);
        fclose(reportedData);

        // Diff against expected file — if mismatch, fail the test
        // Need to offset the reported data by 1 line
        if (diff_files(pathToExpectedData, pathToReportedData, 1)) {
          closedir(dir);
          return 1;
        }
      }
    }
  }

  // Cleanup and exit
  closedir(dir);
  return 0;
}
