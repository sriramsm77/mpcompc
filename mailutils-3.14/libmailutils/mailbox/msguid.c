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

#include <config.h>
#include <stdlib.h>

#include <mailutils/types.h>
#include <mailutils/message.h>
#include <mailutils/errno.h>
#include <mailutils/sys/message.h>

int
mu_message_get_uid (mu_message_t msg, size_t *puid)
{
  if (msg == NULL)
    return EINVAL;
  if (msg->_get_uid)
    return msg->_get_uid (msg, puid);
  *puid = 0;
  return 0;
}

int
mu_message_set_uid (mu_message_t msg, int (*_get_uid) (mu_message_t, size_t *),
		    void *owner)
{
  if (msg == NULL)
    return EINVAL;
  if (msg->owner != owner)
    return EACCES;
  msg->_get_uid = _get_uid;
  return 0;
}
