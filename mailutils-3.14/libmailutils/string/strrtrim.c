/* This file is part of GNU Mailutils
   Copyright (C) 2009-2022 Free Software Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <mailutils/types.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>

size_t
mu_rtrim_class (char *str, int class)
{
  size_t len;
  
  if (!*str)
    return 0;
  for (len = strlen (str); len > 0 && mu_c_is_class (str[len-1], class); len--)
    ;
  str[len] = 0;
  return len;
}

size_t
mu_rtrim_cset (char *str, const char *cset)
{
  size_t len;
  
  if (!*str)
    return 0;
  for (len = strlen (str); len > 0 && strchr (cset, str[len-1]) != NULL; len--)
    ;
  str[len] = 0;
  return len;
}
