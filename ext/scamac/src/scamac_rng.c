#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>

#include "scamac_hash.h"
#include "scamac_rng.h"

#include "scamac_rng_jump1024table_inc.c"

static void generate_1024(uint64_t *st, uint64_t *ranv)
{
  int i;
  for (i = 0; i < 1024; i++) {
    ranv[i]     = st[i % 16] * UINT64_C(1181783497276652981);
    uint64_t s0 = st[i % 16];
    uint64_t s1 = st[(i + 1) % 16];
    s1 ^= s1 << 31;
    s1 ^= s1 >> 11;
    s0 ^= s0 >> 30;
    st[(i + 1) % 16] = s0 ^ s1;
  }
}

static void jump_ahead_pow2(int n, uint64_t *st)
{
  assert((n >= 0) && (n <= 512));
  int i, j, k;
  uint64_t tt[16];
  for (k = 0; k < 16; k++) {
    tt[k] = st[k];
    st[k] = 0;
  }
  for (i = 0; i < 16; i++) {
    for (j = 0; j < 64; j++) {
      if (JUMP1024[16 * n + i] & (UINT64_C(1) << j)) {
        for (k = 0; k < 16; k++) {
          st[k] ^= tt[(k + j) % 16];
        }
      }
      uint64_t s0 = tt[j % 16];
      uint64_t s1 = tt[(j + 1) % 16];
      s1 ^= s1 << 31;
      s1 ^= s1 >> 11;
      s0 ^= s0 >> 30;
      tt[(j + 1) % 16] = s0 ^ s1;
    }
  }
}

static void jump_ahead(uint64_t n, uint64_t *st)
{
  int i = 0;
  while (n) {
    if (n & 1) {
      jump_ahead_pow2(i, st);
    }
    i++;
    n = n >> 1;
  }
}

static void seed_to_state(uint64_t seed, uint64_t *st)
{
  int i;
  for (i = 0; i < 16; i++) {
    st[i] = seed;
    seed  = (~seed) * UINT64_C(12530480023924335475);
  }
  jump_ahead_pow2(17, st);
  jump_ahead_pow2(27, st);
  jump_ahead_pow2(37, st);
}

ScamacErrorCode scamac_ransrc_alloc(scamac_rng_seed_ty seed, scamac_ransrc_st **ransrc)
{
  if (!ransrc) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }
  scamac_ransrc_st *rs = malloc(sizeof *rs);
  if (!rs) {
    return SCAMAC_EMALLOCFAIL;
  }
  ScamacErrorCode err;
  err = scamac_ransrc_set_seed(seed, rs);
  if (err) {
    free(rs);
    return err;
  }
  *ransrc = rs;
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_ransrc_free(scamac_ransrc_st *ransrc)
{
  free(ransrc);
  return SCAMAC_EOK;
}

ScamacErrorCode scamac_ransrc_set_seed(scamac_rng_seed_ty seed, scamac_ransrc_st *ransrc)
{
  if (!ransrc) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }
  seed_to_state(seed, ransrc->sinit);
  ransrc->sliceidx = -1;
  return SCAMAC_EOK;
}

uint64_t scamac_ransrc_uint64(scamac_ransrc_st *ransrc, uint64_t i)
{
  if ((i >> 10) != ransrc->sliceidx) {
    uint64_t st[16];
    int j;
    for (j = 0; j < 16; j++) {
      st[j] = ransrc->sinit[j];
    }
    jump_ahead((i >> 10) << 10, st);
    generate_1024(st, ransrc->prns);
    ransrc->sliceidx = i >> 10;
  }
  return ransrc->prns[i % 1024];
}

double scamac_ransrc_double(scamac_ransrc_st *ransrc, double a, double b, uint64_t i)
{
  uint64_t u = scamac_ransrc_uint64(ransrc, i);
  double v   = ((double)(u >> 11)) / (1.0 + (UINT64_MAX >> 11));
  return a + (b - a) * v;
}

ScamacErrorCode scamac_ranvec_uint64(
    scamac_rng_seed_ty seed, uint64_t iv, uint64_t nv, uint64_t *ranvec)
{
  if (!ranvec) {
    return SCAMAC_ENULL | 4 << SCAMAC_ESHIFT;
  }

  uint64_t st[16];
  seed_to_state(seed, st);

  uint64_t mi;
  mi = (iv >> 10) << 10;

  jump_ahead(mi, st);

  uint64_t i, j;

  for (i = mi; i < iv; i++) {
    uint64_t s0 = st[i % 16];
    uint64_t s1 = st[(i + 1) % 16];
    s1 ^= s1 << 31;
    s1 ^= s1 >> 11;
    s0 ^= s0 >> 30;
    st[(i + 1) % 16] = s0 ^ s1;
  }

  j = 0;
  for (i = iv; i < iv + nv; i++) {
    ranvec[j] = st[i % 16] * UINT64_C(1181783497276652981);
    j++;
    uint64_t s0 = st[i % 16];
    uint64_t s1 = st[(i + 1) % 16];
    s1 ^= s1 << 31;
    s1 ^= s1 >> 11;
    s0 ^= s0 >> 30;
    st[(i + 1) % 16] = s0 ^ s1;
  }

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_ranvec_double(
    scamac_rng_seed_ty seed, uint64_t iv, uint64_t nv, double a, double b, double *ranvec)
{
  if (!ranvec) {
    return SCAMAC_ENULL | 4 << SCAMAC_ESHIFT;
  }

  uint64_t st[16];
  seed_to_state(seed, st);

  uint64_t mi;
  mi = (iv >> 10) << 10;

  jump_ahead(mi, st);

  uint64_t i, j;

  for (i = mi; i < iv; i++) {
    uint64_t s0 = st[i % 16];
    uint64_t s1 = st[(i + 1) % 16];
    s1 ^= s1 << 31;
    s1 ^= s1 >> 11;
    s0 ^= s0 >> 30;
    st[(i + 1) % 16] = s0 ^ s1;
  }

  j = 0;
  for (i = iv; i < iv + nv; i++) {
    uint64_t u = st[i % 16] * UINT64_C(1181783497276652981);
    double v   = ((double)(u >> 11)) / (1.0 + (UINT64_MAX >> 11));
    ranvec[j]  = a + (b - a) * v;
    j++;
    uint64_t s0 = st[i % 16];
    uint64_t s1 = st[(i + 1) % 16];
    s1 ^= s1 << 31;
    s1 ^= s1 >> 11;
    s0 ^= s0 >> 30;
    st[(i + 1) % 16] = s0 ^ s1;
  }

  return SCAMAC_EOK;
}

scamac_rng_seed_ty scamac_rng_string_to_seed(const char *str)
{
  if (!str) {
    return 0;
  }
  size_t l = strlen(str);
  while (l > 0) {
    if (str[0] == ' ') {
      str++;
      l--;
    } else {
      break;
    }
  }
  if (l < 1) {
    return 0;
  }
  if (l <= 20) {
    int is_dec = 1;
    unsigned int i;
    for (i = 0; i < l; i++) {
      if (str[i] < '0' || str[i] > '9') {
        is_dec = 0;
        break;
      }
    }
    if (is_dec) {
      uint64_t seed;
      sscanf(str, "%" SCNu64, &seed);
      return seed;
    }
  }
  if (l <= 18) {
    int is_hex = 1;
    if (l > 2) {
      if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
        l -= 2;
      }
    }
    unsigned int i;
    for (i = 0; i < l; i++) {
      if (str[i] < '0' || str[i] > 'f') {
        is_hex = 0;
        break;
      }
      if (str[i] > '9' && str[i] < 'A') {
        is_hex = 0;
        break;
      }
      if (str[i] > 'F' && str[i] < 'a') {
        is_hex = 0;
        break;
      }
    }
    if (is_hex) {
      uint64_t seed;
      sscanf(str, "%" SCNx64, &seed);
      return seed;
    }
  }

  uint64_t seed = 0;
  uint32_t digest[4];
  ScamacHashState st;
  scamac_hash_init(&st);
  scamac_hash_process(&st, strlen(str) * sizeof *str, (void *)str);
  scamac_hash_finish(&st);
  scamac_hash_digest(&st, digest);
  seed = (((uint64_t)digest[1]) << 32) + digest[0];

  return seed;
}

scamac_rng_seed_ty scamac_rng_entropy(const char *str)
{
  scamac_rng_seed_ty rseed;
  rseed = scamac_rng_string_to_seed(str);

  char entstr[80];

  time_t rawtime;
  time(&rawtime);
  snprintf(entstr, 80, "%s", ctime(&rawtime));

  rseed += scamac_rng_string_to_seed(entstr);

  return rseed;
}
