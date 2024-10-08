/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2002-2022 Free Software Foundation, Inc.

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

/* MH refile command */

#include <mh.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

static char prog_doc[] = N_("File messages in other folders");
static char args_doc[] = N_("MSGLIST FOLDER [FOLDER...]");

int link_flag = 0;
int preserve_flag = 0;
char *source_file = NULL;
mu_list_t folder_name_list = NULL;
mu_list_t folder_mbox_list = NULL;

void
add_folder (const char *folder)
{
  if (!folder_name_list && mu_list_create (&folder_name_list))
    {
      mu_error (_("cannot create folder list"));
      exit (1);
    }
  mu_list_append (folder_name_list, mu_strdup (folder));
}

static void
add_folder_option (struct mu_parseopt *po, struct mu_option *opt,
		   const char *arg)
{
  add_folder (arg);
}

static struct mu_option options[] = {
  { "folder",  0, N_("FOLDER"), MU_OPTION_DEFAULT,
    N_("specify folder to operate upon"),
    mu_c_string, NULL, add_folder_option },
  { "draft",   0, NULL, MU_OPTION_DEFAULT,
    N_("use <mh-dir>/draft as the source message"),
    mu_c_string, &source_file, NULL, "draft" },
  { "copy",    0, NULL, MU_OPTION_DEFAULT,
    N_("preserve the source folder copy"),
    mu_c_bool, &link_flag },
  { "link",    0, NULL, MU_OPTION_ALIAS },
  { "preserve", 0, NULL, MU_OPTION_HIDDEN,
    N_("try to preserve message sequence numbers"),
    mu_c_bool, NULL, mh_opt_notimpl_warning },
  { "source", 0, N_("FOLDER"), MU_OPTION_DEFAULT,
    N_("specify source folder; it will become the current folder after the program exits"),
    mu_c_string, NULL, mh_opt_set_folder },
  { "src", 0, NULL, MU_OPTION_ALIAS },
  { "file", 0, N_("FILE"), MU_OPTION_DEFAULT,
    N_("use FILE as the source message"),
    mu_c_string, &source_file },
  MU_OPTION_END
};

void
open_folders (void)
{
  int rc;
  mu_iterator_t itr;

  if (!folder_name_list)
    {
      mu_error (_("no folder specified"));
      exit (1);
    }

  if ((rc = mu_list_create (&folder_mbox_list)) != 0)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_list_create", NULL, rc);
      exit (1);
    }

  if ((rc = mu_list_get_iterator (folder_name_list, &itr)) != 0)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_list_get_iterator", NULL, rc);
      exit (1);
    }

  for (mu_iterator_first (itr); !mu_iterator_is_done (itr); mu_iterator_next (itr))
    {
      char *name = NULL;
      mu_mailbox_t mbox;
      
      mu_iterator_current (itr, (void **)&name);
      mbox = mh_open_folder (name, MU_STREAM_RDWR|MU_STREAM_CREAT);
      mu_list_append (folder_mbox_list, mbox);
      free (name);
    }
  mu_iterator_destroy (&itr);
  mu_list_destroy (&folder_name_list);
}

void
enumerate_folders (void (*f) (void *, mu_mailbox_t), void *data)
{
  mu_iterator_t itr;

  if (mu_list_get_iterator (folder_mbox_list, &itr))
    {
      mu_error (_("cannot create iterator"));
      exit (1);
    }

  for (mu_iterator_first (itr); !mu_iterator_is_done (itr); mu_iterator_next (itr))
    {
      mu_mailbox_t mbox;
      mu_iterator_current (itr, (void **)&mbox);
      (*f) (data, mbox);
    }
  mu_iterator_destroy (&itr);
}
  
void
_close_folder (void *unused, mu_mailbox_t mbox)
{
  mu_mailbox_close (mbox);
  mu_mailbox_destroy (&mbox);
}

void
close_folders (void)
{
  enumerate_folders (_close_folder, NULL);
}

void
refile_folder (void *data, mu_mailbox_t mbox)
{
  mu_message_t msg = data;
  int rc;
  
  rc = mu_mailbox_append_message (mbox, msg);
  if (rc)
    {
      mu_error (_("error appending message: %s"), mu_strerror (rc));
      exit (1);
    }
}

void
refile (mu_message_t msg)
{
  enumerate_folders (refile_folder, msg);
}

int
refile_iterator (size_t num, mu_message_t msg, void *data)
{
  enumerate_folders (refile_folder, msg);
  if (!link_flag)
    {
      mu_attribute_t attr;
      mu_message_get_attribute (msg, &attr);
      mu_attribute_set_deleted (attr);
    }
  return 0;
}

int
main (int argc, char **argv)
{
  mu_msgset_t msgset;
  mu_mailbox_t mbox;
  int status, i, j;

  mh_getopt (&argc, &argv, options, 0, args_doc, prog_doc, NULL);
  /* Collect any surplus folders */
  for (i = j = 0; i < argc; i++)
    {
      if (argv[i][0] == '+')
	add_folder (argv[i]);
      else
	argv[j++] = argv[i];
    }
  argv[j] = NULL;
  argc = j;
  
  open_folders ();

  if (source_file)
    {
      mu_message_t msg;
      
      if (argc > 0)
	{
	  mu_error (_("both message set and source file given"));
	  exit (1);
	}
      msg = mh_file_to_message (mu_folder_directory (), source_file);
      refile (msg);
      if (!link_flag)
	unlink (source_file);
      status = 0;
    }
  else
    {
      mbox = mh_open_folder (mh_current_folder (), MU_STREAM_RDWR);
      mh_msgset_parse (&msgset, mbox, argc, argv, "cur");

      status = mu_msgset_foreach_message (msgset, refile_iterator, NULL);

      mh_sequences_elim (msgset);

      mu_mailbox_expunge (mbox);
      mu_mailbox_close (mbox);
      mu_mailbox_destroy (&mbox);
    }

  close_folders ();
  
  return status;
}
