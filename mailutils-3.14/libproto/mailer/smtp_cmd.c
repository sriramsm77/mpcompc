/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2010-2022 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <mailutils/errno.h>
#include <mailutils/cctype.h>
#include <mailutils/list.h>
#include <mailutils/util.h>
#include <mailutils/smtp.h>
#include <mailutils/stream.h>
#include <mailutils/sys/smtp.h>

/* Send an arbitrary command */
int
mu_smtp_cmd (mu_smtp_t smtp, int argc, char **argv)
{
  int status, i;
  
  if (!smtp)
    return EINVAL;
  if (MU_SMTP_FISSET (smtp, _MU_SMTP_ERR))
    return MU_ERR_FAILURE;
  status = mu_smtp_write (smtp, "%s", argv[0]);
  MU_SMTP_CHECK_ERROR (smtp, status);
  for (i = 1; i < argc; i++)
    {
      status = mu_smtp_write (smtp, " %s", argv[i]);
      MU_SMTP_CHECK_ERROR (smtp, status);
    }
  
  status = mu_smtp_write (smtp, "\r\n");
  MU_SMTP_CHECK_ERROR (smtp, status);
  status = mu_smtp_response (smtp);
  MU_SMTP_CHECK_ERROR (smtp, status);

  if (smtp->replcode[0] > '3')
    return MU_ERR_REPLY;
  
  return 0;
}

      
