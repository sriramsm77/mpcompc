/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2010-2022 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _MAILUTILS_SYS_XSCRIPT_STREAM_H
# define _MAILUTILS_SYS_XSCRIPT_STREAM_H

# include <mailutils/types.h>
# include <mailutils/stream.h>
# include <mailutils/sys/stream.h>

struct _mu_xscript_channel
{
  int state;
  size_t length;
};

struct _mu_xscript_stream
{
  struct _mu_stream stream;
  mu_stream_t transport;
  mu_stream_t logstr;
  struct _mu_xscript_channel channel[2];
  char *prefix[2];
};

#endif
