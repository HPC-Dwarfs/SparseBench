#include <stdio.h>
#include "scamac_option_inc.h"

static const char *scamac_option_print(scamac_option_ty opt)
{
  switch (opt) {
  case option_backforth:
    return "backforth";
  case option_loose:
    return "loose";
  case option_open:
    return "open";
  case option_ortho:
    return "ortho";
  case option_para:
    return "para";
  case option_periodic:
    return "periodic";
  case option_simple:
    return "simple";
  case option_tight:
    return "tight";
  default:
    assert(0);
    return "ERROR";
  }
}
