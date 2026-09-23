/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_RNG_H
#define SCAMAC_RNG_H

#include "scamac_error.h"
#include <stdint.h>

// xorshift random number generator according to Sebastiano Vigna, arxiv:1402.6246
// [ ACM Trans. Math. Software 42, 4 (2016) ]

typedef uint64_t scamac_rng_seed_ty;

typedef struct {
  uint64_t sinit[16]; // as seeded

  uint64_t sliceidx;
  uint64_t prns[1024];

} scamac_ransrc_st;

ScamacErrorCode scamac_ransrc_alloc(scamac_rng_seed_ty seed, scamac_ransrc_st **ransrc);
ScamacErrorCode scamac_ransrc_free(scamac_ransrc_st *ransrc);

ScamacErrorCode scamac_ransrc_set_seed(scamac_rng_seed_ty seed, scamac_ransrc_st *ransrc);

/* single random numbers from source */
// unsigned integer, 64 bits (i>=0)
uint64_t scamac_ransrc_uint64(scamac_ransrc_st *ransrc, uint64_t i);

// uniform real in [a,b)  (i>=0)
double scamac_ransrc_double(scamac_ransrc_st *ransrc, double a, double b, uint64_t i);

/* random vectors */
// get n random uint64_t's, with indices i ... i+n-1
ScamacErrorCode scamac_ranvec_uint64(
    scamac_rng_seed_ty seed, uint64_t iv, uint64_t nv, uint64_t *ranvec);
ScamacErrorCode scamac_ranvec_double(scamac_rng_seed_ty seed,
    uint64_t iv,
    uint64_t nv,
    double a,
    double b,
    double *ranvec);

/**
 *  converts a NUL-terminated string into a (uint64) seed
 */
scamac_rng_seed_ty scamac_rng_string_to_seed(const char *str);

scamac_rng_seed_ty scamac_rng_entropy(const char *str);

#endif /* SCAMAC_RNG_H */
