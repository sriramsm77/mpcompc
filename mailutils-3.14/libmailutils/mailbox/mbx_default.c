/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

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

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include <confpaths.h>

#include <mailutils/nls.h>
#include <mailutils/mailbox.h>
#include <mailutils/util.h>
#include <mailutils/debug.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/mu_auth.h>
#include <mailutils/folder.h>
#include <mailutils/auth.h>
#include <mailutils/cstr.h>
#include <mailutils/io.h>

#include <mailutils/sys/mailbox.h>

char *mu_ticket_file = "~/.mu-tickets";

static char *_mu_mailbox_pattern;

static char *_default_folder_dir = "Mail";
static char *_mu_folder_dir;

#define USERSUFFIX "${user}"

static int
mu_normalize_mailbox_url (char **pout, const char *dir)
{
  int len;
  
  if (!pout)
    return MU_ERR_OUT_PTR_NULL;
      
  len = strlen (dir);
  if (dir[len-1] == '=')
    {
      if (!(len > 5 && strcmp (dir + len - 5, "user=") == 0))
	return MU_ERR_BAD_FILENAME;
      else
	{
	  int rc = mu_asprintf (pout, "%s%s", dir, USERSUFFIX);
	  if (rc)
	    return rc;
	}
    }
  else
    {
      *pout = mu_make_file_name (dir, USERSUFFIX);
      if (!*pout)
	return errno;
    }
  
  return 0;
}

int
mu_set_mail_directory (const char *p)
{
  if (_mu_mailbox_pattern)
    free (_mu_mailbox_pattern);
  if (!p)
    {
      _mu_mailbox_pattern = NULL;
      return 0;
    }
  return mu_normalize_mailbox_url (&_mu_mailbox_pattern, p);
}

int
mu_set_mailbox_pattern (const char *pat)
{
  char *p;

  if (pat)
    {
      p = strdup (pat);
      if (!p)
        return ENOMEM;
    }
  else
    p = NULL;
  if (_mu_mailbox_pattern)
    free (_mu_mailbox_pattern);
  _mu_mailbox_pattern = p;
  return 0;
}

int
mu_set_folder_directory (const char *p)
{
  char *fdir;

  if (p)
    {
      fdir = strdup (p);
      if (!fdir)
	return ENOMEM;
    }
  else
    fdir = NULL;
  
  if (_mu_folder_dir != _default_folder_dir)
    free (_mu_folder_dir);
  _mu_folder_dir = fdir;
  return 0;
}

const char *
mu_mailbox_url (void)
{
  if (_mu_mailbox_pattern)
    return _mu_mailbox_pattern;
  return MU_PATH_MAILDIR "/" USERSUFFIX;
}

const char *
mu_folder_directory (void)
{
  if (!_mu_folder_dir && _default_folder_dir)
    {
      mu_set_folder_directory (_default_folder_dir);
      _default_folder_dir = NULL;
    }
  return _mu_folder_dir;
}

int
mu_construct_user_mailbox_url (char **pout, const char *name)
{
  int rc;
  const char *pat = mu_mailbox_url ();
  char *result;
    
  rc = mu_str_vexpand (&result, pat, "user", name, NULL);
  if (rc)
    {
      if (rc == MU_ERR_FAILURE)
	{
	  mu_error (_("cannot expand line `%s': %s"), pat, result);
	  free (result);
	}
      else
	mu_error (_("cannot expand line `%s': %s"), pat, mu_strerror (rc));
      return rc;
    }

  *pout = result;
  return 0;
}

static int
split_shortcut (const char *file, const char pfx[], char **user, char **rest)
{
  *user = NULL;
  *rest = NULL;

  if (!strchr (pfx, file[0]))
    return 0;

  if (*++file == 0)
    return 0;
  else
    {
      char *p = strchr (file, '/');
      int len;
      if (p)
        len = p - file + 1;
      else
        len = strlen (file) + 1;

      if (len == 1)
	*user = NULL;
      else
	{
	  *user = calloc (1, len);
	  if (!*user)
	    return ENOMEM;

	  memcpy (*user, file, len);
	  (*user)[len-1] = 0;
	}
      file += len-1;
      if (file[0] == '/')
        file++;
    }

  if (file[0])
    {
      *rest = strdup (file);
      if (!*rest)
        {
          free (*user);
          return ENOMEM;
        }
    }
  
  return 0;
}

static char *
get_homedir (const char *user)
{
  char *homedir = NULL;
  struct mu_auth_data *auth = NULL;
  
  if (user)
    {
      auth = mu_get_auth_by_name (user);
      if (auth)
        homedir = auth->dir;
    }
  else
    {
      /* NOTE: Should we honor ${HOME}?  */
      homedir = getenv ("HOME");
      if (homedir == NULL)
        {
	  auth = mu_get_auth_by_name (user);
	  if (auth)
	    homedir = auth->dir;
        }
    }

  if (homedir)
    homedir = strdup (homedir);
  mu_auth_data_free (auth);
  return homedir;
}

static int
user_mailbox_name (const char *user, char **mailbox_name)
{
  if (!user)
    user = (getenv ("LOGNAME")) ? getenv ("LOGNAME") : getenv ("USER");

  if (user)
    {
      int rc = mu_construct_user_mailbox_url (mailbox_name, user);
      if (rc)
	return rc;
    }
  else
    {
      struct mu_auth_data *auth = mu_get_auth_by_uid (getuid ());

      if (!auth)
        {
          mu_error ("Who am I?");
          return EINVAL;
        }
      *mailbox_name = strdup (auth->mailbox);
      mu_auth_data_free (auth);
    }

  return 0;
}

static int
plus_expand (const char *file, char **buf)
{
  int rc = 0;
  const char *folder_dir = mu_folder_directory ();

  if (!folder_dir)
    {
      char *p = strdup (file);
      if (!p)
	return ENOMEM;
      *buf = p;
    }
  else
    {
      file++;

      if (folder_dir[0] == '/' || mu_is_proto (folder_dir))
	{
	  char *p = mu_make_file_name (folder_dir, file);
	  if (!p)
	    return errno;
	  *buf = p;
	}
      else
	{
	  char *home = get_homedir (NULL);
      
	  if (!home)
	    return ENOENT;
      
	  rc = mu_asprintf (buf, "%s/%s/%s", home, folder_dir, file);
	  free (home);
	}
    }
  
  return rc;
}

static int
percent_expand (const char *file, char **mbox)
{
  char *user = NULL;
  char *path = NULL;
  int status;
  
  if ((status = split_shortcut (file, "%", &user, &path)))
    return status;

  if (path)
    {
      free (user);
      free (path);
      return ENOENT;
    }

  status = user_mailbox_name (user, mbox);
  free (user);
  return status;
}

int
mu_mailbox_attach_ticket (mu_mailbox_t mbox)
{
  int rc;
  mu_folder_t folder = NULL;

  if ((rc = mu_mailbox_get_folder (mbox, &folder)) == 0)
    rc = mu_folder_attach_ticket (folder);
  return rc;
}

/* Expand mailbox name according to the following rules:

   NAME            Expands to
   -------------+------------------------------------
   %            -> system mailbox for the real uid
   %user        -> system mailbox for the given user
   ~/file       -> /home/user/file
   ~user/file   -> /home/user/file
   +file        -> /home/user/Mail/file
   =file        -> /home/user/Mail/file
 */
int
mu_mailbox_expand_name (const char *name, char **expansion)
{
  int status = 0;
  char *p;
  char *mbox = NULL;
  
  if (!name)
    return EINVAL;
  if (!expansion)
    return MU_ERR_OUT_PTR_NULL;

  p = mu_tilde_expansion (name, MU_HIERARCHY_DELIMITER, NULL);
  if (!p)
    return errno;
  switch (p[0])
    {
    case '%':
      status = percent_expand (p, &mbox);
      break;
      
    case '+':
    case '=':
      status = plus_expand (p, &mbox);
      break;
  
    case '/':
      mbox = p;
      p = NULL;
      break;
      
    default:
      if (!mu_is_proto (p))
	{
	  char *dir = mu_getcwd();
	  mbox = mu_make_file_name (dir, p);
	  if (!mbox)
	    status = errno;
	  free (dir);  
	}
      else
	{
	  mbox = p;
	  p = NULL;
	}
    }
  free (p);
  if (status == 0)
    *expansion = mbox;
  return status;
}

/* Expand mailbox name MAIL and create a mailbox structure for it. */
int
mu_mailbox_create_default (mu_mailbox_t *pmbox, const char *mail)
{
  char *mboxname = NULL;
  char *name_ptr = NULL;
  int status = 0;

  /* Sanity.  */
  if (pmbox == NULL)
    return MU_ERR_OUT_PTR_NULL;

  if (mail && *mail == 0)
    mail = NULL;
  
  if (mail == NULL)
    {
      if (!_mu_mailbox_pattern)
	{
	  /* Other utilities may not understand GNU mailutils url namespace, so
	     use FOLDER instead, to not confuse others by using MAIL.  */
	  mail = getenv ("FOLDER");
	  if (!mail)
	    /* Fallback to well-known environment.  */
	    mail = getenv ("MAIL");
	}

      if (!mail)
	{
	  if ((status = user_mailbox_name (NULL, &name_ptr)))
	    return status;
	  mail = name_ptr;
	}
    }

  status = mu_mailbox_expand_name (mail, &mboxname);
  free (name_ptr);
  if (status)
    return status;
  
  status = mu_mailbox_create (pmbox, mboxname);
  free (mboxname);
  if (status == 0)
    mu_mailbox_attach_ticket (*pmbox);
      
  return status;
}
