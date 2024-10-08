/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2005-2022 Free Software Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <mailutils/mailutils.h>

/* Replace all octal escapes in BUF with the corresponding characters. */
static void
decode_octal (char *buf)
{
  char *p;
  unsigned i, n;
  
  for (p = buf; *p;)
    {
      if (*buf == '\\')
	{
	  buf++;
	  switch (*buf)
	    {
	    case 'a':
	      *p++ = '\a';
	      buf++;
	      break;
	      
	    case 'b':
	      *p++ = '\b';
	      buf++;
	      break;
	      
	    case 'f':
	      *p++ = '\f';
	      buf++;
	      break;
	      
	    case 'n':
	      *p++ = '\n';
	      buf++;
	      break;
	      
	    case 'r':
	      *p++ = '\r';
	      buf++;
	      break;
	      
	    case 't':
	      *p++ = '\t';
	      buf++;
	      break;

	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	      n = 0;
	      for (i = 0; i < 3; i++, buf++)
		{
		  unsigned x = *(unsigned char*)buf - '0';
		  if (x > 7)
		    break;
		  n <<= 3;
		  n += x;
		}
	      if (i != 3)
		{
		  buf -= i;
		  *p++ = '\\';
		}
	      else
		*p++ = n;
	      break;

	    default:
	      *p++ = '\\';
	      *p++ = *buf++;
	      break;
	    }
	}
      else
	*p++ = *buf++;
    }
  *p = 0;
}

int
main (int argc, char *argv[])
{
  int rc;
  char *buf = NULL;
  size_t size = 0;
  size_t n;
  char *charset = "iso-8859-1";
  char *encoding = "quoted-printable";
  int octal = 0;
  struct mu_option options[] = {
    { "charset", 'c', "NAME", MU_OPTION_DEFAULT,
      "define input character set", mu_c_string, &charset },
    { "encoding", 'e', "NAME", MU_OPTION_DEFAULT,
      "define input encoding", mu_c_string, &encoding },
    { "octal", 'o', NULL, MU_OPTION_DEFAULT,
      "decode octal escape notations on input", mu_c_bool, &octal },
    MU_OPTION_END
  };  

  mu_set_program_name (argv[0]);
  mu_stdstream_setup (MU_STDSTREAM_RESET_NONE);
  mu_cli_simple (argc, argv,
                 MU_CLI_OPTION_OPTIONS, options,
		 MU_CLI_OPTION_PROG_DOC, "Test RFC 2047 encoding function",
		 MU_CLI_OPTION_EXTRA_INFO, "Defaults are: --charset=iso-8859-1 --encoding=quoted-printable",
		 MU_CLI_OPTION_END);

  while ((rc = mu_stream_getline (mu_strin, &buf, &size, &n)) == 0 && n > 0)
    {
      char *p;
      
      mu_rtrim_class (buf, MU_CTYPE_ENDLN);
      if (octal)
	decode_octal (buf);
	  
      rc = mu_rfc2047_encode (charset, encoding, buf, &p);
      if (rc)
	mu_diag_funcall (MU_DIAG_ERROR, "mu_rfc2047_encode", NULL, rc);
      else if (p)
	mu_printf ("%s\n", p);
      free (p);
    }
  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_getline", NULL, rc);
  return 0;
}
