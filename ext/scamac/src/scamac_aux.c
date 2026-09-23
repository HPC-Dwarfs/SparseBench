#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scamac_aux.h"

int scamac_increase_n_somewhat(int n)
{
  return ((n >> 4) + (n >> 6) + 2) << 4;
}

void scamac_print_int64_nicely(int64_t i, char *str)
{
  snprintf(str, 30, "%" PRId64, i);
  int n = strlen(str);
  if (i < 0) {
    str++;
    n--;
  }
  int nstroke = (n - 1) / 3;
  int j, k;
  str[n + nstroke] = 0;
  k                = n + nstroke - 1;
  for (j = 0; j < n; j++) {
    if ((j > 0) && (j % 3 == 0)) {
      str[k] = '\'';
      k--;
    }
    str[k] = str[n - j - 1];
    k--;
  }
}

void scamac_print_int64_abbr(int64_t i, char *str)
{
  if (i < 0) {
    snprintf(str, 5, "----");
  } else if (i < 1000) {
    snprintf(str, 5, "<1K");
  } else if (i < 10000) {
    double x = (double)i / 1000.0;
    snprintf(str, 5, "%.1fK", x);
  } else if (i < 1000000) {
    int64_t j = i / 1000;
    snprintf(str, 5, "%dK", (int)j);
  } else if (i < 10000000) {
    int64_t j = i / 100000;
    double x  = (double)j / 10.0;
    snprintf(str, 5, "%.1fM", x);
  } else if (i < 1000000000) {
    int64_t j = i / 1000000;
    snprintf(str, 5, "%dM", (int)j);
  } else if (i < 10000000000) {
    int64_t j = i / 100000000;
    double x  = (double)j / 10.0;
    snprintf(str, 5, "%.1fG", x);
  } else if (i < 1000000000000) {
    int64_t j = i / 1000000000;
    snprintf(str, 5, "%dG", (int)j);
  } else if (i < 10000000000000) {
    int64_t j = i / 100000000000;
    double x  = (double)j / 10.0;
    snprintf(str, 5, "%.1fT", x);
  } else if (i < 1000000000000000) {
    int64_t j = i / 1000000000000;
    snprintf(str, 5, "%dT", (int)j);
  } else if (i < 10000000000000000) {
    int64_t j = i / 100000000000000;
    double x  = (double)j / 10.0;
    snprintf(str, 5, "%.1fP", x);
  } else if (i < 1000000000000000000) {
    int64_t j = i / 1000000000000000;
    snprintf(str, 5, "%dP", (int)j);
  } else {
    int64_t j = i / 100000000000000000;
    double x  = (double)j / 10.0;
    snprintf(str, 5, "%.1fE", x);
  }
}

bool scamac_read_int64_nicely(const char *str, int64_t *res)
{
  while (str[0] == ' ') {
    str++;
  }
  int64_t sign;
  if (str[0] == '-') {
    sign = -1;
    str++;
  } else {
    sign = 1;
    if (str[0] == '+') {
      str++;
    }
  }
  if (str[0] == 0) {
    return false;
  }
  int suffix = 0;
  int64_t myres, acc;
  myres = 0;
  acc   = 0;
  while (str[0] != 0) {
    if ((str[0] >= '0') && (str[0] <= '9')) {
      if (acc > INT64_MAX / 10) {
        return false;
      }
      acc = 10 * acc + (str[0] - '0');
    } else if ((str[0] == 'K') && (str[1] == 'i')) {
      if (acc > (INT64_MAX - myres) / 1024) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 1)) {
        return false;
      }
      suffix = 1;
      acc    = 1024 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
      str++;
    } else if ((str[0] == 'M') && (str[1] == 'i')) {
      if (acc > (INT64_MAX - myres) / 1048576) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 2)) {
        return false;
      }
      suffix = 2;
      acc    = 1048576 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
      str++;
    } else if ((str[0] == 'G') && (str[1] == 'i')) {
      if (acc > (INT64_MAX - myres) / 1073741824) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 3)) {
        return false;
      }
      suffix = 3;
      acc    = 1073741824 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
      str++;
    } else if ((str[0] == 'T') && (str[1] == 'i')) {
      if (acc > (INT64_MAX - myres) / 1099511627776) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 4)) {
        return false;
      }
      suffix = 4;
      acc    = 1099511627776 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
      str++;
    } else if ((str[0] == 'P') && (str[1] == 'i')) {
      if (acc > (INT64_MAX - myres) / 1125899906842624) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 5)) {
        return false;
      }
      suffix = 5;
      acc    = 1125899906842624 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
      str++;
    } else if ((str[0] == 'E') && (str[1] == 'i')) {
      if (acc > (INT64_MAX - myres) / 1152921504606846976) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 6)) {
        return false;
      }
      suffix = 6;
      acc    = 1152921504606846976 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
      str++;
    } else if (str[0] == 'K') {
      if (acc > (INT64_MAX - myres) / 1000) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 1)) {
        return false;
      }
      suffix = 1;
      acc    = 1000 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
    } else if (str[0] == 'M') {
      if (acc > (INT64_MAX - myres) / 1000000) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 2)) {
        return false;
      }
      suffix = 2;
      acc    = 1000000 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
    } else if (str[0] == 'G') {
      if (acc > (INT64_MAX - myres) / 1000000000) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 3)) {
        return false;
      }
      suffix = 3;
      acc    = 1000000000 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
    } else if (str[0] == 'T') {
      if (acc > (INT64_MAX - myres) / 1000000000000) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 4)) {
        return false;
      }
      suffix = 4;
      acc    = 1000000000000 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
    } else if (str[0] == 'P') {
      if (acc > (INT64_MAX - myres) / 1000000000000000) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 5)) {
        return false;
      }
      suffix = 5;
      acc    = 1000000000000000 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
    } else if (str[0] == 'E') {
      if (acc > (INT64_MAX - myres) / 1000000000000000000) {
        return false;
      }
      if ((suffix > 0) && (suffix <= 6)) {
        return false;
      }
      suffix = 6;
      acc    = 1000000000000000000 * acc;
      if ((myres != 0) && (acc >= myres)) {
        return false;
      }
      myres += acc;
      acc = 0;
    } else if (str[0] != '\'') {// simply ignore ', but complain about wrong char
      return false;
    }
    str++;
  }
  myres = sign * (myres + acc);
  *res  = myres;
  return true;
}
