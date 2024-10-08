/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003-2022 Free Software Foundation, Inc.

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

/* This source implements a CRLFDOT filter, useful for data I/O in
   such protocols as POP3 and SMTP.

   When encoding, this filter translates each '\n' to "\r\n" and
   "byte-stuffs" the input by outputting an additional '.' in front of
   any '.' appearing in the beginning of a line. Upon end of input,
   it outputs additional ".\r\n".
   
   If created with the "-n" option, it leaves each "\r\n" input sequence
   untranslated, thereby "normalizing" the output (hence the option name).
   
   When decoding, the reverse is performed: each "\r\n" is replaced by a
   '\n', and any '.' appearing in the beginning of a line is removed.
   A single dot on a line by itself marks end of the stream.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <mailutils/errno.h>
#include <mailutils/filter.h>
#include <mailutils/stream.h>

enum crlfdot_decode_state
  {
    crlfdot_decode_init,  /* initial state */
    crlfdot_decode_char,  /* Any character excepting [\r\n.] */
    crlfdot_decode_cr,    /* prev. char was \r */
    crlfdot_decode_crlf,  /* 2 prev. char were \r\n */
    crlfdot_decode_dot,   /* 3 prev. chars were \r\n. */
    crlfdot_decode_dotcr, /* 4 prev. chars were \r\n.\r */
    crlfdot_decode_end    /* final state, a \r\n.\r\n seen. */
  };

static enum crlfdot_decode_state
new_decode_state (enum crlfdot_decode_state state, int c)
{
  switch (state)
    {
    case crlfdot_decode_init:
      switch (c)
	{
	case '\r':
	  return crlfdot_decode_cr;
	case '.':
	  return crlfdot_decode_dot;
	}
      break;
      
    case crlfdot_decode_char:
      switch (c)
	{
	case '\r':
	  return crlfdot_decode_cr;
	}
      break;
      
    case crlfdot_decode_cr:
      switch (c)
	{
	case '\r':
	  return crlfdot_decode_cr;
	case '\n':
	  return crlfdot_decode_crlf;
	}
      break;
      
    case crlfdot_decode_crlf:
      switch (c)
	{
	case '\r':
	  return crlfdot_decode_cr;
	case '.':
	  return crlfdot_decode_dot;
	}
      
    case crlfdot_decode_dot:
      switch (c)
	{
	case '\r':
	  return crlfdot_decode_dotcr;
	}
      break;

    case crlfdot_decode_dotcr:
      switch (c)
	{
	case '\n':
	  return crlfdot_decode_end;
	}

    case crlfdot_decode_end:
      break;
    }
  return crlfdot_decode_char;
}

/* Move min(isize,osize) bytes from iptr to optr, replacing each \r\n
   with \n. */
static enum mu_filter_result
_crlfdot_decoder (void *xd,
		  enum mu_filter_command cmd,
		  struct mu_filter_io *iobuf)
{
  int *pstate = xd;
  size_t i, j;
  const unsigned char *iptr;
  size_t isize;
  char *optr;
  size_t osize;

  switch (cmd)
    {
    case mu_filter_init:
      *pstate = crlfdot_decode_init;
      return mu_filter_ok;
      
    case mu_filter_done:
      return mu_filter_ok;
      
    default:
      break;
    }
  
  iptr = (const unsigned char *) iobuf->input;
  isize = iobuf->isize;
  optr = iobuf->output;
  osize = iobuf->osize;

  for (i = j = 0; *pstate != crlfdot_decode_end && i < isize && j < osize; i++)
    {
      unsigned char c = *iptr++;

      if (c == '\r')
	{
	  if (i + 1 == isize)
	    break;
	  *pstate = new_decode_state (*pstate, c);
	  if (*iptr == '\n')
	    continue;
	}
      else if (c == '.' &&
	       (*pstate == crlfdot_decode_init ||
		*pstate == crlfdot_decode_crlf))
	{
	  /* Make sure we have two more characters in the buffer */
	  if (i + 2 == isize)
	    break;
	  *pstate = new_decode_state (*pstate, c);
	  if (*iptr != '\r')
	    continue;
	}
      else
	*pstate = new_decode_state (*pstate, c);
      optr[j++] = c;
    }
  
  if (*pstate == crlfdot_decode_end)
    {
      j -= 2; /* remove the trailing .\n */
      iobuf->eof = 1;
    }
  iobuf->isize = i;
  iobuf->osize = j;
  return mu_filter_ok;
}

enum crlfdot_encode_at
  {
    crlfdot_encode_init,  /* initial state */
    crlfdot_encode_char,  /* Any character excepting [\r\n] */
    crlfdot_encode_cr,    /* prev. char was \r */
    crlfdot_encode_lf,    /* prev. char was \n */
  };    

struct crlfdot_encode_state
{
  enum crlfdot_encode_at at;
  int normalize;
};

static void
new_encode_state (struct crlfdot_encode_state *state, int c)
{
  enum crlfdot_encode_at at;
  
  switch (c)
    {
    case '\r':
      if (state->normalize)
	{
	  at = crlfdot_encode_cr;
	  break;
	}
    case '\n':
      at = crlfdot_encode_lf;
      break;
    default:
      at = crlfdot_encode_char;
    }
  state->at = at;
}

/* Move min(isize,osize) bytes from iptr to optr, replacing each \n
   with \r\n.  Any input \r\n sequences remain untouched. */
static enum mu_filter_result
_crlfdot_encoder (void *xd,
		  enum mu_filter_command cmd,
		  struct mu_filter_io *iobuf)
{
  enum mu_filter_result result;
  size_t i, j;
  const unsigned char *iptr;
  size_t isize;
  char *optr;
  size_t osize;
  struct crlfdot_encode_state *state = xd;
  
  switch (cmd)
    {
    case mu_filter_init:
      state->at = crlfdot_encode_init;
      return mu_filter_ok;
      
    case mu_filter_done:
      return mu_filter_ok;

    default:
      break;
    }
  
  iptr = (const unsigned char *) iobuf->input;
  isize = iobuf->isize;
  optr = iobuf->output;
  osize = iobuf->osize;

  for (i = j = 0; i < isize && j < osize; i++, iptr++)
    {
      unsigned char c = *iptr;

      if (c == '\n')
	{
	  if (state->at == crlfdot_encode_cr)
	    optr[j++] = c;
 	  else if (j + 1 == osize)
	    {
	      if (i == 0)
		{
		  iobuf->osize = 2;
		  return mu_filter_moreoutput;
		}
	      break;
	    }
	  else
	    {
	      optr[j++] = '\r';
	      optr[j++] = '\n';
	    }
	}
      else if (c == '.' &&
	       (state->at == crlfdot_encode_init ||
		state->at == crlfdot_encode_lf))
	{
 	  if (j + 2 > osize)
	    {
	      if (i == 0)
		{
		  iobuf->osize = 2;
		  return mu_filter_moreoutput;
		}
	      break;
	    }
	  optr[j++] = '.';
	  optr[j++] = '.';
	}
      else
	optr[j++] = c;

      new_encode_state (state, c);
    }

  result = mu_filter_ok;
  if (cmd == mu_filter_lastbuf)
    {
      switch (state->at)
	{
	case crlfdot_encode_init:
	case crlfdot_encode_lf:
	  if (j + 3 > osize)
	    result = mu_filter_again;
	  break;
	      
	default:
	  if (j + 5 > osize)
	    result = mu_filter_again;
	  else
	    {
	      optr[j++] = '\r';
	      optr[j++] = '\n';
	    }
	}

      if (result == mu_filter_ok)
	{
	  optr[j++] = '.';
	  optr[j++] = '\r';
	  optr[j++] = '\n';
	}
    }

  iobuf->isize = i;
  iobuf->osize = j;
  return result;
}

static int
alloc_state (void **pret, int mode, int argc, const char **argv)
{
  switch (mode)
    {
    case MU_FILTER_ENCODE:
      {
	struct crlfdot_encode_state *state = malloc (sizeof (*state));
	if (!state)
	  return ENOMEM;
	state->at = crlfdot_encode_init;
	state->normalize = (argc == 2 && strcmp (argv[1], "-n") == 0);
	*pret = state;
      }
      break;
      
    case MU_FILTER_DECODE:
      *pret = malloc (sizeof (int));
      if (!*pret)
	return ENOMEM;
    }
  return 0;
}

static struct _mu_filter_record _crlfdot_filter = {
  "CRLFDOT",
  alloc_state,
  _crlfdot_encoder,
  _crlfdot_decoder
};

mu_filter_record_t mu_crlfdot_filter = &_crlfdot_filter;
