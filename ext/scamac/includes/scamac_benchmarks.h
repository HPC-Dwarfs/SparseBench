/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  high-level access to the ScaMaC
 *  \ingroup public
 */

#ifndef SCAMAC_BENCHMARKS_H
#define SCAMAC_BENCHMARKS_H

#include "scamac_error.h"
#include "scamac_generator.h"

/** \brief obtain generator for benchmark matrix "Bmname-m-x"
 *  e.g. scamac_benchmark_obtain("Simple",1,1) gives, to quote, "the simplest benchmark matrix ever".
 * \note benchmark names start with a capital letter
 *  \ingroup library
 */
ScamacErrorCode scamac_benchmark_obtain(const char * name, int m, int x, ScamacGenerator ** gen);

int scamac_benchmark_how_many(const char * name);

ScamacErrorCode scamac_list_benchmarks(char ** desc);

#endif /* SCAMAC_BENCHMARKS_H */
