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

#if defined(HAVE_CONFIG_H)
# include <config.h>
#endif
#include "muscript.h"
#include "muscript_priv.h"

struct sieve_log_data
{
  char *user;
  char *hdr;
};

static void
_sieve_action_log (mu_sieve_machine_t mach,
		   const char *action, const char *fmt, va_list ap)
{
  struct sieve_log_data *ldat = mu_sieve_get_data (mach);
  int pfx = 0;
  mu_stream_t stream;
  mu_message_t msg = mu_sieve_get_message (mach);

  mu_sieve_get_diag_stream (mach, &stream);
  mu_stream_printf (stream, "\033s<%d>\033O<%d>", MU_LOG_NOTICE,
		    MU_LOGMODE_LOCUS);

  if (ldat)
    {
      if (ldat->user)
	mu_stream_printf (stream, _("(user %s) "), ldat->user);
      if (ldat->hdr)
	{
	  mu_header_t hdr = NULL;
	  char *val = NULL;
	  mu_message_get_header (msg, &hdr);
	  if (mu_header_aget_value (hdr, ldat->hdr, &val) == 0
	      || mu_header_aget_value (hdr, MU_HEADER_MESSAGE_ID, &val) == 0)
	    {
	      pfx = 1;
	      mu_stream_printf (stream, _("%s on msg %s"), action, val);
	      free (val);
	    }
	}
    }
  
  if (!pfx)
    {
      size_t uid = 0;
      mu_message_get_uid (msg, &uid);
      mu_stream_printf (stream, _("%s on msg uid %lu"), action,
			(unsigned long) uid);
    }
  
  if (fmt && strlen (fmt))
    {
      mu_stream_printf (stream, "; ");
      mu_stream_vprintf (stream, fmt, ap);
    }
  mu_stream_printf (stream, "\n");
  mu_stream_unref (stream);
}

static void
sieve_setenv (mu_sieve_machine_t mach, const char **env)
{
  if (env)
    {
      char *buffer = NULL;
      size_t buflen = 0;
      size_t i;
      char *p;
      
      for (i = 0; env[i]; i++)
	{
	  if (buflen < strlen (env[i]) + 1)
	    {
	      buflen = strlen (env[i]) + 1;
	      buffer = mu_realloc (buffer, buflen);
	    }
	  strcpy (buffer, env[i]);
	  p = strchr (buffer, '=');
	  if (!p)
	    continue;
	  *p++ = 0;
	  mu_sieve_set_environ (mach, buffer, p);
	}
      free (buffer);
    }
}

static int
sieve_init (const char *prog, const char **env, mu_script_descr_t *pdescr)
{
  int rc;
  mu_sieve_machine_t mach;

  rc = mu_sieve_machine_create (&mach);
  if (rc == 0)
    {
      if (mu_script_sieve_log)
	mu_sieve_set_logger (mach, _sieve_action_log);
      sieve_setenv (mach, env);
      rc = mu_sieve_compile (mach, prog);
    }
  *pdescr = (mu_script_descr_t) mach;
  return rc;
}

static int
sieve_log_enable (mu_script_descr_t descr, const char *name, const char *hdr)
{
  mu_sieve_machine_t mach = (mu_sieve_machine_t) descr;
  struct sieve_log_data *ldat;
  size_t size = 0;
  char *p;
  
  if (name)
    size += strlen (name) + 1;
  if (hdr)
    size += strlen (hdr) + 1;

  if (size)
    {
      ldat = calloc (1, sizeof (*ldat) + size);
      if (!ldat)
	return ENOMEM;
      p = (char *) (ldat + 1); 
      if (name)
	{
	  ldat->user = p;
	  p = mu_stpcpy (p, name) + 1;
	}
      if (hdr)
	{
	  ldat->hdr = p;
	  strcpy (p, hdr);
	}
      
      mu_sieve_set_data (mach, ldat);
    }
  mu_sieve_set_logger (mach, _sieve_action_log);
  return 0;
}

static int
sieve_done (mu_script_descr_t descr)
{
  mu_sieve_machine_t mach = (mu_sieve_machine_t) descr;
  void *p = mu_sieve_get_data (mach);
  free (p);
  mu_sieve_machine_destroy (&mach);
  return 0;
}

static int
sieve_proc (mu_script_descr_t descr, mu_message_t msg)
{
  return mu_sieve_message ((mu_sieve_machine_t) descr, msg);
}

struct mu_script_fun mu_script_sieve = {
  "sieve",
  "sv\0siv\0sieve\0",
  sieve_init,
  sieve_done,
  sieve_proc,
  sieve_log_enable
};

