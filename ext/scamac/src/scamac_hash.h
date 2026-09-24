#ifndef SCAMAC_HASH_H
#define SCAMAC_HASH_H

#include <inttypes.h>

#include "scamac_error.h"

struct scamac_md5_state_st {
  uint64_t n; // number of bytes processed
  uint32_t a[4];
  uint32_t rem[16]; // remaining bits of incomplete chunk, at most 16 uint32_t
};

typedef struct scamac_md5_state_st ScamacHashState;

ScamacErrorCode scamac_hash_init(ScamacHashState *st);
ScamacErrorCode scamac_hash_process(ScamacHashState *st, int nb, const uint8_t *data);
ScamacErrorCode scamac_hash_finish(ScamacHashState *st);
ScamacErrorCode scamac_hash_digest(const ScamacHashState *st, uint32_t digest[4]);

void scamac_print_digest(uint32_t digest[4]);

#endif /* SCAMAC_HASH_H */
