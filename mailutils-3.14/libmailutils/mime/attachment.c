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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif

#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/body.h>
#include <mailutils/filter.h>
#include <mailutils/header.h>
#include <mailutils/message.h>
#include <mailutils/stream.h>
#include <mailutils/errno.h>
#include <mailutils/util.h>
#include <mailutils/io.h>

struct _mu_mime_io_buffer
{
  unsigned int refcnt;
  char *charset;
  mu_header_t hdr;
  mu_message_t msg;
  mu_stream_t stream;	/* output file/decoding stream for saving attachment */
  mu_stream_t fstream;	/* output file stream for saving attachment */
};

static int
at_hdr (mu_header_t hdr, const char *content_type, const char *encoding,
	const char *name, const char *filename)
{
  int rc;
  char *val, *str;

  if (filename)
    {
      str = strrchr (filename, '/');
      if (str)
	filename = str + 1;
    }
  
  if (!name)
    {
      name = filename;
    }

  if (name)
    {
      rc = mu_c_str_escape (name, "\\\"", NULL, &str);
      if (rc)
	return rc;
      rc = mu_asprintf (&val, "%s; name=\"%s\"", content_type, str);
      free (str);
      if (rc)
	return rc;
      rc = mu_header_set_value (hdr, MU_HEADER_CONTENT_TYPE, val, 1);
      free (val);
    }
  else
    rc = mu_header_set_value (hdr, MU_HEADER_CONTENT_TYPE, content_type, 1);
  
  if (rc)
    return rc;
  
  if (filename)
    {
      rc = mu_c_str_escape (filename, "\\\"", NULL, &str);
      if (rc)
	return rc;
      rc = mu_asprintf (&val, "%s; filename=\"%s\"", "attachment", str);
      free (str);
      if (rc)
	return rc;
      rc = mu_header_set_value (hdr, MU_HEADER_CONTENT_DISPOSITION, val, 1);
      free (val);
    }
  else
    rc = mu_header_set_value (hdr, MU_HEADER_CONTENT_DISPOSITION, "attachment",
			      1);
  if (rc)
    return rc;
  return mu_header_set_value (hdr, MU_HEADER_CONTENT_TRANSFER_ENCODING,
			      encoding ? encoding : "8bit", 1);
}

/* Create in *NEWMSG an empty attachment of given CONTENT_TYPE and ENCODING.
   NAME, if not NULL, supplies the name of the attachment.
   FILENAME, if not NULL, gives the file name.
 */
int
mu_attachment_create (mu_message_t *newmsg,
		      const char *content_type, const char *encoding,
		      const char *name,
		      const char *filename)
{
  int rc;
  mu_header_t hdr;
  
  if (newmsg == NULL)
    return MU_ERR_OUT_PTR_NULL;

  rc = mu_message_create (newmsg, NULL);
  if (rc)
    return rc;

  rc = mu_header_create (&hdr, NULL, 0);
  if (rc)
    {
      mu_message_destroy (newmsg, NULL);
      return rc;
    }
  mu_message_set_header (*newmsg, hdr, NULL);

  rc = at_hdr (hdr, content_type, encoding, name, filename);

  if (rc)
    mu_message_destroy (newmsg, NULL);
  
  return rc;
}

/* ATT is an attachment created by a previous call to mu_attachment_create().

   Fills in the attachment body with the data from STREAM using the encoding
   stored in the Content-Transfer-Encoding header of ATT.
 */
int
mu_attachment_copy_from_stream (mu_message_t att, mu_stream_t stream)
{
  mu_body_t body;
  mu_stream_t bstr;
  mu_stream_t tstream;
  mu_header_t hdr;
  int rc;
  char *encoding;
  
  mu_message_get_header (att, &hdr);
  rc = mu_header_aget_value_unfold (hdr, MU_HEADER_CONTENT_TRANSFER_ENCODING,
				    &encoding);
  switch (rc)
    {
    case 0:
      break;
      
    case MU_ERR_NOENT:
      return EINVAL;

    default:
      return rc;
    }
  
  mu_message_get_body (att, &body);
  rc = mu_body_get_streamref (body, &bstr);
  if (rc == 0)
    {
      rc = mu_filter_create (&tstream, stream, encoding, MU_FILTER_ENCODE,
			     MU_STREAM_READ);
      if (rc == 0)
	{
	  rc = mu_stream_copy (bstr, tstream, 0, NULL);
	  mu_stream_unref (tstream);
	}
      mu_stream_unref (bstr);
    }
  free (encoding);
  return rc;
}

/* ATT is an attachment created by a previous call to mu_attachment_create().

   Fills in the attachment body with the data from FILENAME using the encoding
   specified in the Content-Transfer-Encoding header.
 */
int
mu_attachment_copy_from_file (mu_message_t att, char const *filename)
{
  mu_stream_t stream;
  int rc;

  rc = mu_file_stream_create (&stream, filename, MU_STREAM_READ);
  if (rc == 0)
    {
      rc = mu_attachment_copy_from_stream (att, stream);
      mu_stream_unref (stream);
    }
  return rc;
} 

int
mu_message_create_attachment (const char *content_type, const char *encoding,
			      const char *filename, mu_message_t *newmsg)
{
  int rc;
  mu_message_t att;
  
  if (content_type == NULL)
    content_type = "text/plain";

  rc = mu_attachment_create (&att, content_type, encoding, NULL, filename);
  if (rc == 0)
    {
      rc = mu_attachment_copy_from_file (att, filename);
      if (rc)
	mu_message_destroy (&att, NULL);
    }
  if (rc == 0)
    *newmsg = att;

  return rc;
}

int
mu_mime_io_buffer_create (mu_mime_io_buffer_t *pinfo)
{
  mu_mime_io_buffer_t info;
  
  if ((info = calloc (1, sizeof (*info))) == NULL)
    return ENOMEM;
  info->refcnt = 1;
  *pinfo = info;
  return 0;
}

int
mu_mime_io_buffer_set_charset (mu_mime_io_buffer_t info, const char *charset)
{
  char *cp = strdup (charset);
  if (!cp)
    return ENOMEM;
  free (info->charset);
  info->charset = cp;
  return 0;
}

void
mu_mime_io_buffer_sget_charset (mu_mime_io_buffer_t info, const char **charset)
{
  *charset = info->charset;
}

int
mu_mime_io_buffer_aget_charset (mu_mime_io_buffer_t info, const char **charset)
{
  *charset = strdup (info->charset);
  if (!charset)
    return ENOMEM;
  return 0;
}

void
mu_mime_io_buffer_destroy (mu_mime_io_buffer_t *pinfo)
{
  if (pinfo && *pinfo)
    {
      mu_mime_io_buffer_t info = *pinfo;
      free (info->charset);
      free (info);
      *pinfo = NULL;
    }
}

static void
_attachment_free (struct _mu_mime_io_buffer *info, int free_message)
{
  if (free_message)
    {
      if (info->msg)
	mu_message_destroy (&info->msg, NULL);
      else if (info->hdr)
	mu_header_destroy (&info->hdr);
    }
  info->msg = NULL;
  info->hdr = NULL;
  info->stream = NULL;
  info->fstream = NULL;
  if (--info->refcnt == 0)
    {
      free (info->charset);
      free (info);
    }
}

static int
_attachment_setup (mu_mime_io_buffer_t *pinfo, mu_message_t msg,
		   mu_stream_t *pstream)
{
  int ret;
  mu_body_t body;
  mu_mime_io_buffer_t info;
  mu_stream_t stream;
    
  if ((ret = mu_message_get_body (msg, &body)) != 0 ||
      (ret = mu_body_get_streamref (body, &stream)) != 0)
    return ret;
  *pstream = stream;
  if (*pinfo)
    {
      info = *pinfo;
      info->refcnt++;
    }
  else
    {
      ret = mu_mime_io_buffer_create (&info);
      if (ret)
	return ret;
    }
  
  *pinfo = info;
  return 0;
}

int
mu_message_save_attachment (mu_message_t msg, const char *filename,
			    mu_mime_io_buffer_t info)
{
  mu_stream_t istream;
  int ret;
  mu_header_t hdr;
  const char *fname = NULL;
  char *partname = NULL;

  if (msg == NULL)
    return EINVAL;

  if ((ret = _attachment_setup (&info, msg, &istream)) != 0)
    return ret;

  if (ret == 0 && (ret = mu_message_get_header (msg, &hdr)) == 0)
    {
      if (filename == NULL)
	{
	  ret = mu_message_aget_decoded_attachment_name (msg, info->charset,
							 &partname, NULL);
	  if (partname)
	    fname = partname;
	}
      else
	fname = filename;
      if (fname
	  && (ret = mu_file_stream_create (&info->fstream, fname,
				     MU_STREAM_WRITE | MU_STREAM_CREAT)) == 0)
	{
	  const char *content_encoding;

	  if (mu_header_sget_value (hdr, MU_HEADER_CONTENT_TRANSFER_ENCODING,
				    &content_encoding))
	    content_encoding = "7bit";
	  ret = mu_filter_create (&info->stream, istream, content_encoding,
				  MU_FILTER_DECODE,
				  MU_STREAM_READ);
	}
    }
  if (info->stream && istream)
    ret = mu_stream_copy (info->fstream, info->stream, 0, NULL);

  if (ret != EAGAIN && info)
    {
      mu_stream_close (info->fstream);
      mu_stream_destroy (&info->stream);
      mu_stream_destroy (&info->fstream);
    }

  mu_stream_destroy (&istream);
  _attachment_free (info, ret); /* FIXME: or 0? */
  
  /* Free fname if we allocated it. */
  if (partname)
    free (partname);

  return ret;
}

int
mu_message_encapsulate (mu_message_t msg, mu_message_t *newmsg,
			mu_mime_io_buffer_t info)
{
  mu_stream_t istream, ostream;
  int ret = 0;
  mu_message_t tmsg = NULL;
  
  if (newmsg == NULL)
    return MU_ERR_OUT_PTR_NULL;

  if (msg == NULL)
    {
      mu_header_t hdr;
      
      ret = mu_message_create (&tmsg, NULL);
      if (ret)
	return ret;
      msg = tmsg;
#define MSG822_HEADER "Content-Type: message/rfc822\n" \
 	              "Content-Transfer-Encoding: 7bit\n\n"
      if ((ret = mu_header_create (&hdr,
				   MSG822_HEADER,
				   sizeof (MSG822_HEADER) - 1)) == 0)
	ret = mu_message_set_header (msg, hdr, NULL);
#undef MSG822_HEADER
      if (ret)
	{
	  mu_message_destroy (&msg, NULL);
	  return ret;
	}
    }
      
  if ((ret = _attachment_setup (&info, msg, &ostream)) != 0)
    {
      mu_message_destroy (&tmsg, NULL);
      return ret;
    }
  info->msg = msg;
  if (ret == 0 && (ret = mu_message_get_streamref (msg, &istream)) == 0)
    {
      mu_stream_seek (istream, 0, MU_SEEK_SET, NULL);
      ret = mu_stream_copy (ostream, istream, 0, NULL);
      mu_stream_destroy (&istream);
    }
  if (ret == 0)
    *newmsg = info->msg;
  mu_stream_destroy (&ostream);
  _attachment_free (info, ret && ret != EAGAIN);
  return ret;
}

#define MESSAGE_RFC822_STR "message/rfc822"

int
mu_message_unencapsulate (mu_message_t msg, mu_message_t *newmsg,
			  mu_mime_io_buffer_t info)
{
  int ret = 0;
  mu_header_t hdr;
  mu_stream_t istream;

  if (msg == NULL)
    return EINVAL;
  if (newmsg == NULL)
    return MU_ERR_OUT_PTR_NULL;

  if (info == NULL /* FIXME: not needed? */
      && (ret = mu_message_get_header (msg, &hdr)) == 0)
    {
      const char *s;
      if (!(mu_header_sget_value (hdr, MU_HEADER_CONTENT_TYPE, &s) == 0 &&
	    mu_c_strncasecmp (s, MESSAGE_RFC822_STR,
			      sizeof (MESSAGE_RFC822_STR) - 1) == 0))
	return EINVAL;
    }
  if ((ret = _attachment_setup (&info, msg, &istream)) != 0)
    return ret;
  ret = mu_stream_to_message (istream, &info->msg);
  mu_stream_unref (istream);
  if (ret == 0)
    *newmsg = info->msg;
  _attachment_free (info, ret && ret != EAGAIN);
  return ret;
}
