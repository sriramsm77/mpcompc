/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

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

#include "imap4d.h"

/*
6.1.3.  LOGOUT Command

   Arguments:  none

   Responses:  REQUIRED untagged response: BYE

   Result:     OK - logout completed
               BAD - command unknown or arguments invalid
*/

int
imap4d_logout (struct imap4d_session *session,
               struct imap4d_command *command, imap4d_tokbuf_t tok)
{
  if (imap4d_tokbuf_argc (tok) != 2)
    return io_completion_response (command, RESP_BAD, "Invalid arguments");
  imap4d_bye_command (OK, command);
  return 0;
}
