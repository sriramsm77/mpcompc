/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2011-2022 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include <mailutils/sockaddr.h>

void
mu_sockaddr_free (struct mu_sockaddr *addr)
{
  if (!addr)
    return;
  free (addr->addr);
  free (addr->str);
  free (addr);
}

void
mu_sockaddr_free_list (struct mu_sockaddr *addr)
{
  if (!addr)
    return;
  if (addr->prev)
    addr->prev->next = NULL;
  while (addr)
    {
      struct mu_sockaddr *next = addr->next;
      mu_sockaddr_free (addr);
      addr = next;
    }
}


    
