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

#include "mail.h"

/*
 * v[isual] [msglist]
 */

static int
visual0 (msgset_t *mspec, mu_message_t msg, void *data)
{
  char *file = mu_tempname (NULL);

  util_do_command ("copy %s", file); 
  util_do_command ("shell %s %s", getenv("VISUAL"), file);

  remove (file);
  free (file);

  /* Mark as read */
  util_mark_read (msg);

  set_cursor (msgset_msgno (mspec));
  
  return 0;
}

int
mail_visual (int argc, char **argv)
{
  return util_foreach_msg (argc, argv, MSG_NODELETED, visual0, NULL);
}

