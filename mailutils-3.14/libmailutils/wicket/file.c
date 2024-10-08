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

/* A "file wicket" implementation */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/auth.h>
#include <mailutils/url.h>
#include <mailutils/stream.h>
#include <mailutils/cstr.h>
#include <mailutils/nls.h>

struct file_wicket
{
  char *filename;
};

static void
_file_wicket_destroy (mu_wicket_t wicket)
{
  struct file_wicket *fw = mu_wicket_get_data (wicket);
  free (fw->filename);
  free (fw);
}

struct file_ticket
{
  char *filename;
  char *user;
  mu_url_t tickurl;
};

static void
file_ticket_destroy (mu_ticket_t ticket)
{
  struct file_ticket *ft = mu_ticket_get_data (ticket);
  if (ft)
    {
      free (ft->filename);
      free (ft->user);
      mu_url_destroy (&ft->tickurl);
      free (ft);
    }
}

int
file_ticket_get_cred (mu_ticket_t ticket, mu_url_t url, const char *challenge,
		      char **pplain, mu_secret_t *psec)
{
  struct file_ticket *ft = mu_ticket_get_data (ticket);
  int rc = 0;

  if (!ft->tickurl)
    {
      rc = mu_wicket_file_match_url (ft->filename, url,
				     MU_URL_PARSE_ALL,
				     &ft->tickurl);
      if (rc)
	return rc;
     }
  if (pplain)
    {
      if (ft->user)
	{
	  *pplain = strdup (ft->user);
	  if (!*pplain)
	    rc = ENOMEM;
	}
      else
	rc = mu_url_aget_user (ft->tickurl, pplain);
    }
  else
    rc = mu_url_get_secret (ft->tickurl, psec);
  return rc;
}

static int
_file_wicket_get_ticket (mu_wicket_t wicket, void *data,
			 const char *user, mu_ticket_t *pticket)
{
  int rc;
  mu_ticket_t ticket;
  struct file_wicket *fw = data;
  struct file_ticket *ft = calloc (1, sizeof (*ft));
  ft->filename = strdup (fw->filename);
  if (!ft->filename)
    {
      free (ft);
      return ENOMEM;
    }
  if (user)
    {
      ft->user = strdup (user);
      if (!ft->user)
	{
	  free (ft->filename);
	  free (ft);
	  return ENOMEM;
	}
    }
  else
    ft->user = NULL;

  rc = mu_ticket_create (&ticket, NULL);
  if (rc)
    {
      free (ft->filename);
      free (ft->user);
      free (ft);
      return rc;
    }

  mu_ticket_set_destroy (ticket, file_ticket_destroy, NULL);
  mu_ticket_set_data (ticket, ft, NULL);
  mu_ticket_set_get_cred (ticket, file_ticket_get_cred, NULL);

  *pticket = ticket;
  return 0;
}

int
mu_wicket_stream_match_url (mu_stream_t stream, struct mu_locus_point *loc,
			    mu_url_t url, int parse_flags,
			    mu_url_t *pticket_url)
{
  int rc;
  mu_url_t u = NULL;
  char *buf = NULL;
  size_t bufsize = 0;
  size_t len;
  mu_url_t pret = NULL;
  int weight = 0;
  int line = loc->mu_line;

  while ((rc = mu_stream_getline (stream, &buf, &bufsize, &len)) == 0
	 && len > 0)
    {
      char *p;
      int err;
      int n;

      loc->mu_line++;
      p = mu_str_stripws (buf);

      /* Skip empty lines and comments. */
      if (*p == 0 || *p == '#')
	continue;

      if ((err = mu_url_create_hint (&u, p, parse_flags, NULL)) != 0)
	{
	  /* Skip erroneous entry */
	  mu_error (_("%s:%u: cannot create URL: %s"),
		    loc->mu_file, loc->mu_line, mu_strerror (err));
	  continue;
	}

      if (!mu_url_has_flag (u, MU_URL_USER|MU_URL_SECRET))
	{
	  mu_error (_("%s:%u: URL is missing required parts"),
		    loc->mu_file, loc->mu_line);
	  mu_url_destroy (&u);
	  continue;
	}

      if (!mu_url_matches_ticket (u, url, &n))
	{
	  mu_url_destroy (&u);
	  continue;
	}

      if (!pret || n < weight)
	{
	  pret = u;
	  weight = n;
	  line = loc->mu_line;
	  if (weight == 0)
	    break;
	}
    }
  free (buf);

  if (rc == 0)
    {
      if (pret)
	{
	  *pticket_url = pret;
	  loc->mu_line = line;
	}
      else
	rc = MU_ERR_NOENT;
    }

  return rc;
}

int
mu_wicket_file_match_url (const char *name, mu_url_t url,
			  int parse_flags,
			  mu_url_t *pticket_url)
{
  mu_stream_t stream;
  int rc;
  struct mu_locus_point loc;

  rc = mu_file_stream_create (&stream, name, MU_STREAM_READ);
  if (rc)
    return rc;
  loc.mu_file = (char*) name;
  loc.mu_line = 0;
  loc.mu_col = 0;
  rc = mu_wicket_stream_match_url (stream, &loc, url, parse_flags,
				   pticket_url);
  mu_stream_close (stream);
  mu_stream_destroy (&stream);
  return rc;
}

int
mu_file_wicket_create (mu_wicket_t *pwicket, const char *filename)
{
  mu_wicket_t wicket;
  int rc;
  struct file_wicket *fw = calloc (1, sizeof (*fw));

  if (!fw)
    return ENOMEM;
  fw->filename = strdup (filename);
  if (!fw->filename)
    {
      free (fw);
      return ENOMEM;
    }

  rc = mu_wicket_create (&wicket);
  if (rc)
    {
      free (fw->filename);
      free (fw);
      return rc;
    }
  mu_wicket_set_data (wicket, fw);
  mu_wicket_set_destroy (wicket, _file_wicket_destroy);
  mu_wicket_set_get_ticket (wicket, _file_wicket_get_ticket);
  *pwicket = wicket;
  return 0;
}
