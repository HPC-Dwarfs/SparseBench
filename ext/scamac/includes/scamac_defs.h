/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  basic ScaMaC definitions
 *  \ingroup public
 */

#ifndef SCAMAC_DEFS_H
#define SCAMAC_DEFS_H

// length of names in SCAMAC
#define SCAMAC_NAME_LENGTH 256

// flags for scamac_info_t
typedef int ScamacFlag;
#define SCAMAC_NONE 0

// valtype
typedef enum {
  // undefined
  SCAMAC_VAL_NONE,
  // real = double
  SCAMAC_VAL_REAL,
  // complex = double complex
  SCAMAC_VAL_COMPLEX
} ScamacValType;

// symmetry
typedef enum {
  SCAMAC_GENERAL=1,
  SCAMAC_SYMMETRIC,
  SCAMAC_HERMITIAN
} ScamacSymmetry;

// flags for generate row
#define SCAMAC_DEFAULT 0
#define SCAMAC_TRANSPOSE (1U << 0)
#define SCAMAC_CONJUGATE (1U << 1)
#define SCAMAC_CONJUGATETRANSPOSE (SCAMAC_TRANSPOSE | SCAMAC_CONJUGATE)
#define SCAMAC_KEEPZEROS (1U << 2)
//#define SCAMAC_INT32 (1U << 3)
//#define SCAMAC_SINGLE (1U << 4)

// parameter types
typedef enum {
  SCAMAC_PAR_NONE=-1,
  SCAMAC_PAR_INT=1,
  SCAMAC_PAR_IDX,
  SCAMAC_PAR_DOUBLE,
  SCAMAC_PAR_BOOL,
  SCAMAC_PAR_RNGSEED,
  SCAMAC_PAR_OPTION
} ScamacPar;

#endif /* SCAMAC_DEFS_H */
