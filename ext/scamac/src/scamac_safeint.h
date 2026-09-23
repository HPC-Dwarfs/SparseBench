/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_SAFEINT_H
#define SCAMAC_SAFEINT_H

#include "scamac_error.h"
#include "scamac_inttypes.h"

// return a+b (or a*b) for a,b >=0 if no overflow occurs,
// otherwise return negative value (signals overflow)
ScamacIdx scamac_safeidx_add(ScamacIdx a, ScamacIdx b);
ScamacIdx scamac_safeidx_mult(ScamacIdx a, ScamacIdx b);

ScamacInt scamac_safeint_add(ScamacInt a, ScamacInt b);
ScamacInt scamac_safeint_mult(ScamacInt a, ScamacInt b);

/*
// c = binomial(a \over\ b)
ScamacErrorCode scamac_safe_binomial(ScamacIdx a, ScamacIdx b, ScamacIdx *c);
*/

#endif /* SCAMAC_SAFEINT_H */
