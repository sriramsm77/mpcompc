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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <mailutils/mailutils.h>

void
usage (mu_stream_t str)
{
  mu_stream_printf (str, "usage: mcf [-l] TYPE/SUBTYPE FILE [FILE...]\n");
}

int
selector (mu_mailcap_entry_t entry, void *data)
{
  char const *type = data;
  char const *pattern;

  MU_ASSERT (mu_mailcap_entry_sget_type (entry, &pattern));
  return mu_mailcap_string_match (pattern, 0, type);
}

static void list_entry (mu_mailcap_entry_t ent, unsigned long n);

static void
cli_locus (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  int *pflag = opt->opt_ptr;
  *pflag |= MU_MAILCAP_FLAG_LOCUS;
}

int
main (int argc, char **argv)
{
  struct mu_mailcap_selector_closure selcl;
  char *type;
  int rc;
  mu_mailcap_finder_t finder;
  mu_mailcap_entry_t entry;
  size_t n;
  int flags = MU_MAILCAP_FLAG_DEFAULT;

  struct mu_option options[] = {
    { "locus", 'l', NULL, MU_OPTION_DEFAULT,
      "record location of each entry in the source file",
      mu_c_incr, &flags, cli_locus },
    MU_OPTION_END
  };

  mu_set_program_name (argv[0]);
  mu_cli_simple (argc, argv,
		 MU_CLI_OPTION_OPTIONS, options,
		 MU_CLI_OPTION_PROG_DOC,
		 "extracts entries matching TYPE/SUBTYPE from the "
		 "given mailcap FILEs",
		 MU_CLI_OPTION_PROG_ARGS, "TYPE/SUBTYPE FILE...",
		 MU_CLI_OPTION_RETURN_ARGC, &argc,
		 MU_CLI_OPTION_RETURN_ARGV, &argv,
		 MU_CLI_OPTION_END);
  
  if (argc < 2)
    {
      mu_error ("required arguments missing; try %s --help for assistance",
		mu_program_name);
      return 1;
    }
  type = argv[0];
  mu_printf ("%s\n", type);

  memset (&selcl, 0, sizeof (selcl));
  selcl.selector = selector;
  selcl.data = type;

  MU_ASSERT (mu_mailcap_finder_create (&finder, flags,
				       &selcl,
				       &mu_mailcap_default_error_closure,
				       argv + 1));

  n = 1;
  while ((rc = mu_mailcap_finder_next_match (finder, &entry)) == 0)
    {
      list_entry (entry, n++);
    }
  mu_mailcap_finder_destroy (&finder);

  if (rc != MU_ERR_NOENT)
    {
      mu_diag_funcall (MU_DIAG_ERR,
		       "mu_mailcap_finder_next_match",
		       NULL,
		       rc);
    }

  return 0;
}

struct list_closure
{
  unsigned long n;
};

static int
list_field (char const *name, char const *value, void *data)
{
  struct list_closure *fc = data;
  mu_printf ("\tfields[%lu]: ", fc->n++);
  if (value)
    mu_printf ("%s=%s", name, value);
  else
    mu_printf ("%s", name);
  mu_printf ("\n");
  return 0;
}

static void
list_entry (mu_mailcap_entry_t ent, unsigned long n)
{
  struct mu_locus_range lr = MU_LOCUS_RANGE_INITIALIZER;
  char const *val;
  struct list_closure fc;

  if (mu_mailcap_entry_get_locus (ent, &lr) == 0)
    {
      mu_stream_lprintf (mu_strout, &lr, "entry[%lu]\n", n);
      mu_locus_range_deinit (&lr);
    }
  else
    mu_printf ("entry[%lu]\n", n);
  MU_ASSERT (mu_mailcap_entry_sget_type (ent, &val));
  mu_printf ("\ttypefield: %s\n", val);
  MU_ASSERT (mu_mailcap_entry_sget_command (ent, &val));
  mu_printf ("\tview-command: %s\n", val);
  fc.n = 1;
  mu_mailcap_entry_fields_foreach (ent, list_field, &fc);
  mu_printf ("\n");
}
