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
6.4.3.  EXPUNGE Command

   Arguments:  none

   Responses:  untagged responses: EXPUNGE

   Result:     OK - expunge completed
               NO - expunge failure: can't expunge (e.g. permission
                    denied)
               BAD - command unknown or arguments invalid
*/

int
imap4d_expunge (struct imap4d_session *session,
                struct imap4d_command *command, imap4d_tokbuf_t tok)
{
  if (imap4d_tokbuf_argc (tok) != 2)
    return io_completion_response (command, RESP_BAD, "Invalid arguments");

  imap4d_enter_critical ();
  /* FIXME: check for errors.  */
  mu_mailbox_expunge (mbox);
  imap4d_leave_critical ();
  
  imap4d_sync_invalidate ();
  imap4d_sync ();
  return io_completion_response (command, RESP_OK, "Completed");
}
