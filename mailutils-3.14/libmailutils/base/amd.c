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

/* Mailutils Abstract Mail Directory Layer 
   First draft by Sergey Poznyakoff.
   Thanks Tang Yong Ping <yongping.tang@radixs.com> for initial
   patch (although not used here).

   This module provides basic support for "MH" and "Maildir" formats. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#ifdef WITH_PTHREAD
# ifdef HAVE_PTHREAD_H
#  ifndef _XOPEN_SOURCE
#   define _XOPEN_SOURCE  500
#  endif
#  include <pthread.h>
# endif
#endif

#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/attribute.h>
#include <mailutils/body.h>
#include <mailutils/debug.h>
#include <mailutils/envelope.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/header.h>
#include <mailutils/locker.h>
#include <mailutils/message.h>
#include <mailutils/util.h>
#include <mailutils/datetime.h>
#include <mailutils/property.h>
#include <mailutils/stream.h>
#include <mailutils/url.h>
#include <mailutils/observer.h>
#include <mailutils/sys/stream.h>
#include <mailutils/sys/mailbox.h>
#include <mailutils/sys/message.h>
#include <mailutils/sys/registrar.h>
#include <mailutils/sys/url.h>
#include <mailutils/sys/amd.h>

static void amd_destroy (mu_mailbox_t mailbox);
static int amd_open (mu_mailbox_t, int);
static int amd_close (mu_mailbox_t);
static int amd_get_message (mu_mailbox_t, size_t, mu_message_t *);
static int amd_quick_get_message (mu_mailbox_t mailbox, mu_message_qid_t qid,
				  mu_message_t *pmsg);
static int amd_append_message (mu_mailbox_t, mu_message_t,
			       mu_envelope_t, mu_attribute_t);
static int amd_messages_count (mu_mailbox_t, size_t *);
static int amd_messages_recent (mu_mailbox_t, size_t *);
static int amd_message_unseen (mu_mailbox_t, size_t *);
static int amd_expunge (mu_mailbox_t);
static int amd_sync (mu_mailbox_t);
static int amd_uidnext (mu_mailbox_t mailbox, size_t *puidnext);
static int amd_get_uidvalidity (mu_mailbox_t, unsigned long *);
static int amd_set_uidvalidity (mu_mailbox_t, unsigned long);
static int amd_scan (mu_mailbox_t, size_t, size_t *);
static int amd_is_updated (mu_mailbox_t);
static int amd_get_size (mu_mailbox_t, mu_off_t *);

static int amd_body_size (mu_body_t body, size_t *psize);
static int amd_body_lines (mu_body_t body, size_t *plines);

static int amd_header_fill (void *data, char **pbuf, size_t *plen);

static int amd_get_attr_flags (mu_attribute_t attr, int *pflags);
static int amd_set_attr_flags (mu_attribute_t attr, int flags);
static int amd_unset_attr_flags (mu_attribute_t attr, int flags);

static int amd_pool_open (struct _amd_message *mhm);
static int amd_pool_open_count (struct _amd_data *amd);
static void amd_pool_flush (struct _amd_data *amd);
static struct _amd_message **amd_pool_lookup (struct _amd_message *mhm);

static int amd_remove_mbox (mu_mailbox_t mailbox);


static int amd_body_stream_read (mu_stream_t str, char *buffer,
				 size_t buflen,
				 size_t *pnread);
static int amd_body_stream_size (mu_stream_t str, mu_off_t *psize);
static int amd_body_stream_seek (mu_stream_t str, mu_off_t off, 
				 mu_off_t *presult);

struct _amd_body_stream
{
  struct _mu_stream stream;
  mu_body_t body;
  mu_off_t off;
};

/* AMD Properties */
int
_amd_prop_fetch_off (struct _amd_data *amd, const char *name, mu_off_t *pval)
{
  const char *p;
  mu_off_t n = 0;
  
  if (!amd->prop || mu_property_sget_value (amd->prop, name, &p))
    return MU_ERR_NOENT;
  if (!pval)
    return 0;
  for (; *p; p++)
    {
      if (!mu_isdigit (*p))
	return EINVAL;
      n = n * 10 + *p - '0';
    }
  *pval = n;
  return 0;
}

int
_amd_prop_fetch_size (struct _amd_data *amd, const char *name, size_t *pval)
{
  mu_off_t n;
  int rc = _amd_prop_fetch_off (amd, name, &n);
  if (rc == 0)
    {
      size_t s = n;
      if (s != n)
	return ERANGE;
      if (pval)
	*pval = s;
    }
  return rc;
}

int
_amd_prop_fetch_ulong (struct _amd_data *amd, const char *name,
		       unsigned long *pval)
{
  mu_off_t n;
  int rc = _amd_prop_fetch_off (amd, name, &n);
  if (rc == 0)
    {
      unsigned long s = n;
      if (s != n)
	return ERANGE;
      if (pval)
	*pval = s;
    }
  return rc;
}

int
_amd_prop_store_off (struct _amd_data *amd, const char *name, mu_off_t val)
{
  char nbuf[128];
  char *p;
  int sign = 0;
  
  p = nbuf + sizeof nbuf;
  *--p = 0;
  if (val < 0)
    {
      sign = 1;
      val = - val;
    }
  do
    {
      unsigned d = val % 10;
      if (p == nbuf)
	return ERANGE;
      *--p = d + '0';
      val /= 10;
    }
  while (val);
  if (sign)
    {
      if (p == nbuf)
	return ERANGE;
      *--p = '-';
    }
  return mu_property_set_value (amd->prop, name, p, 1);
}

static int
_amd_prop_create (struct _amd_data *amd)
{
  int rc;
  struct mu_mh_prop *mhprop;
  mhprop = calloc (1, sizeof (mhprop[0]));
  if (!mhprop)
    return ENOMEM;
  mhprop->filename = mu_make_file_name (amd->name, _MU_AMD_PROP_FILE_NAME);
  if (!mhprop->filename)
    {
      free (mhprop);
      return errno;
    }

  if (access (mhprop->filename, F_OK) == 0)
    {
      amd->flags |= MU_AMD_F_PROP;
    }
  
  rc = mu_property_create_init (&amd->prop, mu_mh_property_init, mhprop);
  if (rc)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("mu_property_create_init: %s",
		 mu_strerror (rc)));
      free (mhprop->filename);
      free (mhprop);
    }
  
  return rc;
}

/* Operations on message array */

/* Perform binary search for message MSG on a segment of message array
   of AMD between the indexes FIRST and LAST inclusively.
   If found, return 0 and store index of the located entry in the
   variable PRET. Otherwise, return 1 and place into PRET index of
   the nearest array element that is less than MSG (in the sense of
   amd->msg_cmp)
   Indexes are zero-based. */
   
static int
amd_msg_bsearch (struct _amd_data *amd, mu_off_t first, mu_off_t last,
		 struct _amd_message *msg,
		 mu_off_t *pret)
{
  mu_off_t mid;
  int rc;

  while (first <= last)
    {
      mid = (first + last) / 2;
      rc = amd->msg_cmp (amd->msg_array[mid], msg);
      if (rc > 0)
	last = mid - 1;
      else
	{
	  *pret = mid;
	  if (rc < 0)
	    first = mid + 1;
	  else
	    return 0;
	}
    }
  return 1;
}

/* Search for message MSG in the message array of AMD.
   If found, return 0 and store index of the located entry in the
   variable PRET. Otherwise, return 1 and store in PRET the index of
   the array element that is less than MSG (in the sense of
   amd->msg_cmp)
   Index returned in PRET is 1-based, so *PRET == 0 means that MSG
   is less than the very first element of the message array.

   In other words, when amd_msg_lookup() returns 1, the value in *PRET
   can be regarded as a 0-based index of the array slot where MSG can
   be inserted */

int
amd_msg_lookup (struct _amd_data *amd, struct _amd_message *msg,
		 size_t *pret)
{
  int rc;
  mu_off_t i;
  
  if (amd->msg_count == 0)
    {
      *pret = 0;
      return 1;
    }
  
  rc = amd->msg_cmp (msg, amd->msg_array[0]);
  if (rc < 0)
    {
      *pret = 0;
      return 1;
    }
  else if (rc == 0)
    {
      *pret = 1;
      return 0;
    }
  
  rc = amd->msg_cmp (msg, amd->msg_array[amd->msg_count - 1]);
  if (rc > 0)
    {
      *pret = amd->msg_count;
      return 1;
    }
  else if (rc == 0)
    {
      *pret = amd->msg_count;
      return 0;
    }
  
  rc = amd_msg_bsearch (amd, 0, amd->msg_count - 1, msg, &i);
  *pret = i + 1;
  return rc;
}

#define AMD_MSG_INC 64

/* Prepare the message array for insertion of a new message
   at position INDEX (zero based), by moving its contents
   one slot to the right. If necessary, expand the array by
   AMD_MSG_INC */
int
amd_array_expand (struct _amd_data *amd, size_t index)
{
  if (amd->msg_count == amd->msg_max)
    {
      struct _amd_message **p;
      
      amd->msg_max += AMD_MSG_INC; /* FIXME: configurable? */
      p = realloc (amd->msg_array, amd->msg_max * sizeof (amd->msg_array[0]));
      if (!p)
	{
	  amd->msg_max -= AMD_MSG_INC;
	  return ENOMEM;
	}
      amd->msg_array = p;
    }
  if (amd->msg_count > index)
    memmove (&amd->msg_array[index+1], &amd->msg_array[index],
	     (amd->msg_count-index) * sizeof (amd->msg_array[0]));
  amd->msg_count++;
  return 0;
}

/* Shrink the message array by removing the element at INDEX-COUNT and
   shifting left by COUNT positions all the elements to the right of
   it. */
int
amd_array_shrink (struct _amd_data *amd, size_t index, size_t count)
{
  if (amd->msg_count-index-1 && index < amd->msg_count)
    memmove (&amd->msg_array[index-count+1], &amd->msg_array[index + 1],
	     (amd->msg_count-index-1) * sizeof (amd->msg_array[0]));
  amd->msg_count -= count;
  return 0;
}


int
amd_init_mailbox (mu_mailbox_t mailbox, size_t amd_size,
		  struct _amd_data **pamd) 
{
  int status;
  struct _amd_data *amd;

  if (mailbox == NULL)
    return EINVAL;
  if (amd_size < sizeof (*amd))
    return EINVAL;

  amd = mailbox->data = calloc (1, amd_size);
  if (amd == NULL)
    return ENOMEM;

  /* Back pointer.  */
  amd->mailbox = mailbox;

  status = mu_url_aget_path (mailbox->url, &amd->name);
  if (status)
    {
      free (amd);
      mailbox->data = NULL;
      return status;
    }

  /* Overloading the defaults.  */
  mailbox->_destroy = amd_destroy;

  mailbox->_open = amd_open;
  mailbox->_close = amd_close;

  /* Overloading of the entire mailbox object methods.  */
  mailbox->_get_message = amd_get_message;
  mailbox->_quick_get_message = amd_quick_get_message;
  mailbox->_append_message = amd_append_message;
  mailbox->_messages_count = amd_messages_count;
  mailbox->_messages_recent = amd_messages_recent;
  mailbox->_message_unseen = amd_message_unseen;
  mailbox->_expunge = amd_expunge;
  mailbox->_sync = amd_sync;
  mailbox->_get_uidvalidity = amd_get_uidvalidity;
  mailbox->_set_uidvalidity = amd_set_uidvalidity;
  mailbox->_uidnext = amd_uidnext;

  mailbox->_scan = amd_scan;
  mailbox->_is_updated = amd_is_updated;

  mailbox->_get_size = amd_get_size;
  mailbox->_remove = amd_remove_mbox;
  
  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_TRACE1, ("amd_init(%s)", amd->name));
  *pamd = amd;
  return 0;
}

static void
amd_destroy (mu_mailbox_t mailbox)
{
  struct _amd_data *amd = mailbox->data;
  size_t i;
  
  if (!amd)
    return;

  amd_pool_flush (amd);
  mu_monitor_wrlock (mailbox->monitor);
  for (i = 0; i < amd->msg_count; i++)
    {
      mu_message_destroy (&amd->msg_array[i]->message, amd->msg_array[i]);
      if (amd->msg_free)
	amd->msg_free (amd->msg_array[i]);
      free (amd->msg_array[i]);
    }
  free (amd->msg_array);

  mu_property_destroy (&amd->prop);
  
  if (amd->name)
    free (amd->name);

  free (amd);
  mailbox->data = NULL;
  mu_monitor_unlock (mailbox->monitor);
}

static int
amd_open (mu_mailbox_t mailbox, int flags)
{
  struct _amd_data *amd = mailbox->data;
  struct stat st;

  mailbox->flags = flags;
  if (stat (amd->name, &st) < 0)
    {
      if ((flags & MU_STREAM_CREAT) && errno == ENOENT)
	{
	  int rc;
	  int perms = mu_stream_flags_to_mode (flags, 1);
	  if (mkdir (amd->name, S_IRUSR|S_IWUSR|S_IXUSR|perms))
	    return errno;
	  if (stat (amd->name, &st) < 0)
	    return errno;
	  if (amd->create && (rc = amd->create (amd, flags)))
	    return rc;
	}
      else
	return errno;
    }
    
  if (!S_ISDIR (st.st_mode))
    return EINVAL;

  if (access (amd->name,
	      (flags & (MU_STREAM_WRITE|MU_STREAM_APPEND)) ?
	       W_OK : R_OK | X_OK))
    return errno;

  /* Create/read properties.  It is not an error if this fails. */
  _amd_prop_create (amd);
  
  if (mailbox->locker == NULL)
    mu_locker_create_ext (&mailbox->locker, "/dev/null", NULL);
  
  return 0;
}

static int
amd_close (mu_mailbox_t mailbox)
{
  struct _amd_data *amd;
  int i;
    
  if (!mailbox)
    return EINVAL;

  amd = mailbox->data;
  
  /* Destroy all cached data */
  amd_pool_flush (amd);
  mu_monitor_wrlock (mailbox->monitor);
  for (i = 0; i < amd->msg_count; i++)
    {
      mu_message_destroy (&amd->msg_array[i]->message, amd->msg_array[i]);
      if (amd->msg_free)
	amd->msg_free (amd->msg_array[i]);
      free (amd->msg_array[i]);
    }
  free (amd->msg_array);
  amd->msg_array = NULL;

  mu_property_save (amd->prop);
    
  amd->msg_count = 0; /* number of messages in the list */
  amd->msg_max = 0;   /* maximum message buffer capacity */

  mu_monitor_unlock (mailbox->monitor);
  
  return 0;
}

static int
amd_message_qid (mu_message_t msg, mu_message_qid_t *pqid)
{
  struct _amd_message *mhm = mu_message_get_owner (msg);
  
  return mhm->amd->cur_msg_file_name (mhm, 0, pqid);
}

static void
amd_message_detach (mu_message_t msg)
{
  struct _amd_message *mhm = mu_message_get_owner (msg);
  mhm->message = NULL;
}

struct _amd_message *
_amd_get_message (struct _amd_data *amd, size_t msgno)
{
  msgno--;
  if (msgno >= amd->msg_count)
    return NULL;
  return amd->msg_array[msgno];
}

static int
_amd_attach_message (mu_mailbox_t mailbox, struct _amd_message *mhm,
		     mu_message_t *pmsg)
{
  int status;
  mu_message_t msg;

  /* Check if we already have it.  */
  if (mhm->message)
    {
      if (pmsg)
	*pmsg = mhm->message;
      return 0;
    }

  /* Get an empty message struct.  */
  status = mu_message_create (&msg, mhm);
  if (status != 0)
    return status;

  msg->_detach = amd_message_detach;
  
  /* Set the header.  */
  {
    mu_header_t header = NULL;
    status = mu_header_create (&header, NULL, 0);
    if (status != 0)
      {
	mu_message_destroy (&msg, mhm);
	return status;
      }
    mu_header_set_fill (header, amd_header_fill, msg);
    /*FIXME:
    mu_header_set_get_fvalue (header, amd_header_get_fvalue, msg);
    */
    mu_message_set_header (msg, header, mhm);
  }

  /* Set the attribute.  */
  {
    mu_attribute_t attribute;
    status = mu_attribute_create (&attribute, msg);
    if (status != 0)
      {
	mu_message_destroy (&msg, mhm);
	return status;
      }
    mu_attribute_set_get_flags (attribute, amd_get_attr_flags, msg);
    mu_attribute_set_set_flags (attribute, amd_set_attr_flags, msg);
    mu_attribute_set_unset_flags (attribute, amd_unset_attr_flags, msg);
    mu_message_set_attribute (msg, attribute, mhm);
  }

  /* Prepare the body.  */
  {
    mu_body_t body = NULL;
    struct _amd_body_stream *str;
    
    if ((status = mu_body_create (&body, msg)) != 0)
      return status;

    str = (struct _amd_body_stream *)
              _mu_stream_create (sizeof (*str),
				 mailbox->flags | MU_STREAM_SEEK | 
				 _MU_STR_OPEN);
    if (!str)
      {
	mu_body_destroy (&body, msg);
	mu_message_destroy (&msg, mhm);
	return ENOMEM;
      }
    str->stream.read = amd_body_stream_read;
    str->stream.size = amd_body_stream_size;
    str->stream.seek = amd_body_stream_seek;
    mu_body_set_stream (body, (mu_stream_t) str, msg);
    mu_body_clear_modified (body);
    mu_body_set_size (body, amd_body_size, msg);
    mu_body_set_lines (body, amd_body_lines, msg);
    mu_message_set_body (msg, body, mhm);
    str->body = body;
  }

  /* Set the envelope.  */
  {
    mu_envelope_t envelope = NULL;
    status = mu_message_reconstruct_envelope (msg, &envelope);
    if (status != 0)
      {
	mu_message_destroy (&msg, mhm);
	return status;
      }
    mu_message_set_envelope (msg, envelope, mhm);
  }

  /* Set the UID.  */
  if (mhm->amd->message_uid)
    mu_message_set_uid (msg, mhm->amd->message_uid, mhm);
  mu_message_set_qid (msg, amd_message_qid, mhm);
  
  /* Attach the message to the mailbox mbox data.  */
  mhm->message = msg;
  mu_message_set_mailbox (msg, mailbox, mhm);

  /* Some of mu_message_set_ functions above mark message as modified.
     Undo it now.

     FIXME: Marking message as modified is not always appropriate. Find
     a better way. */
     
  mu_message_clear_modified (msg);

  if (pmsg)
    *pmsg = msg;

  return 0;
}

static int
_amd_scan0 (struct _amd_data *amd, size_t msgno, size_t *pcount,
	    int do_notify)
{
  unsigned long uidval;
  int status = amd->scan0 (amd->mailbox, msgno, pcount, do_notify);
  if (status != 0)
    return status;
  /* Reset the uidvalidity.  */
  if (amd->msg_count == 0 ||
      _amd_prop_fetch_ulong (amd, _MU_AMD_PROP_UIDVALIDITY, &uidval) ||
      !uidval)
    {
      uidval = (unsigned long) amd->mtime;
      _amd_prop_store_off (amd, _MU_AMD_PROP_UIDVALIDITY, uidval);
    }
  return 0;
}

static int
amd_get_message (mu_mailbox_t mailbox, size_t msgno, mu_message_t *pmsg)
{
  int status;
  struct _amd_data *amd = mailbox->data;
  struct _amd_message *mhm;

  /* Sanity checks.  */
  if (pmsg == NULL)
    return MU_ERR_OUT_PTR_NULL;
  if (amd == NULL || msgno < 1)
    return EINVAL;

  /* If we did not start a scanning yet do it now.  */
  if (amd->msg_count == 0)
    {
      status = _amd_scan0 (amd, 1, NULL, 0);
      if (status != 0)
	return status;
    }

  if ((mhm = _amd_get_message (amd, msgno)) == NULL)
    return MU_ERR_NOENT;
  return _amd_attach_message (mailbox, mhm, pmsg);
}

static int
amd_quick_get_message (mu_mailbox_t mailbox, mu_message_qid_t qid,
		       mu_message_t *pmsg)
{
  int status;
  struct _amd_data *amd = mailbox->data;
  if (amd->msg_count)
    {
      mu_message_qid_t vqid;
      mu_message_t msg = amd->msg_array[0]->message;
      status = mu_message_get_qid (msg, &vqid);
      if (status)
	return status;
      status = strcmp (qid, vqid);
      free (vqid);
      if (status)
	return MU_ERR_EXISTS;
      *pmsg = msg;
    }
  else if (amd->qfetch)
    {
      status = amd->qfetch (amd, qid);
      if (status)
	return status;
      return _amd_attach_message (mailbox, amd->msg_array[0], pmsg);
    }
  
  return ENOSYS;
}

static int
_amd_tempstream (struct _amd_data *amd, mu_stream_t *pstr, char **namep)
{
  struct mu_tempfile_hints hints;
  int fd, rc;

  hints.tmpdir = amd->name;
  rc = mu_tempfile (&hints, MU_TEMPFILE_TMPDIR, &fd, namep);
  if (rc == 0)
    rc = mu_fd_stream_create (pstr, *namep, fd, MU_STREAM_WRITE);
  return rc;
}

static int
_amd_message_save (struct _amd_data *amd, struct _amd_message *mhm,
		   mu_envelope_t env,
		   int expunge)
{
  mu_stream_t stream = NULL;
  char *name = NULL, *msg_name, *old_name;
  size_t new_body_start, new_header_lines;
  mu_stream_t ostr;
  mu_stream_stat_buffer stat;  
  mu_message_t msg = mhm->message;
  mu_header_t hdr;
  int status;
  mu_body_t body;
  static char *exclude_headers_tab[] = {
    MU_HEADER_RETURN_PATH,    
    MU_HEADER_X_IMAPBASE,
    MU_HEADER_X_UID,
    MU_HEADER_STATUS,
    MU_HEADER_ENV_DATE,
    MU_HEADER_ENV_SENDER,
    NULL,
  };
  char **exclude_headers = exclude_headers_tab;
  size_t i;
    
  status = amd->new_msg_file_name (mhm, mhm->attr_flags, expunge, &msg_name);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("new_msg_file_name failed: %s",
		 mu_strerror (status)));
      return status;
    }
  
  if (!msg_name)
    {
      /* Unlink the original file */
      char *old_name;
      status = amd->cur_msg_file_name (mhm, 1, &old_name);
      free (msg_name);
      if (status == 0 && unlink (old_name))
	status = errno;
      free (old_name);
      return status;
    }      
    
  status = _amd_tempstream (mhm->amd, &ostr, &name);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("_amd_tempstream failed: %s",
		 mu_strerror (status)));
      free (msg_name);
      return status;
    }
  mu_stream_set_stat (ostr,
		      MU_STREAM_STAT_MASK (MU_STREAM_STAT_OUT) |
		      MU_STREAM_STAT_MASK (MU_STREAM_STAT_OUTLN),
		      stat);

  /* Copy flags */
  if (env)
    {
      char const *s;

      if (mu_envelope_sget_sender (env, &s) == 0)
	mu_stream_printf (ostr, "%s: %s\n", MU_HEADER_RETURN_PATH, s);
      else      
	exclude_headers++; // Preserve MU_HEADER_RETURN_PATH
      if (mu_envelope_sget_date (env, &s) == 0)
	{
	  struct tm tm;
	  struct mu_timezone tz;
	      
	  if (mu_parse_date_dtl (s, NULL, NULL, &tm, &tz, NULL) == 0)
	    {
	      /* Format a "Received:" header with that date */
	      mu_stream_printf (ostr,
				"Received: from %s\n"
				"\tby %s; ",
				"localhost", "localhost");
	      mu_c_streamftime (ostr, MU_DATETIME_FORM_RFC822, &tm, &tz);
	      mu_stream_write (ostr, "\n", 1, NULL);
	    }
	}
    }
  else
    exclude_headers++; // Preserve MU_HEADER_RETURN_PATH
  
  mu_message_get_header (msg, &hdr);
  mu_header_get_streamref (hdr, &stream);
  status = mu_stream_seek (stream, 0, MU_SEEK_SET, NULL);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("mu_stream_seek on header stream failed: %s",
		 mu_strerror (status)));
      goto err;
    }
  status = mu_stream_header_copy (ostr, stream, exclude_headers);
  mu_stream_destroy (&stream);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("mu_stream_header_copy failed: %s",
		 mu_strerror (status)));
      goto err;
    }
  
  if (!(amd->capabilities & MU_AMD_STATUS))
    {
      /* Add status */
      char statbuf[MU_STATUS_BUF_SIZE];
      
      if (mu_attribute_flags_to_string (mhm->attr_flags,
					statbuf, sizeof (statbuf), NULL) == 0
	  && statbuf[0] != 0)
	{
	  mu_stream_printf (ostr, "%s: %s\n", MU_HEADER_STATUS, statbuf);
	}
    }

  /*
   * Header_size can be less than body_start if MU_AMD_DASHDELIM is set
   * and a line of dashes was used originally to delimit body from headers
   * (e.g. for draft file in MH format).
   */
  for (i = mhm->header_size; i < mhm->body_start; i++)
    mu_stream_write (ostr, "-", 1, NULL);
  
  if (mu_stream_err (ostr))
    {
      status = mu_stream_last_error (ostr);
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("output error on temporary stream: %s",
		 mu_strerror (status)));
      goto err;
    }
  
  mu_stream_write (ostr, "\n", 1, NULL);

  new_header_lines = stat[MU_STREAM_STAT_OUTLN];
  new_body_start = stat[MU_STREAM_STAT_OUT];

  /* Clear the counters */
  stat[MU_STREAM_STAT_OUT] = stat[MU_STREAM_STAT_OUTLN] = 0;

  /* Copy message body */
  mu_message_get_body (msg, &body);
  mu_body_get_streamref (body, &stream);
  status = mu_stream_seek (stream, 0, MU_SEEK_SET, NULL);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("mu_stream_seek on body stream failed: %s",
		 mu_strerror (status)));
      goto err;
    }
  
  status = mu_stream_copy (ostr, stream, 0, NULL);
  mu_stream_destroy (&stream);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("copying message body failed: %s",
		 mu_strerror (status)));
      goto err;
    }
  
  mhm->header_lines = new_header_lines;
  mhm->header_size = new_body_start;
  mhm->body_start = new_body_start;
  mhm->body_lines = stat[MU_STREAM_STAT_OUTLN];
  mhm->body_end = stat[MU_STREAM_STAT_OUT];
  
  mu_stream_destroy (&ostr);  

  status = amd->cur_msg_file_name (mhm, 1, &old_name);
  if (status == 0)
    {
      if (rename (name, msg_name))
	{
	  if (errno == ENOENT)
	    mu_observable_notify (amd->mailbox->observable,
				  MU_EVT_MAILBOX_CORRUPT,
				  amd->mailbox);
	  else
	    {
	      status = errno;
	      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
			("renaming %s to %s failed: %s",
			 name, msg_name, mu_strerror (status)));
	    }
	}
      else
	{
	  mode_t perms;
	  
	  perms = mu_stream_flags_to_mode (amd->mailbox->flags, 0);
	  if (perms != 0)
	    {
	      /* It is documented that the mailbox permissions are
		 affected by the current umask, so take it into account
		 here.
		 FIXME: I'm still not sure we should honor umask, though.
		 --gray
	      */
	      mode_t mask = umask (0);
	      chmod (msg_name, (0600 | perms) & ~mask);
	      umask (mask);
	    }
	  if (strcmp (old_name, msg_name))
	    /* Unlink original message */
	    unlink (old_name);
	}
      free (old_name);
    }
 err:
  mu_stream_destroy (&ostr);
  if (status)
    unlink (name);
  free (msg_name);
  free (name);

  return status;
}

static int
amd_append_message (mu_mailbox_t mailbox, mu_message_t msg,
		    mu_envelope_t env, mu_attribute_t atr)
{
  int status;
  struct _amd_data *amd = mailbox->data;
  struct _amd_message *mhm;
  
  if (!mailbox || !msg)
    return EINVAL;

  mhm = calloc (1, amd->msg_size);
  if (!mhm)
    return ENOMEM;
    
  /* If we did not start a scanning yet do it now.  */
  if (amd->msg_count == 0)
    {
      status = _amd_scan0 (amd, 1, NULL, 0);
      if (status != 0)
	{
	  free (mhm);
	  return status;
	}
    }

  amd->has_new_msg = 1;
  
  mhm->amd = amd;
  if (amd->msg_init_delivery)
    {
      status = amd->msg_init_delivery (amd, mhm);
      if (status)
	{
	  free (mhm);
	  return status;
	}
    }
  
  mhm->message = msg;

  if (atr)
    mu_attribute_get_flags (atr, &mhm->attr_flags);
  
  status = _amd_message_save (amd, mhm, env, 0);
  if (status)
    {
      free (mhm);
      return status;
    }

  mhm->message = NULL;
  /* Insert and re-scan the message */
  status = _amd_message_insert (amd, mhm);
  if (status)
    {
      free (mhm);
      return status;
    }

  if (amd->msg_finish_delivery)
    status = amd->msg_finish_delivery (amd, mhm, msg, atr);
  
  if (status == 0 && mailbox->observable)
    {
      char *qid;
      if (amd->cur_msg_file_name (mhm, 0, &qid) == 0)
	{
	  mu_observable_notify (mailbox->observable, 
	                        MU_EVT_MAILBOX_MESSAGE_APPEND,
				qid);
	  free (qid);
	}
    }
  
  return status;
}

static int
amd_messages_count (mu_mailbox_t mailbox, size_t *pcount)
{
  struct _amd_data *amd = mailbox->data;

  if (amd == NULL)
    return EINVAL;

  if (!amd_is_updated (mailbox))
    return _amd_scan0 (amd,  amd->msg_count, pcount, 0);

  if (pcount)
    *pcount = amd->msg_count;

  return 0;
}

/* A "recent" message is the one not marked with MU_ATTRIBUTE_SEEN
   ('O' in the Status header), i.e. a message that is first seen
   by the current session (see attributes.h) */
static int
amd_messages_recent (mu_mailbox_t mailbox, size_t *pcount)
{
  struct _amd_data *amd = mailbox->data;
  size_t count, i;

  /* If we did not start a scanning yet do it now.  */
  if (amd->msg_count == 0)
    {
      int status = _amd_scan0 (amd, 1, NULL, 0);
      if (status != 0)
	return status;
    }
  count = 0;
  for (i = 0; i < amd->msg_count; i++)
    {
      if (MU_ATTRIBUTE_IS_UNSEEN(amd->msg_array[i]->attr_flags))
	count++;
    }
  *pcount = count;
  return 0;
}

/* An "unseen" message is the one that has not been read yet */
static int
amd_message_unseen (mu_mailbox_t mailbox, size_t *pmsgno)
{
  struct _amd_data *amd = mailbox->data;
  size_t i;

  /* If we did not start a scanning yet do it now.  */
  if (amd->msg_count == 0)
    {
      int status = _amd_scan0 (amd, 1, NULL, 0);
      if (status != 0)
	return status;
    }

  for (i = 0; i < amd->msg_count; i++)
    {
      if (MU_ATTRIBUTE_IS_UNREAD(amd->msg_array[0]->attr_flags))
	{
	  *pmsgno = i + 1;
	  break;
	}
    }
  return 0;
}

static int
_compute_mailbox_size_recursive (struct _amd_data *amd, const char *name,
				 mu_off_t *psize)
{
  DIR *dir;
  struct dirent *entry;
  char *buf;
  size_t bufsize;
  size_t dirlen;
  size_t flen;
  int status = 0;
  struct stat sb;

  dir = opendir (name);
  if (!dir)
    return errno;

  dirlen = strlen (name);
  bufsize = dirlen + 32;
  buf = malloc (bufsize);
  if (!buf)
    {
      closedir (dir);
      return ENOMEM;
    }
  
  strcpy (buf, name);
  if (buf[dirlen-1] != '/')
    buf[++dirlen - 1] = '/';
	  
  while ((entry = readdir (dir)))
    {
      switch (entry->d_name[0])
	{
	case '.':
	  break;

	default:
	  flen = strlen (entry->d_name);
	  if (dirlen + flen + 1 > bufsize)
	    {
	      bufsize = dirlen + flen + 1;
	      buf = realloc (buf, bufsize);
	      if (!buf)
		{
		  status = ENOMEM;
		  break;
		}
	    }
	  strcpy (buf + dirlen, entry->d_name);
	  if (stat (buf, &sb) == 0)
	    {
	      if (S_ISREG (sb.st_mode))
		*psize += sb.st_size;
	      else if (S_ISDIR (sb.st_mode))
		_compute_mailbox_size_recursive (amd, buf, psize);
	    }
	  /* FIXME: else? */
	  break;
	}
    }

  free (buf);
  
  closedir (dir);
  return status;
}

static int
compute_mailbox_size (struct _amd_data *amd, mu_off_t *psize)
{
  mu_off_t size = 0;
  int rc = _compute_mailbox_size_recursive (amd, amd->name, &size);
  if (rc == 0)
    {
      rc = _amd_prop_store_off (amd, _MU_AMD_PROP_SIZE, size);
      if (rc == 0 && psize)
	*psize = size;
    }
  return rc;
}

static int
amd_remove_mbox (mu_mailbox_t mailbox)
{
  int rc;
  struct _amd_data *amd = mailbox->data;
  
  if (!amd->remove)
    return ENOSYS;
  rc = amd->remove (amd);
  if (rc == 0)
    {
      char *name;

      name = mu_make_file_name (amd->name, _MU_AMD_PROP_FILE_NAME);
      if (!name)
	return ENOMEM;
      if (unlink (name) && errno != ENOENT)
	rc = errno;
      free (name);
    }

  if (rc == 0)
    {
      if (rmdir (amd->name) && errno != ENOENT)
	{
	  rc = errno;
	  /* POSIX.1-2001 allows EEXIST to be returned if the directory
	     contained entries other than . and .. */
	  if (rc == EEXIST)
	    rc = ENOTEMPTY;
	}
    }
  
  return rc;
}

static int
_amd_update_message (struct _amd_data *amd, struct _amd_message *mhm,
		     int expunge, int *upd)
{
  int flg, rc;
      
  if (mhm->message)
    flg = mu_message_is_modified (mhm->message);
  else if (mhm->attr_flags & MU_ATTRIBUTE_MODIFIED)
    flg = MU_MSG_ATTRIBUTE_MODIFIED;
  else
    return 0;
  
  if (!flg)
    return 0;

  if (flg == MU_MSG_ATTRIBUTE_MODIFIED && amd->chattr_msg)
    {
      rc = amd->chattr_msg (mhm, expunge);
      if (rc)
	{
	  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		    ("_amd_update_message: chattr_msg failed: %s",
		     mu_strerror (rc)));
	  return rc;
	}
    }
  else
    {
      rc = _amd_attach_message (amd->mailbox, mhm, NULL);
      if (rc)
	{
	  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		    ("_amd_update_message: _amd_attach_message failed: %s",
		     mu_strerror (rc)));
	  return rc;
	}
	  
      rc = _amd_message_save (amd, mhm, NULL, expunge);
      if (rc)
	{
	  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		    ("_amd_update_message: _amd_message_save failed: %s",
		     mu_strerror (rc)));
	  return rc;
	}
    }
  *upd = 1;
  return rc;
}

static int
amd_expunge (mu_mailbox_t mailbox)
{
  struct _amd_data *amd = mailbox->data;
  struct _amd_message *mhm;
  size_t i;
  int updated = amd->has_new_msg;
  size_t expcount = 0;
  size_t last_expunged = 0;
  
  if (amd == NULL)
    return EINVAL;

  if (amd->msg_count == 0)
    return 0;

  for (i = 0; i < amd->msg_count; i++)
    {
      mhm = amd->msg_array[i];
      
      if (mhm->attr_flags & MU_ATTRIBUTE_DELETED)
	{
	  int rc;
	  struct _amd_message **pp;

	  if (amd->delete_msg)
	    {
	      rc = amd->delete_msg (amd, mhm);
	      if (rc)
		return rc;
	    }
	  else
	    {
	      char *old_name;
	      char *new_name;

	      rc = amd->cur_msg_file_name (mhm, 1, &old_name);
	      if (rc)
		return rc;
	      rc = amd->new_msg_file_name (mhm, mhm->attr_flags, 1,
					   &new_name);
	      if (rc)
		{
		  free (old_name);
		  return rc;
		}

	      if (new_name)
		{
		  /* FIXME: It may be a good idea to have a capability flag
		     in struct _amd_data indicating that no actual removal
		     is needed (e.g. for traditional MH). It will allow to
		     bypass lots of no-op code here. */
		  if (strcmp (old_name, new_name) &&
		      /* Rename original message */
		      rename (old_name, new_name))
		    {
		      if (errno == ENOENT)
			mu_observable_notify (mailbox->observable,
					      MU_EVT_MAILBOX_CORRUPT,
					      mailbox);
		      else
			{
			  rc = errno;
			  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
				    ("renaming %s to %s failed: %s",
				     old_name, new_name, mu_strerror (rc)));
			}
		    }
		}
	      else
		/* Unlink original file */
		unlink (old_name);
	      
	      free (old_name);
	      free (new_name);
	    }

	  pp = amd_pool_lookup (mhm);
	  if (pp)
	    *pp = NULL;
	  amd_message_stream_close (mhm);
	  mu_message_destroy (&mhm->message, mhm);
	  if (amd->msg_free)
	    amd->msg_free (mhm);
	  free (mhm);
	  amd->msg_array[i] = NULL;
	  last_expunged = i;
	  updated = 1;

	  {
	    size_t expevt[2] = { i + 1, expcount };
	    mu_observable_notify (mailbox->observable,
				  MU_EVT_MAILBOX_MESSAGE_EXPUNGE,
				  expevt);
	    ++expcount;
	  }
	}
      else
	{
	  _amd_update_message (amd, mhm, 1, &updated);/*FIXME: Error checking*/
	}
    }

  if (expcount)
    {
      int reset_uidvalidity;
      
      last_expunged++;

      /* See the description of MU_AMD_VOLATILE_UIDNEXT in amd.h for
	 details.
      */
      reset_uidvalidity = (amd->capabilities & MU_AMD_VOLATILE_UIDNEXT) 
	                   && last_expunged == amd->msg_count;
      
      do
	{
	  size_t j;
	  
	  for (j = 1; j < last_expunged && !amd->msg_array[last_expunged-j-1];
	       j++)
	    ;
	  amd_array_shrink (amd, last_expunged - 1, j);
	  for (last_expunged -= j;
	       last_expunged > 0 && amd->msg_array[last_expunged - 1];
	       last_expunged--)
	    ;
	}
      while (last_expunged);

      if (reset_uidvalidity)
	{
      	  /*
	   * The following is equivalent to
	   * mu_mailbox_uidvalidity_reset (amd->mailbox);
	   */

	  struct timeval tv;
	  gettimeofday (&tv, NULL);
	  amd_set_uidvalidity (amd->mailbox, tv.tv_sec);
	}
    }
  
  if (updated && !amd->mailbox_size)
    {
      compute_mailbox_size (amd, NULL);
    }
  return 0;
}

static int
amd_sync (mu_mailbox_t mailbox)
{
  struct _amd_data *amd = mailbox->data;
  struct _amd_message *mhm;
  size_t i;
  int updated = amd->has_new_msg;
  
  if (amd == NULL)
    return EINVAL;

  if (amd->msg_count == 0)
    return 0;

  /* Find the first dirty(modified) message.  */
  for (i = 0; i < amd->msg_count; i++)
    {
      mhm = amd->msg_array[i];
      if ((mhm->attr_flags & MU_ATTRIBUTE_MODIFIED)
	  || (mhm->message && mu_message_is_modified (mhm->message)))
	break;
    }

  for ( ; i < amd->msg_count; i++)
    {
      mhm = amd->msg_array[i];
      _amd_update_message (amd, mhm, 0, &updated); 
    }

  if (updated && !amd->mailbox_size)
    {
      compute_mailbox_size (amd, NULL);
    }

  return 0;
}

static inline int
amd_initial_scan (struct _amd_data *amd)
{
  if (!(amd->flags & MU_AMD_F_INIT_SCAN))
    {
      int status = _amd_scan0 (amd, 1, NULL, 0);
      if (status != 0)
        return status;
      amd->flags |= MU_AMD_F_INIT_SCAN;
    }
  return 0;
}

static int
amd_get_uidvalidity (mu_mailbox_t mailbox, unsigned long *pval)
{
  struct _amd_data *amd = mailbox->data;
  int status = amd_initial_scan (amd);
  if (status != 0)
    return status;
  return _amd_prop_fetch_ulong (amd, _MU_AMD_PROP_UIDVALIDITY, pval);
}

static int
amd_set_uidvalidity (mu_mailbox_t mailbox, unsigned long uidvalidity)
{
  struct _amd_data *amd = mailbox->data;
  size_t uidnext;
  int status = amd_initial_scan (amd);
  if (status != 0)
    return status;

  if (amd->msg_count == 0)
    uidnext = 1;
  else
    {
      mu_message_t msg;
      
      if ((status = amd_get_message (mailbox, amd->msg_count - 1, &msg)) != 0 ||
	  (status = mu_message_get_uid (msg, &uidnext)) != 0)
	return status;
      uidnext++;
    }
  status = _amd_prop_store_off (amd, _MU_AMD_PROP_UIDNEXT, uidnext);
  if (status == 0)
    status = _amd_prop_store_off (amd, _MU_AMD_PROP_UIDVALIDITY, uidvalidity);
  return status;
}

static int
amd_uidnext (mu_mailbox_t mailbox, size_t *puidnext)
{
  struct _amd_data *amd = mailbox->data;
  int status = amd_initial_scan (amd);
  if (status != 0)
    return status;
  return _amd_prop_fetch_size (amd, _MU_AMD_PROP_UIDNEXT, puidnext);
}

/* FIXME: effectively the same as mbox_cleanup */
void
amd_cleanup (void *arg)
{
  mu_mailbox_t mailbox = arg;
  mu_monitor_unlock (mailbox->monitor);
}

int
_amd_message_lookup_or_insert (struct _amd_data *amd,
			       struct _amd_message *key,
			       size_t *pindex)
{
  int result = 0;
  size_t index;
  if (amd_msg_lookup (amd, key, &index))
    {
      /* Not found. Index points to the array cell where msg would
	 be placed */
      result = amd_array_expand (amd, index);
      if (result)
	return result;
      else
	result = MU_ERR_NOENT;
    }
  else
    result = 0;
  *pindex = index;
  return result;
}

/* Insert message msg into the message list on the appropriate position */
int
_amd_message_insert (struct _amd_data *amd, struct _amd_message *msg)
{
  size_t index;
  int rc = _amd_message_lookup_or_insert (amd, msg, &index);

  if (rc == MU_ERR_NOENT)
    {
      amd->msg_array[index] = msg;
      msg->amd = amd;
    }
  else if (rc == 0)
    {
      /*FIXME: Found? Shouldn't happen */
      return EEXIST;
    }
  else
    return rc;
  return 0;
}

/* Append message to the end of the array, expanding it if necessary */
int
_amd_message_append (struct _amd_data *amd, struct _amd_message *msg)
{
  size_t index = amd->msg_count;
  int rc = amd_array_expand (amd, index);
  if (rc)
    return rc;
  amd->msg_array[index] = msg;
  msg->amd = amd;
  return 0;
}

static int
msg_array_comp (const void *a, const void *b)
{
  struct _amd_message **ma = (struct _amd_message **) a;
  struct _amd_message **mb = (struct _amd_message **) b;
  struct _amd_data *amd = (*ma)->amd;
  return amd->msg_cmp (*ma, *mb);
}

void
amd_sort (struct _amd_data *amd)
{
  if (amd->msg_count)
    qsort (amd->msg_array, amd->msg_count, sizeof (amd->msg_array[0]),
	   msg_array_comp);
}

/* Scan given message and fill amd_message_t fields.
   NOTE: the function assumes mhm->stream != NULL. */
static int
amd_scan_message (struct _amd_message *mhm)
{
  mu_stream_t stream = mhm->stream;
  struct _amd_data *amd = mhm->amd;
  int amd_capa = amd->capabilities;
  char *msg_name;
  struct stat st;
  int status;
  
  /* Check if the message was modified after the last scan */
  status = mhm->amd->cur_msg_file_name (mhm, 1, &msg_name);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("amd_scan_message: cur_msg_file_name=%s",
		 mu_strerror (status)));      
      return status;
    }
  
  if (stat (msg_name, &st) == 0 && st.st_mtime == mhm->mtime)
    {
      /* Nothing to do */
      free (msg_name);
      return 0;
    }

  status = mu_stream_seek (stream, 0, MU_SEEK_SET, NULL);
  if (status)
    mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
	      ("amd_scan_message(%s): mu_stream_seek=%s",
	       msg_name, mu_strerror (status)));      
  else
    {
      enum
      {
	amd_scan_init,
	amd_scan_header,
	amd_scan_header_newline,
	amd_scan_header_dash,
	amd_scan_header_expect,
	amd_scan_body,
      } state = amd_scan_init;
      static char expect[] = "status:";
      int i;
      
      char cur;
      size_t n;
      size_t ndash;
      
      mhm->mtime = st.st_mtime;
      mhm->header_lines = 0;      
      mhm->body_lines = 0;

      while ((status = mu_stream_read (stream, &cur, 1, &n)) == 0)
	{
	  if (n == 0)
	    break;
	  
	  switch (state)
	    {
	    case amd_scan_init:
	      state = amd_scan_header_newline;
	      i = 0;
	      break;

	    case amd_scan_header:
	      if (cur == '\n')
		state = amd_scan_header_newline;
	      break;

	    case amd_scan_header_newline:
	      mhm->header_lines++;
	      if (cur == '\n')
		{
		  status = mu_stream_seek (stream, 0, MU_SEEK_CUR,
					   &mhm->body_start);
		  if (status)
		    {
		      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
				("%s:%s (%s): %s",
				 __func__, "mu_stream_seek",
				 amd->name,
				 mu_strerror (status)));
		      return status;
		    }
		  mhm->body_end = mhm->body_start;
		  mhm->header_size = mhm->body_start;
		  state = amd_scan_body;
		}
	      else if ((amd_capa & MU_AMD_DASHDELIM) && cur == '-')
		{
		  state = amd_scan_header_dash;
		  ndash = 1;
		}
	      else
		{
		  state = amd_scan_header;
		  if (!(amd_capa & MU_AMD_STATUS))
		    {
		      i = 0;
		      if (expect[i] == mu_tolower (cur))
			{
			  i++;
			  state = amd_scan_header_expect;
			  break;
			}
		    }
		}
	      break;

	    case amd_scan_header_dash:
	      if (cur == '\n')
		{
		  status = mu_stream_seek (stream, 0, MU_SEEK_CUR,
					   &mhm->body_start);
		  if (status)
		    {
		      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
				("%s:%s (%s): %s",
				 __func__, "mu_stream_seek",
				 amd->name,
				 mu_strerror (status)));
		      return status;
		    }
		  mhm->body_end = mhm->body_start;
		  mhm->header_size = mhm->body_start - ndash;
		  state = amd_scan_body;
		}
	      else if (cur == '-')
		ndash++;
	      else
		state = amd_scan_header;
	      break;
	      
	    case amd_scan_header_expect:
	      if (cur == '\n')
		{
		  state = amd_scan_header_newline;
		}
	      else
		{
		  int c = mu_tolower (cur);
		  if (expect[i] != c)
		    {
		      state = amd_scan_header;
		      break;
		    }

		  if (c == ':')
		    {
		      char *buf = NULL;
		      size_t size = 0;
		      size_t n;

		      status = mu_stream_getline (stream, &buf, &size, &n);
		      if (status)
			{
			  mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
				    ("%s:%s (%s): %s",
				     __func__, "mu_stream_getline",
				     amd->name,
				     mu_strerror (status)));
			  return status;
			}
		      if (n > 0)
			{
			  int deleted = mhm->attr_flags & MU_ATTRIBUTE_DELETED;
			  buf[n-1] = 0;
			  mu_attribute_string_to_flags (buf, &mhm->attr_flags);
			  mhm->attr_flags |= deleted;
			}

		      free (buf);
		      state = amd_scan_header_newline;
		    }
		  else
		    {
		      i++;
		      if (expect[i] == 0)
			state = amd_scan_header_newline;
		    }
		}
	      break;

	    case amd_scan_body:
	      mhm->body_end++;
	      if (cur == '\n')
		{
		  mhm->body_lines++;
		}
	      break;
	    }
	}
    }  
  free (msg_name);
  return status;
}

static int
amd_scan (mu_mailbox_t mailbox, size_t msgno, size_t *pcount)
{
  struct _amd_data *amd = mailbox->data;

  if (! amd_is_updated (mailbox))
    return _amd_scan0 (amd, msgno, pcount, 1);

  if (pcount)
    *pcount = amd->msg_count;

  return 0;
}

/* Is the internal representation of the mailbox up to date.
   Return 1 if so, 0 otherwise. */
static int
amd_is_updated (mu_mailbox_t mailbox)
{
  struct stat st;
  struct _amd_data *amd = mailbox->data;

  if (stat (amd->name, &st) < 0)
    return 1;

  return amd->mtime == st.st_mtime;
}

static int
amd_get_size (mu_mailbox_t mailbox, mu_off_t *psize)
{
  struct _amd_data *amd = mailbox->data;
  if (amd->mailbox_size)
    return amd->mailbox_size (mailbox, psize);
  if (_amd_prop_fetch_off (amd, _MU_AMD_PROP_SIZE, psize))
    return compute_mailbox_size (amd, psize);
  return 0;
}

/* Return number of open streams residing in a message pool */
static int
amd_pool_open_count (struct _amd_data *amd)
{
  int cnt = amd->pool_last - amd->pool_first;
  if (cnt < 0)
    cnt += MAX_OPEN_STREAMS;
  return cnt;
}

/* Look up a _amd_message in the pool of open messages.
   If the message is found in the pool, returns the address of
   the pool slot occupied by it. Otherwise returns NULL. */
static struct _amd_message **
amd_pool_lookup (struct _amd_message *mhm)
{
  struct _amd_data *amd = mhm->amd;
  int i;

  for (i = amd->pool_first; i != amd->pool_last; )
    {
      if (amd->msg_pool[i] == mhm)
	return &amd->msg_pool[i];
      if (++i == MAX_OPEN_STREAMS)
	i = 0;
    }
  return NULL;
}

/* Open a stream associated with the message mhm. If the stream is
   already open, do nothing.
   NOTE: We could have reused the NULL holes in the msg_pool, but
   that hardly is worth the effort, since the holes appear only when
   expunging. On the other hand this may be useful when MAX_OPEN_STREAMS
   size is very big. "Premature optimization is the root of all evil" */
static int
amd_pool_open (struct _amd_message *mhm)
{
  int status;
  struct _amd_data *amd = mhm->amd;
  if (amd_pool_lookup (mhm))
    return 0;
  if (amd_pool_open_count (amd) == MAX_OPEN_STREAMS-1)
    {
      amd_message_stream_close (amd->msg_pool[amd->pool_first++]);
      amd->pool_first %= MAX_OPEN_STREAMS;
    }
  status = amd_message_stream_open (mhm);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("amd_pool_open: amd_message_stream_open=%s",
		 mu_strerror (status)));
      return status;
    }
  amd->msg_pool[amd->pool_last++] = mhm;
  amd->pool_last %= MAX_OPEN_STREAMS;
  return 0;
}

static void
amd_pool_flush (struct _amd_data *amd)
{
  int i;
  
  for (i = amd->pool_first; i != amd->pool_last; )
    {
      if (amd->msg_pool[i])
	amd_message_stream_close (amd->msg_pool[i]);
      if (++i == MAX_OPEN_STREAMS)
	i = 0;
    }
  amd->pool_first = amd->pool_last = 0;
}

/* Attach a stream to a given message structure. The latter is supposed
   to be already added to the open message pool. */
int
amd_message_stream_open (struct _amd_message *mhm)
{
  struct _amd_data *amd = mhm->amd;
  char *filename;
  int status;
  int flags = 0;

  status = amd->cur_msg_file_name (mhm, 1, &filename);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("amd_message_stream_open: cur_msg_file_name=%s",
		 mu_strerror (status)));
      return status;
    }
  
  /* The message should be at least readable */
  if (amd->mailbox->flags & (MU_STREAM_WRITE|MU_STREAM_APPEND))
    flags |= MU_STREAM_RDWR;
  else 
    flags |= MU_STREAM_READ;
  status = mu_file_stream_create (&mhm->stream, filename, flags);
  if (status)
    mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
	      ("amd_message_stream_open: mu_file_stream_create(%s)=%s",
	       filename, mu_strerror (status)));      

  free (filename);

  if (status != 0)
    return status;

  /* FIXME: Select buffer size dynamically */
  mu_stream_set_buffer (mhm->stream, mu_buffer_full, 16384);
  
  status = amd_scan_message (mhm);
  if (status)
    {
      mu_debug (MU_DEBCAT_MAILBOX, MU_DEBUG_ERROR,
		("amd_message_stream_open: amd_scan_message=%s",
		 mu_strerror (status)));
      amd_message_stream_close (mhm);
    }
  
  return status;
}

/* Close the stream associated with the given message. */
void
amd_message_stream_close (struct _amd_message *mhm)
{
  if (mhm)
    {
      mu_stream_destroy (&mhm->stream);
    }
}

int
amd_check_message (struct _amd_message *mhm)
{
  if (mhm->body_end == 0)
    return amd_pool_open (mhm);
  return 0;
}

/* Reading functions */
static int
amd_body_stream_read (mu_stream_t is, char *buffer, size_t buflen,
		      size_t *pnread)
{
  struct _amd_body_stream *amdstr = (struct _amd_body_stream *)is;
  mu_body_t body = amdstr->body;
  mu_message_t msg = mu_body_get_owner (body);
  struct _amd_message *mhm = mu_message_get_owner (msg);
  size_t nread = 0;
  int status = 0;
  mu_off_t ln;

  status = amd_pool_open (mhm);
  if (status)
    return status;

  if (buffer == NULL || buflen == 0)
    {
      *pnread = nread;
      return 0;
    }

  mu_monitor_rdlock (mhm->amd->mailbox->monitor);
#ifdef WITH_PTHREAD
  /* read() is cancellation point since we're doing a potentially
     long operation.  Lets make sure we clean the state.  */
  pthread_cleanup_push (amd_cleanup, (void *)mhm->amd->mailbox);
#endif

  ln = mhm->body_end - (mhm->body_start + amdstr->off);
  if (ln > 0)
    {
      nread = ((size_t)ln < buflen) ? (size_t)ln : buflen;
      status = mu_stream_seek (mhm->stream, mhm->body_start + amdstr->off,
			       MU_SEEK_SET, NULL);
      if (status == 0)
	{
	  status = mu_stream_read (mhm->stream, buffer, nread, &nread);
	  amdstr->off += nread;
	}
    }

  *pnread = nread;

  mu_monitor_unlock (mhm->amd->mailbox->monitor);
#ifdef WITH_PTHREAD
  pthread_cleanup_pop (0);
#endif

  return status;
}

static int
amd_body_stream_seek (mu_stream_t str, mu_off_t off, mu_off_t *presult)
{
  int rc;
  size_t size;
  struct _amd_body_stream *amdstr = (struct _amd_body_stream *)str;
  
  rc = amd_body_size (amdstr->body, &size);
  if (rc)
    return rc;

  if (off < 0 || off > size)
    return ESPIPE;

  amdstr->off = off;
  if (presult)
    *presult = off;
  return 0;
}

/* Return corresponding sizes */

static int
amd_body_stream_size (mu_stream_t stream, mu_off_t *psize)
{
  mu_body_t body = ((struct _amd_body_stream *)stream)->body;
  size_t size;
  int rc = amd_body_size (body, &size);
  if (rc == 0)
    *psize = size;
  return rc;
}

static int
amd_body_size (mu_body_t body, size_t *psize)
{
  int status;
  mu_message_t msg = mu_body_get_owner (body);
  struct _amd_message *mhm = mu_message_get_owner (msg);
  if (mhm == NULL)
    return EINVAL;
  status = amd_check_message (mhm);
  if (status)
    return status;
  if (psize)
    *psize = mhm->body_end - mhm->body_start;
  return 0;
}

static int
amd_body_lines (mu_body_t body, size_t *plines)
{
  int status;
  mu_message_t msg = mu_body_get_owner (body);
  struct _amd_message *mhm = mu_message_get_owner (msg);
  if (mhm == NULL)
    return EINVAL;
  status = amd_check_message (mhm);
  if (status)
    return status;
  if (plines)
    *plines = mhm->body_lines;
  return 0;
}

/* Headers */
static int
amd_header_fill (void *data, char **pbuf, size_t *plen)
{
  char *buffer;
  size_t len;
  mu_message_t msg = data;
  struct _amd_message *mhm = mu_message_get_owner (msg);
  int status, rc;
  mu_off_t pos;
  
  status = amd_pool_open (mhm);
  if (status)
    return status;

  len = mhm->header_size;
  buffer = malloc (len);
  if (!buffer)
    return ENOMEM;
  
  status = mu_stream_seek (mhm->stream, 0, MU_SEEK_CUR, &pos);
  if (status)
    return status;
  status = mu_stream_seek (mhm->stream, 0, MU_SEEK_SET, NULL);
  if (status)
    return status;

  status = mu_stream_read (mhm->stream, buffer, len, NULL);
  rc = mu_stream_seek (mhm->stream, pos, MU_SEEK_SET, NULL);

  if (!status)
    status = rc;
  
  if (status)
    {
      free (buffer);
      return status;
    }

  *plen = len;
  *pbuf = buffer;
  return 0;
}

/* Attributes */
static int
amd_get_attr_flags (mu_attribute_t attr, int *pflags)
{
  mu_message_t msg = mu_attribute_get_owner (attr);
  struct _amd_message *mhm = mu_message_get_owner (msg);

  if (mhm == NULL)
    return EINVAL;
  if (!(mhm->amd->capabilities & MU_AMD_STATUS))
    {
      /* If AMD implementation doesn't handle status (attribute) bits, they
	 must be retrieved from the Status: header.  To ensure that, the
	 message must be scanned: */
      int rc = amd_check_message (mhm);
      if (rc)
	return rc;
    }
  if (pflags)
    *pflags = mhm->attr_flags;
  return 0;
}

static int
amd_set_attr_flags (mu_attribute_t attr, int flags)
{
  mu_message_t msg = mu_attribute_get_owner (attr);
  struct _amd_message *mhm = mu_message_get_owner (msg);

  if (mhm == NULL)
    return EINVAL;
  mhm->attr_flags |= flags;
  return 0;
}

static int
amd_unset_attr_flags (mu_attribute_t attr, int flags)
{
  mu_message_t msg = mu_attribute_get_owner (attr);
  struct _amd_message *mhm = mu_message_get_owner (msg);

  if (mhm == NULL)
    return EINVAL;
  mhm->attr_flags &= ~flags;
  return 0;
}


int
amd_remove_dir (const char *name)
{
  DIR *dir;
  struct dirent *ent;
  char *namebuf;
  size_t namelen, namesize;
  int rc = 0;
  int has_subdirs = 0;
  
  namelen = strlen (name);
  namesize = namelen + 128;
  namebuf = malloc (namesize);
  if (!namebuf)
    return ENOMEM;
  memcpy (namebuf, name, namelen);
  if (namebuf[namelen - 1] != '/')
    namebuf[namelen++] = '/';
  
  dir = opendir (name);
  if (!dir)
    return errno;
  while ((ent = readdir (dir)))
    {
      struct stat st;
      size_t len;

      if (strcmp (ent->d_name, ".") == 0 ||
	  strcmp (ent->d_name, "..") == 0)
	continue;
      len = strlen (ent->d_name);
      if (namelen + len >= namesize)
	{
	  char *p;

	  namesize += len + 1;
	  p = realloc (namebuf, namesize);
	  if (!p)
	    {
	      rc = ENOMEM;
	      break;
	    }
	}
      strcpy (namebuf + namelen, ent->d_name);
      if (stat (namebuf, &st) == 0 && S_ISDIR (st.st_mode))
	{
	  has_subdirs = 1;
	  continue;
	}
      
      if (unlink (namebuf))
	{
	  rc = errno;
	  mu_diag_output (MU_DIAG_WARNING,
			  "failed to remove %s: %s",
			  namebuf, mu_strerror (rc));
	  break;
	}
    }
  closedir (dir);
  free (namebuf);

  if (rc == 0 && !has_subdirs)
    {
      if (rmdir (name))
	{
	  rc = errno;
	  /* POSIX.1-2001 allows EEXIST to be returned if the directory
	     contained entries other than . and .. */
	  if (rc == EEXIST)
	    rc = ENOTEMPTY;
	}
    }
  return rc;
}

int
amd_reset_uidvalidity (struct _amd_data *amd)
{
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return _amd_prop_store_off (amd, _MU_AMD_PROP_UIDVALIDITY, tv.tv_sec);
}

int
amd_update_uidnext (struct _amd_data *amd, size_t *newval)
{
  int rc;
  size_t curval;
  
  rc = _amd_prop_fetch_size (amd, _MU_AMD_PROP_UIDNEXT, &curval);
  if (rc == MU_ERR_NOENT)
    curval = 1;
  else if (rc)
    return rc;
  if (*newval < curval)
    {
      *newval = curval;
      return 0;
    }
  return _amd_prop_store_off (amd, _MU_AMD_PROP_UIDNEXT, *newval);
}

int
amd_alloc_uid (struct _amd_data *amd, size_t *newval)
{
  int rc;
  size_t retval;
  
  rc = _amd_prop_fetch_size (amd, _MU_AMD_PROP_UIDNEXT, &retval);
  if (rc == MU_ERR_NOENT)
    retval = 1;
  else if (rc)
    return rc;
  rc = _amd_prop_store_off (amd, _MU_AMD_PROP_UIDNEXT, retval + 1);
  if (rc)
    return rc;
  *newval = retval;
  return 0;
}


