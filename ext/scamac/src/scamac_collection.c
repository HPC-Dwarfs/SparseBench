#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>

#include "scamac_collection.h"
#include "scamac_internal.h"
#include "scamac_benchmarks.h"
#include "scamac_aux.h"

#include "scamac_collection_inc.h"

#include "scamac_option_inc.c"

ScamacErrorCode scamac_generator_obtain(const char *matname, ScamacGenerator **gen)
{
  if (!matname) {
    return SCAMAC_ENULL | (1 << SCAMAC_ESHIFT);
  }
  if (!gen) {
    return SCAMAC_ENULL | (2 << SCAMAC_ESHIFT);
  }

#include "scamac_collection_example_inc.c"

  return SCAMAC_EFAIL;
}

ScamacErrorCode scamac_generator_set_int(
    ScamacGenerator *gen, const char *parname, int val)
{
  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!parname) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }

  gen->needs_finalization = true;

#include "scamac_collection_set_int_inc.c"

  return SCAMAC_EFAIL | SCAMAC_EINTERNAL;
}

ScamacErrorCode scamac_generator_set_idx(
    ScamacGenerator *gen, const char *parname, ScamacIdx val)
{
  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!parname) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }

  gen->needs_finalization = true;

#include "scamac_collection_set_idx_inc.c"

  return SCAMAC_EFAIL | SCAMAC_EINTERNAL;
}

ScamacErrorCode scamac_generator_set_double(
    ScamacGenerator *gen, const char *parname, double val)
{
  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!parname) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }

  gen->needs_finalization = true;

#include "scamac_collection_set_double_inc.c"

  return SCAMAC_EFAIL | SCAMAC_EINTERNAL;
}

ScamacErrorCode scamac_generator_set_bool(
    ScamacGenerator *gen, const char *parname, bool val)
{
  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!parname) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }

  gen->needs_finalization = true;

#include "scamac_collection_set_bool_inc.c"

  return SCAMAC_EFAIL | SCAMAC_EINTERNAL;
}

ScamacErrorCode scamac_generator_set_seed(
    ScamacGenerator *gen, const char *parname, uint64_t seed)
{
  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!parname) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }

  gen->needs_finalization = true;

  scamac_rng_seed_ty val  = seed;

#include "scamac_collection_set_rngseed_inc.c"

  return SCAMAC_EFAIL | SCAMAC_EINTERNAL;
}

ScamacErrorCode scamac_generator_set_seed_str(
    ScamacGenerator *gen, const char *parname, const char *seedstr)
{
  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!parname) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }

  gen->needs_finalization = true;

  scamac_rng_seed_ty val  = scamac_rng_string_to_seed(seedstr);

#include "scamac_collection_set_rngseed_str_inc.c"

  return SCAMAC_EFAIL | SCAMAC_EINTERNAL;
}

ScamacErrorCode scamac_generator_set_option(
    ScamacGenerator *gen, const char *parname, const char *option)
{
  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!parname) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }
  if (!option) {
    return SCAMAC_ENULL | 3 << SCAMAC_ESHIFT;
  }

  gen->needs_finalization = true;

#include "scamac_collection_set_option_inc.c"

  return SCAMAC_EFAIL | SCAMAC_EINTERNAL;
}

ScamacErrorCode scamac_list_examples(char **desc)
{
  if (desc) {
    char *my_string;
#include "scamac_collection_list_examples_inc.c"
    *desc = my_string;
    return SCAMAC_EOK;
  } else {
    return SCAMAC_ENULL;
  }
}

ScamacErrorCode scamac_example_desc(
    const char *matname, int *valtype, int *symmetry, char **desc)
{
  if (!matname) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }

#include "scamac_collection_example_desc_inc.c"

  return SCAMAC_EINVALID | 1 << SCAMAC_ESHIFT;
}

ScamacErrorCode scamac_example_parameters(const char *matname, char **desc)
{
  if (!matname) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!desc) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }

  char *my_string;
#include "scamac_collection_list_parameters_inc.c"

  if (*desc) {
    *desc = NULL;
  }
  return SCAMAC_EINVALID | 1 << SCAMAC_ESHIFT;
}

ScamacPar scamac_identify_parameter(const char *matname, const char *parname)
{
  if (matname && parname) {
#include "scamac_collection_identify_parameter_inc.c"
    return SCAMAC_PAR_NONE;
  } else {
    return SCAMAC_PAR_NONE;
  }
}

static char *strip_string(char *s)
{
  if (s) {
    while (*s == ' ') {
      s++;
    }
    int n = strlen(s);
    while (n > 0 && s[n - 1] == ' ') {
      s[n - 1] = 0;
      n--;
    }
    return s;
  } else {
    return NULL;
  }
}

static void delete_last_char(const char *ch, char *s)
{
  int l = strlen(s);
  if (l > 0) {
    if (s[l - 1] == ch[0]) {
      s[l - 1] = 0;
    }
  }
}

ScamacErrorCode scamac_generator_parameter_desc(
    const ScamacGenerator *gen, const char *format, char **desc)
{
  if (!gen) {
    return SCAMAC_ENULL | 1 << SCAMAC_ESHIFT;
  }
  if (!format) {
    return SCAMAC_ENULL | 2 << SCAMAC_ESHIFT;
  }
  if (!desc) {
    return SCAMAC_ENULL | 3 << SCAMAC_ESHIFT;
  }

  bool print_name       = false;
  bool print_as_argstr  = false;
  bool print_double_hex = false;

  if (!strcmp(format, "argstr")) {
    print_name       = true;
    print_as_argstr  = true;
    print_double_hex = false;
  } else if (!strcmp(format, "desc")) {
    print_name       = true;
    print_as_argstr  = false;
    print_double_hex = false;
  } else {
    return SCAMAC_EINVALID | 2 << SCAMAC_ESHIFT;
  }

  char sepchar[2];
  if (print_as_argstr) {
    snprintf(sepchar, 2, "%s", ",");
  } else {
    snprintf(sepchar, 2, "%s", "\n");
  }

  char format_name[30], format_int[30], format_idx[30], format_double[30],
      format_bool[30], format_rngseed[30], format_option[30];

  snprintf(format_name, 30, "%%s%s", sepchar);
  snprintf(format_int, 30, "%%s=%%d%s", sepchar);
  snprintf(format_idx, 30, "%%s=%%" SCAMACPRIDX "%s", sepchar);
  if (print_double_hex) {
    snprintf(format_double, 30, "%%s=%%a%s", sepchar);
  } else {
    if (print_as_argstr) {
      snprintf(format_double, 30, "%%s=%%1.16e%s", sepchar);
    } else {
      snprintf(format_double, 30, "%%s=%%1.16g%s", sepchar);
    }
  }
  snprintf(format_bool, 30, "%%s=%%s%s", sepchar);
  snprintf(format_rngseed, 30, "%%s=%%" PRIu64 "%s", sepchar);
  snprintf(format_option, 30, "%%s=%%s%s", sepchar);

  *desc = NULL;

  char *my_string;
#include "scamac_collection_example_data_inc.c"

  return SCAMAC_ECORRUPTED | 1 << SCAMAC_ESHIFT;
}

static int split_string(const char *s, char **split, char ***sfst)
{
  if (s) {

    char *s2;
    s2 = malloc((strlen(s) + 1) * sizeof *s2);
    memcpy(s2, s, strlen(s) * sizeof *s2);
    s2[strlen(s)] = 0;

    int n         = 0;
    char *s1      = s2;

    while (s1) {
      n++;
      s1 = strpbrk(s1, ",");
      if (s1) {
        s1++;
      }
    }

    char **my_list = malloc(2 * n * sizeof *my_list);

    s1             = s2;
    n              = 0;
    while (s1) {
      my_list[2 * n]     = s1;
      my_list[2 * n + 1] = NULL;
      n++;
      s1 = strpbrk(s1, ",");
      if (s1) {
        *s1 = 0;
        s1++;
      }
    }

    int i;
    for (i = 0; i < n; i++) {
      s1 = strpbrk(my_list[2 * i], "=");
      if (s1) {
        *s1                = 0;
        my_list[2 * i + 1] = s1 + 1;
      }
    }

    for (i = 0; i < 2 * n; i++) {
      if (my_list[i]) {
        my_list[i] = strip_string(my_list[i]);
      }
    }

    *split = s2;
    *sfst  = my_list;
    return n;

  } else {
    *split = NULL;
    *sfst  = NULL;
    return 0;
  }
}

static ScamacErrorCode atobool(const char *bs, bool *val)
{
  if (!bs) {
    return SCAMAC_ENULL | (1 << SCAMAC_ESHIFT);
  }
  if (!val) {
    return SCAMAC_ENULL | (2 << SCAMAC_ESHIFT);
  }
  if (!strcmp(bs, "true") || !strcmp(bs, "True") || !strcmp(bs, "TRUE")) {
    *val = true;
    return SCAMAC_EOK;
  } else if (!strcmp(bs, "false") || !strcmp(bs, "False") || !strcmp(bs, "FALSE")) {
    *val = false;
    return SCAMAC_EOK;
  } else {
    return SCAMAC_EINVALID | (2 << SCAMAC_ESHIFT);
  }
}

ScamacErrorCode scamac_parse_argstr(
    const char *argstr, ScamacGenerator **gen, char **errdesc)
{

  ScamacErrorCode err = SCAMAC_EOK;

  if (!argstr) {
    if (errdesc) {
      char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
      snprintf(str, SCAMAC_NAME_LENGTH, "missing argstr");
      *errdesc = str;
    }
    return SCAMAC_ENULL;
  }

  if (!gen) {
    if (errdesc) {
      char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
      snprintf(str, SCAMAC_NAME_LENGTH, "gen. pointer = NULL");
      *errdesc = str;
    }
    return SCAMAC_ENULL;
  }

  ScamacGenerator *my_gen = NULL;

  char *my_s              = NULL;
  char **my_slst          = NULL;

  int n                   = split_string(argstr, &my_s, &my_slst);

  if (n < 1) {
    if (errdesc) {
      char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
      snprintf(str, SCAMAC_NAME_LENGTH, "few parameters");
      *errdesc = str;
    }
    err = SCAMAC_EFAIL;
    goto finalise;
  }

  if (my_slst[1]) {
    if (errdesc) {
      char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
      snprintf(str, SCAMAC_NAME_LENGTH, "matrix name accepts no values");
      *errdesc = str;
    }
    err = SCAMAC_EFAIL;
    goto finalise;
  }

  if (!my_slst[0]) {
    if (errdesc) {
      char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
      snprintf(str, SCAMAC_NAME_LENGTH, "matrix name required");
      *errdesc = str;
    }
    err = SCAMAC_EFAIL;
    goto finalise;
  }

  if (strlen(my_slst[0]) > 2 && my_slst[0][0] == 'B' && my_slst[0][1] == 'm') {
    char bmname[101], dummy[2];
    int m, x;
    int n_read;
    n_read = sscanf(my_slst[0], "Bm%100[^-]-%9d-%9d %1s", bmname, &m, &x, dummy);
    if (n_read != 3) {
      err = SCAMAC_EFAIL;
    } else {
      if (n > 1) {
        err = SCAMAC_EFAIL;
        if (errdesc) {
          char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
          snprintf(str,
              SCAMAC_NAME_LENGTH,
              "Benchmark matrices >Bm%s-m-x< do not accept additional parameters",
              bmname);
          *errdesc = str;
        }
        goto finalise;
      }
      err = scamac_benchmark_obtain(bmname, m, x, &my_gen);
    }
  } else {
    err = scamac_generator_obtain(my_slst[0], &my_gen);
  }

  if (err) {
    if (errdesc) {
      char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
      snprintf(str, SCAMAC_NAME_LENGTH, "matrix >%s< does not exist", my_slst[0]);
      *errdesc = str;
    }
    err = SCAMAC_EFAIL;
    goto finalise;
  }

  assert(my_gen != NULL);

  int i;
  int n_read;
  char dummy[2];
  bool err_read   = false;
  bool err_overfl = false;
  for (i = 1; i < n; i++) {
    ScamacPar par_type =
        scamac_identify_parameter(scamac_generator_query_name(my_gen), my_slst[2 * i]);
    if (par_type == SCAMAC_PAR_NONE) {
      if (errdesc) {
        char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
        snprintf(str, SCAMAC_NAME_LENGTH, "Parameter %s does not exist", my_slst[2 * i]);
        *errdesc = str;
      }
      err = SCAMAC_EFAIL;
    } else if (par_type == SCAMAC_PAR_INT) {
      if (!my_slst[2 * i + 1]) {
        err_read = true;
      } else {
        int par;
        n_read = sscanf(my_slst[2 * i + 1], "%9d %1s", &par, dummy);
        if (n_read != 1) {
          err_read = true;
        } else {
          err = scamac_generator_set_int(my_gen, my_slst[2 * i], par);
          if (scamac_error_is(err, SCAMAC_ERANGE)) {
            char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
            snprintf(
                str, SCAMAC_NAME_LENGTH, "Parameter %s out of range", my_slst[2 * i]);
            *errdesc = str;
          }
        }
      }
      if (err_read) {
        if (errdesc) {
          char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
          snprintf(
              str, SCAMAC_NAME_LENGTH, "Parameter %s requires INT value", my_slst[2 * i]);
          *errdesc = str;
        }
        err = SCAMAC_EFAIL;
      }
    } else if (par_type == SCAMAC_PAR_IDX) {
      if (!my_slst[2 * i + 1]) {
        err_read = true;
      } else {
        int64_t par;
        bool did_read;
        did_read = scamac_read_int64_nicely(my_slst[2 * i + 1], &par);
        if (!did_read) {
          err_read = true;
        } else if (par > SCAMAC_IDX_MAX) {
          err_read   = true;
          err_overfl = true;
        } else {
          err = scamac_generator_set_idx(my_gen, my_slst[2 * i], par);
          if (scamac_error_is(err, SCAMAC_ERANGE)) {
            char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
            snprintf(
                str, SCAMAC_NAME_LENGTH, "Parameter %s out of range", my_slst[2 * i]);
            *errdesc = str;
          }
        }
      }
      if (err_read) {
        if (errdesc) {
          char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
          if (!err_overfl) {
            snprintf(str,
                SCAMAC_NAME_LENGTH,
                "Parameter %s requires INT or IDX value",
                my_slst[2 * i]);
          } else {
            snprintf(str,
                SCAMAC_NAME_LENGTH,
                "Parameter %s requires value <= %" SCAMACPRIDX,
                my_slst[2 * i],
                SCAMAC_IDX_MAX);
          }
          *errdesc = str;
        }
        err = SCAMAC_EFAIL;
      }
    } else if (par_type == SCAMAC_PAR_DOUBLE) {
      if (!my_slst[2 * i + 1]) {
        err_read = true;
      } else {
        double par;
        n_read = sscanf(my_slst[2 * i + 1], "%lf %1s", &par, dummy);
        if (n_read != 1) {
          err_read = true;
        } else {
          if (isnan(par) || isinf(par)) {
            err_read = true;
          } else {
            err = scamac_generator_set_double(my_gen, my_slst[2 * i], par);
            if (scamac_error_is(err, SCAMAC_ERANGE)) {
              char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
              snprintf(
                  str, SCAMAC_NAME_LENGTH, "Parameter %s out of range", my_slst[2 * i]);
              *errdesc = str;
            }
          }
        }
      }
      if (err_read) {
        if (errdesc) {
          char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
          snprintf(str,
              SCAMAC_NAME_LENGTH,
              "Parameter %s requires DOUBLE value",
              my_slst[2 * i]);
          *errdesc = str;
        }
        err = SCAMAC_EFAIL;
      }
    } else if (par_type == SCAMAC_PAR_BOOL) {
      if (!my_slst[2 * i + 1]) {
        err_read = true;
      } else {
        bool par;
        err = atobool(my_slst[2 * i + 1], &par);
        if (err) {
          err_read = true;
        } else {
          scamac_generator_set_bool(my_gen, my_slst[2 * i], par);
        }
      }
      if (err_read) {
        if (errdesc) {
          char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
          snprintf(str,
              SCAMAC_NAME_LENGTH,
              "Parameter %s requires BOOL value",
              my_slst[2 * i]);
          *errdesc = str;
        }
        err = SCAMAC_EFAIL;
      }
    } else if (par_type == SCAMAC_PAR_RNGSEED) {
      if (!my_slst[2 * i + 1]) {
        err_read = true;
      } else {
        if (strlen(my_slst[2 * i + 1]) < 1) {
          err_read = true;
        } else {
          scamac_generator_set_seed_str(my_gen, my_slst[2 * i], my_slst[2 * i + 1]);
        }
      }
      if (err_read) {
        if (errdesc) {
          char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
          snprintf(str,
              SCAMAC_NAME_LENGTH,
              "Parameter %s requires RNGSEED (dec,hex,string) value",
              my_slst[2 * i]);
          *errdesc = str;
        }
        err = SCAMAC_EFAIL;
      }
    } else if (par_type == SCAMAC_PAR_OPTION) {
      if (!my_slst[2 * i + 1]) {
        err_read = true;
      } else {
        if (strlen(my_slst[2 * i + 1]) < 1) {
          err_read = true;
        } else {
          err = scamac_generator_set_option(my_gen, my_slst[2 * i], my_slst[2 * i + 1]);
          if (err) {
            if (errdesc) {
              char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
              snprintf(str,
                  SCAMAC_NAME_LENGTH,
                  "Unknown option for parameter %s : %s",
                  my_slst[2 * i],
                  my_slst[2 * i + 1]);
              *errdesc = str;
            }
            err = SCAMAC_EFAIL;
          }
        }
      }
      if (err_read) {
        if (errdesc) {
          char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
          snprintf(str,
              SCAMAC_NAME_LENGTH,
              "Parameter %s requires OPTION (string) value",
              my_slst[2 * i]);
          *errdesc = str;
        }
        err = SCAMAC_EFAIL;
      }
    } else {
      if (errdesc) {
        char *str = malloc(SCAMAC_NAME_LENGTH * sizeof *str);
        snprintf(str, SCAMAC_NAME_LENGTH, "Unknown parameter type");
        *errdesc = str;
      }
      err = SCAMAC_EFAIL;
    }
    if (err) {
      break;
    }
  }

finalise:
  free(my_s);
  free(my_slst);

  if (!err) {
    *gen = my_gen;
    if (errdesc) {
      *errdesc = NULL;
    }
  }

  return err;
}
