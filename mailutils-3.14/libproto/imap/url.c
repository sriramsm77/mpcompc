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
   Public License along with this library  If not, see
   <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>

#include <mailutils/sys/registrar.h>
#include <mailutils/sys/url.h>
#include <mailutils/sys/imap.h>
#include <mailutils/util.h>

static void url_imap_destroy (mu_url_t url);

static void
url_imap_destroy (mu_url_t url MU_ARG_UNUSED)
{
}

static int
url_imap_get_path (const mu_url_t url, char *bufptr, size_t bufsize,
		   size_t *rsize)
{
  bufsize = mu_cpystr (bufptr, "INBOX", bufsize);
  if (rsize)
    *rsize = bufsize;
  return 0;
}

/*
  IMAP URLs:
    imap://[<user>[;AUTH=<auth>]@]<host>[/<mailbox>]
    imap://[<user>[:<pass>]@]<host>[/<mailbox>]
*/

int
_mu_imap_url_init (mu_url_t url)
{
  if (url->port == 0)
    url->port = MU_IMAP_PORT;
  
  url->_destroy = url_imap_destroy;
  url->_get_path = url_imap_get_path;
    
  if (!url->host || url->qargc)
    return EINVAL;

  /* fill in default auth, if necessary */
  if (!url->auth)
    {
      url->auth = malloc (1 + 1);
      if (!url->auth)
	return ENOMEM;

      url->auth[0] = '*';
      url->auth[1] = '\0';
    }

  return 0;
}

/*
  IMAPS URLs:
    imaps://[<user>[;AUTH=<auth>]@]<host>[/<mailbox>]
    imaps://[<user>[:<pass>]@]<host>[/<mailbox>]
*/

int
_mu_imaps_url_init (mu_url_t url)
{
  if (url->port == 0)
    url->port = MU_IMAPS_PORT;
  
  url->_destroy = url_imap_destroy;
  url->_get_path = url_imap_get_path;

  if (!url->host || url->qargc)
    return EINVAL;

  /* fill in default auth, if necessary */
  if (!url->auth)
    {
      url->auth = malloc (1 + 1);
      if (!url->auth)
	return ENOMEM;

      url->auth[0] = '*';
      url->auth[1] = '\0';
    }

  return 0;
}

