/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ScaMaC data structure and macro definitions
 *  \ingroup public
 */

#ifndef SCAMAC_ERROR_H
#define SCAMAC_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* error codes */
/* we support up to 2^6 = 63 error codes + 1 "EOK"
 * The error codes ENULL ... ECORRUPTED can be OR'ed with the position of the problematic input value,
 * that is if the i-th parameter is problematic, with 1 <= i <= 31, functions return, e.g,
 * SCAMAC_EINVALID | (i << SCAMAC_ESHIFT) or SCAMAC_EINVALID | (i << SCAMAC_ESHIFT) | SCAMAC_EINTERNAL
 * The shift is given by SCAMAC_ESHIFT = 7
 */
/* This definition must agree with the list of error names
   in scamac_error_desc() in scamac_collection.c */
#define SCAMAC_ESHIFT 6
#define SCAMAC_EMASK ((1U << SCAMAC_ESHIFT) - 1)
typedef enum {
// +++ basic status
// routine completed successfully
  SCAMAC_EOK = 0,
// routine failed for an unspecified reason
  SCAMAC_EFAIL,
// +++
// +++ problems with specific function parameters (on input)
// a null pointer was supplied in the wrong place
  SCAMAC_ENULL,
// an input parameter, or the combination of all parameters, is invalid
  SCAMAC_EINVALID,
  // an (integer or double) input parameter is outside of the valid range (e.g, is negative when a positive value is expected)
  SCAMAC_ERANGE,
  // an object passed to a routine, probably constructed elsewhere, is corrupted. Can signal an internal error,
  // or improper use of ScaMaC routines & algorithms.
  // example: If a complex matrix is passed to a routine that expects a real matrix, EINVALID is returned.
  // But if the valtype of the matrix is neither real nor complex, ECORRUPTED is returned, because that's not a valid matrix at all.
  SCAMAC_ECORRUPTED,
  // +++
  // +++
  // the requested computation is outside of the scope of the function (e.g., Lanczos for non-symmetric matrices)
  SCAMAC_ESCOPE,
  // the combination of all input parameters is invalid (but each individual parameter may be valid)
  SCAMAC_EINPUT,
  // +++
  // +++ problems during execution or on return
  // index/matrix dimension exceeds possible integer range (2^31 ~ 2E9 for int32, 2^63 ~ 9E18 for int64)
  SCAMAC_EOVERFLOW,
  // memory allocation failed
  SCAMAC_EMALLOCFAIL,
  // an integer parameter is too large, i.e., larger then SCAMACHUGE
  SCAMAC_EHUGEINT,
  // an integer value computed in the routine is too large, i.e., larger then SCAMACHUGE
  SCAMAC_EHUGECOMP,
  // a (parameter) warning. Not necessarily an error
  SCAMAC_EWARNING,
  // algorithm (e.g., Lanczos) not converged
  SCAMAC_ENOTCONVERGED,
  // row to short
  SCAMAC_ESHORTROW,
  // the error originates from an internal call to a ScaMaC routine. This is a flag that can be combined with the other error codes,
  // as in SCAMAC_EFAIL | SCAMAC_EINTERNAL
  SCAMAC_EINTERNAL = (1U << (2 * SCAMAC_ESHIFT))
} ScamacErrorCode;

/* error handler */
/* Only global variable in scamac */
typedef void (*scamac_error_handler_ft)(void);
extern scamac_error_handler_ft scamac_error_handler;

// macros for assertions.
// The macros call exit(EXIT_FAILURE) or scamac_error_handler
#ifdef NDEBUG
#define SCAMAC_ASSERT(err)                                                               \
  do {                                                                                   \
  } while (0)

#define SCAMAC_APPROVE(exp)                                                              \
  do {                                                                                   \
    exp;                                                                                 \
  } while (0)

#else /* NO NDEBUG */

#define SCAMAC_ASSERT(expr)                                                              \
  do {                                                                                   \
    if (!(expr)) {                                                                       \
      fprintf(stderr,                                                                    \
          "\n*************\n*** ABORT ***\n*************\n%s\n\nassertion in function "  \
          ">%s< at line %d failed.\n\n",                                                 \
          __FILE__,                                                                      \
          __func__,                                                                      \
          __LINE__);                                                                     \
      fprintf(stderr, "%s\n", #expr);                                                    \
      fprintf(stderr, "* * * * * * *\n\naborted\n");                                     \
      if (scamac_error_handler) {                                                        \
        scamac_error_handler();                                                          \
      } else {                                                                           \
        exit(EXIT_FAILURE);                                                              \
      }                                                                                  \
    }                                                                                    \
  } while (0)

#define SCAMAC_APPROVE(cmd)                                                              \
  do {                                                                                   \
    ScamacErrorCode unique_142b176e_err;                                                 \
    unique_142b176e_err = cmd;                                                           \
    if (unique_142b176e_err) {                                                           \
      fprintf(stderr,                                                                    \
          "\n*************\n*** ABORT ***\n*************\n%s\n\nin function >%s< at "    \
          "line %d\n\n>%s< failed with\n\n",                                             \
          __FILE__,                                                                      \
          __func__,                                                                      \
          __LINE__,                                                                      \
          #cmd);                                                                         \
      fprintf(stderr,                                                                    \
          "%s%s\n",                                                                      \
          scamac_error_desc(unique_142b176e_err),                                        \
          scamac_error_dpar(unique_142b176e_err));                                       \
      fprintf(stderr, "* * * * * * *\n\naborted\n");                                     \
      if (scamac_error_handler) {                                                        \
        scamac_error_handler();                                                          \
      } else {                                                                           \
        exit(EXIT_FAILURE);                                                              \
      }                                                                                  \
    }                                                                                    \
  } while (0)

#endif /* NDEBUG */

// macros for error handling. Replace as you wish.
#define SCAMAC_CHKERR(err)                                                               \
  do {                                                                                   \
    if (err) {                                                                           \
      fprintf(stderr,                                                                    \
          "\n*************\n*** ABORT ***\n*************\n%s\n\nfunction >%s< at line "  \
          "%d failed with\n\n",                                                          \
          __FILE__,                                                                      \
          __func__,                                                                      \
          __LINE__);                                                                     \
      fprintf(stderr, "%s%s\n", scamac_error_desc(err), scamac_error_dpar(err));         \
      fprintf(stderr, "* * * * * * *\n\naborted\n");                                     \
      if (scamac_error_handler) {                                                        \
        scamac_error_handler();                                                          \
      } else {                                                                           \
        exit(EXIT_FAILURE);                                                              \
      }                                                                                  \
    }                                                                                    \
  } while (0)

#define SCAMAC_TRY(exp)                                                                  \
  do {                                                                                   \
    ScamacErrorCode unique_142b176e_err;                                                 \
    unique_142b176e_err = exp;                                                           \
    if (unique_142b176e_err) {                                                           \
      fprintf(stderr,                                                                    \
          "\n*************\n*** ABORT ***\n*************\n%s\n\nin function >%s< at "    \
          "line %d\n\n>%s< failed with\n\n",                                             \
          __FILE__,                                                                      \
          __func__,                                                                      \
          __LINE__,                                                                      \
          #exp);                                                                         \
      fprintf(stderr,                                                                    \
          "%s%s\n",                                                                      \
          scamac_error_desc(unique_142b176e_err),                                        \
          scamac_error_dpar(unique_142b176e_err));                                       \
      fprintf(stderr, "* * * * * * *\n\naborted\n");                                     \
      if (scamac_error_handler) {                                                        \
        scamac_error_handler();                                                          \
      } else {                                                                           \
        exit(EXIT_FAILURE);                                                              \
      }                                                                                  \
    }                                                                                    \
  } while (0)

#ifdef NDEBUG

#define SCAMAC_RETERR(err)                                                               \
  do {                                                                                   \
    if (err) {                                                                           \
      return scamac_error_set_internal(err);                                             \
    }                                                                                    \
  } while (0)

#define SCAMAC_GOTOERR(err, label)                                                       \
  do {                                                                                   \
    if (err) {                                                                           \
      err = scamac_error_set_internal(err);                                              \
      goto label;                                                                        \
    }                                                                                    \
  } while (0)

#else /* NO NDEBUG */

#define SCAMAC_RETERR(err)                                                               \
  do {                                                                                   \
    if (err) {                                                                           \
      fprintf(stderr,                                                                    \
          "\n*************\n*** ERROR ***\n*************\n%s\n\nfunction >%s< at line "  \
          "%d encountered an error\n\n",                                                 \
          __FILE__,                                                                      \
          __func__,                                                                      \
          __LINE__);                                                                     \
      fprintf(stderr,                                                                    \
          "%s%s\n* * * * * * *\n\n",                                                     \
          scamac_error_desc(err),                                                        \
          scamac_error_dpar(err));                                                       \
      return scamac_error_set_internal(err);                                             \
    }                                                                                    \
  } while (0)

#define SCAMAC_GOTOERR(err, label)                                                       \
  do {                                                                                   \
    if (err) {                                                                           \
      fprintf(stderr,                                                                    \
          "\n*************\n*** ERROR ***\n*************\n%s\n\nfunction >%s< at line "  \
          "%d encountered an error\n\n",                                                 \
          __FILE__,                                                                      \
          __func__,                                                                      \
          __LINE__);                                                                     \
      fprintf(stderr,                                                                    \
          "%s%s\n* * * * * * *\n\n",                                                     \
          scamac_error_desc(err),                                                        \
          scamac_error_dpar(err));                                                       \
      err = scamac_error_set_internal(err);                                              \
      goto label;                                                                        \
    }                                                                                    \
  } while (0)

#endif

#ifdef NDEBUG
#define SCAMAC_REPORT(str)                                                               \
  do {                                                                                   \
  } while (0)

#else /* NO NDEBUG */

#define SCAMAC_REPORT(str)                                                               \
  do {                                                                                   \
    fprintf(stderr, "-- report >%s< line %d: %s\n", __func__, __LINE__, str);            \
  } while (0)

#endif

#ifdef NDEBUG
#define SCAMAC_LOG(...)                                                                  \
  do {                                                                                   \
  } while (0)
#else /* NO NDEBUG */
#define SCAMAC_LOG(...)                                                                  \
  do {                                                                                   \
    fprintf(stderr, ">%s : %d<\n  ", __func__, __LINE__);                                \
    fprintf(stderr, __VA_ARGS__);                                                        \
    fprintf(stderr, "\n");                                                               \
  } while (0)
#endif

/** \brief Return error name.
 */
const char *scamac_error_desc(ScamacErrorCode err);
int scamac_error_par(ScamacErrorCode err);
const char *scamac_error_dpar(ScamacErrorCode err);
/* discard warnings (SCAMAC_EWARNING) */
ScamacErrorCode scamac_error_discard_warning(ScamacErrorCode err);
/* set INTERNAL flag for errors caused by wrong function calls (but not for, e.g., EMALLOCFAIL) */
ScamacErrorCode scamac_error_set_internal(ScamacErrorCode err);
ScamacErrorCode scamac_error_set_par(ScamacErrorCode err, int par);
bool scamac_error_is(ScamacErrorCode err1, ScamacErrorCode err2);

#endif /* SCAMAC_ERROR_H */
