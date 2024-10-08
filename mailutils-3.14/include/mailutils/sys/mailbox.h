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

#ifndef _MAILUTILS_SYS_MAILBOX_H
# define _MAILUTILS_SYS_MAILBOX_H

# include <sys/types.h>
# include <time.h>
# include <stdio.h>

# include <mailutils/monitor.h>
# include <mailutils/mailbox.h>
# include <mailutils/iterator.h>

# ifdef __cplusplus
extern "C" {
# endif

/* Mailbox-specific flags */
#define _MU_MAILBOX_OPEN    0x10000000
#define _MU_MAILBOX_REMOVED 0x20000000
#define _MU_MAILBOX_NOTIFY  0x40000000
  
#define _MU_MAILBOX_MASK    0xF0000000

struct _mu_mailbox
{
  /* Data */
  mu_observable_t observable;
  mu_property_t property;
  mu_locker_t locker;
  mu_stream_t stream;
  mu_url_t url;
  int flags;
  mu_folder_t folder;
  mu_monitor_t monitor;
  mu_iterator_t iterator;

  /* Biff notification */
  char *notify_user;                   /* User name */
  int notify_fd;                       /* Socket descriptor */
  struct sockaddr *notify_sa;          /* Source sockaddr */
  
  /* Back pointer to the specific mailbox */
  void *data;

  /* Public methods */

  void (*_destroy)         (mu_mailbox_t);

  int  (*_open)            (mu_mailbox_t, int);
  int  (*_close)           (mu_mailbox_t);
  int  (*_remove)          (mu_mailbox_t);
  
  /* messages */
  int  (*_get_message)     (mu_mailbox_t, size_t, mu_message_t *);
  int  (*_append_message)  (mu_mailbox_t, mu_message_t, mu_envelope_t,
			    mu_attribute_t);
  int  (*_messages_count)  (mu_mailbox_t, size_t *);
  int  (*_messages_recent) (mu_mailbox_t, size_t *);
  int  (*_message_unseen)  (mu_mailbox_t, size_t *);
  int  (*_expunge)         (mu_mailbox_t);
  int  (*_sync)            (mu_mailbox_t);
  int  (*_get_uidvalidity) (mu_mailbox_t, unsigned long *);
  int  (*_set_uidvalidity) (mu_mailbox_t, unsigned long);
  int  (*_uidnext)         (mu_mailbox_t, size_t *);
  int  (*_get_property)    (mu_mailbox_t, mu_property_t *);

  int  (*_scan)            (mu_mailbox_t, size_t, size_t *);
  int  (*_is_updated)      (mu_mailbox_t);

  int  (*_get_size)        (mu_mailbox_t, mu_off_t *);

  int  (*_quick_get_message) (mu_mailbox_t, mu_message_qid_t, mu_message_t *);
  int  (*_get_uidls) (mu_mailbox_t, mu_list_t);

  int  (*_translate) (mu_mailbox_t, int cmd, size_t, size_t *);
  int  (*_copy) (mu_mailbox_t, mu_msgset_t, const char *, int);
  int  (*_get_atime) (mu_mailbox_t, time_t *);
};

# ifdef __cplusplus
}
# endif

#endif /* _MAILUTILS_SYS_MAILBOX_H */
