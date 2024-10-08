/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2010-2022 Free Software Foundation, Inc.

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

#include <config.h>
#include <mailutils/mailutils.h>
#include <mailutils/nls.h>
#include <sysexits.h>
#include "mu.h"

char ldflags_docstring[] = N_("list libraries required to link");
static char ldflags_args_doc[] = N_("KEYWORD [KEYWORD...]");


#ifdef WITH_TLS
# define NEEDAUTH "-lmu_auth " AUTHLIBS
#else
# define NEEDAUTH NULL
#endif
#define NOTALL   1

struct lib_descr {
  char *name;
  char *libname;
  int weight;
  int flags;
} lib_descr[] = {
  { "mbox",    "-lmu_mbox", 0 },
#ifdef ENABLE_DOTMAIL
  { "dotmail", "-lmu_dotmail", 0 },
#endif
#ifdef ENABLE_MH
  { "mh",      "-lmu_mh",   0 },
#endif
#ifdef ENABLE_MAILDIR
  { "maildir", "-lmu_maildir", 0 },
#endif
#ifdef ENABLE_IMAP  
  { "imap",    "-lmu_imap", 0 },
  { "imap",    NEEDAUTH,    2 },
#endif
#ifdef ENABLE_POP  
  { "pop",    "-lmu_pop",   0 },
  { "pop",    NEEDAUTH,     2 },
#endif
#ifdef ENABLE_DBM
  { "dbm",    "-lmu_dbm",   0 },
  { "dbm",    DBMLIBS,      2 },
#endif
  { "mailer", "-lmu_mailer", 0 },
  { "sieve",  "-lmu_sieve",  0, NOTALL },
  { "auth",   "-lmu_auth " AUTHLIBS, 2 },
#ifdef WITH_GUILE	      
  { "guile",  "-lmu_scm " GUILE_LIBS, -1, NOTALL },
#endif
#ifdef WITH_PYTHON
  { "python", "-lmu_py " PYTHON_LIBS, -1, NOTALL },
#endif
  { "compat" },  /* deprecated */
  { "cfg"    },  /* deprecated */
  { "argp"   },  /* deprecated */
  { "nntp"   },  /* deprecated */
  { NULL }
};

struct lib_entry {
  int level;
  char *ptr;
} lib_entry[16];

int nentry;

void
add_entry (int level, char *ptr)
{
  int i;

  if (!ptr || !*ptr)
    return;
  if (nentry >= sizeof(lib_entry)/sizeof(lib_entry[0]))
    {
      mu_error (_("too many arguments"));
      exit (1);
    }
  
  for (i = 0; i < nentry; i++)
    if (strcmp (lib_entry[i].ptr, ptr) == 0)
      return;
  lib_entry[nentry].level = level;
  lib_entry[nentry].ptr = ptr;
  nentry++;
}

/* Sort the entries by their level. */
void
sort_entries (void)
{
  int j;

  for (j = 0; j < nentry; j++)
    {
      int i;
	      
      for (i = j; i < nentry; i++)
	if (lib_entry[j].level > lib_entry[i].level)
	  {
	    struct lib_entry tmp;
	    tmp = lib_entry[i];
	    lib_entry[i] = lib_entry[j];
	    lib_entry[j] = tmp;
	  }
    }
}


int
main (int argc, char **argv)
{
  int j;

  mu_action_getopt (&argc, &argv, NULL, ldflags_docstring, ldflags_args_doc);
  
  add_entry (-100, LINK_FLAGS);
  add_entry (100, LINK_POSTFLAGS);
  add_entry (8, "-lmailutils");
#ifdef ENABLE_NLS
  if (sizeof (I18NLIBS) > 1)
    add_entry (10, I18NLIBS);
#endif

  for ( ; argc > 0; argc--, argv++)
    {
      if (strcmp (argv[0], "all") == 0)
	{
	  struct lib_descr *p;
		  
	  for (p = lib_descr; p->name; p++)
	    {
	      if (p->flags & NOTALL)
		continue;
	      add_entry (p->weight, p->libname);
	    }
	}
      else
	{
	  struct lib_descr *p;
	  int found = 0;
	  
	  for (p = lib_descr; p->name; p++)
	    {
	      if (mu_c_strcasecmp (p->name, argv[0]) == 0)
		{
		  add_entry (p->weight, p->libname);
		  found = 1;
		}
	    }

	  if (!found)
	    {
	      mu_error (_("unknown keyword: %s"), argv[0]);
	      return 1;
	    }
	}
    }
  
  sort_entries ();
	  
  /* At least one entry is always present */
  mu_printf ("%s", lib_entry[0].ptr);

  /* Print the rest of them separated by a space */
  for (j = 1; j < nentry; j++)
    mu_printf (" %s", lib_entry[j].ptr);
  mu_printf ("\n");
  return 0;
}
