/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2011-2022 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#include <config.h>
#include <string.h>
#include <mailutils/types.h>
#include <mailutils/imaputil.h>
#include <mailutils/cctype.h>

/* Match STRING against the IMAP4 wildcard pattern PATTERN. */

#define WILD_FALSE 0
#define WILD_TRUE  1
#define WILD_ABORT 2

int
_wild_match (const char *pat, const char *name, char delim, int icase)
{
  while (pat && *pat)
    {
      if (*name == 0 && *pat != '*' && *pat != '%')
	return WILD_ABORT;
      switch (*pat)
	{
	case '*':
	  while (*++pat == '*')
	    ;
	  if (*pat == 0)
	    return WILD_TRUE;
	  while (*name)
	    {
	      int res = _wild_match (pat, name++, delim, icase);
	      if (res != WILD_FALSE)
		return res;
	    }
	  return WILD_ABORT;

	case '%':
	  while (*++pat == '%')
	    ;
	  if (*pat == 0)
	    return strchr (name, delim) ? WILD_FALSE : WILD_TRUE;
	  while (*name && *name != delim)
	    {
	      int res = _wild_match (pat, name++, delim, icase);
	      if (res != WILD_FALSE)
		return res;
	    }
	  return _wild_match (pat, name, delim, icase);
	  
	default:
	  if (icase ? mu_toupper (*pat) != mu_toupper (*name)
	            : *pat != *name)
	    return WILD_FALSE;
	  pat++;
	  name++;
	}
    }
  return *name == 0;
}

int
mu_imap_wildmatch (const char *pattern, const char *name, int delim)
{
  return _wild_match (pattern, name, delim, 0) != WILD_TRUE;
}

int
mu_imap_wildmatch_ci (const char *pattern, const char *name, int delim)
{
  return _wild_match (pattern, name, delim, 1) != WILD_TRUE;
}
