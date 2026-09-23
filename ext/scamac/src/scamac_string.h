/** \file
 *  \author Andreas Alvermann (University of Greifswald)
 *  \date   October 2017 --- today
 *  \brief  ...
 *  \ingroup internal
 */

#ifndef SCAMAC_STRING_H
#define SCAMAC_STRING_H

typedef struct {
  int nalloc;
  int nstr;
  char *str;
} scamac_string_st;

/* extendable string routines */

void scamac_string_empty(scamac_string_st *estr);
void scamac_string_append(scamac_string_st *estr, const char *str);
char *scamac_string_get(const scamac_string_st *estr);

void scamac_strappend(char **str, const char *app);
void scamac_strempty(char **str);

#endif /* SCAMAC_STRING_H */
