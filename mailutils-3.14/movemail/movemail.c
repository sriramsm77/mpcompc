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
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.  */

#if defined(HAVE_CONFIG_H)
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <mailutils/mailutils.h>
#include <mailutils/tls.h>
#include "mailutils/cli.h"
#include <muaux.h>
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#include <sys/ioctl.h>

static int reverse_order;
static int preserve_mail; 
static int emacs_mode;
static int uidl_option;
static int verbose_option;
static int ignore_errors;
static char *program_id_option;
static size_t max_messages_option;
static int notify;
static int progress_meter_option;

  /* These bits tell what to do when an error occurs: */
#define ONERROR_SKIP     0x01  /* Skip to the next message */
#define ONERROR_DELETE   0x02  /* Delete the source message */
#define ONERROR_COUNT    0x04  /* Count it as processed */ 

static int onerror_flags;  

size_t msg_count = 0;         /* Number of processed messages */
size_t get_err_count = 0;     /* Number of message retrieval errors */ 
size_t app_err_count = 0;     /* Number of message appending errors */

enum set_ownership_mode
  {
    copy_owner_id,
    copy_owner_name,
    set_owner_id,
    set_owner_name
  };
#define SET_OWNERSHIP_MAX 4

struct user_id
{
  uid_t uid;
  gid_t gid;
};

struct set_ownership_method
{
  enum set_ownership_mode mode;
  union
  {
    char *name;
    struct user_id id;
  } owner;
};

static struct set_ownership_method so_methods[SET_OWNERSHIP_MAX];
static int so_method_num;

struct set_ownership_method *
get_next_so_method ()
{
  if (so_method_num == MU_ARRAY_SIZE (so_methods))
    {
      mu_error (_("ownership method table overflow"));
      exit (1);
    }
  return so_methods + so_method_num++;
}

mu_kwd_t method_kwd[] = {
  { "copy-id", copy_owner_id }, 
  { "copy-name", copy_owner_name },
  { "set-name", set_owner_name },
  { "user", set_owner_name },
  { "set-id", set_owner_id },
  { NULL }
};

static int
set_mailbox_ownership (const char *str)
{
  if (strcmp (str, "clear") == 0)
    so_method_num = 0;
  else
    {
      int code;
      char *p;
      size_t len = strcspn (str, "=");
      struct set_ownership_method *meth;
	  
      if (mu_kwd_xlat_name_len (method_kwd, str, len, &code))
	{
	  mu_error (_("invalid ownership method: %s"), str);
	  return 1;
	}
      
      meth = get_next_so_method ();
      meth->mode = code;
      switch (meth->mode)
	{
	case copy_owner_id:
	case copy_owner_name:
	  break;
	  
	case set_owner_id:
	  if (!str[len])
	    {
	      mu_error (_("ownership method %s requires value"), str);
	      return 1;
	    }
	  str += len + 1;
	  meth->owner.id.uid = strtoul (str, &p, 0);
	  if (*p)
	    {
	      if (*p == ':')
		{
		  str = p + 1;
		  meth->owner.id.gid = strtoul (str, &p, 0);
		  if (*p)
		    {
		      mu_error (_("expected gid number, but found %s"), str);
		      return 1;
		    }
		}
	      else
		{
		  mu_error (_("expected uid number, but found %s"), str);
		  return 1;
		}
	    }
	  else
	    meth->owner.id.gid = (gid_t) -1;
	  break;
	  
	case set_owner_name:
	  if (!str[len])
	    {
	      mu_error (_("ownership method %s requires value"), str);
	      return 1;
	    }
	  meth->owner.name = mu_strdup (str + len + 1);
	}
    }
  return 0;
}

static int
set_mailbox_ownership_list (char const *str)
{
  if (!strchr (str, ','))
    return set_mailbox_ownership (str);
  else
    {
      struct mu_wordsplit ws;
      size_t i;
      
      ws.ws_delim = ",";
      if (mu_wordsplit (str, &ws, MU_WRDSF_DEFFLAGS|MU_WRDSF_DELIM))
	{
	  mu_error (_("cannot parse %s: %s"),
		    str, mu_wordsplit_strerror (&ws));
	  return 1;
	}

      for (i = 0; i < ws.ws_wordc; i++)
	if (set_mailbox_ownership (ws.ws_wordv[i]))
	  return 1;
      mu_wordsplit_free (&ws);
      return 0;
    }
}

static int
set_onerror_action (void *item, void *data)
{
  char *str = item;

  if (strcmp (str, "abort") == 0)
    onerror_flags = 0;
  else
    {
      static struct mu_kwd onerror_kw[] = {
	{ "skip", ONERROR_SKIP },
	{ "delete", ONERROR_DELETE },
	{ "count", ONERROR_COUNT },
	{ NULL }
      };
      int flag, clr = 0;

      if (strncmp (str, "no", 2) == 0)
	{
	  clr = 1;
	  str += 2;
	}
      if (mu_kwd_xlat_name (onerror_kw, str, &flag))
	{
	  mu_error (_("unknown keyword: %s"), str);
	  return MU_ERR_FAILURE;
	}
      if (clr)
	onerror_flags &= ~flag;
      else
	onerror_flags |= flag;
    }
  return 0;
}

static int
set_onerror_actions (char const *str)
{
  mu_list_t list;
  int rc;
  
  mu_list_create (&list);
  mu_list_set_destroy_item (list, mu_list_free_item);
  mu_string_split (str, ",", list);
  rc = mu_list_foreach (list, set_onerror_action, NULL);
  mu_list_destroy (&list);
  return rc;
}

static void
cli_mailbox_ownership (struct mu_parseopt *po, struct mu_option *opt,
		       char const *arg)
{
  if (set_mailbox_ownership_list (arg))
    exit (po->po_exit_error);
}

static void
cli_onerror (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  if (set_onerror_actions (arg))
    exit (po->po_exit_error);
}

static struct mu_option movemail_options[] = {
  { "preserve", 'p', NULL, MU_OPTION_DEFAULT,
    N_("preserve the source mailbox"),
    mu_c_bool, &preserve_mail },
  { "keep-messages", 0, NULL, MU_OPTION_ALIAS },
  
  { "reverse",  'r', NULL, MU_OPTION_DEFAULT,
    N_("reverse the sorting order"),
    mu_c_bool, &reverse_order },
  
  { "emacs",      0, NULL, MU_OPTION_DEFAULT,
    N_("output information used by Emacs rmail interface"),
    mu_c_bool, &emacs_mode },
  
  { "uidl",     'u', NULL, MU_OPTION_DEFAULT,
    N_("use UIDLs to avoid downloading the same message twice"),
    mu_c_bool, &uidl_option },
  
  { "verbose",  'v', NULL, MU_OPTION_DEFAULT,
    N_("increase verbosity level"),
    mu_c_incr, &verbose_option },
  
  { "owner",    'P', N_("MODELIST"), MU_OPTION_DEFAULT,
    N_("control mailbox ownership"),
    mu_c_string, cli_mailbox_ownership },
  
  { "ignore-errors", 0, NULL, MU_OPTION_DEFAULT,
    N_("try to continue after errors"),
    mu_c_bool, &ignore_errors },
  
  { "onerror",       0, N_("KW[,KW...]"), MU_OPTION_DEFAULT,
    N_("what to do on errors"),
    mu_c_string, NULL, cli_onerror },
  
  { "program-id",    0, N_("FMT"), MU_OPTION_DEFAULT,
    N_("set program identifier for diagnostics (default: program name)"),
    mu_c_string, &program_id_option },
  
  { "max-messages",  0, N_("NUMBER"), MU_OPTION_DEFAULT,
    N_("process at most NUMBER messages"),
    mu_c_size, &max_messages_option },
  
  { "notify",        0, NULL,   MU_OPTION_DEFAULT,
    N_("enable biff notification"),
    mu_c_bool, &notify },

  { "progress-meter", 'm', NULL,   MU_OPTION_DEFAULT,
    N_("enable progress meter"),
    mu_c_bool, &progress_meter_option },

  MU_OPTION_END
}, *options[] = { movemail_options, NULL };

static int
cb_mailbox_ownership (void *data, mu_config_value_t *val)
{
  int i;
  
  if (val->type == MU_CFG_STRING)
    set_mailbox_ownership_list (val->v.string);
		
  if (mu_cfg_assert_value_type (val, MU_CFG_LIST))
    return 1;

  for (i = 0; i < val->v.arg.c; i++)
    {
      if (mu_cfg_assert_value_type (&val->v.arg.v[i], MU_CFG_STRING))
	return 1;
      if (set_mailbox_ownership (val->v.arg.v[i].v.string))
	return 1;
    }
  return 0;
}

static int
cb_onerror (void *data, mu_config_value_t *val)
{
  switch (val->type)
    {
    case MU_CFG_LIST:
      mu_list_foreach (val->v.list, set_onerror_action, NULL);
      break;

    case MU_CFG_STRING:
      set_onerror_actions (val->v.string);
      break;

    default:
      mu_error ("%s", _("too many arguments"));
    }
  return 0;
}
  
struct mu_cfg_param movemail_cfg_param[] = {
  { "preserve", mu_c_bool, &preserve_mail, 0, NULL,
    N_("Do not remove messages from the source mailbox.") },
  { "reverse",  mu_c_bool, &reverse_order, 0, NULL,
    N_("Reverse message sorting order.") },
  { "emacs", mu_c_bool, &emacs_mode, 0, NULL,
    N_("Output information used by Emacs rmail interface.") },
  { "uidl", mu_c_bool, &uidl_option, 0, NULL,
    N_("Use UIDLs to avoid downloading the same message twice.") },
  { "verbose", mu_c_int, &verbose_option, 0, NULL,
    N_("Set verbosity level.") },
  { "program-id", mu_c_string, &program_id_option, 0, NULL,
    N_("Set program identifier string (default: program name)") },
  { "mailbox-ownership", mu_cfg_callback, NULL, 0,
    cb_mailbox_ownership,
    N_("Define a list of methods for setting mailbox ownership. Valid "
       "methods are:\n"
       " copy-id          get owner UID and GID from the source mailbox\n"
       " copy-name        get owner name from the source mailbox URL\n"
       " set-id=UID[:GID] set supplied UID and GID\n"
       " set-name=USER    make destination mailbox owned by USER"),
    N_("methods: list") },
  { "max-messages", mu_c_size, &max_messages_option, 0, NULL,
    N_("Copy at most <count> messages."),
    N_("count") },
  { "ignore-errors", mu_c_bool, &ignore_errors, 0, NULL,
    N_("Continue after an error.") },
  { "onerror", mu_cfg_callback, NULL, 0, cb_onerror,
    N_("What to do after an error. Argument is a list of:\n"
       " abort  -  terminate the program (the default)\n"
       " skip   -  skip to the next message\n"
       " delete -  delete this one and to the next message\n"
       " count  -  count this message as processed\n"
       "Each keyword can be prefixed with \"no\" to reverse its meaning."),
    N_("arg: list") },
  { NULL }
};

struct mu_cli_setup cli = {
  options,
  movemail_cfg_param,
  N_("GNU movemail -- move messages across mailboxes."),
  N_("inbox-url destfile [POP-password]")
};

static char *movemail_capa[] = {
  "debug",
  "locking",
  "mailbox",
  "auth",
  NULL 
};

void
die (mu_mailbox_t mbox, const char *msg, int status)
{
  mu_url_t url = NULL;
  
  mu_mailbox_get_url (mbox, &url);
  if (emacs_mode)
    mu_error (_("%s:mailbox `%s': %s: %s"),
	      mu_errname (status),
	      mu_url_to_string (url),
	      msg,
	      mu_strerror (status));
  else
    mu_error (_("mailbox `%s': %s: %s"),
	      mu_url_to_string (url), msg, mu_strerror (status));
  exit (1);
}

void
lock_mailbox (mu_mailbox_t mbox)
{
  mu_locker_t lock;
  int status;
  
  status = mu_mailbox_get_locker (mbox, &lock);
  if (status)
    die (mbox, _("cannot retrieve locker"), status);
      
  if (!lock)
    /* Remote mailboxes have no lockers */
    return;

  status = mu_locker_lock (lock);

  if (status)
    die (mbox, _("cannot lock"), status);
}


void
attach_passwd_ticket (mu_mailbox_t mbx, char *passwd)
{
  mu_folder_t folder = NULL;
  mu_authority_t auth = NULL;
  mu_secret_t secret;
  mu_ticket_t t;
  int rc;

  rc = mu_secret_create (&secret, passwd, strlen (passwd));
  if (rc)
    {
      mu_error ("mu_secret_create: %s", mu_strerror (rc));
      exit (1);
    }
  
  mu_ticket_create (&t, NULL);
  mu_ticket_set_secret (t, secret);

  if ((rc = mu_mailbox_get_folder (mbx, &folder)))
    die (mbx, _("mu_mailbox_get_folder failed"), rc);

  if ((rc = mu_folder_get_authority (folder, &auth)))
    die (mbx, _("mu_folder_get_authority failed"), rc);

  if (auth && (rc = mu_authority_set_ticket (auth, t)))
    die (mbx, _("mu_authority_set_ticket failed"), rc);
}


/* Create and open a mailbox associated with the given URL,
   flags and (optionally) password */
void
open_mailbox (mu_mailbox_t *mbx, char *name, int flags, char *passwd)
{
  int status = mu_mailbox_create_default (mbx, name);

  if (status)
    {
      if (name)
	mu_error (_("could not create mailbox `%s': %s"),
		  name,
		  mu_strerror (status));
      else
	mu_error (_("could not create default mailbox: %s"),
		  mu_strerror (status));
      exit (1);
    }

  if (passwd)
    attach_passwd_ticket (*mbx, passwd);
  status = mu_mailbox_open (*mbx, flags);
  if (status)
    die (*mbx, _("cannot open"), status);
  lock_mailbox (*mbx);
}

int
move_message (mu_mailbox_t dst, mu_message_t msg, size_t msgno)
{
  int rc;

  if ((rc = mu_mailbox_append_message (dst, msg)) != 0)
    {
      mu_error (_("cannot append message %lu: %s"),
		(unsigned long) msgno, mu_strerror (rc));
      if (!(onerror_flags & ONERROR_DELETE))
	return rc;
    }
  if (!preserve_mail)
    {
      mu_attribute_t attr;
      mu_message_get_attribute (msg, &attr);
      mu_attribute_set_deleted (attr);
    }
  return rc;
}

int
movemail (mu_mailbox_t dst, mu_message_t msg, size_t msgno)
{
  int rc = move_message (dst, msg, msgno);
  if (rc == 0)
    ++msg_count;
  else
    {
      app_err_count++;
      if (onerror_flags)
	{
	  if (onerror_flags & ONERROR_COUNT)
	    ++msg_count;
	}
      else
	return 1;
    }
  return max_messages_option && msg_count >= max_messages_option;
}

/* Open source mailbox using compatibility syntax. Source_name is
   of the form:

     po:USERNAME[:POP-SERVER]

   if POP-SERVER part is omitted, the MAILHOST environment variable
   will be consulted. */
void
compatibility_mode (mu_mailbox_t *mbx, char *source_name, char *password,
		    int flags)
{
  char *tmp;
  char *user_name = strtok (source_name+3, ":");
  char *host = strtok (NULL, ":");
  if (!host)
    host = getenv ("MAILHOST");
  if (!host)
    {
      mu_error (_("hostname of the POP3 server is unknown"));
      exit (1);
    }
  mu_asprintf (&tmp, "pop://%s@%s", user_name, host);
  open_mailbox (mbx, tmp, flags, password);
  free (tmp);
}

static mu_mailbox_t source, dest;

static void
close_mailboxes (void)
{
  mu_mailbox_close (dest);
  mu_mailbox_close (source);
}  

static int
get_mbox_owner_id (mu_mailbox_t mbox, mu_url_t url, struct user_id *id)
{
  const char *s;
  int rc = mu_url_sget_scheme  (url, &s);
  if (rc)
    die (mbox, _("cannot get scheme"), rc);
  if ((strcmp (s, "/") == 0
       || strcmp (s, "mbox") == 0
       || strcmp (s, "mh") == 0
       || strcmp (s, "maildir") == 0))
    {
      struct stat st;
      
      rc = mu_url_sget_path  (url, &s);
      if (rc)
	die (mbox, _("cannot get path"), rc);
      if (stat (s, &st))
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "stat", s, errno);
	  exit (1);
	}
      id->uid = st.st_uid;
      id->gid = st.st_gid;
      return 0;
    }
  else if (verbose_option)
    mu_diag_output (MU_DIAG_WARNING,
		    _("ignoring copy-name: not a local mailbox"));
  return 1;
}

static int
get_user_id (const char *name, struct user_id *id)
{
  struct mu_auth_data *auth = mu_get_auth_by_name (name);
  
  if (!auth)
    {
      if (verbose_option)
	mu_diag_output (MU_DIAG_WARNING, _("no such user: %s"), name);
      return 1;
    }

  id->uid = auth->uid;
  id->gid = auth->gid;
  mu_auth_data_free (auth);
  return 0;
}  

static int
get_mbox_owner_name (mu_mailbox_t mbox, mu_url_t url, struct user_id *id)
{
  const char *s;
  int rc = mu_url_sget_user (url, &s);
  if (rc)
    /* FIXME */
    die (mbox, _("cannot get mailbox owner name"), rc);

  return get_user_id (s, id);
}

static int
guess_mbox_owner (mu_mailbox_t mbox, struct user_id *id)
{
  mu_url_t url = NULL;
  int rc;
  struct set_ownership_method *meth;
  
  rc = mu_mailbox_get_url (mbox, &url);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mailbox_get_url", NULL, rc);
      exit (1);
    }

  rc = 1;
  for (meth = so_methods; rc == 1 && meth < so_methods + so_method_num; meth++)
    {
      switch (meth->mode)
	{
	case copy_owner_id:
	  rc = get_mbox_owner_id (mbox, url, id);
	  break;
	  
	case copy_owner_name:
	  rc = get_mbox_owner_name (mbox, url, id);
	  break;
	  
	case set_owner_id:
	  id->uid = meth->owner.id.uid;
	  rc = 0;
	  if (meth->owner.id.gid == (gid_t)-1)
	    {
	      struct passwd *pw = getpwuid (id->uid);
	      if (pw)
		id->gid = pw->pw_gid;
	      else
		{
		  if (verbose_option)
		    mu_diag_output (MU_DIAG_WARNING,
				    _("no user with uid %lu found"),
				    (unsigned long) id->uid);
		  rc = 1;
		}
	    }
	  else
	    id->gid = meth->owner.id.gid;
	  break;
	  
	case set_owner_name:
	  rc = get_user_id (meth->owner.name, id);
	  break;
	}
    }
  
  return rc;
}

static void
switch_owner (mu_mailbox_t mbox)
{
  struct user_id user_id;

  if (so_method_num == 0)
    return;

  if (getuid ())
    {
      if (verbose_option)
	mu_diag_output (MU_DIAG_WARNING,
			_("ignoring mailbox-ownership statement"));
      return;
    }
  
  if (guess_mbox_owner (mbox, &user_id) == 0)
    {
      if (mu_switch_to_privs (user_id.uid, user_id.gid, NULL))
	exit (1);
    }
  else
    {
      mu_error (_("no suitable method for setting mailbox ownership"));
      exit (1);
    }
}

static int
_compare_uidls (const void *item, const void *value)
{
  const struct mu_uidl *a = item;
  const struct mu_uidl *b = value;

  return strcmp (a->uidl, b->uidl);
}

struct movemail_getvar_closure
{
  const char *source_name;
  const char *dest_name;
  mu_url_t source_url;
  mu_url_t dest_url;
};

#define SEQ(s, n, l) \
  (((l) == (sizeof(s) - 1)) && memcmp (s, n, l) == 0)

static int
get_url_part (mu_url_t url, const char *name, size_t nlen, char **ret)
{
  int rc;
  
  if (!url)
    return MU_WRDSE_UNDEF;
  if (SEQ ("user", name, nlen))
    rc = mu_url_aget_user (url, ret);
  else if (SEQ ("host", name, nlen))
    rc = mu_url_aget_host (url, ret);
  else if (SEQ ("port", name, nlen))
    rc = mu_url_aget_portstr (url, ret);
  else if (SEQ ("path", name, nlen))
    rc = mu_url_aget_path (url, ret);
  else
    return MU_WRDSE_UNDEF;

  switch (rc)
    {
    case 0:
      break;
      
    case MU_ERR_NOENT:
      return MU_WRDSE_UNDEF;

    default:
      if (mu_asprintf (ret, "%s", mu_strerror (rc)))
	return MU_WRDSE_NOSPACE;
      return MU_WRDSE_USERERR;
    }
  
  return MU_WRDSE_OK;
}
    
static int
movemail_getvar (char **ret, const char *name, size_t nlen, void *data)
{
  struct movemail_getvar_closure *pc = data;
  const char *s;

  if (nlen > 7 && memcmp ("source_", name, 7) == 0)
    return get_url_part (pc->source_url, name + 7, nlen - 7, ret);

  if (nlen > 5 && memcmp ("dest_", name, 5) == 0)
    return get_url_part (pc->dest_url, name + 5, nlen - 5, ret);
  
  if (SEQ ("progname", name, nlen))
    s = mu_program_name;
  else if (SEQ ("source", name, nlen))
    s = pc->source_name;
  else if (SEQ ("dest", name, nlen))
    s = pc->dest_name;
  else
    return MU_WRDSE_UNDEF;
    
  *ret = strdup (s);
  if (!*ret)
    return MU_WRDSE_NOSPACE;
  return MU_WRDSE_OK;
}

static void
set_program_id (const char *source_name, const char *dest_name)
{
  int rc;
  struct mu_wordsplit ws;
  struct movemail_getvar_closure clos;

  clos.source_name = source_name;
  clos.dest_name = dest_name;
  rc = mu_mailbox_get_url (source, &clos.source_url);
  if (rc)
    mu_diag_output (MU_DIAG_INFO,
		    _("cannot obtain source mailbox URL: %s"),
		    mu_strerror (rc));
  rc = mu_mailbox_get_url (dest, &clos.dest_url);
  if (rc)
    mu_diag_output (MU_DIAG_INFO,
		    _("cannot obtain destination mailbox URL: %s"),
		    mu_strerror (rc));
  
  ws.ws_getvar = movemail_getvar;
  ws.ws_closure = &clos;
  if (mu_wordsplit (program_id_option, &ws,
		    MU_WRDSF_NOSPLIT | MU_WRDSF_NOCMD |
		    MU_WRDSF_GETVAR | MU_WRDSF_CLOSURE))
    {
      mu_error (_("cannot expand line `%s': %s"), program_id_option,
		mu_wordsplit_strerror (&ws));
      return;
    }
  
  /* FIXME: Don't use mu_set_program_name here, because it
     plays wise with its argument. We need a mu_set_diag_prefix
     function. */
  mu_program_name = ws.ws_wordv[0];
  ws.ws_wordc = 0;
  mu_wordsplit_free (&ws);
  mu_stdstream_strerr_setup (MU_STRERR_STDERR);
}

static int
screen_width (void)
{
  struct winsize ws;
  ws.ws_col = 0;
  if (ioctl(1, TIOCGWINSZ, (char *) &ws) < 0)
    {
      char *p = getenv ("COLUMNS");
      if (p)
	ws.ws_col = atol (p);
    }
  if (ws.ws_col == 0)
    return 80;
  return ws.ws_col;
}

static void
progress_format (size_t pos, size_t count)
{
  int n;
  
  fputc ('\r', stdout);
  n = printf ("message %zu/%zu", pos, count);
  n = screen_width () - n;
  while (n--)
    fputc (' ', stdout);
  fflush (stdout);
}

void
progress_start (mu_iterator_t itr)
{
  size_t count;
  
  if (!progress_meter_option)
    return;

  if (mu_iterator_ctl (itr, mu_itrctl_count, &count))
    {
      progress_meter_option = 0;
      return;
    }
  progress_format (0, count);
}

void
progress_mark (mu_iterator_t itr)
{
  size_t count, pos;
  
  if (!progress_meter_option)
    return;

  if (mu_iterator_ctl (itr, mu_itrctl_count, &count)
      || mu_iterator_ctl (itr, mu_itrctl_tell, &pos))
    {
      progress_meter_option = 0;
      return;
    }
  if (reverse_order)
    pos = count - pos + 1;
  progress_format (pos, count);
}

void
progress_stop (void)
{
  if (progress_meter_option)
    fputc ('\n', stdout);
}

int
main (int argc, char **argv)
{
  size_t total;
  int rc = 0;
  char *source_name, *dest_name;
  int flags;
  mu_list_t src_uidl_list = NULL;
  mu_iterator_t itr;
  
  /* Native Language Support */
  MU_APP_INIT_NLS ();
  MU_AUTH_REGISTER_ALL_MODULES ();
  
  /* Register the desired "mailbox" formats.  */
  mu_register_all_formats ();
  /* Register authentication modules */
  mu_auth_register_module (&mu_auth_tls_module);
  
  /* argument parsing */
  mu_cli (argc, argv, &cli, movemail_capa, NULL, &argc, &argv);

  if (argc < 2 || argc > 3)
    {
      mu_error (_("wrong number of arguments"));
      return 1;
    }

  if (ignore_errors)
    onerror_flags |= ONERROR_SKIP|ONERROR_COUNT;

  if (!isatty (1))
    progress_meter_option = 0;
  
  if (emacs_mode)
    {      
      /* Undo the effect of configuration options that may affect
	 interaction with Emacs. */
      mu_registrar_set_default_record (mu_mbox_record);
      mu_stdstream_strerr_setup (MU_STRERR_STDERR);
    }
  
  atexit (close_mailboxes);
  
  source_name = argv[0];
  dest_name = argv[1];

  flags = preserve_mail ? MU_STREAM_READ : MU_STREAM_RDWR;
  
  if (strncmp (source_name, "po:", 3) == 0)
    compatibility_mode (&source, source_name, argv[2], flags);
  else
    open_mailbox (&source, source_name, flags, argv[2]);

  switch_owner (source);
  
  open_mailbox (&dest, dest_name,
		MU_STREAM_APPEND | MU_STREAM_READ | MU_STREAM_CREAT, NULL);

  if (program_id_option)
    set_program_id (source_name, dest_name);

  if (notify)
    {
      rc = mu_mailbox_set_notify (dest, NULL);
      if (rc)
	mu_error (_("failed to set up notification: %s"),
		  mu_strerror (rc));
    }
  
  rc = mu_mailbox_messages_count (source, &total);
  if (rc)
    {
      mu_error(_("cannot count messages: %s"), mu_strerror (rc));
      exit (1);
    }
  
  if (verbose_option)
    {
      mu_diag_output (MU_DIAG_INFO,
		      _("number of messages in source mailbox: %lu"),
		      (unsigned long) total);
      if (max_messages_option)
	mu_diag_output (MU_DIAG_INFO,
			reverse_order ?
			ngettext ("will process last %lu message",
				  "will process last %lu messages",
				  max_messages_option) :
			ngettext ("will process first %lu message",
				  "will process first %lu messages",
				  max_messages_option),
			(unsigned long) max_messages_option);
    }
  
  if (uidl_option)
    {
      mu_list_t dst_uidl_list = NULL;

      rc = mu_mailbox_get_uidls (source, &src_uidl_list);
      if (rc)
	die (source, _("cannot get UIDLs"), rc);
      rc = mu_mailbox_get_uidls (dest, &dst_uidl_list);
      if (rc)
	die (dest, _("cannot get UIDLs"), rc);

      mu_list_set_comparator (dst_uidl_list, _compare_uidls);
      
      mu_list_get_iterator (src_uidl_list, &itr);
      for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
	   mu_iterator_next (itr))
	{
	  struct mu_uidl *uidl;
	      
	  mu_iterator_current (itr, (void **)&uidl);
	  if (mu_list_locate (dst_uidl_list, uidl, NULL) == 0)
	    mu_iterator_ctl (itr, mu_itrctl_delete, NULL);
	}
      mu_list_destroy (&dst_uidl_list);
      mu_list_set_comparator (src_uidl_list, NULL);

      rc = mu_iterator_ctl (itr, mu_itrctl_set_direction, &reverse_order);
      if (rc)
	{
	  mu_error (_("cannot set iteration direction: %s"), mu_strerror (rc));
	  exit (1);
	}
      
      progress_start (itr);
      for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
	   mu_iterator_next (itr))
	{
	  struct mu_uidl *uidl;
	  mu_message_t msg;
	  mu_header_t hdr;

	  mu_iterator_current (itr, (void **)&uidl);

	  if ((rc = mu_mailbox_get_message (source, uidl->msgno, &msg)) != 0)
	    {
	      mu_error (_("cannot read message %lu: %s"),
			(unsigned long) uidl->msgno, mu_strerror (rc));
	      get_err_count++;
	      continue;
	    }

	  /* Check if the downloaded message has X-UIDL header. If not,
	     add one. This check should disappear one mailutils implements
	     alternative storage for mailbox meta-data. */
	  if ((rc = mu_message_get_header (msg, &hdr)))
	    {
	      mu_error (_("%lu: cannot get header: %s"),
			(unsigned long) uidl->msgno, mu_strerror (rc));
	    }
	  else
	    {
	      char const *suidl = NULL;

	      if ((rc = mu_header_sget_value (hdr, MU_HEADER_X_UIDL, &suidl)))
		{
		  if (rc != MU_ERR_NOENT)
		    mu_error (_("%lu: cannot get %s: %s"),
			      (unsigned long) uidl->msgno, MU_HEADER_X_UIDL,
			      mu_strerror (rc));
		}
	      else if (strcmp (suidl, uidl->uidl))
		{
		  mu_error (_("%lu: stored and reported UIDL differ; fixing"),
			    (unsigned long) uidl->msgno);
		  suidl = NULL;
		}

	      if ((rc = mu_header_set_value (hdr, MU_HEADER_X_UIDL,
					     uidl->uidl, 1)))
		{
		  mu_error (_("%lu: cannot set header: %s"),
			    (unsigned long) uidl->msgno, mu_strerror (rc));
		}
	    }
	  progress_mark (itr);
	  if (movemail (dest, msg, uidl->msgno))
	    break;
	}
    }
  else
    {
      rc = mu_mailbox_get_iterator (source, &itr);
      if (rc)
	{
	  mu_error (_("cannot obtain mailbox iterator: %s"), mu_strerror (rc));
	  return 1;
	}
  
      rc = mu_iterator_ctl (itr, mu_itrctl_set_direction, &reverse_order);
      if (rc)
	{
	  mu_error (_("cannot set iteration direction: %s"),
		    mu_strerror (rc));
	  return 1;
	}

      progress_start (itr);
      for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
	   mu_iterator_next (itr))
	{
	  mu_message_t msg;
	  size_t msgno;
	  
	  rc = mu_iterator_ctl (itr, mu_itrctl_tell, &msgno);
	  if (rc)
	    {
	      mu_error (_("cannot get iterator position: %s"),
			mu_strerror (rc));
	      return 1;
	    }

	  rc = mu_iterator_current (itr, (void **)&msg);
	  if (rc)
	    {
	      mu_error (_("cannot read message %lu: %s"),
			(unsigned long) msgno, mu_strerror (rc));
	      get_err_count++;
	      continue;
	    }
	  
	  if (movemail (dest, msg, msgno))
	    break;
	  progress_mark (itr);
	}
    }
  progress_stop ();
  mu_iterator_destroy (&itr);
  
  if (verbose_option)
    {
      mu_diag_output (MU_DIAG_INFO,
		      _("number of processed messages: %lu"),
		      (unsigned long) msg_count);
      mu_diag_output (MU_DIAG_INFO,
		      _("number of errors: %lu / %lu"),
		      (unsigned long) get_err_count,
		      (unsigned long) app_err_count);
    }
  
  if (app_err_count && !(onerror_flags & (ONERROR_DELETE|ONERROR_COUNT)))
    preserve_mail = 1;
  if (onerror_flags & ONERROR_COUNT)
    app_err_count = 0;
      
  mu_mailbox_sync (dest);
  rc = mu_mailbox_close (dest);
  mu_mailbox_destroy (&dest);
  if (rc)
    mu_error (_("cannot close destination mailbox: %s"), mu_strerror (rc));
  else if (!preserve_mail)
    mu_mailbox_expunge (source);

  mu_mailbox_close (source);
  mu_mailbox_destroy (&source);
  
  return !(rc == 0 && (app_err_count + get_err_count) == 0);
}
