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

#ifndef _MAILUTILS_ATTRIBUTE_H
#define _MAILUTILS_ATTRIBUTE_H

#include <mailutils/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Message attributes:
 */
  
/* The message has been replied to: */
#define MU_ATTRIBUTE_ANSWERED 0x01

/* The message is explicitly flagged for some purpose: */
#define MU_ATTRIBUTE_FLAGGED  0x02

/* The message is deleted.  It will remain in this state until it
   is expunged (and, consequently, physically removed from the
   mailbox, or the attribute is removed. */
#define MU_ATTRIBUTE_DELETED  0x04

/* This is a draft message */
#define MU_ATTRIBUTE_DRAFT    0x08

/* The user has seen the message.  Notice, that it does not mean that
   they have read it, all it implies is that the user is aware about
   existence of this message in the mailbox (e.g. it was displayed in
   some kind of mailbox listing, as the mail "h" command). */
#define MU_ATTRIBUTE_SEEN     0x10

/* The user has read the message at least partially. */
#define MU_ATTRIBUTE_READ     0x20

/* The message was modified, but not yet saved to the mailbox. */  
#define MU_ATTRIBUTE_MODIFIED 0x40

/* The message has been forwarded to someone else */ 
#define MU_ATTRIBUTE_FORWARDED 0x80

/* A message is recent if the current session is the first session
   to have been notified about it. Practically, a message is considered
   "recent" if it does not have MU_ATTRIBUTE_SEEN set. For consistency
   a pseudo-attribute is provided: */
#define MU_ATTRIBUTE_RECENT   0 

#define MU_ATTRIBUTE_IS_UNSEEN(f) \
      ((f) == 0 || ! ((f) & MU_ATTRIBUTE_SEEN))

#define MU_ATTRIBUTE_IS_UNREAD(f) \
      ((f) == 0 || ! ((f) & MU_ATTRIBUTE_READ))

extern int mu_attribute_create          (mu_attribute_t *, void *);
extern void mu_attribute_destroy        (mu_attribute_t *, void *);
extern void *mu_attribute_get_owner    (mu_attribute_t);
extern int mu_attribute_is_modified     (mu_attribute_t);
extern int mu_attribute_clear_modified  (mu_attribute_t);
extern int mu_attribute_set_modified    (mu_attribute_t attr);

extern int mu_attribute_is_userflag     (mu_attribute_t, int);
extern int mu_attribute_is_seen         (mu_attribute_t);
extern int mu_attribute_is_answered     (mu_attribute_t);
extern int mu_attribute_is_flagged      (mu_attribute_t);
extern int mu_attribute_is_deleted      (mu_attribute_t);
extern int mu_attribute_is_draft        (mu_attribute_t);
extern int mu_attribute_is_recent       (mu_attribute_t);
extern int mu_attribute_is_read         (mu_attribute_t);
extern int mu_attribute_is_forwarded    (mu_attribute_t);

extern int mu_attribute_set_userflag    (mu_attribute_t, int);
extern int mu_attribute_set_seen        (mu_attribute_t);
extern int mu_attribute_set_answered    (mu_attribute_t);
extern int mu_attribute_set_flagged     (mu_attribute_t);
extern int mu_attribute_set_deleted     (mu_attribute_t);
extern int mu_attribute_set_draft       (mu_attribute_t);
extern int mu_attribute_set_recent      (mu_attribute_t);
extern int mu_attribute_set_read        (mu_attribute_t);
extern int mu_attribute_set_forwarded   (mu_attribute_t);

extern int mu_attribute_unset_userflag  (mu_attribute_t, int);
extern int mu_attribute_unset_seen      (mu_attribute_t);
extern int mu_attribute_unset_answered  (mu_attribute_t);
extern int mu_attribute_unset_flagged   (mu_attribute_t);
extern int mu_attribute_unset_deleted   (mu_attribute_t);
extern int mu_attribute_unset_draft     (mu_attribute_t);
extern int mu_attribute_unset_recent    (mu_attribute_t);
extern int mu_attribute_unset_read      (mu_attribute_t);
extern int mu_attribute_unset_forwarded (mu_attribute_t);

extern int mu_attribute_get_flags       (mu_attribute_t, int *);
extern int mu_attribute_set_flags       (mu_attribute_t, int);
extern int mu_attribute_unset_flags     (mu_attribute_t, int);

extern int mu_attribute_set_set_flags   (mu_attribute_t,
				      int (*_set_flags) (mu_attribute_t, int),
				      void *);
extern int mu_attribute_set_unset_flags (mu_attribute_t,
				      int (*_unset_flags) (mu_attribute_t, int),
				      void *);
extern int mu_attribute_set_get_flags   (mu_attribute_t,
				      int (*_get_flags) (mu_attribute_t, int *),
				      void *);
extern int mu_attribute_is_equal        (mu_attribute_t, mu_attribute_t att2);

extern int mu_attribute_copy            (mu_attribute_t, mu_attribute_t);

/* Maximum size of buffer for mu_attribute_to_string call, including nul */
#define MU_STATUS_BUF_SIZE sizeof("AFDdOPR")

extern int mu_attribute_to_string       (mu_attribute_t, char *, size_t, size_t *);
extern int mu_attribute_flags_to_string (int flags, char *buffer, size_t len, size_t *pn);
extern int mu_attribute_string_to_flags (const char *, int *);
static int mu_string_to_flags (const char *, int *) MU_DEPRECATED;
static inline int mu_string_to_flags (const char *b, int *f)
{
  return mu_attribute_string_to_flags (b, f);
}
#ifdef __cplusplus
}
#endif

#endif /* _MAILUTILS_ATTRIBUTE_H */
