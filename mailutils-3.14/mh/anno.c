/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003-2022 Free Software Foundation, Inc.

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

/* MH annotate command */

#include <mh.h>

static char prog_doc[] = N_("Annotate messages");
static char args_doc[] = N_("[MSGLIST]");

//static int inplace;       /* Annotate the message in place */
static int anno_date = 1; /* Add date to the annotation */
static char *component;   /* header field */
static char *anno_text;   /* header field value */

static struct mu_option options[] = {
  { "inplace", 0, NULL, MU_OPTION_HIDDEN,
    N_("annotate the message in place"),
    mu_c_bool, NULL, mh_opt_notimpl_warning },
  { "date", 0, NULL, MU_OPTION_DEFAULT,
    N_("add FIELD: date header"),
    mu_c_bool, &anno_date },
  { "component", 0, N_("FIELD"), MU_OPTION_DEFAULT,
    N_("add this FIELD to the message header"),
    mu_c_string, &component },
  { "text", 0, N_("STRING"), MU_OPTION_DEFAULT,
    N_("field value for the component"),
    mu_c_string, &anno_text },
  MU_OPTION_END
};

int
anno (size_t n, mu_message_t msg, void *call_data)
{
  mh_annotate (msg, component, anno_text, anno_date);
  return 0;
}

int
main (int argc, char **argv)
{
  int rc;
  mu_mailbox_t mbox;
  mu_msgset_t msgset;
  size_t len;

  mh_getopt (&argc, &argv, options, MH_GETOPT_DEFAULT_FOLDER,
	     args_doc, prog_doc, NULL);
  if (anno_text)
    {
      char *arg = anno_text;
      mh_quote (arg, &anno_text);
      free (arg);
    }
  
  mbox = mh_open_folder (mh_current_folder (), MU_STREAM_RDWR);

  if (!component)
    {
      size_t size = 0;
      char *p;

      if (isatty (0))
	{
	  mu_printf (_("Component name: "));
	  mu_stream_flush (mu_strout);
	}
      rc = mu_stream_getline (mu_strin, &component, &size, NULL);
      if (rc)
	{
	  mu_error (_("error reading input stream: %s"), mu_strerror (rc));
	  exit (1);
	}
      p = mu_str_stripws (component);
      if (*p == 0)
	{
	  mu_error (_("invalid component name"));
	  exit (1);
	}
      if (p > component)
	memmove (component, p, strlen (p) + 1);
    }

  if (!anno_text && !anno_date)
    exit (0);

  len = strlen (component);
  if (len > 0 && component[len-1] == ':')
    component[len-1] = 0;
  
  mh_msgset_parse (&msgset, mbox, argc, argv, "cur");
  rc = mu_msgset_foreach_message (msgset, anno, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_msgset_foreach_message", NULL, rc);
      exit (1);
    }
      
  mh_mailbox_set_cur (mbox, mh_msgset_first (msgset, RET_UID));
  mu_msgset_free (msgset);
  mh_global_save_state ();
  mu_mailbox_sync (mbox);
  mu_mailbox_close (mbox);
  mu_mailbox_destroy (&mbox);
  return rc;
}
      

				  
  



