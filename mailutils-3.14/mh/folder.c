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

/* MH folder command */

#include <mh.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <dirent.h>

static char prog_doc[] = N_("set or list current folder or message");
static char args_doc[] = N_("[ACTION] [MSG]");

typedef int (*folder_action) (void);

static int action_print (void);
static int action_list (void);
static int action_push (void);
static int action_pop (void);
static int action_pack (void);
static folder_action action = action_print;

int show_all = 0; /* List all folders. Raised by --all switch */
int create_flag = -1; /* Create non-existent folders (--create).
		         -1: Prompt before creating
		          0: Do not create
		          1: Always create without prompting */
int fast_mode = 0; /* Fast operation mode. (--fast) */
  /* The following two vars are three-state, -1 meaning the default for
     current mode */
int print_header = -1; /* Display the header line (--header) */
int print_total = -1;  /* Display total stats */
int verbose = 0;   /* Verbosely list actions taken */
size_t pack_start; /* Number to be assigned to the first message in packed
		      folder. 0 means do not change first message number. */
int dry_run;       /* Dry run mode */ 
const char *push_folder; /* Folder name to push on stack */

const char *mh_seq_name; /* Name of the mh sequence file (defaults to
			    .mh_sequences) */
int has_folder;    /* Folder has been explicitely given */
int recurse_option = 0;
size_t max_depth = 1;  /* Maximum recursion depth (0 means infinity) */ 

#define OPTION_IS_SET(opt) ((opt) == -1 ? show_all : opt)

void
set_action (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  if (strcmp (opt->opt_long, "print") == 0)
    action = action_print;
  else if (strcmp (opt->opt_long, "pack") == 0)
    {
      action = action_pack;
      if (arg)
	{
	  if (mu_str_to_c (arg, mu_c_size, &pack_start, NULL))
	    {
	      mu_parseopt_error (po, _("%s: invalid number"), arg);
	      exit (po->po_exit_error);
	    }
	}
    }
  else if (strcmp (opt->opt_long, "list") == 0)
    action = action_list;
  else if (strcmp (opt->opt_long, "push") == 0)
    {
      action = action_push;
      if (arg)
	{
	  push_folder = mh_current_folder ();
	  mh_set_current_folder (arg);
	}
    }
  else if (strcmp (opt->opt_long, "pop") == 0)
    action = action_pop;
  else
    abort ();
}

void
set_folder (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  has_folder = 1;
  push_folder = mh_current_folder ();
  mh_set_current_folder (arg);
}

static struct mu_option options[] = {
  MU_OPTION_GROUP (N_("Actions are:")),
  { "print", 0, NULL, MU_OPTION_DEFAULT,
    N_("list the folders (default)"),
    mu_c_string, NULL, set_action },
  { "list",   0, NULL, MU_OPTION_DEFAULT,
    N_("list the contents of the folder stack"),
    mu_c_string, NULL, set_action },
  { "pack",   0, N_("NUMBER"), MU_OPTION_ARG_OPTIONAL,
    N_("remove holes in message numbering, begin numbering from NUMBER (default: first message number)"),
    mu_c_string, NULL, set_action },
  { "push",   0, N_("FOLDER"), MU_OPTION_ARG_OPTIONAL,
    N_("push the folder on the folder stack. If FOLDER is specified, it is pushed. "
       "Otherwise, if a folder is given in the command line (via + or --folder), "
       "it is pushed on stack. Otherwise, the current folder and the top of the folder "
       "stack are exchanged"),
    mu_c_string, NULL, set_action },
  { "pop",    0, NULL, MU_OPTION_DEFAULT,
    N_("pop the folder off the folder stack"), 
    mu_c_string, NULL, set_action },
  
  MU_OPTION_GROUP (N_("Options are:")),
  
  { "folder", 0, N_("FOLDER"), MU_OPTION_DEFAULT,
    N_("specify folder to operate upon"),
    mu_c_string, NULL, set_folder },
  { "all",    0,    NULL, MU_OPTION_DEFAULT,
    N_("list all folders"),
    mu_c_bool, &show_all },
  { "create", 0, NULL, MU_OPTION_DEFAULT, 
    N_("create non-existing folders"),
    mu_c_bool, &create_flag },
  { "fast",   0, NULL, MU_OPTION_DEFAULT, 
    N_("list only the folder names"),
    mu_c_bool, &fast_mode },
  { "header", 0, NULL, MU_OPTION_DEFAULT, 
    N_("print the header line"),
    mu_c_bool, &print_header },
  { "recurse",0, NULL, MU_OPTION_DEFAULT,
    N_("scan folders recursively"),
    mu_c_bool, &recurse_option },
  { "total",  0, NULL, MU_OPTION_DEFAULT, 
    N_("output the total statistics"),
    mu_c_bool, &print_total },
  { "verbose", 0, NULL, MU_OPTION_DEFAULT,
    N_("verbosely list actions taken"),
    mu_c_bool, &verbose },
  { "dry-run", 0, NULL, MU_OPTION_DEFAULT,
    N_("do nothing, print what would be done (with --pack)"),
    mu_c_bool, &dry_run },
  MU_OPTION_END
};

/* ************************************************************* */
/* Printing */

struct folder_info
{
  char *name;              /* Folder name */
  size_t message_count;    /* Number of messages in this folder */
  size_t min;              /* First used sequence number (=uid) */
  size_t max;              /* Last used sequence number */
  size_t cur;              /* UID of the current message */
  size_t others;           /* Number of non-message files */ 
};

mu_list_t folder_info_list; /* Memory storage for folder info */

size_t message_count;             /* Total number of messages */

void
install_folder_info (const char *name, struct folder_info const *info,
		     size_t skip_prefix_len)
{
  struct folder_info *new_info = mu_alloc (sizeof (*new_info));
  *new_info = *info;
  new_info->name = mu_strdup (new_info->name + skip_prefix_len);
  mu_list_append (folder_info_list, new_info);
  message_count += info->message_count;
}

static int
folder_info_comp (const void *a, const void *b)
{
  return strcmp (((struct folder_info *)a)->name,
		 ((struct folder_info *)b)->name);
}

static void
read_seq_file (struct folder_info *info, const char *prefix, const char *name)
{
  char *pname = NULL;
  mu_property_t prop;
  const char *p;
  
  pname = mh_safe_make_file_name (prefix, name);
  prop = mh_read_property_file (pname, 1);
  
  if (mu_property_sget_value (prop, "cur", &p) == 0)
    info->cur = strtoul (p, NULL, 0);
  mu_property_destroy (&prop);
}

static void
_scan (const char *name, size_t depth, size_t skip_prefix_len)
{
  DIR *dir;
  struct dirent *entry;
  struct folder_info info;
  char *p;
  struct stat st;
  size_t uid;

  dir = opendir (name);

  if (!dir && errno == ENOENT)
    {
      if (create_flag)
	{
	  if (mh_check_folder (name, create_flag == -1))
	    {
	      push_folder = NULL;
	      return;
	    }
	  dir = opendir (name);
	}
      else
	exit (1);
    }

  if (!dir)
    {
      mu_error (_("cannot scan folder %s: %s"), name, strerror (errno));
      return;
    }

  if (max_depth == 1)
    {
      if (fast_mode && depth > 0)
	{
	  memset (&info, 0, sizeof (info));
	  info.name = (char*) name;
	  install_folder_info (name, &info, skip_prefix_len);
	  closedir (dir);
	  return;
	}
    }
  
  if (max_depth && depth > max_depth)
    {
      closedir (dir);
      return;
    }
  
  memset (&info, 0, sizeof (info));
  info.name = mu_strdup (name);
  while ((entry = readdir (dir)))
    {
      if (entry->d_name[0] == '.')
	{
	  if (strcmp (entry->d_name, mh_seq_name) == 0)
	    read_seq_file (&info, name, entry->d_name);
	}
      else if (entry->d_name[0] != ',')
	{
	  p = mh_safe_make_file_name (name, entry->d_name);
	  if (stat (p, &st) < 0)
	    mu_diag_funcall (MU_DIAG_ERROR, "stat", p, errno);
	  else if (S_ISDIR (st.st_mode))
	    {
	      info.others++;
	      _scan (p, depth+1, skip_prefix_len);
	    }
	  else
	    {
	      char *endp;
	      uid = strtoul (entry->d_name, &endp, 10);
	      if (*endp)
		info.others++;
	      else
		{
		  info.message_count++;
		  if (info.min == 0 || uid < info.min)
		    info.min = uid;
		  if (uid > info.max)
		    info.max = uid;
		}
	    }
	}
    }
  
  if (info.cur)
    {
      p = mh_safe_make_file_name (name, mu_umaxtostr (0, info.cur));
      if (stat (p, &st) < 0 || !S_ISREG (st.st_mode))
	info.cur = 0;
      free (p);
    }
  closedir (dir);
  if (depth > 0)
    install_folder_info (name, &info, skip_prefix_len);
}

static void
folder_scan (const char *name, size_t depth)
{
  const char *folder_dir = mu_folder_directory ();
  size_t skip_prefix_len;

  skip_prefix_len = strlen (folder_dir);
  if (folder_dir[skip_prefix_len - 1] == '/')
    skip_prefix_len++;
  if (strncmp (name, folder_dir, skip_prefix_len) == 0)
    skip_prefix_len++;  /* skip past the slash */
  else
    skip_prefix_len = 0;
  _scan (name, depth, skip_prefix_len);
}

static int
_folder_info_printer (void *item, void *data)
{
  struct folder_info *info = item;
  int len = strlen (info->name);

  if (len < 22)
    printf ("%22.22s", info->name);
  else
    printf ("%s", info->name);
  
  if (strcmp (info->name, mh_current_folder ()) == 0)
    printf ("+");
  else
    printf (" ");
  
  if (info->message_count)
    {
      printf (ngettext(" has %4lu message  (%4lu-%4lu)",
		       " has %4lu messages (%4lu-%4lu)",
		       info->message_count),
	      (unsigned long) info->message_count,
	      (unsigned long) info->min,
	      (unsigned long) info->max);
      if (info->cur)
	printf ("; cur=%4lu", (unsigned long) info->cur);
    }
  else
    {
      printf (_(" has no messages"));
    }
  
  if (info->others)
    {
      if (!info->cur)
	printf (";           ");
      else
	printf ("; ");
      printf (_("(others)"));
    }
  printf (".\n");
  return 0;
}

static int
_folder_name_printer (void *item, void *data)
{
  struct folder_info *info = item;
  printf ("%s\n", info->name);
  return 0;
}

static void
print_all (void)
{
  mu_list_foreach (folder_info_list, _folder_info_printer, NULL);
}

static void
print_fast (void)
{
  mu_list_foreach (folder_info_list, _folder_name_printer, NULL);
}

static int
action_print (void)
{
  mh_seq_name = mh_global_profile_get ("mh-sequences", MH_SEQUENCES_FILE);

  mu_list_create (&folder_info_list);

  if (show_all)
    {
      folder_scan (mu_folder_directory (), 0);
    }
  else
    {
      char *p = mh_expand_name (NULL, mh_current_folder (), NAME_ANY);
      folder_scan (p, 1);
      free (p);
    }
  
  mu_list_sort (folder_info_list, folder_info_comp);

  if (fast_mode)
    print_fast ();
  else
    {
      if (OPTION_IS_SET (print_header))
	printf (_("Folder                  # of messages     (  range  )  cur msg   (other files)\n"));
		
      print_all ();

      if (OPTION_IS_SET (print_total))
	{
	  size_t folder_count;

	  mu_list_count (folder_info_list, &folder_count);
	  printf ("\n%24.24s=", _("TOTAL"));
	  printf (ngettext ("%4lu message  ", "%4lu messages ",
			    message_count),
		  (unsigned long) message_count);
	  printf (ngettext ("in %4lu folder", "in %4lu folders",
			    folder_count),
		  (unsigned long) folder_count);
	  printf ("\n");
	}
    }
  if (push_folder)
    mh_global_save_state ();

  return 0;
}


/* ************************************************************* */
/* Listing */

static int
action_list (void)
{
  const char *stack = mh_global_context_get ("Folder-Stack", NULL);

  printf ("%s", mh_current_folder ());
  if (stack && stack[0])
    printf (" %s", stack);
  printf ("\n");
  return 0;
}


/* ************************************************************* */
/* Push & pop */

static void
get_stack (size_t *pc, char ***pv)
{
  struct mu_wordsplit ws;
  const char *stack = mh_global_context_get ("Folder-Stack", NULL);
  if (!stack)
    {
      *pc = 0;
      *pv = NULL;
    }
  else if (mu_wordsplit (stack, &ws, MU_WRDSF_DEFFLAGS))
    {
      mu_error (_("cannot split line `%s': %s"), stack,
		mu_wordsplit_strerror (&ws));
      exit (1);
    }
  else
    {
      mu_wordsplit_get_words (&ws, pc, pv);
      mu_wordsplit_free (&ws);
    }
}

static void
set_stack (int c, char **v)
{
  char *str;
  int status = mu_argcv_string (c, v, &str);
  if (status)
    {
      mu_error ("%s", mu_strerror (status));
      exit (1);
    }
  mu_argcv_free (c, v);
  mh_global_context_set ("Folder-Stack", str);
  free (str);
}

static void
push_val (size_t *pc, char ***pv, const char *val)
{
  size_t c = *pc;
  char **v = *pv;

  c++;
  if (c == 1)
    {
      v = mu_calloc (c + 1, sizeof (*v));
    }
  else
    {
      v = mu_realloc (v, (c + 1) * sizeof (*v));
      memmove (&v[1], &v[0], c * sizeof (*v));
    }
  v[0] = mu_strdup (val);

  *pv = v;
  *pc = c;
}

static char *
pop_val (size_t *pc, char ***pv)
{
  char *val;
  size_t c;
  char **v;
  
  if (*pc == 0)
    return NULL;
  c = *pc;
  v = *pv;
  val = v[0];
  memmove (&v[0], &v[1], c * sizeof (*v));
  c--;

  *pc = c;
  *pv = v;
  return val;
}
  
static int
action_push (void)
{
  size_t c;
  char **v;

  get_stack (&c, &v);
  
  if (push_folder)
    push_val (&c, &v, push_folder);
  else 
    {
      char *t = v[0];
      v[0] = mu_strdup (mh_current_folder ());
      mh_set_current_folder (t);
      free (t);
    }

  set_stack (c, v);

  action_list ();
  mh_global_save_state ();
  return 0;
}

static int
action_pop (void)
{
  size_t c;
  char **v;

  get_stack (&c, &v);

  if (c)
    {
      char *p = pop_val (&c, &v);
      set_stack (c, v);
      mh_set_current_folder (p);
      free (p);
    }

  action_list ();
  mh_global_save_state ();
  return 0;
}


/* ************************************************************* */
/* Packing */

struct pack_tab
{
  size_t orig;
  size_t new;
};

static int
pack_rename (struct pack_tab *tab, int reverse)
{
  int rc;
  const char *s1;
  const char *s2;
  const char *from, *to;
  
  s1 = mu_umaxtostr (0, tab->orig);
  s2 = mu_umaxtostr (1, tab->new);

  if (!reverse)
    {
      from = s1;
      to = s2;
    }
  else
    {
      from = s2;
      to = s1;
    }

  if (verbose)
    fprintf (stderr, _("Renaming %s to %s\n"), from, to);

  if (!dry_run)
    {
      if ((rc = rename (from, to)))
	mu_error (_("cannot rename `%s' to `%s': %s"),
		  from, to, mu_strerror (errno));
    }
  else
    rc = 0;
  
  return rc;
}

/* Reverse ordering of COUNT entries in array TAB */
static void
reverse (struct pack_tab *tab, size_t count)
{
  size_t i, j;

  for (i = 0, j = count-1; i < j; i++, j--)
    {
      size_t tmp;
      tmp = tab[i].orig;
      tab[i].orig = tab[j].orig;
      tab[j].orig = tmp;

      tmp = tab[i].new;
      tab[i].new = tab[j].new;
      tab[j].new = tmp;
    }
} 

static void
roll_back (const char *folder_name, struct pack_tab *pack_tab, size_t i)
{
  size_t start;
  
  if (i == 0)
    return;
  
  start = --i;
  mu_error (_("rolling back changes..."));
  do
    if (pack_rename (pack_tab + i, 1))
      {
	mu_error (_("CRITICAL ERROR: Folder `%s' left in an inconsistent state, because an error\n"
		    "occurred while trying to roll back the changes.\n"
		    "Message range %s-%s has been renamed to %s-%s."),
		  folder_name,
		  mu_umaxtostr (0, pack_tab[0].orig),
                  mu_umaxtostr (1, pack_tab[start].orig),
		  mu_umaxtostr (2, pack_tab[0].new),
                  mu_umaxtostr (3, pack_tab[start].new));
	mu_error (_("You will have to fix it manually."));
	exit (1);
      }
  while (i-- > 0);
  mu_error (_("folder `%s' restored successfully"), folder_name);
}

struct fixup_data
{
  mu_mailbox_t mbox;
  const char *folder_dir;
  struct pack_tab *pack_tab;
  size_t count;
};

static int
pack_cmp (const void *a, const void *b)
{
  const struct pack_tab *pa = a;
  const struct pack_tab *pb = b;

  if (pa->orig < pb->orig)
    return -1;
  else if (pa->orig > pb->orig)
    return 1;
  return 0;
}

static size_t
pack_xlate (struct pack_tab *pack_tab, size_t count, size_t n)
{
  struct pack_tab key, *p;

  key.orig = n;
  p = bsearch (&key, pack_tab, count, sizeof pack_tab[0], pack_cmp);
  return p ? p->new : 0;
}

static void
_fixup (const char *name, const char *value, struct fixup_data *fd, int flags)
{
  size_t i;
  int rc;
  struct mu_wordsplit ws;
  mu_msgset_t msgset;

  if (verbose)
    fprintf (stderr, "Sequence `%s'...\n", name);

  if (mu_wordsplit (value, &ws, MU_WRDSF_DEFFLAGS))
    {
      mu_error (_("cannot split line `%s': %s"), value,
		mu_wordsplit_strerror (&ws));
      return;
    }

  rc = mu_msgset_create (&msgset, fd->mbox, MU_MSGSET_UID);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_msgset_create", NULL, rc);
      exit (1);
    }
  
  for (i = 0; i < ws.ws_wordc; i++)
    {
      size_t n = pack_xlate (fd->pack_tab, fd->count,
			     strtoul (ws.ws_wordv[i], NULL, 0));
      if (n)
	{
	  rc = mu_msgset_add_range (msgset, n, n, MU_MSGSET_UID);
	  if (rc)
	    {
	      mu_diag_funcall (MU_DIAG_ERROR, "mu_msgset_add_range", NULL, rc);
	      exit (1);
	    }
	}
    }

  mu_wordsplit_free (&ws);
  
  mh_seq_add (fd->mbox, name, msgset, flags | SEQ_ZERO);
  mu_msgset_free (msgset);

  if (verbose)
    {
      const char *p = mh_seq_read (fd->mbox, name, flags);
      fprintf (stderr, "Sequence %s: %s\n", name, p);
    }
}

static int
fixup_global (const char *name, const char *value, void *data)
{
  _fixup (name, value, data, 0);
  return 0;
}

static int
fixup_private (const char *name, const char *value, void *data)
{
  struct fixup_data *fd = data;
  int nlen = strlen (name);  
  if (nlen < 4 || memcmp (name, "atr-", 4))
    return 0;
  name += 4;

  nlen = strlen (name) - strlen (fd->folder_dir);
  if (nlen > 0 && strcmp (name + nlen, fd->folder_dir) == 0)
    {
      char *s = mu_alloc (nlen);
      memcpy (s, name, nlen - 1);
      s[nlen-1] = 0;
      _fixup (s, value, fd, SEQ_PRIVATE);
      free (s);
    }
  return 0;
}

int
action_pack (void)
{
  const char *folder_dir = mh_expand_name (NULL, mh_current_folder (), 
                                           NAME_ANY);
  mu_mailbox_t mbox = mh_open_folder (mh_current_folder (), MU_STREAM_RDWR);
  struct pack_tab *pack_tab;
  size_t i, count, start;
  int status;
  struct fixup_data fd;
  
  /* Allocate pack table */
  if (mu_mailbox_messages_count (mbox, &count))
    {
      mu_error (_("cannot read input mailbox: %s"), mu_strerror (errno));
      return 1;
    }
  pack_tab = mu_calloc (count, sizeof pack_tab[0]); /* Never freed. No use to
		 				       try to. */

  /* Populate it with message numbers */
  if (verbose)
    fprintf (stderr, _("Getting message numbers.\n"));
    
  for (i = 0; i < count; i++)
    {
      mu_message_t msg;
      status = mu_mailbox_get_message (mbox, i + 1, &msg);
      if (status)
	{
	  mu_error (_("%lu: cannot get message: %s"),
		    (unsigned long) i, mu_strerror (status));
	  return 1;
	}
      mh_message_number (msg, &pack_tab[i].orig);
    }
  if (verbose)
    fprintf (stderr, ngettext ("%s message number collected.\n",
			       "%s message numbers collected.\n",
			       (unsigned long) count),
	     mu_umaxtostr (0, count));
  
  mu_mailbox_close (mbox);
  mu_mailbox_destroy (&mbox);

  /* Compute new message numbers */
  if (pack_start == 0)
    pack_start = pack_tab[0].orig;

  for (i = 0, start = pack_start; i < count; i++)
    pack_tab[i].new = start++;

  if (pack_start > pack_tab[0].orig)
    {
      if (verbose)
	fprintf (stderr, _("Reverting pack table.\n"));
      reverse (pack_tab, i);
    }
  
  /* Change to the folder directory and rename messages */
  status = chdir (folder_dir);
  if (status)
    {
      mu_error (_("cannot change to directory `%s': %s"),
		folder_dir, mu_strerror (status));
      return 1;
    }

  for (i = 0; i < count; i++)
    {
      if (pack_rename (pack_tab + i, 0))
	{
	  roll_back (folder_dir, pack_tab, i);
	  return 1;
	}
    }

  if (verbose)
    fprintf (stderr, _("Finished packing messages.\n"));

  /* Fix-up sequences */
  if (!dry_run)
    {
      mbox = mh_open_folder (mh_current_folder (), MU_STREAM_RDWR);
      fd.mbox = mbox;
      fd.folder_dir = folder_dir;
      fd.pack_tab = pack_tab;
      fd.count = count;
      if (verbose)
	fprintf (stderr, _("Fixing global sequences\n"));
      mh_global_sequences_iterate (mbox, fixup_global, &fd);
      if (verbose)
	fprintf (stderr, _("Fixing private sequences\n"));
      mh_global_context_iterate (fixup_private, &fd);
      mu_mailbox_uidvalidity_reset (mbox);
      mu_mailbox_close (mbox);
      mu_mailbox_destroy (&mbox);
      mh_global_save_state ();
    }
  
  return 0;
}

int
main (int argc, char **argv)
{
  int index = 0;
  mu_msgset_t msgset;

  mh_getopt (&argc, &argv, options, 0, args_doc, prog_doc, NULL);
  if (recurse_option)
    max_depth = 0;

  /* If  folder  is invoked by a name ending with "s" (e.g.,  folders),
     `-all'  is  assumed */
  if (mu_program_name[strlen (mu_program_name) - 1] == 's')
    show_all = 1;
  
  if (has_folder)
    {
      /* If a +folder is given along with the -all switch, folder will, in
	 addition to setting the current folder, list the top-level
	 subfolders for the current folder (with -norecurse) or list all
	 sub-folders under the current folder recursively (with -recurse). */
      if (show_all && max_depth)
	max_depth = 2;
      show_all = 0;
    }
    
  if (argc - index == 1)
    {
      mu_mailbox_t mbox = mh_open_folder (mh_current_folder (),
					  MU_STREAM_RDWR);
      mh_msgset_parse (&msgset, mbox, argc - index, argv + index, "cur");
      mh_mailbox_set_cur (mbox, mh_msgset_first (msgset, RET_UID));
      mu_msgset_free (msgset);
      mh_global_save_state ();
      mu_mailbox_close (mbox);
      mu_mailbox_destroy (&mbox);
    }
  else if (argc - index > 1)
    {
      mu_error (_("too many arguments"));
      exit (1);
    }
  
  return (*action) ();
}
