#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "scamac_error.h"

scamac_error_handler_ft scamac_error_handler = NULL;

const char *scamac_error_desc(ScamacErrorCode err)
{
  const int MaxErrorN                       = 14;
  static const char *ErrorStrings[]         = { /* OK */ "== OK: Hurrah! ==",
    /* FAIL */ "== error FAIL (Failed for no reason) ==",
    /* NULL */ "== error NULL (Unexpected NULL pointer) ==",
    /* INVALID */ "== error INVALID (Invalid parameter) ==",
    /* RANGE */ "== error RANGE (Parameter out of range) ==",
    /* CORRUPTED */ "== error CORRUPTED (Object is corrupted) ==",
    /* SCOPE */ "== error SCOPE (Requested omputation not covered) ==",
    /* INPUT */ "== error INPUT (Invalid set of input parameters) ==",
    /* OVERFLOW */ "== error OVERFLOW (Matrix size exceeds IDX_MAX) ==",
    /* MALLOCFAIL */ "== error MALLOCFAIL (Memory allocation failed) ==",
    /* HUGEINT */ "== error HUGEINT (An integer parameters is too large) ==",
    /* HUGECOMP */ "== error HUGECOMP (A computed integer is too large) ==",
    /* WARNING */ "== error WARNING (A warning -- might be ignored) ==",
    /* NOTCONVERGED */ "== error NOTCONVERGED (Failed to converged) ==",
    /* SHORTROW */ "== error SHORTROW (Short row) ==" };
  static const char *InternalErrorStrings[] = { /* OK */ "== OK: Hurrah! ==",
    /* FAIL */ "== internal error FAIL (Failed for no reason) ==",
    /* NULL */ "== internal error NULL (Unexpected NULL pointer) ==",
    /* INVALID */ "== internal error INVALID (Invalid parameter) ==",
    /* RANGE */ "== internal error RANGE (Parameter out of range) ==",
    /* CORRUPTED */ "== internal error CORRUPTED (Object is corrupted) ==",
    /* SCOPE */ "== internal error SCOPE (Requested omputation not covered) ==",
    /* INPUT */ "== internal error INPUT (Invalid set of input parameters) ==",
    /* OVERFLOW */ "== internal error OVERFLOW (Matrix size exceeds IDX_MAX) ==",
    /* MALLOCFAIL */ "== internal error MALLOCFAIL (Memory allocation failed) ==",
    /* HUGEINT */ "== internal error HUGEINT (An integer parameters is too large) ==",
    /* HUGECOMP */ "== internal error HUGECOMP (A computed integer is too large) ==",
    /* WARNING */ "== internal error WARNING (A warning -- might be ignored) ==",
    /* NOTCONVERGED */ "== internal error NOTCONVERGED (Failed to converged) ==",
    /* SHORTROW */ "== internal error SHORTROW (Short row) ==" };

  int errno                                 = err & SCAMAC_EMASK;
  int isinternal                            = err & SCAMAC_EINTERNAL;
  if (errno > MaxErrorN) {
    errno = 1;
  }
  if (isinternal) {
    return InternalErrorStrings[errno];
  } else {
    return ErrorStrings[errno];
  }
}

int scamac_error_par(ScamacErrorCode err)
{
  int parno = (err >> SCAMAC_ESHIFT) & SCAMAC_EMASK;
  return parno;
}

const char *scamac_error_dpar(ScamacErrorCode err)
{
  static const char *ErrorParStrings[] = { "",
    " at parameter (1) ==",
    " at parameter (2) ==",
    " at parameter (3) ==",
    " at parameter (4) ==",
    " at parameter (5) ==",
    " at parameter (6) ==",
    " at parameter (7) ==",
    " at parameter (8) ==",
    " at parameter (9) ==",
    " at parameter (10) ==",
    " at parameter (11) ==",
    " at parameter (12) ==",
    " at parameter (13) ==",
    " at parameter (14) ==",
    " at parameter (15) ==",
    " at parameter (16) ==",
    " at parameter (17) ==",
    " at parameter (18) ==",
    " at parameter (19) ==",
    " at parameter (20) ==",
    " at parameter (21) ==",
    " at parameter (22) ==",
    " at parameter (23) ==",
    " at parameter (24) ==",
    " at parameter (25) ==",
    " at parameter (26) ==",
    " at parameter (27) ==",
    " at parameter (28) ==",
    " at parameter (29) ==",
    " at parameter (30) ==",
    " at parameter (31) ==",
    " at parameter (32) ==",
    " at parameter (33) ==",
    " at parameter (34) ==",
    " at parameter (35) ==",
    " at parameter (36) ==",
    " at parameter (37) ==",
    " at parameter (38) ==",
    " at parameter (39) ==",
    " at parameter (40) ==",
    " at parameter (41) ==",
    " at parameter (42) ==",
    " at parameter (43) ==",
    " at parameter (44) ==",
    " at parameter (45) ==",
    " at parameter (46) ==",
    " at parameter (47) ==",
    " at parameter (48) ==",
    " at parameter (49) ==",
    " at parameter (50) ==",
    " at parameter (51) ==",
    " at parameter (52) ==",
    " at parameter (53) ==",
    " at parameter (54) ==",
    " at parameter (55) ==",
    " at parameter (56) ==",
    " at parameter (57) ==",
    " at parameter (58) ==",
    " at parameter (59) ==",
    " at parameter (60) ==",
    " at parameter (61) ==",
    " at parameter (62) ==",
    " at parameter (63) ==" };
  int parno                            = scamac_error_par(err);
  int errno                            = err & SCAMAC_EMASK;
  if (errno == SCAMAC_ENULL || errno == SCAMAC_EINVALID || errno == SCAMAC_ERANGE ||
      errno == SCAMAC_ECORRUPTED || errno == SCAMAC_EWARNING) {
    return ErrorParStrings[parno];
  } else {
    return ErrorParStrings[0];
  }
}

ScamacErrorCode scamac_error_discard_warning(ScamacErrorCode err)
{
  return (err & SCAMAC_EMASK) == SCAMAC_EWARNING ? SCAMAC_EOK : err;
}

ScamacErrorCode scamac_error_set_internal(ScamacErrorCode err)
{
  if ((err == SCAMAC_ENULL) || (err == SCAMAC_EINVALID) || (err == SCAMAC_ERANGE) ||
      (err == SCAMAC_ESCOPE) || (err == SCAMAC_EINPUT) || ((err) == SCAMAC_ESHORTROW) ||
      (err == SCAMAC_EHUGEINT)) {
    return err | SCAMAC_EINTERNAL;
  } else {
    return err;
  }
}

ScamacErrorCode scamac_error_set_par(ScamacErrorCode err, int par)
{
  SCAMAC_ASSERT((par >= 1) && (par < 64));
  return (err & SCAMAC_EMASK) | (par << SCAMAC_ESHIFT);
}

bool scamac_error_is(ScamacErrorCode err1, ScamacErrorCode err2)
{
  return (err1 & SCAMAC_EMASK) == (err2 & SCAMAC_EMASK);
}
