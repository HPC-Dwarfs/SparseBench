/* Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved. This file is part of SparseBench.
 * Use of this source code is governed by a MIT style
 * license that can be found in the LICENSE file. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parameter.h"

#include "cli.h" // BenchType / CHEBFD, for the ChebFD section of printParameter
#define MAXLINE 4096

/* Replace the owned string *target with a copy of value. */
void setParameterString(char **target, const char *value)
{
  char *dup = strdup(value);
  if (dup == NULL) {
    fprintf(stderr, "Error: could not copy parameter value '%s'\n", value);
    exit(EXIT_FAILURE);
  }
  free(*target);
  *target = dup;
}

void setParameterFilename(Parameter *param, const char *filename)
{
  setParameterString(&param->filename, filename);
}

void freeParameter(Parameter *param)
{
  free(param->filename);
  param->filename = NULL;
  free(param->solverVariant);
  param->solverVariant = NULL;
  free(param->gmresOrtho);
  param->gmresOrtho = NULL;
}

/* Stopping mode as shown in the parameter echo and the result summary. */
void formatStopMode(const Parameter *param, char *buf, size_t len)
{
  if (param->eps > 0.0) {
    snprintf(buf, len, "relative tolerance %g (itermax %d)", param->eps, param->itermax);
  } else {
    snprintf(buf, len, "fixed (itermax %d)", param->itermax);
  }
}

void initParameter(Parameter *param)
{
  param->filename      = NULL;
  param->solverVariant = NULL;
  param->gmresOrtho    = NULL;
  setParameterFilename(param, "generate");
  setParameterString(&param->solverVariant, "standard");
  setParameterString(&param->gmresOrtho, "mgs");
  param->nx         = 100;
  param->ny         = 100;
  param->nz         = 100;
  param->itermax    = 150;
  param->eps        = 0.0;
  param->blockwidth = NUMVEC;
  param->restart    = 30;
#ifdef SCS
  param->C     = SELL_CHUNK;
  param->Sigma = SELL_SIGMA;
#endif
  param->verbose   = 0;
  param->device    = 0;
  param->allocType = ALLOC_MANAGED;
  param->chebNb    = 0;
  // NTS : if spectrum bounds unpassed Gershgorin is used
  // NTS : lancozs kern mu=2 used by paper
  param->cheb.a           = 0.0;
  param->cheb.b           = 0.0;
  param->cheb.lam_lo      = 0.0;
  param->cheb.lam_hi      = 0.0;
  param->cheb.Np          = 0;
  param->cheb.NS          = 0;
  param->cheb.kernel      = 3;
  param->cheb.mu          = 2; // from paper
  param->cheb.have_bounds = 0;
  param->cheb.have_target = 0;
}

void readParameter(Parameter *param, const char *filename)
{
  FILE *fp = fopen(filename, "r");
  char line[MAXLINE];
  int i;

  int have_a      = param->cheb.have_bounds;
  int have_b      = param->cheb.have_bounds;
  int have_lam_lo = param->cheb.have_target;
  int have_lam_hi = param->cheb.have_target;

  if (!fp) {
    fprintf(stderr, "Could not open parameter file: %s\n", filename);
    exit(EXIT_FAILURE);
  }

  while (fgets(line, MAXLINE, fp) != NULL) {
    for (i = 0; line[i] != '\0' && line[i] != '#'; i++)
      ;
    line[i] = '\0';

    /* Delimiter set includes \n/\r so that values parsed from a line read by
     * fgets do not retain a trailing newline (which would break later strcmp
     * on string params such as `filename`). */
    char *tok = strtok(line, " \t\r\n");
    char *val = strtok(NULL, " \t\r\n");

#define PARSE_KEY(key, target, conv, extra_flag_to_set)                                  \
  if (strcmp(tok, key) == 0) {                                                           \
    target = conv(val);                                                                  \
    extra_flag_to_set;                                                                   \
  }
#define NO_FLAG ((void)0)

    if (tok != NULL && val != NULL) {
      if (strcmp(tok, "filename") == 0) {
        setParameterFilename(param, val);
      }
      if (strcmp(tok, "solver_variant") == 0) {
        setParameterString(&param->solverVariant, val);
      }
      if (strcmp(tok, "gmres_ortho") == 0) {
        setParameterString(&param->gmresOrtho, val);
      }
      PARSE_KEY("nx", param->nx, atoi, NO_FLAG);
      PARSE_KEY("ny", param->ny, atoi, NO_FLAG);
      PARSE_KEY("nz", param->nz, atoi, NO_FLAG);
      PARSE_KEY("itermax", param->itermax, atoi, NO_FLAG);
      PARSE_KEY("eps", param->eps, atof, NO_FLAG);
      PARSE_KEY("restart", param->restart, atoi, NO_FLAG);
      // NTS : `cheb_` prefix, populating the nested struct
      PARSE_KEY("cheb_a", param->cheb.a, atof, have_a = 1);
      PARSE_KEY("cheb_b", param->cheb.b, atof, have_b = 1);
      PARSE_KEY("cheb_lam_lo", param->cheb.lam_lo, atof, have_lam_lo = 1);
      PARSE_KEY("cheb_lam_hi", param->cheb.lam_hi, atof, have_lam_hi = 1);
      PARSE_KEY("cheb_Np", param->cheb.Np, atoi, NO_FLAG);
      PARSE_KEY("cheb_NS", param->cheb.NS, atoi, NO_FLAG);
      PARSE_KEY("cheb_kernel", param->cheb.kernel, atoi, NO_FLAG);
      PARSE_KEY("cheb_mu", param->cheb.mu, atoi, NO_FLAG);
      // NTS : allocTypeFromName validates and exits on typos
      PARSE_KEY("gpu_alloc", param->allocType, allocTypeFromName, NO_FLAG);
      PARSE_KEY("cheb_nb", param->chebNb, atoi, NO_FLAG);
      if (strcmp(tok, "gpu_stream_mb") == 0) {
        fprintf(stderr,
            "Warning: gpu_stream_mb is obsolete (the matrix is device-resident and "
            "the search space is streamed; see cheb_nb) — ignored.\n");
      }
    }
  }

  // cheb_a and cheb_b must be supplied together; exactly one silently
  // enables user-bounds mode with the other left at its 0.0 default,
  // disabling the Gershgorin spectrum fallback.
  if (have_a != have_b) {
    fprintf(stderr,
        "Error: 'cheb_a' and 'cheb_b' must be supplied together "
        "(both or neither). Supplying only one silently disables the "
        "Gershgorin spectrum fallback.\n");
    exit(EXIT_FAILURE);
  }
  param->cheb.have_bounds = have_a;

  // Likewise for the target interval: a lone bound would silently pair with
  // the other's 0.0 default and target the wrong interval.
  if (have_lam_lo != have_lam_hi) {
    fprintf(stderr,
        "Error: 'cheb_lam_lo' and 'cheb_lam_hi' must be supplied together "
        "(both or neither).\n");
    exit(EXIT_FAILURE);
  }
  param->cheb.have_target = have_lam_lo;

  fclose(fp);
}

void printParameter(Parameter *param)
{
  printf("Parameters\n");
  printf("Iterative solver parameters:\n");
  printf("\tfile name: %s\n", param->filename);
  printf("\tnx: %d\n", param->nx);
  printf("\tny: %d\n", param->ny);
  printf("\tnz: %d\n", param->nz);
  printf("\tMax iterations: %d\n", param->itermax);
  char stopMode[128];
  formatStopMode(param, stopMode, sizeof(stopMode));
  printf("\tStop mode: %s\n", stopMode);
  if (BenchType == CG || BenchType == GMRES) {
    printf("\tSolver variant: %s\n", param->solverVariant);
  }
  if (BenchType == GMRES) {
    printf("\tGMRES orthogonalization: %s\n", param->gmresOrtho);
  }
  printf("\tBlock width: %d\n", param->blockwidth);
  printf("\tGMRES restart dimension: %d\n", param->restart);
#ifdef SCS
  printf("\tSell chunk: %d\n", param->C);
  printf("\tSell sigma: %d\n", param->Sigma);
#endif
  printf("\tVerbose Level: %d\n", param->verbose);
  printf("\tGPU device index: %d\n", param->device);
  printf("\tGPU allocation: %s%s\n",
      allocTypeName(param->allocType),
      BenchType == CHEBFD ? " (ChebFD: matrix managed+prefetched, blocks pinned)" : "");

  if (BenchType == CHEBFD) {
    printf("ChebFD parameters:\n");
    printf("\tspectrum [a,b]: %g, %g%s\n",
        param->cheb.a,
        param->cheb.b,
        param->cheb.have_bounds ? "" : " (auto: Gershgorin)");
    printf("\ttarget interval: [%g, %g]\n", param->cheb.lam_lo, param->cheb.lam_hi);
    printf("\tNp / NS: %d / %d\n", param->cheb.Np, param->cheb.NS);
    printf("\tkernel / mu: %d / %d\n", param->cheb.kernel, param->cheb.mu);
    printf("\tstreamed sub-block width cheb_nb: %d%s\n",
        param->chebNb,
        param->chebNb > 0 ? "" : " (default 16)");
  }
}
