/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2007-2022 Free Software Foundation, Inc.

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

#include "libmda.h"

int exit_code;

int
mda_switch_user_id (struct mu_auth_data *auth, int user)
{
  int rc;
  uid_t uid;
  
  if (!auth || auth->change_uid == 0)
    return 0;
  
  if (user)
    uid = auth->uid;
  else
    uid = 0;
  
#if defined(HAVE_SETREUID)
  rc = setreuid (0, uid);
#elif defined(HAVE_SETRESUID)
  rc = setresuid (-1, uid, -1);
#elif defined(HAVE_SETEUID)
  rc = seteuid (uid);
#else
# error "No way to reset user privileges?"
#endif
  if (rc < 0)
    mda_error ("setreuid(0, %d): %s (r=%d, e=%d)",
	       uid, strerror (errno), getuid (), geteuid ());
  return rc;
}

static int temp_errors[] = {
#ifdef EAGAIN
  EAGAIN, /* Try again */
#endif
#ifdef EBUSY
  EBUSY, /* Device or resource busy */
#endif
#ifdef EPROCLIM
  EPROCLIM, /* Too many processes */
#endif
#ifdef EUSERS
  EUSERS, /* Too many users */
#endif
#ifdef ECONNABORTED
  ECONNABORTED, /* Software caused connection abort */
#endif
#ifdef ECONNREFUSED
  ECONNREFUSED, /* Connection refused */
#endif
#ifdef ECONNRESET
  ECONNRESET, /* Connection reset by peer */
#endif
#ifdef EDEADLK
  EDEADLK, /* Resource deadlock would occur */
#endif
#ifdef EDEADLOCK
  EDEADLOCK, /* Resource deadlock would occur */
#endif
#ifdef EFBIG
  EFBIG, /* File too large */
#endif
#ifdef EHOSTDOWN
  EHOSTDOWN, /* Host is down */
#endif
#ifdef EHOSTUNREACH
  EHOSTUNREACH, /* No route to host */
#endif
#ifdef EMFILE
  EMFILE, /* Too many open files */
#endif
#ifdef ENETDOWN
  ENETDOWN, /* Network is down */
#endif
#ifdef ENETUNREACH
  ENETUNREACH, /* Network is unreachable */
#endif
#ifdef ENETRESET
  ENETRESET, /* Network dropped connection because of reset */
#endif
#ifdef ENFILE
  ENFILE, /* File table overflow */
#endif
#ifdef ENOBUFS
  ENOBUFS, /* No buffer space available */
#endif
#ifdef ENOMEM
  ENOMEM, /* Out of memory */
#endif
#ifdef ENOSPC
  ENOSPC, /* No space left on device */
#endif
#ifdef EROFS
  EROFS, /* Read-only file system */
#endif
#ifdef ESTALE
  ESTALE, /* Stale NFS file handle */
#endif
#ifdef ETIMEDOUT
  ETIMEDOUT,  /* Connection timed out */
#endif
#ifdef EWOULDBLOCK
  EWOULDBLOCK, /* Operation would block */
#endif
};
  
static void
guess_retval (int ec)
{
  int i;
  /* Temporary failures override hard errors. */
  if (exit_code == EX_TEMPFAIL)
    return;
#ifdef EDQUOT
  if (ec == EDQUOT)
    {
      exit_code = EX_QUOTA;
      return;
    }
#endif

  for (i = 0; i < sizeof (temp_errors)/sizeof (temp_errors[0]); i++)
    if (temp_errors[i] == ec)
      {
	exit_code = EX_TEMPFAIL;
	return;
      }
  exit_code = EX_UNAVAILABLE;
}

void
mda_error (const char *fmt, ...)
{
  va_list ap;
  guess_retval (errno);
  va_start (ap, fmt);
  mu_verror (fmt, ap);
  va_end (ap);
}

#ifndef USE_MAILBOX_QUOTAS
# define mda_mailquota_cfg NULL
#endif

static struct mu_cli_capa mda_cli_capa[] = {
  { "forward", NULL, mda_forward_cfg },
  { "deliver", mda_deliver_options, mda_deliver_cfg },
  { "quota", NULL, mda_mailquota_cfg },
  { "script", mda_script_options, mda_script_cfg },
  { NULL }
};

void
mda_cli_capa_init (void)
{
  size_t i;

  for (i = 0; mda_cli_capa[i].name; i++)
    mu_cli_capa_register (&mda_cli_capa[i]);
}
