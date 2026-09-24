/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_AUX_H
#define SCAMAC_AUX_H

#include "scamac_inttypes.h"

#include <stdbool.h>
#include <inttypes.h>

#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef MAX3
#define MAX3(x, y, z) (((x) > MAX(y, z)) ? (x) : MAX(y, z))
#endif
#ifndef MIN3
#define MIN3(x, y, z) (((x) < MIN(y, z)) ? (x) : MIN(y, z))
#endif

// return a number larger than n
int scamac_increase_n_somewhat(int n);

/**
 *   Caveat: Considers only i>=0. If i<0, output is "----"
 *   str: at least char[30]
 */
void scamac_print_int64_nicely(int64_t i, char *str);

/**
 *   str: at least char[5]
 */
void scamac_print_int64_abbr(int64_t i, char *str);

bool scamac_read_int64_nicely(const char *str, int64_t *res);

#endif /* SCAMAC_AUX_H */
