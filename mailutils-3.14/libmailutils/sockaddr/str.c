/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2011-2022 Free Software Foundation, Inc.

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
# include <config.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

#include <mailutils/sockaddr.h>
#include <mailutils/errno.h>
#include <mailutils/io.h>

static char *default_sockaddr_text = "[not enogh memory]";

#define S_UN_NAME(sa, salen) \
	((salen <= mu_offsetof(struct sockaddr_un,sun_path)) ?	\
	  "" : (sa)->sun_path)

int
mu_sys_sockaddr_format (char **pbuf,
			enum mu_sockaddr_format fmt,
			const struct sockaddr *sa, socklen_t salen)
{
  int rc = MU_ERR_FAILURE;
  
  switch (sa->sa_family)
    {
#ifdef MAILUTILS_IPV6
    case AF_INET:
    case AF_INET6:
      {
	char host[NI_MAXHOST];
	char srv[NI_MAXSERV];
	if (getnameinfo (sa, salen,
			 host, sizeof (host), srv, sizeof (srv),
			 NI_NUMERICHOST|NI_NUMERICSERV) == 0)
	  {
	    switch (fmt)
	      {
	      case mu_sockaddr_format_default:
		if (sa->sa_family == AF_INET6)
		  rc = mu_asprintf (pbuf, "inet6://[%s]:%s", host, srv);
		else
		  rc = mu_asprintf (pbuf, "inet://%s:%s", host, srv);
		break;

	      case mu_sockaddr_format_ehlo:
		rc = mu_asprintf (pbuf, "[%s]", host);
		break;
	      }
	  }
	else
	  {
	    switch (fmt)
	      {
	      case mu_sockaddr_format_default:
		rc = mu_asprintf (pbuf, "%s://[getnameinfo failed]",
				  sa->sa_family == AF_INET ?
				  "inet" : "inet6");
		break;

	      case mu_sockaddr_format_ehlo:
		return MU_ERR_FAILURE;
	      }
	  }
	break;
      }
#else
    case AF_INET:
      {
	struct sockaddr_in *s_in = (struct sockaddr_in *)sa;
	switch (fmt)
	  {
	  case mu_sockaddr_format_default:
	    rc = mu_asprintf (pbuf, "inet://%s:%hu",
			      inet_ntoa (s_in->sin_addr), s_in->sin_port);
	    break;
	    
	  case mu_sockaddr_format_ehlo:
	    rc = mu_asprintf (pbuf, "[%s]", inet_ntoa (s_in->sin_addr));
	    break;
	  }
	break;
      }
#endif
      
    case AF_UNIX:
      {
	struct sockaddr_un *s_un = (struct sockaddr_un *)sa;
	
	switch (fmt)
	  {
	  case mu_sockaddr_format_default:
	    if (S_UN_NAME (s_un, salen)[0] == 0)
	      rc = mu_asprintf (pbuf, "unix://[anonymous socket]");
	    else
	      rc = mu_asprintf (pbuf, "unix://%s", s_un->sun_path);
	    break;

	  case mu_sockaddr_format_ehlo:
	    rc = mu_asprintf (pbuf, "localhost");
	  }
	break;
      }

    default:
      rc = mu_asprintf (pbuf, "family:%d", sa->sa_family);
    }
  return rc;
}

char *
mu_sys_sockaddr_to_astr (const struct sockaddr *sa, int salen)
{
  char *buf = NULL;
  mu_sys_sockaddr_format (&buf, mu_sockaddr_format_default, sa, salen);
  return buf;
}

int
mu_sockaddr_format (struct mu_sockaddr *sa, char **pbuf,
		    enum mu_sockaddr_format fmt)
{
  return mu_sys_sockaddr_format (pbuf, fmt, sa->addr, sa->addrlen);
}

const char *
mu_sockaddr_str (struct mu_sockaddr *sa)
{
  if (!sa->str
      && mu_sockaddr_format (sa, &sa->str, mu_sockaddr_format_default))
    return default_sockaddr_text;
  return sa->str;
}


