/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2019-2022 Free Software Foundation, Inc.

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

#include <config.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef WITH_PTHREAD
# include <pthread.h>
#endif
#include <sys/stat.h>
#include <signal.h>
#include <mailutils/sys/dotmail.h>
#include <mailutils/sys/mailbox.h>
#include <mailutils/sys/message.h>
#include <mailutils/diag.h>
#include <mailutils/errno.h>
#include <mailutils/url.h>
#include <mailutils/property.h>
#include <mailutils/io.h>
#include <mailutils/observer.h>
#include <mailutils/filter.h>
#include <mailutils/stream.h>
#include <mailutils/locker.h>
#include <mailutils/nls.h>
#include <mailutils/header.h>
#include <mailutils/attribute.h>
#include <mailutils/envelope.h>
#include <mailutils/util.h>
#include <mailutils/cctype.h>

static void
dotmail_destroy (mu_mailbox_t mailbox)
{
  size_t i;
  struct mu_dotmail_mailbox *dmp = mailbox->data;

  if (!dmp)
    return;

  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_TRACE1,
	    ("%s (%s)", __func__, dmp->name));
  mu_monitor_wrlock (mailbox->monitor);
  for (i = 0; i < dmp->mesg_count; i++)
    {
      mu_dotmail_message_free (dmp->mesg[i]);
    }
  free (dmp->mesg);
  free (dmp->name);
  free (dmp);
  mailbox->data = NULL;
  mu_monitor_unlock (mailbox->monitor);
}

static int
dotmail_mailbox_init_stream (struct mu_dotmail_mailbox *dmp)
{
  int rc;
  mu_mailbox_t mailbox = dmp->mailbox;

  /*
   * Initialize stream flags.  If append mode is requested, convert it to
   * read-write, so that dotmail_flush_unlocked be able to update the
   * X-IMAPbase header in the first message, if necessary.
   */
  dmp->stream_flags = mailbox->flags;
  if (dmp->stream_flags & MU_STREAM_APPEND)
    dmp->stream_flags = (dmp->stream_flags & ~MU_STREAM_APPEND) | MU_STREAM_RDWR;
  rc = mu_mapfile_stream_create (&mailbox->stream, dmp->name, dmp->stream_flags);
  if (rc)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("%s:%s (%s): %s",
		 __func__, "mu_mapfile_stream_create", dmp->name,
		 mu_strerror (rc)));

      /* Fallback to regular file stream */
      rc = mu_file_stream_create (&mailbox->stream, dmp->name, mailbox->flags);
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("%s:%s (%s): %s",
		 __func__, "mu_file_stream_create", dmp->name,
		 mu_strerror (rc)));

      if (rc)
	return rc;
    }

  mu_stream_set_buffer (mailbox->stream, mu_buffer_full, 0);
  return 0;
}

static int
dotmail_open (mu_mailbox_t mailbox, int flags)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  int rc;

  if (!dmp)
    return EINVAL;

  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_TRACE1,
	    ("%s(%s, 0x%x)", __func__, dmp->name, mailbox->flags));

  mailbox->flags = flags;

  rc = dotmail_mailbox_init_stream (dmp);

  if (mailbox->locker == NULL
      && (flags & (MU_STREAM_WRITE | MU_STREAM_APPEND | MU_STREAM_CREAT)))
    {
      rc = mu_locker_create_ext (&mailbox->locker, dmp->name, NULL);
      if (rc)
	mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		  ("%s:%s (%s): %s",
		   __func__, "mu_locker_create_ext", dmp->name,
		   mu_strerror (rc)));
    }

  return rc;
}

enum
  {
    FLUSH_SYNC,
    FLUSH_EXPUNGE, /* implies SYNC */
    FLUSH_UIDVALIDITY
  };

static int dotmail_flush (struct mu_dotmail_mailbox *dmp, int flag);

static int
dotmail_close (mu_mailbox_t mailbox)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  size_t i;

  if (!dmp)
    return EINVAL;

  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_TRACE1,
	    ("%s (%s)", __func__, dmp->name));

  if (dmp->uidvalidity_changed)
    dotmail_flush (dmp, FLUSH_UIDVALIDITY);
  
  mu_locker_unlock (mailbox->locker);
  mu_monitor_wrlock (mailbox->monitor);
  for (i = 0; i < dmp->mesg_count; i++)
    {
      mu_dotmail_message_free (dmp->mesg[i]);
    }
  free (dmp->mesg);
  dmp->mesg = NULL;
  dmp->mesg_count = dmp->mesg_max = 0;
  dmp->size = 0;
  dmp->uidvalidity = 0;
  dmp->uidnext = 0;
  mu_monitor_unlock (mailbox->monitor);
  mu_stream_destroy (&mailbox->stream);
  return 0;
}

static int
dotmail_remove (mu_mailbox_t mailbox)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;

  if (!dmp)
    return EINVAL;
  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_TRACE1,
	    ("%s (%s)", __func__, dmp->name));
  if (unlink (dmp->name))
    return errno;
  return 0;
}

static int
dotmail_is_updated (mu_mailbox_t mailbox)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  mu_off_t size = 0;

  if (!dmp)
    return 0;

  if (mu_stream_size (mailbox->stream, &size) != 0)
    return 1;
  if (size < dmp->size)
    {
      mu_observable_notify (mailbox->observable, MU_EVT_MAILBOX_CORRUPT,
			    mailbox);
      mu_diag_output (MU_DIAG_EMERG, _("mailbox corrupted, shrank in size"));
      return 0;
    }
  return (dmp->size == size);
}

#ifdef WITH_PTHREAD
void
dotmail_cleanup (void *arg)
{
  mu_mailbox_t mailbox = arg;
  mu_monitor_unlock (mailbox->monitor);
  mu_locker_unlock (mailbox->locker);
}
#endif

static int
dotmail_alloc_message (struct mu_dotmail_mailbox *dmp,
		       struct mu_dotmail_message **dmsg_ptr)
{
  struct mu_dotmail_message *dmsg;

  if (dmp->mesg_count == dmp->mesg_max)
    {
      size_t n = dmp->mesg_max;
      void *p;

      if (n == 0)
	n = 64;
      else
	{
	  if ((size_t) -1 / 3 * 2 / sizeof (dmp->mesg[0]) <= n)
	    return ENOMEM;
	  n += (n + 1) / 2;
	}
      p = realloc (dmp->mesg, n * sizeof (dmp->mesg[0]));
      if (!p)
	return ENOMEM;
      dmp->mesg = p;
      dmp->mesg_max = n;
    }
  dmsg = calloc (1, sizeof (*dmsg));
  if (!dmsg)
    return ENOMEM;
  dmsg->mbox = dmp;
  dmsg->num = dmp->mesg_count;
  dmp->mesg[dmp->mesg_count++] = dmsg;
  *dmsg_ptr = dmsg;
  return 0;
}

static int
dotmail_dispatch (mu_mailbox_t mailbox, int evt, void *data)
{
  if (!mailbox->observable)
    return 0;

  mu_monitor_unlock (mailbox->monitor);
  if (mu_observable_notify (mailbox->observable, evt, data))
    {
      if (mailbox->locker)
	mu_locker_unlock (mailbox->locker);
      return EINTR;
    }
  mu_monitor_wrlock (mailbox->monitor);
  return 0;
}

/* Notes on the UID subsystem

   1. The values of uidvalidity and uidnext are stored in the
      X-IMAPbase header in the first message.
   2. Message UID is stored in the X-UID header in that message.
   3. To minimize unwanted modifications to the mailbox, the
      UID subsystem is initialized only in the following cases:

      3a. Upon mailbox scanning, if the first message contains a
	  valid X-IMAPbase header. In this case, the
	  dotmail_rescan_unlocked function initializes each
	  message's uid value from the X-UID header. The first
	  message that lacks X-UID or with an X-UID that cannot
	  be parsed, gets assigned new UID. The subsequent
	  messages are assigned new UIDs no matter whether they
	  have X-UID headers. In this case, the uidvalidity value
	  is reset to the current timestamp, to indicate that all
	  UIDs might have changed.

      3b. When any of the following functions are called for
	  the first time: dotmail_uidvalidity, dotmail_uidnext,
	  dotmail_message_uid. This means that the caller used
	  mu_mailbox_uidvalidity, mu_mailbox_uidnext, or
	  mu_message_get_uid.
	  In this case, each message is assigned a UID equal to
	  its ordinal number (1-based) in the mailbox.
	  This is done by the mu_dotmail_mailbox_uid_setup function.

   4. When a message is appended to the mailbox, any existing
      X-IMAPbase and X-UID headers are removed from it. If the
      UID subsystem is initialized, the message is assigned a new
      UID.
   5. Assigning new UID to a message does not change its attributes.
      Instead, its uid_modified flag is set.
*/

/* Allocate next available UID for the mailbox.
   The caller must ensure that the UID subsystem is initialized.
*/
static unsigned long
dotmail_alloc_next_uid (struct mu_dotmail_mailbox *mbox)
{
  mbox->uidvalidity_changed = 1;
  return mbox->uidnext++;
}

static void
dotmail_message_alloc_uid (struct mu_dotmail_message *dmsg)
{
  free (dmsg->hdr[mu_dotmail_hdr_x_uid]);
  dmsg->hdr[mu_dotmail_hdr_x_uid] = NULL;
  dmsg->uid = dotmail_alloc_next_uid (dmsg->mbox);
  dmsg->uid_modified = 1;
}

/* Width of the decimal representation of the maximum value of the unsigned
 * type t.  146/485 is the closest approximation of log10(2):
 *
 *  log10(2) = .301030
 *  146/485  = .301031
*/
#define UINT_STRWIDTH(t) ((int)((sizeof(t) * 8 * 146 + 484) / 485))

/*
 * The format for the X-IMAPbase header is:
 *
 *    X-IMAPbase: <V> <N>
 *
 * where <V> and <N> are current values of the uidvalidity and uidnext
 * parameters, correspondingly.
 *
 * The header is stored in the first message.  To avoid rewriting entire
 * mailbox when one of the parameters changes, the values of <V> and <N>
 * are left-padded with spaces to the maximum width of their data types.
 *
 * Offset of the header in the mailbox and its length (without the
 * trailing newline) are stored in x_imapbase_off and x_imapbase_len
 * members of struct mu_dotmail_mailbox.
 *
 * The X_IMAPBASE_MAX macro returns maximum size of the buffer necessary
 * for formatting the X-IMAPbase header.  In fact, it is 2 bytes wider
 * than necessary (due to the two '0' in the sample string below).
 */
#define X_IMAPBASE_MAX(d)		   \
  (sizeof (MU_HEADER_X_IMAPBASE ": 0 0") + \
   UINT_STRWIDTH ((d)->uidvalidity) +	   \
   UINT_STRWIDTH ((d)->uidnext))

static int
dotmail_rescan_unlocked (mu_mailbox_t mailbox, mu_off_t offset)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  mu_stream_t stream;
  char cur;
  size_t n;
  enum dotmail_scan_state
  {
    dotmail_scan_init,
    dotmail_scan_header,
    dotmail_scan_header_newline,
    dotmail_scan_header_expect,
    dotmail_scan_body,
    dotmail_scan_body_newline,
    dotmail_scan_dot
  } state = dotmail_scan_init;
  struct mu_dotmail_message *dmsg;
  size_t lines = 0;
  int rc;
  static char *expect[] = {
    "status:    ",
    "x-imapbase:",
    "x-uid:     ",
  };
  int i, j;
  int force_init_uids = 0;

  if (!(dmp->stream_flags & MU_STREAM_READ))
    return 0;

  rc = mu_streamref_create (&stream, mailbox->stream);
  if (rc)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("%s:%s (%s): %s",
		 __func__, "mu_streamref_create", dmp->name,
		 mu_strerror (rc)));
      return rc;
    }

  rc = mu_stream_seek (stream, offset, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("%s:%s (%s): %s",
		 __func__, "mu_stream_seek", dmp->name,
		 mu_strerror (rc)));
      return rc;
    }

  while ((rc = mu_stream_read (stream, &cur, 1, &n)) == 0)
    {
      if (n == 0)
	{
	  if (state != dotmail_scan_init && state != dotmail_scan_dot)
	    {
	      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			("%s (%s): message %lu ended prematurely",
			 __func__, dmp->name,
			 (unsigned long) dmp->mesg_count));
	      --dmp->mesg_count;
	      // FIXME: status = MU_ERR_PARSE;
	    }
	  break;
	}

      if (cur == '\n')
	{
	  /* Ping them every 1000 lines. Should be tunable.  */
	  if (((lines + 1) % 1000) == 0)
	    dotmail_dispatch (mailbox, MU_EVT_MAILBOX_PROGRESS, NULL);
	}

      switch (state)
	{
	case dotmail_scan_init:
	  /* Start new message */
	  rc = dotmail_alloc_message (dmp, &dmsg);
	  if (rc)
	    {
	      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			("%s:%s (%s): %s",
			 __func__, "dotmail_alloc_message", dmp->name,
			 mu_strerror (rc)));
	      return rc;
	    }
	  rc = mu_stream_seek (stream, 0, MU_SEEK_CUR, &dmsg->message_start);
	  if (rc)
	    {
	      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			("%s:%s (%s): %s",
			 __func__, "mu_stream_seek", dmp->name,
			 mu_strerror (rc)));
	      return rc;
	    }
	  --dmsg->message_start;
	  state = dotmail_scan_header_newline;
	  i = j = 0;
	  break;

	case dotmail_scan_header:
	  if (cur == '\n')
	    {
	      state = dotmail_scan_header_newline;
	    }
	  break;

	case dotmail_scan_header_newline:
	  if (cur == '\n')
	    {
	      rc = mu_stream_seek (stream, 0, MU_SEEK_CUR, &dmsg->body_start);
	      if (rc)
		{
		  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			    ("%s:%s (%s): %s",
			     __func__, "mu_stream_seek", dmp->name,
			     mu_strerror (rc)));
		  return rc;
		}
	      state = dotmail_scan_body_newline;
	    }
	  else
	    {
	      state = dotmail_scan_header;
	      j = 0;
	      for (i = 0; i < MU_DOTMAIL_HDR_MAX; i++)
		{
		  if (expect[i][j] == mu_tolower (cur))
		    {
		      j++;
		      state = dotmail_scan_header_expect;
		      break;
		    }
		}
	    }
	  break;

	case dotmail_scan_header_expect:
	  if (cur == '\n')
	    {
	      state = dotmail_scan_header_newline;
	    }
	  else
	    {
	      int c = mu_tolower (cur);
	      if (expect[i][j] != c)
		{
		  if (++i == MU_DOTMAIL_HDR_MAX
		      || memcmp (expect[i-1], expect[i], j)
		      || expect[i][j] != c)
		    {
		      state = dotmail_scan_header;
		      break;
		    }
		}

	      if (c == ':')
		{
		  char *buf = NULL;
		  size_t size = 0;
		  size_t n;

		  rc = mu_stream_getline (stream, &buf, &size, &n);
		  if (rc)
		    {
		      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
				("%s:%s (%s): %s",
				 __func__, "mu_stream_getline",
				 dmsg->mbox->name,
				 mu_strerror (rc)));
		      return rc;
		    }
		  if (n > 0)
		    {
		      buf[n-1] = 0;
		      dmsg->hdr[i] = buf;
		    }
		  else
		    free (buf);

		  if (i == mu_dotmail_hdr_x_imapbase)
		    {
		      mu_off_t off;
		      rc = mu_stream_seek (stream, 0, MU_SEEK_CUR, &off);
		      if (rc)
			{
			  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
				    ("%s:%s (%s): %s",
				     __func__, "mu_stream_seek", dmp->name,
				     mu_strerror (rc)));
			  return rc;
			}
		      dmp->x_imapbase_len = j + n;
		      dmp->x_imapbase_off = off - dmp->x_imapbase_len - 1;
		    }
		  
		  state = dotmail_scan_header_newline;
		}
	      else
		{
		  j++;
		  if (expect[i][j] == 0)
		    state = dotmail_scan_header_newline;
		}
	    }
	  break;

	case dotmail_scan_body:
	  dmsg->body_size++;
	  if (cur == '\n')
	    {
	      dmsg->body_lines++;
	      state = dotmail_scan_body_newline;
	    }
	  break;

	case dotmail_scan_body_newline:
	  dmsg->body_size++;
	  if (cur == '.')
	    state = dotmail_scan_dot;
	  else if (cur == '\n')
	    /* keep state */;
	  else
	    state = dotmail_scan_body;
	  break;

	case dotmail_scan_dot:
	  if (cur == '\n')
	    {
	      size_t count;

	      dmsg->body_lines_scanned = 1;

	      rc = mu_stream_seek (stream, 0, MU_SEEK_CUR, &dmsg->message_end);
	      if (rc)
		{
		  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			    ("%s:%s (%s): %s",
			     __func__, "mu_stream_seek", dmp->name,
			     mu_strerror (rc)));
		  return rc;
		}
	      dmsg->message_end -= 2;
	      dmsg->body_size--;

	      if (dmsg->num == 0)
		{
		  if (dmsg->hdr[mu_dotmail_hdr_x_imapbase]
		      && sscanf (dmsg->hdr[mu_dotmail_hdr_x_imapbase],
				 "%lu %lu",
				 &dmp->uidvalidity, &dmp->uidnext) == 2)
		    dmp->uidvalidity_scanned = 1;
		}

	      if (dmp->uidvalidity_scanned)
		{
		  if (!(!force_init_uids
			&& dmsg->hdr[mu_dotmail_hdr_x_uid]
			&& sscanf (dmsg->hdr[mu_dotmail_hdr_x_uid],
				   "%lu", &dmsg->uid) == 1
			&& dmsg->uid < dmp->uidnext
			&& (dmsg->num == 0
			    || dmsg->uid > dmp->mesg[dmsg->num - 1]->uid)))
		    {
		      force_init_uids = 1;
		      dmp->uidvalidity = (unsigned long) time (NULL);
		      dmp->uidvalidity_changed = 1;
		    }
		  
		  if (force_init_uids)
		    dotmail_message_alloc_uid (dmsg);
		}

	      /* Every 100 mesgs update the lock, it should be every minute.  */
	      if (mailbox->locker && (dmp->mesg_count % 100) == 0)
		mu_locker_touchlock (mailbox->locker);

	      count = dmp->mesg_count;
	      dotmail_dispatch (mailbox, MU_EVT_MESSAGE_ADD, &count);

	      state = dotmail_scan_init;
	    }
	  else
	    {
	      if (cur == '.')
		dmsg->body_dot_stuffed = 1;
	      else
		dmsg->body_size++;
	      state = dotmail_scan_body;
	    }
	  break;
	}
    }

  if (rc)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("%s:%s (%s): %s",
		 __func__, "mu_stream_read", dmp->name,
		 mu_strerror (rc)));
    }
  mu_stream_unref (stream);

  return rc;
}

/* Scan the mailbox starting from the given offset */
static int
dotmail_rescan (mu_mailbox_t mailbox, mu_off_t offset)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  int rc;

  if (!dmp)
    return EINVAL;

  if (!(dmp->stream_flags & MU_STREAM_READ))
    return 0;
      
  mu_monitor_wrlock (mailbox->monitor);
#ifdef WITH_PTHREAD
  pthread_cleanup_push (dotmail_cleanup, (void *)mailbox);
#endif

  rc = mu_stream_size (mailbox->stream, &dmp->size);
  if (rc != 0)
    {
      mu_monitor_unlock (mailbox->monitor);
      return rc;
    }

  if (mailbox->locker && (rc = mu_locker_lock (mailbox->locker)))
    {
      mu_monitor_unlock (mailbox->monitor);
      return rc;
    }

  rc = dotmail_rescan_unlocked (mailbox, offset);

  if (mailbox->locker)
    mu_locker_unlock (mailbox->locker);
  mu_monitor_unlock (mailbox->monitor);

#ifdef WITH_PTHREAD
  pthread_cleanup_pop (0);
#endif

  return rc;
}

static int
dotmail_refresh (mu_mailbox_t mailbox)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;

  if (dotmail_is_updated (mailbox))
    return 0;
  return dotmail_rescan (mailbox,
			 dmp->mesg_count == 0
			   ? 0
			   : dmp->mesg[dmp->mesg_count - 1]->message_end + 2);
}

static int
dotmail_scan (mu_mailbox_t mailbox, size_t i, size_t *pcount)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;

  if (!dmp)
    return EINVAL;

  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_TRACE1,
	    ("%s (%s)", __func__, dmp->name));

  if (i == 0 || (dmp->mesg_count && i > dmp->mesg_count))
    return EINVAL;

  if (!dotmail_is_updated (mailbox))
    {
      int rc;

      while (i < dmp->mesg_count)
	mu_dotmail_message_free (dmp->mesg[dmp->mesg_count--]);

      rc = dotmail_refresh (mailbox);
      if (rc)
	return rc;
    }
  else if (mailbox->observable)
    {
      for (; i <= dmp->mesg_count; i++)
	{
	  size_t tmp = i;
	  if (mu_observable_notify (mailbox->observable, MU_EVT_MESSAGE_ADD,
				    &tmp) != 0)
	    break;
	  /* FIXME: Hardcoded value! Must be configurable */
	  if (((i + 1) % 50) == 0)
	    mu_observable_notify (mailbox->observable, MU_EVT_MAILBOX_PROGRESS,
				  NULL);
	}
    }
  if (pcount)
    *pcount = dmp->mesg_count;
  return 0;
}

static int
dotmail_messages_recent (mu_mailbox_t mailbox, size_t *pcount)
{
  size_t i;
  size_t count = 0;
  struct mu_dotmail_mailbox *dmp = mailbox->data;

  int rc = dotmail_refresh (mailbox);
  if (rc)
    return rc;

  for (i = 0; i < dmp->mesg_count; i++)
    {
      mu_dotmail_message_attr_load (dmp->mesg[i]);
      if (MU_ATTRIBUTE_IS_UNSEEN (dmp->mesg[i]->attr_flags))
	++count;
    }

  *pcount = count;

  return 0;
}

static int
dotmail_message_unseen (mu_mailbox_t mailbox, size_t *pmsgno)
{
  size_t i;
  struct mu_dotmail_mailbox *dmp = mailbox->data;

  int rc = dotmail_refresh (mailbox);
  if (rc)
    return rc;

  for (i = 0; i < dmp->mesg_count; i++)
    {
      mu_dotmail_message_attr_load (dmp->mesg[i]);
      if (MU_ATTRIBUTE_IS_UNREAD (dmp->mesg[i]->attr_flags))
	{
	  *pmsgno = i + 1;
	  return 0;
	}
    }

  *pmsgno = 0;
  return 0;
}

/* Initialize the mailbox UID subsystem. See the Notes above. */
int
mu_dotmail_mailbox_uid_setup (struct mu_dotmail_mailbox *dmp)
{
  if (!dmp->uidvalidity_scanned)
    {
      size_t i;
      int rc = dotmail_refresh (dmp->mailbox);
      if (rc || dmp->uidvalidity_scanned)
	return rc;

      dmp->uidvalidity = (unsigned long)time (NULL);
      dmp->uidnext = 1;
      dmp->uidvalidity_scanned = 1;
      dmp->uidvalidity_changed = 1;
      
      for (i = 0; i < dmp->mesg_count; i++)
	dotmail_message_alloc_uid (dmp->mesg[i]);
    }
  return 0;
}

static int
dotmail_get_uidvalidity (mu_mailbox_t mailbox, unsigned long *puidvalidity)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  int rc = mu_dotmail_mailbox_uid_setup (dmp);
  if (rc == 0)
    *puidvalidity = dmp->uidvalidity;
  return rc;
}

static int
dotmail_set_uidvalidity (mu_mailbox_t mailbox, unsigned long uidvalidity)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  int rc = mu_dotmail_mailbox_uid_setup (dmp);
  if (rc == 0)
    dmp->uidvalidity = uidvalidity;
  return rc;
}

static int
dotmail_uidnext (mu_mailbox_t mailbox, size_t *puidnext)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  int rc = mu_dotmail_mailbox_uid_setup (dmp);
  if (rc == 0)
    *puidnext = dmp->uidnext;
  return rc;
}

static int
dotmail_get_message (mu_mailbox_t mailbox, size_t msgno, mu_message_t *pmsg)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  int rc;

  if (!dmp || msgno < 1)
    return EINVAL;
  if (pmsg == NULL)
    return MU_ERR_OUT_PTR_NULL;

  if (dmp->mesg_count == 0)
    {
      rc = dotmail_scan (mailbox, 1, NULL);
      if (rc)
	return rc;
    }

  if (msgno > dmp->mesg_count)
    return MU_ERR_NOENT;

  return mu_dotmail_message_get (dmp->mesg[msgno-1], pmsg);
}

static int
qid2off (mu_message_qid_t qid, mu_off_t *pret)
{
  mu_off_t ret = 0;
  for (;*qid; qid++)
    {
      if (!('0' <= *qid && *qid <= '9'))
	return 1;
      ret = ret * 10 + *qid - '0';
    }
  *pret = ret;
  return 0;
}

static int
dotmail_quick_get_message (mu_mailbox_t mailbox, mu_message_qid_t qid,
			   mu_message_t *pmsg)
{
  int rc;
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  struct mu_dotmail_message *dmsg;
  mu_off_t offset;

  if (mailbox == NULL || qid2off (qid, &offset)
      || !(mailbox->flags & MU_STREAM_QACCESS))
    return EINVAL;

  if (dmp->mesg_count == 0)
    {
      rc = dotmail_rescan (mailbox, offset);
      if (rc)
	return rc;
      if (dmp->mesg_count == 0)
	return MU_ERR_NOENT;
    }

  dmsg = dmp->mesg[0];
  if (dmsg->message_start != offset)
    return MU_ERR_EXISTS;
  if (dmsg->message)
    {
      if (pmsg)
	*pmsg = dmsg->message;
      return 0;
    }
  return mu_dotmail_message_get (dmsg, pmsg);
}

static int
mailbox_append_message (mu_mailbox_t mailbox, mu_message_t msg,
			mu_envelope_t env, mu_attribute_t atr)
{
  int rc;
  mu_off_t size;
  mu_stream_t istr, flt;
  static char *exclude_headers[] = {
    MU_HEADER_X_IMAPBASE,
    MU_HEADER_X_UID,
    MU_HEADER_STATUS,
    NULL,
    NULL
  };
  struct mu_dotmail_mailbox *dmp = mailbox->data;

  rc = mu_stream_seek (mailbox->stream, 0, MU_SEEK_END, &size);
  if (rc)
    return rc;

  rc = mu_message_get_streamref (msg, &istr);
  if (rc)
    return rc;

  do
    {
      char statbuf[MU_STATUS_BUF_SIZE];
      
      if (atr)
	{
	  rc = mu_attribute_to_string (atr, statbuf, MU_STATUS_BUF_SIZE, NULL);
	  if (rc)
	    break;
	}      
      else
	statbuf[0] = 0;
      
      if (env)
	{
	  char const *s;

	  if (mu_envelope_sget_sender (env, &s) == 0)
	    exclude_headers[3] = MU_HEADER_RETURN_PATH;
	  if (mu_envelope_sget_date (env, &s) == 0)
	    {
	      struct tm tm;
	      struct mu_timezone tz;
	      
	      if (mu_parse_date_dtl (s, NULL, NULL, &tm, &tz, NULL) == 0)
		{
		  /* Format a "Received:" header with that date */
		  mu_stream_printf (mailbox->stream,
				    "Received: from %s\n"
				    "\tby %s; ",
				    "localhost", "localhost");
		  mu_c_streamftime (mailbox->stream,
				    MU_DATETIME_FORM_RFC822, &tm, &tz);
		  mu_stream_write (mailbox->stream, "\n", 1, NULL);
		}
	    }
	}
      
      rc = mu_stream_header_copy (mailbox->stream, istr, exclude_headers);
      if (rc)
	break;

      if (env)
	{
	  char const *s;

	  if (mu_envelope_sget_sender (env, &s) == 0)
	    {
	      /* Create a Return-Path header */
	      mu_stream_printf (mailbox->stream, "%s: %s\n",
				MU_HEADER_RETURN_PATH, s);
	    }
	}
      
      /* Write status header */
      if (statbuf[0])
	mu_stream_printf (mailbox->stream,
			  "%s: %s\n", MU_HEADER_STATUS, statbuf);      
      
      /* Write UID-related data */
      if (dmp->uidvalidity_scanned)
	{
	  if (dmp->mesg_count == 0)
	    mu_stream_printf (mailbox->stream, "%s: %*lu %*lu\n",
			      MU_HEADER_X_IMAPBASE,
			      UINT_STRWIDTH (dmp->uidvalidity),
			      dmp->uidvalidity,
			      UINT_STRWIDTH (dmp->uidnext),
			      dmp->uidnext);
	  mu_stream_printf (mailbox->stream, "%s: %lu\n",
			    MU_HEADER_X_UID,
			    dotmail_alloc_next_uid (dmp));
	}

      rc = mu_stream_write (mailbox->stream, "\n", 1, NULL);
      if (rc)
	break;

      rc = mu_filter_create (&flt, istr, "DOT",
			     MU_FILTER_ENCODE, MU_STREAM_READ);
      mu_stream_destroy (&istr);
      rc = mu_stream_copy (mailbox->stream, flt, 0, NULL);
      mu_stream_unref (flt);
    }
  while (0);

  if (rc)
    {
      mu_stream_destroy (&istr);
      rc = mu_stream_truncate (mailbox->stream, size);
      if (rc)
	mu_error (_("cannot truncate stream after failed append: %s"),
		  mu_stream_strerror (mailbox->stream, rc));
      return rc;
    }

  /* Rescan the message */
  rc = dotmail_rescan_unlocked (mailbox, size);
  if (rc)
    return rc;

  if (mailbox->observable)
    {
      char *buf = NULL;
      mu_asprintf (&buf, "%lu", (unsigned long) size);
      mu_observable_notify (mailbox->observable,
			    MU_EVT_MAILBOX_MESSAGE_APPEND, buf);
      free (buf);
    }

  return 0;
}

static int
dotmail_append_message (mu_mailbox_t mailbox, mu_message_t msg,
			mu_envelope_t env, mu_attribute_t atr)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  int rc;

  rc = dotmail_refresh (mailbox);
  if (rc)
    return rc;
  
  mu_monitor_wrlock (mailbox->monitor);
  if (mailbox->locker && (rc = mu_locker_lock (mailbox->locker)) != 0)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("%s(%s):%s: %s",
		 __func__, dmp->name, "mu_locker_lock",
		 mu_strerror (rc)));
    }
  else
    {
      rc = mailbox_append_message (mailbox, msg, env, atr);

      if (mailbox->locker)
	mu_locker_unlock (mailbox->locker);
    }
  mu_monitor_unlock (mailbox->monitor);
  return rc;
}

static int
dotmail_messages_count (mu_mailbox_t mailbox, size_t *pcount)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  int rc;

  if (!dmp)
    return EINVAL;

  rc = dotmail_refresh (mailbox);
  if (rc)
    return rc;

  if (pcount)
    *pcount = dmp->mesg_count;

  return 0;
}

static int
dotmail_get_size (mu_mailbox_t mailbox, mu_off_t *psize)
{
  mu_off_t size;
  int rc;

  rc  = mu_stream_size (mailbox->stream, &size);
  if (rc != 0)
    return rc;
  if (psize)
    *psize = size;
  return 0;
}

static int
dotmail_stat (mu_mailbox_t mailbox, struct stat *st)
{
  int rc;
  mu_transport_t trans[2];
  
  rc = mu_stream_ioctl (mailbox->stream, MU_IOCTL_TRANSPORT,
			MU_IOCTL_OP_GET, trans);
  if (rc == 0)
    {
      if (fstat ((int) (intptr_t) trans[0], st))
	rc = errno;
    }
  return rc;
}

static int
dotmail_set_priv (struct mu_dotmail_mailbox *dmp, struct stat *st)
{
  int rc;
  mu_transport_t trans[2];
  
  rc = mu_stream_ioctl (dmp->mailbox->stream, MU_IOCTL_TRANSPORT,
			MU_IOCTL_OP_GET, trans);
  if (rc == 0)
    {
      int fd = (intptr_t) trans[0];
      if (fchmod (fd, st->st_mode))
	{
	  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		    ("%s:%s: chmod failed: %s",
		     __func__, dmp->name, strerror (errno)));
	  rc = errno;
	}
      else if (fchown (fd, st->st_uid, st->st_gid))
	{
	  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		    ("%s:%s: chown failed: %s",
		     __func__, dmp->name, strerror (errno)));
	  rc = errno;
	}
    }
  return rc;
}

static int
dotmail_get_atime (mu_mailbox_t mailbox, time_t *return_time)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  struct stat st;
  int rc;
  
  if (dmp == NULL)
    return EINVAL;
  if ((rc = dotmail_stat (mailbox, &st)) == 0)
    *return_time = st.st_atime;
  return rc;
}

struct mu_dotmail_flush_tracker
{
  struct mu_dotmail_mailbox *dmp;
  size_t *ref;
  size_t mesg_count;
};

static int
tracker_init (struct mu_dotmail_flush_tracker *trk,
	      struct mu_dotmail_mailbox *dmp)
{
  trk->ref = calloc (dmp->mesg_count, sizeof (trk->ref[0]));
  if (!trk->ref)
    return ENOMEM;
  trk->dmp = dmp;
  trk->mesg_count = 0;
  return 0;
}

static void
tracker_free (struct mu_dotmail_flush_tracker *trk)
{
  free (trk->ref);
}

static struct mu_dotmail_message *
tracker_next_ref (struct mu_dotmail_flush_tracker *trk, size_t orig_num)
{
  trk->ref[trk->mesg_count++] = orig_num;
  return trk->dmp->mesg[orig_num];
}

static void
dotmail_tracker_sync (struct mu_dotmail_flush_tracker *trk)
{
  struct mu_dotmail_mailbox *dmp = trk->dmp;
  size_t i;

  if (trk->mesg_count == 0)
    {
      for (i = 0; i < dmp->mesg_count; i++)
	mu_dotmail_message_free (dmp->mesg[i]);
      dmp->size = 0;
      dmp->uidvalidity_scanned = 0;
    }
  else
    {
      /* Mark */
      for (i = 0; i < trk->mesg_count; i++)
	dmp->mesg[trk->ref[i]]->mark = 1;
      /* Sweep */
      for (i = 0; i < dmp->mesg_count; i++)
	if (!dmp->mesg[i]->mark)
	  mu_dotmail_message_free (dmp->mesg[i]);
      /* Reorder */
      for (i = 0; i < trk->mesg_count; i++)
	{
	  dmp->mesg[i] = dmp->mesg[trk->ref[i]];
	  dmp->mesg[i]->mark = 0;
	}
      dmp->mesg_count = trk->mesg_count;
      dmp->size = dmp->mesg[dmp->mesg_count - 1]->message_end + 2;
    }
  dmp->mesg_count = trk->mesg_count;
}

/* Write to the output stream DEST messages in the range [from,to).
   Update TRK accordingly.
*/
static int
dotmail_mailbox_copy_unchanged (struct mu_dotmail_flush_tracker *trk,
				size_t from, size_t to,
				mu_stream_t dest)
{
  if (to > from)
    {
      size_t i;
      mu_off_t start, stop, off;
      int rc;
      struct mu_dotmail_mailbox *dmp = trk->dmp;

      start = dmp->mesg[from]->message_start;
      if (to == dmp->mesg_count)
	stop = dmp->mesg[to-1]->message_end + 2;
      else
	stop = dmp->mesg[to]->message_start;

      rc = mu_stream_seek (dest, 0, MU_SEEK_CUR, &off);
      if (rc)
	return rc;
      off -= start;
      /* Fixup offsets */
      for (i = from; i < to; i++)
	{
	  struct mu_dotmail_message *ref = tracker_next_ref (trk, i);
	  ref->message_start += off;
	  ref->body_start += off;
	  ref->message_end += off;
	  // FIXME: Used to clear body_lines_scanned here.
	  // Not sure it is needed.
	}

      /* Copy data */
      rc = mu_stream_seek (dmp->mailbox->stream, start, MU_SEEK_SET, NULL);
      if (rc)
	return rc;
      return mu_stream_copy (dest, dmp->mailbox->stream, stop - start, NULL);
    }
  return 0;
}

/* Flush the mailbox described by the tracker TRK to the stream TEMPSTR.
   First modified message is I (0-based). EXPUNGE is 1 if the
   MU_ATTRIBUTE_DELETED attribute is to be honored.
*/
static int
dotmail_flush_temp (struct mu_dotmail_flush_tracker *trk,
		    size_t i,
		    mu_stream_t tempstr, int expunge)
{
  struct mu_dotmail_mailbox *dmp = trk->dmp;
  size_t start = 0;
  size_t save_imapbase = 0;
  size_t expcount = 0;
  int rc;

  rc = mu_stream_seek (trk->dmp->mailbox->stream, 0, MU_SEEK_SET, NULL);
  if (rc)
    return rc;
  while (i < dmp->mesg_count)
    {
      struct mu_dotmail_message *dmsg = dmp->mesg[i];

      if (expunge && (dmsg->attr_flags & MU_ATTRIBUTE_DELETED))
	{
	  size_t expevt[2] = { i + 1, expcount };

	  rc = dotmail_mailbox_copy_unchanged (trk, start, i, tempstr);
	  if (rc)
	    return rc;
	  mu_observable_notify (dmp->mailbox->observable,
				MU_EVT_MAILBOX_MESSAGE_EXPUNGE,
				expevt);
	  expcount++;
	  mu_message_destroy (&dmsg->message, dmsg);

	  /* Make sure uidvalidity and next uid are preserved even if
	     the first message (where they are saved) is deleted */
	  if (i == save_imapbase)
	    {
	      save_imapbase = i + 1;
	      if (save_imapbase < dmp->mesg_count)
		dmp->mesg[save_imapbase]->attr_flags |= MU_ATTRIBUTE_MODIFIED;
	    }
	  i++;
	  start = i;
	  continue;
	}

      if (dmsg->uid_modified
	  || (dmsg->attr_flags & MU_ATTRIBUTE_MODIFIED)
	  || mu_message_is_modified (dmsg->message))
	{
	  rc = dotmail_mailbox_copy_unchanged (trk, start, i, tempstr);
	  if (rc)
	    return rc;

	  free (dmsg->hdr[mu_dotmail_hdr_x_imapbase]);
	  dmsg->hdr[mu_dotmail_hdr_x_imapbase] = NULL;
	  if (save_imapbase == i)
	    {
	      mu_asprintf (&dmsg->hdr[mu_dotmail_hdr_x_imapbase], "%*lu %*lu",
			   UINT_STRWIDTH (dmp->uidvalidity),
			   dmp->uidvalidity,
			   UINT_STRWIDTH (dmp->uidnext),
			   dmp->uidnext);
	    }

	  rc = mu_dotmail_message_reconstruct (tempstr, dmsg,
					       tracker_next_ref (trk, i));
	  if (rc)
	    return rc;
	  i++;
	  start = i;
	  continue;
	}

      i++;
    }
  rc = dotmail_mailbox_copy_unchanged (trk, start, i, tempstr);
  if (rc)
    return rc;
  return mu_stream_flush (tempstr);
}

/*
 * Copy the temporary mailbox stream TEMPSTR to the mailbox referred to by
 * the tracker TRK.
 */
static inline int
dotmail_copyback (struct mu_dotmail_flush_tracker *trk, mu_stream_t tempstr)
{
  int rc;
  mu_stream_t mbx_stream = trk->dmp->mailbox->stream;
  mu_off_t size;
  
  rc = mu_stream_seek (tempstr, 0, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("%s: can't rewind temporary file: %s",
		 __func__, mu_strerror (rc)));
      return rc;
    }

  rc = mu_stream_seek (mbx_stream, 0, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("%s: can't rewind mailbox %s: %s",
		 __func__, trk->dmp->name, mu_strerror (rc)));
      return rc;
    }

  rc = mu_stream_copy (mbx_stream, tempstr, 0, &size);
  if (rc)
    {
      mu_error (_("copying back to mailbox %s failed: %s"),
		trk->dmp->name, mu_strerror (rc));
      return rc;
    }
  rc = mu_stream_truncate (mbx_stream, size);
  if (rc)
    {
      mu_error (_("cannot truncate mailbox stream: %s"),
		mu_stream_strerror (mbx_stream, rc));
      return rc;
    }
  
  dotmail_tracker_sync (trk);
  return 0;
}

/* Flush the mailbox described by the tracker TRK to the stream TEMPSTR.
   EXPUNGE is 1 if the MU_ATTRIBUTE_DELETED attribute is to be honored.
   Assumes that simultaneous access to the mailbox has been blocked.
*/
static int
dotmail_flush_unlocked (struct mu_dotmail_flush_tracker *trk, int mode)
{
  struct mu_dotmail_mailbox *dmp = trk->dmp;
  int rc;
  size_t dirty;
  mu_stream_t tempstr;
  struct mu_tempfile_hints hints;
  int tempfd;
  char *tempname;
  char *p;

  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_TRACE1,
	    ("%s (%s)", __func__, dmp->name));
  if (dmp->mesg_count == 0)
    return 0;
  if (mode == FLUSH_UIDVALIDITY && !dmp->uidvalidity_changed)
    return 0;
  
  rc = dotmail_refresh (dmp->mailbox);
  if (rc)
    return rc;

  if (dmp->uidvalidity_changed)
    {
      size_t i;
      char buf[X_IMAPBASE_MAX (dmp)];
      int n;
      mu_stream_t stream = dmp->mailbox->stream;

      /*
       * Format the X-IMAPbase header and check if it will fit in place
       * of the existing one (if any).  If so, write it at once and return.
       */
      n = snprintf (buf, sizeof (buf), "%s: %*lu %*lu",
		    MU_HEADER_X_IMAPBASE,
		    UINT_STRWIDTH (dmp->uidvalidity),
		    dmp->uidvalidity,
		    UINT_STRWIDTH (dmp->uidnext),
		    dmp->uidnext);
      
      if (dmp->x_imapbase_len && dmp->x_imapbase_len >= n)
	{
	  rc = mu_stream_seek (stream, dmp->x_imapbase_off, MU_SEEK_SET, NULL);
	  if (rc)
	    {
	      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			("%s:%s (%s): %s",
			 __func__, "mu_stream_seek", dmp->name,
			 mu_strerror (rc)));
	      return rc;
	    }
	  rc = mu_stream_printf (stream, "%-*s",
				 (int) dmp->x_imapbase_len,
				 buf);
	  if (rc)
	    {
	      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			("%s:%s (%s): %s",
			 __func__, "mu_stream_printf", dmp->name,
			 mu_strerror (rc)));
	      
	    }
	  return 0;
	}
      else
	{
	  /*
	   * There is no X-IMAPbase header yet or it is not wide enough to
	   * accept the current value.  Fall back to reformatting entire
	   * mailbox.  Clear any other changes that might have been done
	   * to its messages.
	   */
	  dirty = 0;
	  dmp->mesg[0]->uid_modified = 1;
	  
	  for (i = 1; i < dmp->mesg_count; i++)
	    {
	      struct mu_dotmail_message *dmsg = dmp->mesg[i];
	      dmsg->uid_modified = 0;
	      dmsg->attr_flags &= ~(MU_ATTRIBUTE_MODIFIED|MU_ATTRIBUTE_DELETED);
	    }
	}
    }
  
  for (dirty = 0; dirty < dmp->mesg_count; dirty++)
    {
      struct mu_dotmail_message *dmsg = dmp->mesg[dirty];
      if (dmsg->uid_modified)
	break;
      mu_dotmail_message_attr_load (dmsg);
      if ((dmsg->attr_flags & MU_ATTRIBUTE_MODIFIED)
	  || (dmsg->attr_flags & MU_ATTRIBUTE_DELETED)
	  || (dmsg->message && mu_message_is_modified (dmsg->message)))
	break;
    }

  rc = 0;
  if (dirty < dmp->mesg_count)
    {  
      p = strrchr (dmp->name, '/');
      if (p)
	{
	  size_t l = p - dmp->name;
	  hints.tmpdir = malloc (l + 1);
	  if (!hints.tmpdir)
	    return ENOMEM;
	  memcpy (hints.tmpdir, dmp->name, l);
	  hints.tmpdir[l] = 0;
	}
      else
	{
	  hints.tmpdir = mu_getcwd ();
	  if (!hints.tmpdir)
	    return ENOMEM;
	}
      rc = mu_tempfile (&hints, MU_TEMPFILE_TMPDIR, &tempfd, &tempname);
      if (rc == 0)
	{
	  rc = mu_fd_stream_create (&tempstr, tempname, tempfd,
				    MU_STREAM_RDWR|MU_STREAM_SEEK);
	}
      else if (rc == EACCES)
	{
	  /*
	   * Mail spool directory is not writable for the user. Fall
	   * back to using temporary stream located elsewhere. When
	   * ready, it will be copied back to the mailbox.
	   *
	   * Reset the tempname to NULL to instruct the code below
	   * which approach to take.
	   */
	  tempname = NULL;
	  
	  rc = mu_temp_file_stream_create (&tempstr, NULL, 0);
	}
      
      if (rc)
	{
	  free (hints.tmpdir);
	  close (tempfd);
	  free (tempname);
	  return rc;
	}
      
      rc = dotmail_flush_temp (trk, dirty, tempstr, mode == FLUSH_EXPUNGE);
      if (rc == 0)
	{
	  if (tempname)
	    {
	      /* Mail spool is writable. Rename the temporary copy back
		 to mailbox */
	      char *backup;
	      struct stat st;
	      
	      if ((rc = dotmail_stat (dmp->mailbox, &st)) != 0)
		{
		  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			    ("%s:%s: stat failed: %s",
			     __func__, dmp->name, strerror (errno)));
		}
	      else
		{
		  mu_stream_flush (tempstr);
		  backup = mu_tempname (hints.tmpdir);
		  if (rename (dmp->name, backup))
		    {
		      rc = errno;
		      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
				("%s:%s: failed to rename to backup file %s: %s",
				 __func__, dmp->name, tempname,
				 mu_strerror (rc)));
		      unlink (backup);
		    }
		  else
		    {
		      rc = rename (tempname, dmp->name);
		      if (rc == 0)
			{
			  /* Success. Synchronize internal data with the
			     counter. */
			  dotmail_tracker_sync (trk);
			  mu_stream_destroy (&dmp->mailbox->stream);
			  rc = dotmail_mailbox_init_stream (dmp);
			  if (rc == 0)
			    dotmail_set_priv (dmp, &st);
			}
		      else
			{
			  int rc1;
			  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
				    ("%s: failed to rename temporary file %s %s: %s",
				     __func__, tempname, dmp->name,
				     mu_strerror (rc)));
			  rc1 = rename (backup, dmp->name);
			  if (rc1)
			    {
			      mu_error (_("failed to restore %s from backup %s: %s"),
					dmp->name, backup, mu_strerror (rc1));
			      mu_error (_("backup left in %s"), backup);
			      free (backup);
			      backup = NULL;
			    }
			}
		    }
		  
		  if (backup)
		    {
		      unlink (backup);
		      free (backup);
		    }
		  unlink (tempname);
		}
	    }
	  else
	    {
	      /* Mail spool not writable.  Copy the tempstr back to mailbox. */
	      rc = dotmail_copyback (trk, tempstr);
	    }	    
	}
      free (tempname);
      free (hints.tmpdir);
      mu_stream_unref (tempstr);
    }
  
  dmp->uidvalidity_changed = 0;  
  
  return rc;
}

/* Flush the changes in the mailbox DMP to disk storage.
   EXPUNGE is 1 if the MU_ATTRIBUTE_DELETED attribute is to be honored.
   Block simultaneous access for the duration of the process.

   This is done by creating a temporary mailbox on the same device as
   DMP and by transferring all messages (whether changed or not) to
   it. If the process succeeds, old mailbox is removed and the temporary
   one is renamed to it. In case of failure, the temporary is removed and
   the original mailbox remains unchanged.
*/
static int
dotmail_flush (struct mu_dotmail_mailbox *dmp, int mode)
{
  int rc;
  sigset_t signalset;
#ifdef WITH_PTHREAD
  int state;
#endif
  struct mu_dotmail_flush_tracker trk;

  /* Lock the mailbox */
  if (dmp->mailbox->locker
      && (rc = mu_locker_lock (dmp->mailbox->locker)) != 0)
    return rc;

#ifdef WITH_PTHREAD
  pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &state);
#endif
  sigemptyset (&signalset);
  sigaddset (&signalset, SIGTERM);
  sigaddset (&signalset, SIGHUP);
  sigaddset (&signalset, SIGTSTP);
  sigaddset (&signalset, SIGINT);
  sigaddset (&signalset, SIGWINCH);
  sigprocmask (SIG_BLOCK, &signalset, 0);

  rc = tracker_init (&trk, dmp);
  if (rc == 0)
    {
      rc = dotmail_flush_unlocked (&trk, mode);
      tracker_free (&trk);
    }

#ifdef WITH_PTHREAD
  pthread_setcancelstate (state, &state);
#endif
  sigprocmask (SIG_UNBLOCK, &signalset, 0);

  if (dmp->mailbox->locker)
    mu_locker_unlock (dmp->mailbox->locker);
  return rc;
}

static int
dotmail_expunge (mu_mailbox_t mailbox)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  return dotmail_flush (dmp, FLUSH_EXPUNGE);
}

static int
dotmail_sync (mu_mailbox_t mailbox)
{
  struct mu_dotmail_mailbox *dmp = mailbox->data;
  return dotmail_flush (dmp, FLUSH_SYNC);
}

int
mu_dotmail_mailbox_init (mu_mailbox_t mailbox)
{
  int status;
  struct mu_dotmail_mailbox *dmp;
  mu_property_t property = NULL;

  if (mailbox == NULL)
    return EINVAL;

  /* Allocate specific mbox data.  */
  dmp = calloc (1, sizeof (*dmp));
  if (dmp == NULL)
    return ENOMEM;

  /* Back pointer.  */
  dmp->mailbox = mailbox;

  status = mu_url_aget_path (mailbox->url, &dmp->name);
  if (status)
    {
      free (dmp);
      return status;
    }

  mailbox->data = dmp;

  /* Overloading the defaults.  */
  mailbox->_destroy = dotmail_destroy;
  mailbox->_open = dotmail_open;
  mailbox->_close = dotmail_close;
  mailbox->_remove = dotmail_remove;
  mailbox->_scan = dotmail_scan;
  mailbox->_is_updated = dotmail_is_updated;

  mailbox->_get_message = dotmail_get_message;
  mailbox->_quick_get_message = dotmail_quick_get_message;
  mailbox->_messages_count = dotmail_messages_count;
  mailbox->_messages_recent = dotmail_messages_recent;
  mailbox->_message_unseen = dotmail_message_unseen;

  mailbox->_append_message = dotmail_append_message;

  mailbox->_expunge = dotmail_expunge;
  mailbox->_sync = dotmail_sync;

  mailbox->_get_uidvalidity = dotmail_get_uidvalidity;
  mailbox->_set_uidvalidity = dotmail_set_uidvalidity;
  mailbox->_uidnext = dotmail_uidnext;
  mailbox->_get_size = dotmail_get_size;
  mailbox->_get_atime = dotmail_get_atime;

  mu_mailbox_get_property (mailbox, &property);
  mu_property_set_value (property, "TYPE", "DOTMAIL", 1);

  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_TRACE1,
	    ("%s (%s)", __func__, dmp->name));
  return 0;
}
