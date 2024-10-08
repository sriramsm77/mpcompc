/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003-2022 Free Software Foundation, Inc.

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
#include <mailutils/mailutils.h>

void
print (char *p, int printable)
{
  for (; *p; p++)
    {
      if (printable && *p != '\n' && !mu_isprint (*p))
	printf ("\\%03o", *(unsigned char *) p);
      else
	putchar (*p);
    }
}

int
main (int argc, char *argv[])
{
  char buf[256];
  int printable = 0;
  char *charset = "iso-8859-1";
  struct mu_option options[] = {
    { "charset", 'c', "NAME", MU_OPTION_DEFAULT,
      "define output character set", mu_c_string, &charset },
    { "printable", 'p', NULL, MU_OPTION_DEFAULT,
      "make sure the output is printable", mu_c_incr, &printable },
    MU_OPTION_END
  };  

  mu_set_program_name (argv[0]);
  mu_cli_simple (argc, argv,
                 MU_CLI_OPTION_OPTIONS, options,
		 MU_CLI_OPTION_PROG_DOC, "Test RFC 2047 decoding function",
		 MU_CLI_OPTION_END);
		 
  while (fgets (buf, sizeof (buf), stdin))
    {
      char *p = NULL;
      int rc, len;

      len = strlen (buf);
      if (len > 0 && buf[len - 1] == '\n')
	buf[len - 1] = 0;
      rc = mu_rfc2047_decode (charset, buf, &p);
      if (rc)
	fprintf (stderr, "%s", mu_strerror (rc));
      else if (p)
	print (p, printable);
      putchar ('\n');
      free (p);
    }      
    return 0;
}
