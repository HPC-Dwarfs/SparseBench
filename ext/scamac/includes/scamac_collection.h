/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  high-level access to the ScaMaC
 *  \ingroup public
 */

#ifndef SCAMAC_COLLECTION_H
#define SCAMAC_COLLECTION_H

#include "scamac_error.h"
#include "scamac_generator.h"
#include <stdbool.h>

ScamacErrorCode scamac_generator_obtain(const char * matname, ScamacGenerator ** gen);

ScamacErrorCode scamac_parse_argstr(const char * argstr, ScamacGenerator ** gen, char ** errdesc);

ScamacErrorCode scamac_generator_set_int        (ScamacGenerator * gen, const char * parname, int val);
ScamacErrorCode scamac_generator_set_idx        (ScamacGenerator * gen, const char * parname, ScamacIdx val);
ScamacErrorCode scamac_generator_set_double     (ScamacGenerator * gen, const char * parname, double val);
ScamacErrorCode scamac_generator_set_bool       (ScamacGenerator * gen, const char * parname, bool val);
/** \brief set random seed
 *  \ingroup library
 */
ScamacErrorCode scamac_generator_set_rngseed    (ScamacGenerator * gen, const char * parname, uint64_t seed);
/** \brief set random seed, reading from an arbitrary (random) string
 *  \details The random values used in matrix generation depend on a seed value, which is given by a matrix parameter normally called "seed".
 *  Random number generation is consistent:
 *  For the same seed, the same random values are used independently of the way the matrix is generated.
 *  \ingroup library
 */
ScamacErrorCode scamac_generator_set_rngseed_str(ScamacGenerator * gen, const char * parname, const char * seedstr);
ScamacErrorCode scamac_generator_set_option     (ScamacGenerator * gen, const char * parname, const char * option);

/** textual representation of generator parameters.
 * Returns a string that, when passed to scamac_parse_argstr, produces an identical generator.
 * Useful, e.g., to pass around a generator between different processes
 *
 * format="argstr","desc"
 */
ScamacErrorCode scamac_generator_parameter_desc(const ScamacGenerator * gen, const char * format, char ** desc);


ScamacErrorCode scamac_list_examples(char ** desc);
ScamacErrorCode scamac_example_parameters(const char * matname, char ** desc);
ScamacErrorCode scamac_example_desc(const char * matname, int * valtype, int * symmetry, char ** desc);

ScamacPar scamac_identify_parameter(const char * matname, const char * parname);

/**
 *     list possible values of "option" parameter
 */
//ScamacErrorCode scamac_parameter_option_values(const char * matname, const char * parname, char ** desc);


#endif /* SCAMAC_COLLECTION_H */
