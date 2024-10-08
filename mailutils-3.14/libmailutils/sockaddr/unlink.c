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

struct mu_sockaddr *
mu_sockaddr_unlink (struct mu_sockaddr *addr)
{
  struct mu_sockaddr *p;

  if (!addr)
    return NULL;

  p = addr->prev;
  if (p)
    p->next = addr->next;
  
  p = addr->next;
  if (p)
    p->prev = addr->prev;

  addr->prev = addr->next = NULL;
  
  return p;
}

