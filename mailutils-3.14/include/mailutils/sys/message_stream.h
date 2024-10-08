/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2009-2022 Free Software Foundation, Inc.

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

#ifndef _MAILUTILS_SYS_MESSAGE_STREAM_H
#define _MAILUTILS_SYS_MESSAGE_STREAM_H

#include <mailutils/sys/stream.h>

struct mu_substring_location
{
  size_t start;
  size_t length;
};

struct _mu_message_stream
{
  struct _mu_stream stream;
  mu_stream_t transport;  /* Actual stream */
  mu_off_t offset;

  int construct_envelope; /* If 1, construct the envelope. */
  char *envelope_string;             /* The From_ line, if found. */
  size_t envelope_length;            /* Total length of the From_ line,
					including trailing whitespace (if any)
					and final \n. */
  struct mu_substring_location from; /* Sender location in envelope_string. */
  struct mu_substring_location date; /* Date location in envelope_string. */

  struct mu_substring_location mark; /* Location of the header separator in
					stream. */

  mu_off_t body_start;               /* Start of the body. */
  mu_off_t body_end;                 /* End of the body. */
};

struct _mu_body_stream
{
  struct _mu_stream stream;
  mu_off_t offset;
  struct _mu_message_stream *message_stream;
};

#endif


