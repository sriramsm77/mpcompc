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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <mailutils/datetime.h>

void
mu_datetime_tz_local (struct mu_timezone *tz)
{
  tz->utc_offset = mu_utc_offset ();
  tz->tz_name = NULL;
}
