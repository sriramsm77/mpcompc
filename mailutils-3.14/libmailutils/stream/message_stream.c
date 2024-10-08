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

/* This file implements an MH draftfile stream: a read-only stream used
   to transparently pass MH draftfiles to mailers. The only difference
   between the usual RFC822 and MH draft is that the latter allows to use
   a string of dashes to separate the headers from the body. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <mailutils/types.h>
#include <mailutils/address.h>
#include <mailutils/alloc.h>
#include <mailutils/envelope.h>
#include <mailutils/message.h>
#include <mailutils/header.h>
#include <mailutils/body.h>
#include <mailutils/attribute.h>
#include <mailutils/stream.h>
#include <mailutils/util.h>
#include <mailutils/datetime.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/sys/message_stream.h>


static int
envelope_substr (struct _mu_message_stream *str,
		 struct mu_substring_location *loc,
		 char *buf, size_t len, size_t *pnwrite)
{
  if (buf)
    {
      if (len == 0)
	return EINVAL;
      --len;
      if (len > loc->length)
	len = loc->length;
      strncpy (buf, str->envelope_string + loc->start, len);
      buf[len] = 0;
      if (pnwrite)
	*pnwrite = len;
    }
  else if (!pnwrite)
    return EINVAL;
  else
    *pnwrite = loc->length;
  return 0;
}

static int
_env_msg_date (mu_envelope_t envelope, char *buf, size_t len, size_t *pnwrite)
{
  struct _mu_message_stream *str =
    mu_message_get_owner (mu_envelope_get_owner (envelope));
  
  if (!str)
    return EINVAL;
  return envelope_substr (str, &str->date, buf, len, pnwrite);
}

static int
_env_msg_sender (mu_envelope_t envelope, char *buf, size_t len,
		 size_t *pnwrite)
{
  struct _mu_message_stream *str =
    mu_message_get_owner (mu_envelope_get_owner (envelope));
  
  if (!str)
    return EINVAL;
  return envelope_substr (str, &str->from, buf, len, pnwrite);
}


static int
_message_read (mu_stream_t stream, char *optr, size_t osize, size_t *nbytes)
{
  int rc;
  struct _mu_message_stream *s = (struct _mu_message_stream*) stream;
  mu_off_t offset = s->offset + s->envelope_length;
  size_t rsize;
  
  if (offset < s->mark.start)
    {
      if (offset + osize >= s->mark.start)
	osize = s->mark.start - offset;
    }
  else
    offset += s->mark.length;
  /* FIXME: Seeking each time before read is awkward. The streamref
     should be modified to take care of it */
  rc = mu_stream_seek (s->transport, offset, MU_SEEK_SET, NULL);
  if (rc == 0)
    rc = mu_stream_read (s->transport, optr, osize, &rsize);
  if (rc == 0)
    {
      s->offset += rsize;
      if (nbytes)
	*nbytes = rsize;
    }
  else
    s->stream.last_err = rc;
  return rc;
}
  
static int
_message_size (mu_stream_t stream, mu_off_t *psize)
{
  struct _mu_message_stream *s = (struct _mu_message_stream*) stream;
  int rc = mu_stream_size (s->transport, psize);
  
  if (rc == 0)
    *psize -= s->envelope_length + s->mark.length;
  return rc;
}

static int
is_header_start (const char *buf)
{
  for (; *buf; buf++)
    {
      if (mu_isalnum (*buf) || *buf == '-' || *buf == '_')
	continue;
      if (*buf == ':')
	return 1;
    }
  return 0;
}

static int
is_header_cont (const char *buf)
{
  return mu_isblank (*buf);
}

static int
_message_open (mu_stream_t stream)
{
  struct _mu_message_stream *str = (struct _mu_message_stream*) stream;
  int rc;
  char *buffer = NULL;
  size_t bufsize = 0;
  size_t offset, len;
  mu_off_t body_start, body_end;
  mu_stream_t transport = str->transport;
  int has_headers = 0;
  
  rc = mu_stream_seek (transport, 0, MU_SEEK_SET, NULL);
  if (rc)
    return rc;
  offset = 0;
  while ((rc = mu_stream_getline (transport, &buffer, &bufsize, &len)) == 0
	 && len > 0)
    {
      if (offset == 0 && memcmp (buffer, "From ", 5) == 0)
	{
	  str->envelope_length = len;
	  if (str->construct_envelope)
	    {
	      len = mu_rtrim_class (buffer, MU_CTYPE_SPACE);
	      str->envelope_string = strdup (buffer);
	      if (!str->envelope_string)
		return ENOMEM;

	      str->from.start = 5;
	      str->from.length =
		strcspn (str->envelope_string + str->from.start, " \t");

	      str->date.start = str->from.start + str->from.length +
		strspn (str->envelope_string + str->from.start +
			str->from.length, " \t");
	      str->date.length = len - str->date.start;

	      str->construct_envelope = 0;
	    }
	}
      else if (mu_mh_delim (buffer))
	{
	  str->mark.start = offset;
	  str->mark.length = len - 1; /* do not count the terminating
					 newline */
	  break;
	}
      else
	{
	  if (!is_header_start (buffer))
	    {
	      if (has_headers && is_header_cont (buffer))
		{
		  offset += len;
		  continue;
		}
	      return MU_ERR_INVALID_EMAIL;
	    }
	  has_headers = 1;
	}
      offset += len;
    }

  free (buffer);

  rc = mu_stream_seek (transport, 0, MU_SEEK_CUR, &body_start);
  if (rc)
    return rc;
  rc = mu_stream_size (transport, &body_end);
  if (rc)
    return rc;
  
  str->body_start = body_start;
  str->body_end = body_end - 1;
  
  return 0;
}

static int
_message_close (mu_stream_t stream)
{
  struct _mu_message_stream *s = (struct _mu_message_stream*) stream;
  return s->stream.last_err = mu_stream_close (s->transport);
}

static void
_message_done (mu_stream_t stream)
{
  struct _mu_message_stream *s = (struct _mu_message_stream*) stream;

  free (s->envelope_string);
  mu_stream_destroy (&s->transport);
}

static int
_message_seek (struct _mu_stream *stream, mu_off_t off, mu_off_t *presult)
{ 
  struct _mu_message_stream *s = (struct _mu_message_stream*) stream;
  mu_off_t size;

  mu_stream_size (stream, &size);
  if (off < 0 || off >= size)
    return ESPIPE;
  s->offset = off;
  *presult = off;
  return 0;
}

const char *
_message_error_string (struct _mu_stream *stream, int rc)
{
  struct _mu_message_stream *str = (struct _mu_message_stream*) stream;
  return mu_stream_strerror (str->transport, rc);
}

static int
mu_message_stream_create (mu_stream_t *pstream, mu_stream_t src, int flags,
			  int construct_envelope)
{
  struct _mu_message_stream *s;
  int sflag;
  int rc;
  mu_stream_t stream;
  
  mu_stream_get_flags (src, &sflag);
  sflag &= MU_STREAM_SEEK;
  
  if (!flags)
    flags = MU_STREAM_READ;
  if (flags & (MU_STREAM_WRITE|MU_STREAM_CREAT|MU_STREAM_APPEND))
    return EINVAL;
  s = (struct _mu_message_stream *) _mu_stream_create (sizeof (*s),
						       flags | sflag);
  if (!s)
    return ENOMEM;

  rc = mu_streamref_create (&s->transport, src);
  if (rc)
    {
      free (s);
      return rc;
    }
  s->construct_envelope = construct_envelope;
  s->stream.open = _message_open;
  s->stream.close = _message_close;
  s->stream.done = _message_done;
  s->stream.read = _message_read;
  s->stream.size = _message_size;
  s->stream.seek = _message_seek;
  s->stream.error_string = _message_error_string;

  stream = (mu_stream_t)s;
  rc = mu_stream_open (stream);
  if (rc)
    mu_stream_destroy (&stream);
  else
    *pstream = stream;
  return rc;
}


/* *************************** MH draft message **************************** */



static int
_body_obj_size (mu_body_t body, size_t *size)
{
  mu_message_t msg = mu_body_get_owner (body);
  struct _mu_message_stream *str = mu_message_get_owner (msg);

  if (size)
    *size = str->body_end - str->body_start + 1;
  return 0;
}

int
mu_message_from_stream_with_envelope (mu_message_t *pmsg,
				      mu_stream_t instream,
				      mu_envelope_t env)
{
  mu_message_t msg;
  mu_body_t body;
  mu_header_t hdr;
  mu_stream_t bstream;
  mu_stream_t draftstream;
  int rc;
  struct _mu_message_stream *sp;
  
  /* FIXME: Perhaps MU_STREAM_NO_CLOSE is needed */
  if ((rc = mu_message_stream_create (&draftstream, instream, 0, !env)))
    return rc;

  if ((rc = mu_message_create (&msg, draftstream)))
    {
      mu_stream_destroy (&draftstream);
      return rc;
    }
  
  mu_message_set_stream (msg, draftstream, draftstream);

  sp = (struct _mu_message_stream *) draftstream;

  if (!env)
    {
      if (sp->construct_envelope)
	{
	  /*
	   * FIXME: Currently the envelope *must* be owned by the message,
	   * otherwise _mu_message_free won't destroy it.
	   * The same holds true for attribute and body.
	   */
	  rc = mu_message_reconstruct_envelope (msg, &env);
	  sp->construct_envelope = 0;
	}
      else if ((rc = mu_envelope_create (&env, msg)) == 0)
	{
	  mu_envelope_set_date (env, _env_msg_date, msg);
	  mu_envelope_set_sender (env, _env_msg_sender, msg);
	}

      if (rc)
	{
	  mu_message_destroy (&msg, draftstream);
	  mu_stream_destroy (&draftstream);
	  return rc;
	}
    }
  mu_message_set_envelope (msg, env, draftstream);

  if (mu_message_get_header (msg, &hdr) == 0)
    {
      char const *s;
      if (mu_header_sget_value (hdr, MU_HEADER_STATUS, &s) == 0)
	{
	  mu_attribute_t atr;
	  int flags = 0;
	  mu_attribute_string_to_flags (s, &flags);
	  mu_message_get_attribute (msg, &atr);
	  mu_attribute_unset_flags (atr, ~flags);
	  mu_attribute_set_flags (atr, flags);
	}
    }
  
  mu_body_create (&body, msg);

  rc = mu_streamref_create_abridged (&bstream, instream,
				     sp->body_start, sp->body_end);
  if (rc)
    {
      mu_body_destroy (&body, msg);
      mu_message_destroy (&msg, draftstream);
      mu_stream_destroy (&draftstream);
      return rc;
    }
  
  mu_body_set_stream (body, bstream, msg);
  mu_body_set_size (body, _body_obj_size, msg);
  mu_message_set_body (msg, body, draftstream);

  *pmsg = msg;
  return 0;
}

int
mu_stream_to_message (mu_stream_t instream, mu_message_t *pmsg)
{
  return mu_message_from_stream_with_envelope (pmsg, instream, NULL);
}
