/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003-2022 Free Software Foundation, Inc.

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

#include <mailutils/sys/pop3.h>

int
mu_pop3_uidl_all (mu_pop3_t pop3, mu_iterator_t *piterator)
{
  int status = mu_pop3_uidl_all_cmd (pop3);
  if (status)
    return status;
  status = mu_pop3_iterator_create (pop3, piterator);
  MU_POP3_CHECK_ERROR (pop3, status);
  pop3->state = MU_POP3_UIDL_RX;

  return status;
}
