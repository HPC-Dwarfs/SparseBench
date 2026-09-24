#include <stdlib.h>
#include <string.h>

#include "scamac_aux.h"
#include "scamac_string.h"

void scamac_string_empty(scamac_string_st *estr)
{
  if (estr) {
    estr->nalloc = 256;
    estr->str    = malloc(estr->nalloc * sizeof *(estr->str));
    estr->str[0] = 0;
    estr->nstr   = 0;
  }
}

void scamac_string_append(scamac_string_st *estr, const char *str)
{
  if (estr && str) {
    int nstr = estr->nstr;
    int l    = strlen(str);
    if (l > 0) {
      estr->nstr = estr->nstr + l;
      if (scamac_increase_n_somewhat(estr->nstr) > estr->nalloc) {
        estr->nalloc = scamac_increase_n_somewhat(estr->nstr);
        estr->str    = realloc(estr->str, estr->nalloc * sizeof *(estr->str));
      }
      strncpy(&(estr->str[nstr]), str, l);
      estr->str[estr->nstr] = 0;
    }
  }
}

char *scamac_string_get(const scamac_string_st *estr)
{
  if (estr) {
    return estr->str;
  } else {
    return NULL;
  }
}

void scamac_strappend(char **str, const char *app)
{
  if (!app) {
    return;
  }
  size_t lena = strlen(app);
  if (*str) {
    size_t lens = strlen(*str);
    *str        = realloc(*str, (lens + lena + 1) * sizeof(char));
    strncpy(*str + lens, app, lena);
    (*str)[lens + lena] = 0;
  } else {
    *str = malloc((lena + 1) * sizeof(char));
    strncpy(*str, app, lena);
    (*str)[lena] = 0;
  }
}

void scamac_strempty(char **str)
{
  if (*str) {
    str[0] = 0;
  }
}
