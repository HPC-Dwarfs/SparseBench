/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ScaMaC index/integer types
 *  \ingroup public
 */

#ifndef SCAMAC_INTTYPES_H
#define SCAMAC_INTTYPES_H

#include "scamac_config.h"
#include <inttypes.h>


/*
 * some integer parameters, e.g., n_sites or n_fermions for the Hubbard example,
 * or the number of sites per axis for a 3D Laplace discretization grid,
 * should be "reasonably small" (even if the resulting matrix is "huge").
 * By definition, an integer n is "reasonably small" if abs(n) <= SCAMACHUGEINT.
 *
 * SCAMACHUGEINT acts mainly as a failsafe against "irresponsible" parameter choices
 * (e.g., when generating matrices in a loop).
 *
 * We set, deliberately, SCAMACHUGEINT = 2^20, but this value is not mandatory.
 * SCAMACHUGEINT should be much smaller than SCAMACIDXMAX.
 *
 * Note: The memory consumption for generator tables or workspace does not exceed
 * a reasonable multiple of SCAMACHUGEINT (here, some MByte).
 *
 */
#define SCAMACHUGEINT (1 << 20)

/* index and integer type
 * ScamacIdx is the (possibly huge) index type for ScamacGenerator etc.
 * ScamacInt is the (usually smaller) integer type for actual storage of sparse matrices,
 * as in scamac_sparsemat.h for the Scamac toolkit 
 * We use signed int because - well, we just do (and FORTRAN!).
 */
#ifdef SCAMAC_INDEX_TYPE_int64
typedef int64_t ScamacIdx;
typedef int32_t ScamacInt;
//typedef int_fast32_t ScamacInt;
static const ScamacIdx SCAMAC_IDX_MAX = INT64_MAX;
static const ScamacInt SCAMAC_INT_MAX = INT32_MAX;
//static const ScamacInt SCAMAC_INT_MAX = INT_FAST32_MAX;
#define SCAMACPRIDX PRId64
#define SCAMACPRINT PRId32
#elif defined SCAMAC_INDEX_TYPE_int32
typedef int32_t ScamacIdx;
typedef int32_t ScamacInt;
//typedef int_fast32_t ScamacInt;
static const ScamacIdx SCAMAC_IDX_MAX = INT32_MAX;
static const ScamacInt SCAMAC_INT_MAX = INT32_MAX;
//static const ScamacInt SCAMAC_INT_MAX = INT_FAST32_MAX;
#define SCAMACPRIDX PRId32
#define SCAMACPRINT PRIdFAST32
#else
typedef int ScamacIdx;
#error "Unknown SCAMAC_INDEX_TYPE"
#endif


#endif /* SCAMAC_INTTYPES_H */
