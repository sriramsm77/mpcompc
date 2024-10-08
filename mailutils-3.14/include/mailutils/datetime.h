/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2011-2022 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library.  If not, see
   <http://www.gnu.org/licenses/>. */

#ifndef _MAILUTILS_DATETIME_H
#define _MAILUTILS_DATETIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <time.h>
#include <mailutils/types.h>

  /* ----------------------- */
  /* Date & time functions   */
  /* ----------------------- */

/* Argument ranges:

   year != 0, AD if > 0, BC if < 0
   1 <= month <= 12
   1 <= day <= maxday(month)
*/
/* Compute Julian Day for the given date */
int mu_datetime_julianday (int year, int month, int day);
/* Compute day of week (Sunday - 0) */
int mu_datetime_dayofweek (int year, int month, int day);
/* Compute ordinal date (1-based) */
int mu_datetime_dayofyear (int year, int month, int day);
/* Return number of days in the year */
int mu_datetime_year_days (int year);

/* Day of week and month names in C locale */
extern const char *_mu_datetime_short_month[];
extern const char *_mu_datetime_full_month[];
extern const char *_mu_datetime_short_wday[];
extern const char *_mu_datetime_full_wday[];


struct mu_timezone
{
  int utc_offset;  /* Seconds east of UTC. */

  const char *tz_name;
    /* Nickname for this timezone, if known. It is always considered
       to be a pointer to static string, so will never be freed. */
};

#define MU_PD_MASK_SECOND   00001
#define MU_PD_MASK_MINUTE   00002
#define MU_PD_MASK_HOUR     00004
#define MU_PD_MASK_DAY      00010
#define MU_PD_MASK_MONTH    00020
#define MU_PD_MASK_YEAR     00040
#define MU_PD_MASK_TZ       00100 
#define MU_PD_MASK_MERIDIAN 00200
#define MU_PD_MASK_ORDINAL  00400
#define MU_PD_MASK_NUMBER   01000

#define MU_PD_MASK_TIME MU_PD_MASK_SECOND|MU_PD_MASK_MINUTE|MU_PD_MASK_HOUR
#define MU_PD_MASK_DATE MU_PD_MASK_DAY|MU_PD_MASK_MONTH|MU_PD_MASK_YEAR
#define MU_PD_MASK_DOW MU_PD_MASK_NUMBER

int mu_parse_date_dtl (const char *p, const time_t *now, 
		       time_t *rettime, struct tm *rettm,
		       struct mu_timezone *rettz,
		       int *retflags);
int mu_parse_date (const char *p, time_t *rettime, const time_t *now);

int mu_utc_offset (void);
int mu_timezone_offset (const char *buf, int *off);  
void mu_datetime_tz_local (struct mu_timezone *tz);
void mu_datetime_tz_utc (struct mu_timezone *tz);
time_t mu_datetime_to_utc (struct tm *timeptr, struct mu_timezone *tz);
size_t mu_strftime (char *s, size_t max, const char *format, struct tm *tm);

int mu_c_streamftime (mu_stream_t str, const char *fmt, struct tm *tm,
		      struct mu_timezone *tz);
int mu_scan_datetime (const char *input, const char *fmt, struct tm *tm,
		      struct mu_timezone *tz, char **endp);

  /* Common datetime formats: */
#define MU_DATETIME_FROM         "%a %b %e %H:%M:%S %Y"
/* Length of an envelope date in C locale,
   not counting the terminating nul character */
#define MU_DATETIME_FROM_LENGTH 24

#define MU_DATETIME_IMAP         "%d-%b-%Y %H:%M:%S %z"
#define MU_DATETIME_INTERNALDATE "%e-%b-%Y%$ %H:%M:%S %z"

  /* RFC2822 date.  Scan format contains considerable allowances which would
     stun formatting functions, therefore two distinct formats are provided:
     one for outputting and one for scanning: */
#define MU_DATETIME_FORM_RFC822  "%a, %e %b %Y %H:%M:%S %z"
#define MU_DATETIME_SCAN_RFC822  "%[%a, %]%e %b %Y %H:%M%[:%S%] %z"

static inline int
mu_timeval_cmp (struct timeval const *a, struct timeval const *b)
{
    if (a->tv_sec < b->tv_sec)
        return -1;
    if (a->tv_sec > b->tv_sec)
        return 1;
    if (a->tv_usec < b->tv_usec)
        return -1;
    if (a->tv_usec > b->tv_usec)
        return 1;
    return 0;
}

static inline struct timeval
mu_timeval_add (struct timeval const *a, struct timeval const *b)
{
  struct timeval sum;

  sum.tv_sec = a->tv_sec + b->tv_sec;
  sum.tv_usec = a->tv_usec + b->tv_usec;
  if (sum.tv_usec >= 1000000)
    {
      ++sum.tv_sec;
      sum.tv_usec -= 1000000;
    }
  
  return sum;
}
  
static inline struct timeval
mu_timeval_sub (struct timeval const *a, struct timeval const *b)
{
  struct timeval diff;

  diff.tv_sec = a->tv_sec - b->tv_sec;
  diff.tv_usec = a->tv_usec - b->tv_usec;
  if (diff.tv_usec < 0)
    { 
      --diff.tv_sec;
      diff.tv_usec += 1000000;
    }
  
  return diff;
}
  
#ifdef __cplusplus
}
#endif
  
#endif
