#include "scamac_hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define F(X, Y, Z) ((((Y) ^ (Z)) & (X)) ^ (Z))
#define G(X, Y, Z) ((((Y) ^ (X)) & (Z)) ^ (Y))
#define H(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define I(X, Y, Z) ((Y) ^ ((X) | ~(Z)))

#define CLROT(X, n) (((X) << (n)) | ((X) >> (32 - (n))))

ScamacErrorCode scamac_hash_init(ScamacHashState *st)
{
  if (!st) {
    return SCAMAC_ENULL;
  }
  st->a[0] = 0x67452301;
  st->a[1] = 0xefcdab89;
  st->a[2] = 0x98badcfe;
  st->a[3] = 0x10325476;
  st->n    = 0;
  return SCAMAC_EOK;
}

static void md5_process_chunk(struct scamac_md5_state_st *st, const uint32_t *chunk)
{
  uint32_t A, B, C, D;
  A = st->a[0];
  B = st->a[1];
  C = st->a[2];
  D = st->a[3];

  A = B + CLROT(A + F(B, C, D) + chunk[0] + 0xD76AA478, 7);
  D = A + CLROT(D + F(A, B, C) + chunk[1] + 0xE8C7B756, 12);
  C = D + CLROT(C + F(D, A, B) + chunk[2] + 0x242070DB, 17);
  B = C + CLROT(B + F(C, D, A) + chunk[3] + 0xC1BDCEEE, 22);
  A = B + CLROT(A + F(B, C, D) + chunk[4] + 0xF57C0FAF, 7);
  D = A + CLROT(D + F(A, B, C) + chunk[5] + 0x4787C62A, 12);
  C = D + CLROT(C + F(D, A, B) + chunk[6] + 0xA8304613, 17);
  B = C + CLROT(B + F(C, D, A) + chunk[7] + 0xFD469501, 22);
  A = B + CLROT(A + F(B, C, D) + chunk[8] + 0x698098D8, 7);
  D = A + CLROT(D + F(A, B, C) + chunk[9] + 0x8B44F7AF, 12);
  C = D + CLROT(C + F(D, A, B) + chunk[10] + 0xFFFF5BB1, 17);
  B = C + CLROT(B + F(C, D, A) + chunk[11] + 0x895CD7BE, 22);
  A = B + CLROT(A + F(B, C, D) + chunk[12] + 0x6B901122, 7);
  D = A + CLROT(D + F(A, B, C) + chunk[13] + 0xFD987193, 12);
  C = D + CLROT(C + F(D, A, B) + chunk[14] + 0xA679438E, 17);
  B = C + CLROT(B + F(C, D, A) + chunk[15] + 0x49B40821, 22);

  A = B + CLROT(A + G(B, C, D) + chunk[1] + 0xF61E2562, 5);
  D = A + CLROT(D + G(A, B, C) + chunk[6] + 0xC040B340, 9);
  C = D + CLROT(C + G(D, A, B) + chunk[11] + 0x265E5A51, 14);
  B = C + CLROT(B + G(C, D, A) + chunk[0] + 0xE9B6C7AA, 20);
  A = B + CLROT(A + G(B, C, D) + chunk[5] + 0xD62F105D, 5);
  D = A + CLROT(D + G(A, B, C) + chunk[10] + 0x02441453, 9);
  C = D + CLROT(C + G(D, A, B) + chunk[15] + 0xD8A1E681, 14);
  B = C + CLROT(B + G(C, D, A) + chunk[4] + 0xE7D3FBC8, 20);
  A = B + CLROT(A + G(B, C, D) + chunk[9] + 0x21E1CDE6, 5);
  D = A + CLROT(D + G(A, B, C) + chunk[14] + 0xC33707D6, 9);
  C = D + CLROT(C + G(D, A, B) + chunk[3] + 0xF4D50D87, 14);
  B = C + CLROT(B + G(C, D, A) + chunk[8] + 0x455A14ED, 20);
  A = B + CLROT(A + G(B, C, D) + chunk[13] + 0xA9E3E905, 5);
  D = A + CLROT(D + G(A, B, C) + chunk[2] + 0xFCEFA3F8, 9);
  C = D + CLROT(C + G(D, A, B) + chunk[7] + 0x676F02D9, 14);
  B = C + CLROT(B + G(C, D, A) + chunk[12] + 0x8D2A4C8A, 20);

  A = B + CLROT(A + H(B, C, D) + chunk[5] + 0xFFFA3942, 4);
  D = A + CLROT(D + H(A, B, C) + chunk[8] + 0x8771F681, 11);
  C = D + CLROT(C + H(D, A, B) + chunk[11] + 0x6D9D6122, 16);
  B = C + CLROT(B + H(C, D, A) + chunk[14] + 0xFDE5380C, 23);
  A = B + CLROT(A + H(B, C, D) + chunk[1] + 0xA4BEEA44, 4);
  D = A + CLROT(D + H(A, B, C) + chunk[4] + 0x4BDECFA9, 11);
  C = D + CLROT(C + H(D, A, B) + chunk[7] + 0xF6BB4B60, 16);
  B = C + CLROT(B + H(C, D, A) + chunk[10] + 0xBEBFBC70, 23);
  A = B + CLROT(A + H(B, C, D) + chunk[13] + 0x289B7EC6, 4);
  D = A + CLROT(D + H(A, B, C) + chunk[0] + 0xEAA127FA, 11);
  C = D + CLROT(C + H(D, A, B) + chunk[3] + 0xD4EF3085, 16);
  B = C + CLROT(B + H(C, D, A) + chunk[6] + 0x04881D05, 23);
  A = B + CLROT(A + H(B, C, D) + chunk[9] + 0xD9D4D039, 4);
  D = A + CLROT(D + H(A, B, C) + chunk[12] + 0xE6DB99E5, 11);
  C = D + CLROT(C + H(D, A, B) + chunk[15] + 0x1FA27CF8, 16);
  B = C + CLROT(B + H(C, D, A) + chunk[2] + 0xC4AC5665, 23);

  A = B + CLROT(A + I(B, C, D) + chunk[0] + 0xF4292244, 6);
  D = A + CLROT(D + I(A, B, C) + chunk[7] + 0x432AFF97, 10);
  C = D + CLROT(C + I(D, A, B) + chunk[14] + 0xAB9423A7, 15);
  B = C + CLROT(B + I(C, D, A) + chunk[5] + 0xFC93A039, 21);
  A = B + CLROT(A + I(B, C, D) + chunk[12] + 0x655B59C3, 6);
  D = A + CLROT(D + I(A, B, C) + chunk[3] + 0x8F0CCC92, 10);
  C = D + CLROT(C + I(D, A, B) + chunk[10] + 0xFFEFF47D, 15);
  B = C + CLROT(B + I(C, D, A) + chunk[1] + 0x85845DD1, 21);
  A = B + CLROT(A + I(B, C, D) + chunk[8] + 0x6FA87E4F, 6);
  D = A + CLROT(D + I(A, B, C) + chunk[15] + 0xFE2CE6E0, 10);
  C = D + CLROT(C + I(D, A, B) + chunk[6] + 0xA3014314, 15);
  B = C + CLROT(B + I(C, D, A) + chunk[13] + 0x4E0811A1, 21);
  A = B + CLROT(A + I(B, C, D) + chunk[4] + 0xF7537E82, 6);
  D = A + CLROT(D + I(A, B, C) + chunk[11] + 0xBD3AF235, 10);
  C = D + CLROT(C + I(D, A, B) + chunk[2] + 0x2AD7D2BB, 15);
  B = C + CLROT(B + I(C, D, A) + chunk[9] + 0xEB86D391, 21);

  st->a[0] += A;
  st->a[1] += B;
  st->a[2] += C;
  st->a[3] += D;
}

ScamacErrorCode scamac_hash_process(ScamacHashState *st, int nb, const uint8_t *data)
{
  if (!st) {
    return SCAMAC_ENULL;
  }
  if (nb == 0) {
    return SCAMAC_EOK;
  }
  if (nb < 0) {
    return SCAMAC_ERANGE | 2 << SCAMAC_ESHIFT;
  }

  if (!data) {
    return SCAMAC_ENULL;
  }

  int k = 0;

  if (st->n % 64 != 0) {
    int nn = 64 - (st->n % 64);
    if (nn > nb) {
      nn = nb;
    }
    memcpy((uint8_t *)(st->rem) + st->n % 64, data, nn);
    (st->n) += nn;
    k += nn;
    if (st->n % 64 == 0) {
      md5_process_chunk(st, st->rem);
    }
  }

  while (k + 64 <= nb) {
    md5_process_chunk(st, (uint32_t *)&(data[k]));
    (st->n) += 64;
    k += 64;
  }

  memcpy(&(st->rem), data + k, nb - k);
  (st->n) += nb - k;

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_hash_finish(ScamacHashState *st)
{
  if (!st) {
    return SCAMAC_ENULL;
  }
  uint8_t pad[72];

  int nn;
  nn = 64 - 9 - (st->n % 64);
  if (nn < 0) {
    nn += 64;
  }

  pad[0] = 0x80;
  int i;
  for (i = 0; i < nn; i++) {
    pad[i + 1] = 0;
  }
  pad[nn + 8] = (uint8_t)(((8 * st->n) >> (64 - 8)) & 0xFF);
  pad[nn + 7] = (uint8_t)(((8 * st->n) >> (64 - 16)) & 0xFF);
  pad[nn + 6] = (uint8_t)(((8 * st->n) >> (64 - 24)) & 0xFF);
  pad[nn + 5] = (uint8_t)(((8 * st->n) >> (64 - 32)) & 0xFF);
  pad[nn + 4] = (uint8_t)(((8 * st->n) >> (64 - 40)) & 0xFF);
  pad[nn + 3] = (uint8_t)(((8 * st->n) >> (64 - 48)) & 0xFF);
  pad[nn + 2] = (uint8_t)(((8 * st->n) >> (64 - 56)) & 0xFF);
  pad[nn + 1] = (uint8_t)(((8 * st->n) >> (64 - 64)) & 0xFF);

  scamac_hash_process(st, nn + 9, pad);

  return SCAMAC_EOK;
}

ScamacErrorCode scamac_hash_digest(const ScamacHashState *st, uint32_t digest[4])
{
  if (!st) {
    return SCAMAC_ENULL;
  }
  digest[0] = st->a[0];
  digest[1] = st->a[1];
  digest[2] = st->a[2];
  digest[3] = st->a[3];
  return SCAMAC_EOK;
}

void scamac_print_digest(uint32_t digest[4])
{
  uint8_t *d = (uint8_t *)digest;
  int i;
  for (i = 0; i < 16; i++) {
    printf("%02" PRIx8, d[i]);
  }
}
