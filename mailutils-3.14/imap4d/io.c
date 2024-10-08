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

#include "imap4d.h"
#include <mailutils/property.h>
#include <mailutils/datetime.h>

mu_stream_t iostream;

static void
log_cipher (mu_stream_t stream)
{
  mu_property_t prop;
  int rc = mu_stream_ioctl (stream, MU_IOCTL_TLSSTREAM,
			    MU_IOCTL_TLS_GET_CIPHER_INFO, &prop);
  if (rc)
    {
      mu_diag_output (MU_DIAG_INFO, _("TLS established"));
      mu_diag_output (MU_DIAG_ERROR, _("can't get TLS details: %s"),
		      mu_strerror (rc));
    }
  else
    {
      char const *cipher, *mac, *proto;
      if (mu_property_sget_value (prop, "cipher", &cipher))
	cipher = "UNKNOWN";	
      if (mu_property_sget_value (prop, "mac", &mac))
	mac = "UNKNOWN";
      if (mu_property_sget_value (prop, "protocol", &proto))
	proto = "UNKNOWN";
      
      mu_diag_output (MU_DIAG_INFO, _("TLS established using %s-%s (%s)"),
		      cipher, mac, proto);
      
      mu_property_destroy (&prop);
    }
}

void
io_setio (int ifd, int ofd, struct mu_tls_config *tls_conf)
{
  mu_stream_t str, istream, ostream;
  int rc;
  
  if (ifd == -1)
    imap4d_bye (ERR_NO_IFILE);
  if (ofd == -1)
    imap4d_bye (ERR_NO_OFILE);

  if (tls_conf)
    {
      rc = mu_tlsfd_stream_create (&str, ifd, ofd, tls_conf, MU_TLS_SERVER);
      if (rc)
	{
	  mu_error (_("failed to create TLS stream: %s"), mu_strerror (rc));
	  imap4d_bye (ERR_STREAM_CREATE);
	}
      log_cipher (str);
    }
  else
    {
      if (mu_stdio_stream_create (&istream, ifd, MU_STREAM_READ))
	imap4d_bye (ERR_STREAM_CREATE);
      mu_stream_set_buffer (istream, mu_buffer_line, 0);
  
      if (mu_stdio_stream_create (&ostream, ofd, MU_STREAM_WRITE))
	imap4d_bye (ERR_STREAM_CREATE);
      mu_stream_set_buffer (ostream, mu_buffer_line, 0);
  
      /* Combine the two streams into an I/O one. */
      if (mu_iostream_create (&str, istream, ostream))
	imap4d_bye (ERR_STREAM_CREATE);

      mu_stream_unref (istream);
      mu_stream_unref (ostream);
    }
  
  /* Convert all writes to CRLF form.
     There is no need to convert reads, as the code ignores extra \r anyway.
  */
  rc = mu_filter_create (&iostream, str, "CRLF", MU_FILTER_ENCODE,
			 MU_STREAM_WRITE | MU_STREAM_RDTHRU);
  mu_stream_unref (str);
  if (rc)
    imap4d_bye (ERR_STREAM_CREATE);
  /* Change buffering scheme: filter streams are fully buffered by default. */
  mu_stream_set_buffer (iostream, mu_buffer_line, 0);
  
  if (imap4d_transcript)
    {
      mu_stream_t dstr, xstr;
      
      rc = mu_dbgstream_create (&dstr, MU_DIAG_DEBUG);
      if (rc)
	mu_error (_("cannot create debug stream; transcript disabled: %s"),
		  mu_strerror (rc));
      else
	{
	  rc = mu_xscript_stream_create (&xstr, iostream, dstr, NULL);
	  mu_stream_unref (dstr);
	  if (rc)
	    mu_error (_("cannot create transcript stream: %s"),
		      mu_strerror (rc));
	  else
	    {
	      mu_stream_unref (iostream);
	      iostream = xstr;
	    }
	}
    }
}

int
imap4d_init_tls_server (struct mu_tls_config *tls_conf)
{
  int rc;
  
  rc = mu_starttls (&iostream, tls_conf, MU_TLS_SERVER);
  if (rc == 0)
    log_cipher (iostream);
  else
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_starttls", NULL, rc);
      if (rc == MU_ERR_TRANSPORT_SET)
	imap4d_bye (ERR_NO_IFILE);
    }
  return rc;
}

/* Status Code to String.  */
static const char *
sc2string (int rc)
{
  switch (rc)
    {
    case RESP_OK:
      return "OK ";

    case RESP_BAD:
      return "BAD ";

    case RESP_NO:
      return "NO ";

    case RESP_BYE:
      return "BYE ";

    case RESP_PREAUTH:
      return "PREAUTH ";
    }
  return "";
}

/* FIXME: Check return values from the output functions */

int
io_copy_out (mu_stream_t str, size_t size)
{
  int rc;
  struct mu_buffer_query oldbuf, newbuf;
  
  oldbuf.type = MU_TRANSPORT_OUTPUT;
  if (mu_stream_ioctl (iostream, MU_IOCTL_TRANSPORT_BUFFER,
		       MU_IOCTL_OP_GET, &oldbuf) == 0)
    {
      newbuf.type = MU_TRANSPORT_OUTPUT;
      newbuf.buftype = mu_buffer_full;
      newbuf.bufsize = 64*1024;
      mu_stream_ioctl (iostream, MU_IOCTL_TRANSPORT_BUFFER,
		       MU_IOCTL_OP_SET, &newbuf);
    }

  rc = mu_stream_copy (iostream, str, size, NULL);

  mu_stream_ioctl (iostream, MU_IOCTL_TRANSPORT_BUFFER,
		   MU_IOCTL_OP_SET, &oldbuf);

  return rc;
}

int
io_send_bytes (const char *buf, size_t size)
{
  return mu_stream_write (iostream, buf, size, NULL);
}

int
io_sendf (const char *format, ...)
{
  int status;
  va_list ap;

  va_start (ap, format);
  status = mu_stream_vprintf (iostream, format, ap);
  va_end (ap);
  return status;
}

/* Send NIL if empty string, change the quoted string to a literal if the
   string contains: double quotes, CR, LF, and '\\'.  CR, LF will be changed
   to spaces.  */
int
io_send_qstring (const char *buffer)
{
  if (buffer == NULL || *buffer == '\0')
    return io_sendf ("NIL");
  if (strpbrk (buffer, "\"\r\n\\"))
    {
      char *s;
      int ret;
      char *b = mu_strdup (buffer);
      while ((s = strchr (b, '\n')) || (s = strchr (b, '\r')))
	*s = ' ';
      ret = io_send_literal (b);
      free (b);
      return ret;
    }
  return io_sendf ("\"%s\"", buffer);
}

int
io_send_astring (const char *buffer)
{
  if (buffer == NULL)
    return io_sendf ("NIL");
  else if (*buffer == 0)
    return io_sendf ("\"\"");
  if (strpbrk (buffer, "\"\r\n\\"))
    {
      char *s;
      int ret;
      char *b = mu_strdup (buffer);
      while ((s = strchr (b, '\n')) || (s = strchr (b, '\r')))
	*s = ' ';
      ret = io_send_literal (b);
      free (b);
      return ret;
    }
  return io_sendf ("\"%s\"", buffer);
}

int
io_send_literal (const char *buffer)
{
  return io_sendf ("{%lu}\n%s", (unsigned long) strlen (buffer), buffer);
}

/* Send an untagged response.  */
int
io_untagged_response (int rc, const char *format, ...)
{
  int status;
  va_list ap;

  if (iostream == NULL)
    {
      if (rc == RESP_BYE)
	/* RESP_BYE can be emitted when iostream has not been initialized,
	   e.g. when a recently started child receives termination signal
	   during initialization. */
	return 0;
      mu_diag_output (MU_DIAG_ERROR,
		      /* TRANSLATORS: %s is replaced with the untagged response
			 name, followed by a space character. */
		      _("iostream is NULL while trying to send the %suntagged response"),
		      sc2string (rc));
      exit (EX_SOFTWARE);
    }
  
  mu_stream_printf (iostream, "* %s", sc2string (rc));
  va_start (ap, format);
  status = mu_stream_vprintf (iostream, format, ap);
  va_end (ap);
  mu_stream_write (iostream, "\n", 1, NULL);
  return status;
}

/* Send the completion response and reset the state.  */
int
io_format_completion_response (mu_stream_t str,
			       struct imap4d_command *command, int rc, 
			       const char *format, va_list ap)
{
  int new_state;
  int status = 0;
  const char *sc = sc2string (rc);

  imap4d_sync ();
  
  mu_stream_printf (str, "%s %s%s ",
		    command->tag, sc, command->name);
  mu_stream_vprintf (str, format, ap);
  mu_stream_write (str, "\n", 1, NULL);

  /* Reset the state.  */
  if (rc == RESP_OK)
    new_state = command->success;
  else if (command->failure <= state)
    new_state = command->failure;
  else
    new_state = STATE_NONE;

  if (new_state != STATE_NONE)
    {
      if (new_state == STATE_AUTH)
	set_xscript_level (MU_XSCRIPT_NORMAL);
      state = new_state;
    }
  
  return status;
}

int
io_completion_response (struct imap4d_command *command, int rc, 
                        const char *format, ...)
{
  va_list ap;
  int status;

  va_start (ap, format);
  status = io_format_completion_response (iostream, command, rc, format, ap);
  va_end (ap);
  return status;
}

int
io_stream_completion_response (mu_stream_t str,
			       struct imap4d_command *command, int rc, 
			       const char *format, ...)
{
  va_list ap;
  int status;
  
  va_start (ap, format);
  status = io_format_completion_response (str, command, rc, format, ap);
  va_end (ap);
  return status;
}

void
io_flush ()
{
  mu_stream_flush (iostream);
}

void
io_getline (char **pbuf, size_t *psize, size_t *pnbytes)
{
  size_t len;
  int rc;
  struct timeval tv, *to;

  if (idle_timeout)
    {
      tv.tv_sec = idle_timeout;
      tv.tv_usec = 0;
      to = &tv;
    }
  else
    to = NULL;

  rc = mu_stream_timed_getline (iostream, pbuf, psize, to, &len);
  if (rc == 0)
    {
      char *s = *pbuf;

      if (len == 0)
        {
          imap4d_bye (ERR_NO_IFILE);
          /*FIXME rc = ECONNABORTED;*/
        }
      len = mu_rtrim_class (s, MU_CTYPE_ENDLN);
      if (pnbytes)
	*pnbytes = len;
    }
  else if (rc == MU_ERR_TIMEOUT)
    imap4d_bye (ERR_TIMEOUT);
  else
    {
      mu_error (_("read error: %s"), mu_strerror (rc));
      imap4d_bye (ERR_NO_IFILE);
    }
}


static size_t
unquote (char *line, size_t len)
{
  char *prev = NULL;
  size_t rlen = len;
  char *p;
  int off = 0;
  while ((p = memchr (line + off, '\\', len - off)))
    {
      if (p[1] == '\\' || p[1] == '"')
	{
	  if (prev)
	    {
	      memmove (prev, line, p - line);
	      prev += p - line;
	    }
	  else
	    prev = p;
	  off = p[1] == '\\';
	  rlen--;
	  len -= p - line + 1;
	  line = p + 1;
	}
    }
  if (prev)
    memmove (prev, line, len);
  return rlen;
}

struct imap4d_tokbuf
{
  char *buffer;
  size_t size;
  size_t level;
  int argc;
  int argmax;
  size_t *argp;
};

struct imap4d_tokbuf *
imap4d_tokbuf_init ()
{
  struct imap4d_tokbuf *tok = mu_alloc (sizeof (tok[0]));
  memset (tok, 0, sizeof (*tok));
  return tok;
}

void
imap4d_tokbuf_destroy (struct imap4d_tokbuf **ptok)
{
  struct imap4d_tokbuf *tok = *ptok;
  free (tok->buffer);
  free (tok->argp);
  free (tok);
  *ptok = NULL;
}

int
imap4d_tokbuf_argc (struct imap4d_tokbuf *tok)
{
  return tok->argc;
}

char *
imap4d_tokbuf_getarg (struct imap4d_tokbuf *tok, int n)
{
  if (n < tok->argc)
    return tok->buffer + tok->argp[n];
  return NULL;
}

static void
imap4d_tokbuf_unquote (struct imap4d_tokbuf *tok, size_t *poff, size_t *plen)
{
  char *buf = tok->buffer + *poff;
  if (buf[0] == '"' && buf[*plen - 1] == '"')
    {
      ++*poff;
      *plen = unquote (buf + 1, *plen - 1);
    }
}

static void
imap4d_tokbuf_decrlf (struct imap4d_tokbuf *tok, size_t off, size_t *plen)
{
  char *buf = tok->buffer + off;
  size_t len = *plen;
  char *p, *end = buf + len;

  for (p = buf; p < end; )
    {
      if (*p == '\r' && p + 1 < end && p[1] == '\n')
	{
	  p++;
	  len--;
	}
      else
	*buf++ = *p++;
    }
  *plen = len;
}	  

static void
imap4d_tokbuf_expand (struct imap4d_tokbuf *tok, size_t size)
{
  if (tok->size - tok->level < size)	       
    {						
      tok->size = tok->level + size;
      tok->buffer = realloc (tok->buffer, tok->size);
      if (!tok->buffer)				
	imap4d_bye (ERR_NO_MEM);
    }
}

#define ISDELIM(c) (strchr ("()", (c)) != NULL)

static size_t
insert_nul (struct imap4d_tokbuf *tok, size_t off)
{
  imap4d_tokbuf_expand (tok, 1);
  if (off < tok->level)
    {
      memmove (tok->buffer + off + 1, tok->buffer + off, tok->level - off);
      tok->level++;
    }
  tok->buffer[off] = 0;
  return off + 1;
}

static size_t
gettok (struct imap4d_tokbuf *tok, size_t off)
{
  char *buf = tok->buffer;
  
  while (off < tok->level && mu_isblank (buf[off]))
    off++;

  if (tok->argc == tok->argmax)
    {
      if (tok->argmax == 0)
	tok->argmax = 16;
      else
	tok->argmax *= 2;
      tok->argp = realloc (tok->argp, tok->argmax * sizeof (tok->argp[0]));
      if (!tok->argp)
	imap4d_bye (ERR_NO_MEM);
    }
  
  if (buf[off] == '"')
    {
      char *start = buf + off + 1;
      char *p = NULL;
      
      while (*start && (p = strchr (start, '"')))
	{
	  if (p == start || p[-1] != '\\')
	    break;
	  start = p + 1;
	}

      if (p)
	{
	  size_t len;
	  off++;
	  len  = unquote (buf + off, p - (buf + off));
	  buf[off + len] = 0;
	  tok->argp[tok->argc++] = off;
	  return p - buf + 1;
	}
    }

  tok->argp[tok->argc++] = off;
  if (ISDELIM (buf[off]))
    return insert_nul (tok, off + 1);

  while (off < tok->level && !mu_isblank (buf[off]))
    {
      if (ISDELIM (buf[off]))
	return insert_nul (tok, off);
      off++;
    }
  insert_nul (tok, off);
  
  return off + 1;
}

static void
imap4d_tokbuf_tokenize (struct imap4d_tokbuf *tok, size_t off)
{
  while (off < tok->level)
    off = gettok (tok, off);
}

static void
check_input_err (int rc, size_t sz)
{
  switch (rc)
    {
    case 0:
      if (sz == 0)
	{
	  mu_diag_output (MU_DIAG_INFO, _("unexpected eof on input"));
	  imap4d_bye (ERR_NO_IFILE);
	}
      break;
      
    case MU_ERR_TIMEOUT:
      imap4d_bye (ERR_TIMEOUT);
      break;

    default:
      {
	const char *p = mu_stream_strerror (iostream, rc);
	if (!p)
	  p = mu_strerror (rc);

	mu_diag_output (MU_DIAG_INFO,
			_("error reading from input stream: %s"), p);
	imap4d_bye (ERR_NO_IFILE);
      }
    }
}

static size_t
imap4d_tokbuf_getline (struct imap4d_tokbuf *tok)
{
  char buffer[512];
  size_t level = tok->level;
  struct timeval tv, *to, stop_time;

  if (idle_timeout)
    {
      gettimeofday (&stop_time, NULL);
      stop_time.tv_sec += idle_timeout;
      to = &tv;
    }
  else
    to = NULL;

  do
    {
      size_t len;
      int rc;

      if (to)
	{
	  struct timeval d;

	  gettimeofday (&d, NULL);
	  if (mu_timeval_cmp (&d, &stop_time) >= 0)
	    {
	      rc = MU_ERR_TIMEOUT;
	      break;
	    }
	  *to = mu_timeval_sub (&stop_time, &d);
	}
      
      rc = mu_stream_timed_readline (iostream, buffer, sizeof (buffer),
				     to, &len);
      check_input_err (rc, len);
      imap4d_tokbuf_expand (tok, len);
      
      memcpy (tok->buffer + tok->level, buffer, len);
      tok->level += len;
    }
  while (tok->level && tok->buffer[tok->level - 1] != '\n');
  tok->buffer[--tok->level] = 0;
  if (tok->level > 0 && tok->buffer[tok->level - 1] == '\r')
    tok->buffer[--tok->level] = 0;
  while (tok->level > 0 && mu_isblank (tok->buffer[tok->level-1]))
    tok->buffer[--tok->level] = 0;
  return level;
}

void
imap4d_readline (struct imap4d_tokbuf *tok)
{
  tok->argc = 0;
  tok->level = 0;
  for (;;)
    {
      char *last_arg;
      size_t off = imap4d_tokbuf_getline (tok);
      imap4d_tokbuf_tokenize (tok, off);
      if (tok->argc == 0)
        break;  
      last_arg = tok->buffer + tok->argp[tok->argc - 1];
      if (last_arg[0] == '{' && last_arg[strlen(last_arg)-1] == '}')
	{
	  int rc;
	  unsigned long number;
	  char *sp = NULL;
	  char *buf;
	  size_t len;
	  
	  number = strtoul (last_arg + 1, &sp, 10);
	  /* Client can ask for non-synchronised literal,
	     if a '+' is appended to the octet count. */
	  if (*sp == '}')
	    io_sendf ("+ GO AHEAD\n");
	  else if (*sp != '+')
	    break;
	  xscript_declare_client_payload (number);
	  imap4d_tokbuf_expand (tok, number + 1);
	  off = tok->level;
	  buf = tok->buffer + off;
          len = 0;
          while (len < number)
            {
               size_t sz;
	       rc = mu_stream_read (iostream, buf + len, number - len, &sz);
               if (rc || sz == 0)
                 break;
               len += sz;
            }
	  check_input_err (rc, len);
	  imap4d_tokbuf_unquote (tok, &off, &len);
	  imap4d_tokbuf_decrlf (tok, off, &len);
	  tok->level += len;
	  tok->buffer[tok->level++] = 0;
	  tok->argp[tok->argc - 1] = off;
	}
      else
	break;
    }
}  

struct imap4d_tokbuf *
imap4d_tokbuf_from_string (char *str)
{
  struct imap4d_tokbuf *tok = imap4d_tokbuf_init ();
  tok->buffer = mu_strdup (str);
  if (!tok->buffer)
    imap4d_bye (ERR_NO_MEM);
  tok->level = strlen (str);
  tok->size = tok->level + 1;
  imap4d_tokbuf_tokenize (tok, 0);
  return tok;
}

void
io_enable_crlf (int enable)
{
  enable = !enable;
  mu_stream_ioctl (iostream, MU_IOCTL_FILTER,
		   MU_IOCTL_FILTER_SET_DISABLED,
		   &enable);
}
