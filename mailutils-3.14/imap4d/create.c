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
#include <unistd.h>

/*
6.3.3.  CREATE Command

   Arguments:  mailbox name

   Responses:  no specific responses for this command

   Result:     OK - create completed
               NO - create failure: can't create mailbox with that name
               BAD - command unknown or arguments invalid
*/  
/* FIXME: How do we do this ??????:
   IF a new mailbox is created with the same name as a mailbox which was
   deleted, its unique identifiers MUST be greater than any unique identifiers
   used in the previous incarnation of the mailbox.  */
int
imap4d_create (struct imap4d_session *session,
               struct imap4d_command *command, imap4d_tokbuf_t tok)
{
  char *name;
  int isdir = 0;
  int rc = RESP_OK;
  const char *msg = "Completed";
  mu_record_t record;
  int mode;

  if (imap4d_tokbuf_argc (tok) != 3)
    return io_completion_response (command, RESP_BAD, "Invalid arguments");

  name = imap4d_tokbuf_getarg (tok, IMAP4_ARG_1);

  if (*name == '\0')
    return io_completion_response (command, RESP_BAD, "Too few arguments");

  /* Creating, "Inbox" should always fail.  */
  if (mu_c_strcasecmp (name, "INBOX") == 0)
    return io_completion_response (command, RESP_BAD, "Already exist");

  /* RFC 3501:
         If the mailbox name is suffixed with the server's hierarchy
	 separator character, this is a declaration that the client intends
	 to create mailbox names under this name in the hierarchy.

     The trailing delimiter will be removed by namespace normalizer, so
     test for it now.
  */
  if (name[strlen (name) - 1] == MU_HIERARCHY_DELIMITER)
    isdir = 1;
  
  /* Allocates memory.  */
  name = namespace_get_name (name, &record, &mode);
  
  if (!name)
    return io_completion_response (command, RESP_NO, "Cannot create mailbox");

  /* It will fail if the mailbox already exists.  */
  if (access (name, F_OK) != 0)
    {
      if (make_interdir (name, MU_HIERARCHY_DELIMITER, MKDIR_PERMISSIONS))
	rc = RESP_NO;
      
      if (rc == RESP_OK)
	{
	  if (isdir)
	    {
	      if (mkdir (name, MKDIR_PERMISSIONS))
		{
		  mu_diag_output (MU_DIAG_ERR,
				  _("Cannot create directory %s: %s"), name,
				  mu_strerror (errno));
		  rc = RESP_NO;
		}	  
	    }
	  else
	    {
	      mu_mailbox_t mbox;

	      rc = mu_mailbox_create_from_record (&mbox, record, name);
	      if (rc)
		{
		  mu_diag_output (MU_DIAG_ERR,
				  _("Cannot create mailbox (%s) %s: %s"), name,
				  record->scheme,
				  mu_strerror (rc));
		  rc = RESP_NO;
		}
	      else if ((rc = mu_mailbox_open (mbox,
					      MU_STREAM_RDWR | MU_STREAM_CREAT
					      | mode)))
		{
		  mu_diag_output (MU_DIAG_ERR,
				  _("Cannot open mailbox (%s) %s: %s"),
				  record->scheme,
				  name, mu_strerror (rc));
		  rc = RESP_NO;
		}
	      else
		{
		  mu_mailbox_close (mbox);
		  mu_mailbox_destroy (&mbox);
		  rc = RESP_OK;
		}
	    }
	}
      if (rc != RESP_OK)
	msg = "Cannot create mailbox";
    }
  else
    {
      rc = RESP_NO;
      msg = "already exists";
    }
  free (name);
  return io_completion_response (command, rc, "%s", msg);
}
