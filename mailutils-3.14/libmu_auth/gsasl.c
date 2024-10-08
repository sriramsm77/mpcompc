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

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <mailutils/cli.h>
#include <mailutils/mu_auth.h>

#ifdef WITH_GSASL

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/nls.h>
#include <mailutils/stream.h>
#include <mailutils/gsasl.h>
#include <mailutils/sys/gsasl-stream.h>
#include <mailutils/filter.h>

#include <gsasl.h>

struct mu_gsasl_module_data mu_gsasl_module_data = {
  .enable = 1,
  .cram_md5_pwd = SITE_CRAM_MD5_PWD
};

static struct mu_cfg_param mu_gsasl_param[] = {
  { "enable", mu_c_bool, &mu_gsasl_module_data.enable, 0, NULL,
    N_("Enable GSASL (default)") },
  { "cram-passwd", mu_c_string, &mu_gsasl_module_data.cram_md5_pwd, 0, NULL,
    N_("Name of GSASL password file."),
    N_("file") },
  { "service", mu_c_string, &mu_gsasl_module_data.service, 0, NULL,
    N_("SASL service name."),
    N_("name") },
  { "realm", mu_c_string, &mu_gsasl_module_data.realm, 0, NULL,
    N_("SASL realm name."),
    N_("name") },
  { "hostname", mu_c_string, &mu_gsasl_module_data.hostname, 0, NULL,
    N_("SASL host name."),
    N_("name") },
  { "anonymous-user", mu_c_string, &mu_gsasl_module_data.anon_user, 0, NULL,
    N_("Anonymous user name."),
    N_("name") },
  
  { NULL }
};

int
mu_gsasl_enabled (void)
{
  return mu_gsasl_module_data.enable;
}


static enum mu_filter_result
_gsasl_encoder (void *xdata,
		enum mu_filter_command cmd,
		struct mu_filter_io *iobuf)
{
  struct _mu_gsasl_filter *flt = xdata;
  
  switch (cmd)
    {
    case mu_filter_init:
      flt->bufptr = NULL;
      flt->bufsize = 0;
      flt->gsasl_err = 0;
      return mu_filter_ok;
      
    case mu_filter_done:
      if (flt->bufptr)
	free (flt->bufptr);
      return mu_filter_ok;
      
    default:
      break;
    }

  if (flt->bufptr == NULL)
    {
      int status = gsasl_encode (flt->sess_ctx, iobuf->input, iobuf->isize,
				 &flt->bufptr, &flt->bufsize);
      /* FIXME: Can it require more input? */
      if (status)
	{
	  flt->gsasl_err = status;
	  return mu_filter_failure;
	}
    }
    
  iobuf->osize = flt->bufsize;

  if (flt->bufsize > iobuf->osize)
    return mu_filter_moreoutput;

  memcpy (iobuf->output, flt->bufptr, flt->bufsize);

  free (flt->bufptr);
  flt->bufptr = NULL;
  flt->bufsize = 0;
  
  return mu_filter_ok;
}
	
	
static enum mu_filter_result
_gsasl_decoder (void *xdata,
		enum mu_filter_command cmd,
		struct mu_filter_io *iobuf)
{
  struct _mu_gsasl_filter *flt = xdata;
  int status;
  
  switch (cmd)
    {
    case mu_filter_init:
      flt->bufptr = NULL;
      flt->bufsize = 0;
      flt->gsasl_err = 0;
      return mu_filter_ok;
      
    case mu_filter_done:
      if (flt->bufptr)
	free (flt->bufptr);
      return mu_filter_ok;
      
    default:
      break;
    }

  if (flt->bufptr == NULL)
    {
      status = gsasl_decode (flt->sess_ctx, iobuf->input, iobuf->isize,
			     &flt->bufptr, &flt->bufsize);
      switch (status)
	{
	case GSASL_OK:
	  break;
	  
	case GSASL_NEEDS_MORE:
	  iobuf->isize++;
	  return mu_filter_moreinput;
	  
	default:
	  flt->gsasl_err = status;
	  return mu_filter_failure;
	}
    }

  iobuf->osize = flt->bufsize;

  if (flt->bufsize > iobuf->osize)
    return mu_filter_moreoutput;
  
  memcpy (iobuf->output, flt->bufptr, flt->bufsize);

  free (flt->bufptr);
  flt->bufptr = NULL;
  flt->bufsize = 0;
  
  return mu_filter_ok;
}
  
int
gsasl_encoder_stream (mu_stream_t *pstr, mu_stream_t transport,
		      Gsasl_session *ctx, int flags)
{
  int rc;
  struct _mu_gsasl_filter *flt = calloc (1, sizeof (*flt));
  flt->sess_ctx = ctx;
  rc = mu_filter_stream_create (pstr, transport, 
				MU_FILTER_ENCODE, 
				_gsasl_encoder,
				flt, flags);
  if (rc == 0)
    mu_stream_set_buffer (*pstr, mu_buffer_line, 0);
  return rc;
}

int
gsasl_decoder_stream (mu_stream_t *pstr, mu_stream_t transport,
		      Gsasl_session *ctx, int flags)
{
  int rc;
  struct _mu_gsasl_filter *flt = calloc (1, sizeof (*flt));
  flt->sess_ctx = ctx;
  rc = mu_filter_stream_create (pstr, transport, 
				MU_FILTER_DECODE, 
				_gsasl_decoder,
				flt, flags);
  if (rc == 0)
    mu_stream_set_buffer (*pstr, mu_buffer_line, 0);
  return rc;
}

int
mu_gsasl_stream_create (mu_stream_t *stream, mu_stream_t transport,
  		        Gsasl_session *ctx, int flags)
{
  int rc;
  mu_stream_t in, out;
  
  if (stream == NULL)
    return MU_ERR_OUT_PTR_NULL;
  rc = gsasl_encoder_stream (&in, transport, ctx, MU_STREAM_READ);
  if (rc)
    return rc;
  rc = gsasl_decoder_stream (&out, transport, ctx, MU_STREAM_WRITE);
  if (rc)
    {
      mu_stream_destroy (&in);
      return rc;
    }
  rc = mu_iostream_create (stream, in, out);
  mu_stream_unref (in);
  mu_stream_unref (out);
  return rc;
}
#else
int
mu_gsasl_enabled (void)
{
  return 0;
}
#define mu_gsasl_param NULL
#endif

struct mu_auth_module mu_auth_gsasl_module = {
  .name = "gsasl",
  .cfg = mu_gsasl_param
};
