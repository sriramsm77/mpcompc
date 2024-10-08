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

/* Initialize MH applications. */

#include <mh.h>
#include <mailutils/url.h>
#include <mailutils/tls.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <sys/ioctl.h>

void
mh_init (void)
{
  mu_stdstream_setup (MU_STDSTREAM_RESET_NONE);
  
  /* Register all mailbox and mailer formats */
  mu_register_all_formats ();

  /* Read user's profile */
  mh_read_profile ();
}

void
mh_init2 (void)
{
  mh_current_folder ();
}

void
mh_err_memory (int fatal)
{
  mu_error (_("not enough memory"));
  if (fatal)
    abort ();
}

static mu_address_t
mh_local_mailbox (void)
{
  static mu_address_t local_mailbox;
  if (!local_mailbox)
    {
      const char *p = mh_global_profile_get ("Local-Mailbox", NULL);
      if (!p)
	p = mu_get_user_email (NULL);
      mu_address_create (&local_mailbox, p);
    }
  return local_mailbox;
}

char const *
mh_get_my_user_name (void)
{
  mu_address_t addr = mh_local_mailbox ();
  char const *s;
  MU_ASSERT (mu_address_sget_local_part (addr, 1, &s));
  return s;
}

char const *
mh_get_my_real_name (void)
{
  mu_address_t addr = mh_local_mailbox ();
  char const *s;
  if (mu_address_sget_personal (addr, 1, &s))
    return NULL;
  return s;
}

char const *
mh_my_email (void)
{
  mu_address_t addr = mh_local_mailbox ();
  char const *s;
  MU_ASSERT (mu_address_sget_printable (addr, &s));
  return s;
}

char const *
mh_my_host (void)
{
  mu_address_t addr = mh_local_mailbox ();
  char const *s;
  MU_ASSERT (mu_address_sget_domain (addr, 1, &s));
  return s;
}

enum part_match_mode
  {
    part_match_local,
    part_match_domain
  };

enum part_match_result
  {
    part_match_false,
    part_match_true,
    part_match_abort
  };

static int match_char_class (char const **pexpr, char c, int icase);

static enum part_match_result
part_match (char const *expr, char const *name, enum part_match_mode mode)
{
  int c;

  while (*expr)
    {
      if (*name == 0 && *expr != '*')
	return part_match_abort;
      switch (*expr)
	{
	case '*':
	  while (*++expr == '*')
	    ;
	  if (*expr == 0)
	    return part_match_true;
	  while (*name)
	    {
	      int res = part_match (expr, name++, mode);
	      if (res != part_match_false)
		return res;
	    }
	  return part_match_abort;

	case '?':
	  expr++;
	  if (*name == 0)
	    return part_match_false;
	  name++;
	  break;
	  
	case '[':
	  if (!match_char_class (&expr, *name, mode == part_match_domain))
	    return part_match_false;
	  name++;
	  break;

	case '\\':
	  if (expr[1])
	    {
	      c = *++expr; expr++;
	      if (*name != mu_wordsplit_c_unquote_char (c))
		return part_match_false;
	      name++;
	      break;
	    }
	  /* fall through */

	default:
	  if (mode == part_match_local)
	    {
	      if (*expr != *name)
		return part_match_false;
	      if ('@' == *name)
		mode = part_match_domain;
	    }
	  else
	    {
	      if (mu_tolower (*expr) != mu_tolower (*name))
		return part_match_false;
	    }
	  expr++;
	  name++;
	}
    }

  if (*name == 0)
    return part_match_true;
  
  if (mode == part_match_local && *name == '@')
    return part_match_true;

  return part_match_false;
}

static int
match_char_class (char const **pexpr, char c, int icase)
{
  int res;
  int rc;
  char const *expr = *pexpr;

  if (icase)
    c = mu_toupper (c);

  expr++;
  if (*expr == '^')
    {
      res = 0;
      expr++;
    }
  else
    res = 1;

  if (*expr == '-' || *expr == ']')
    rc = c == *expr++;
  else
    rc = !res;

  for (; *expr && *expr != ']'; expr++)
    {
      if (rc == res)
	{
	  if (*expr == '\\' && expr[1] == ']')
	    expr++;
	}
      else if (expr[1] == '-')
	{
	  if (*expr == '\\')
	    rc = *++expr == c;
	  else
	    {
	      if (icase)
		rc = mu_toupper (*expr) <= c && c <= mu_toupper (expr[2]);
	      else
		rc = *expr <= c && c <= expr[2];
	      expr += 2;
	    }
	}
      else if (*expr == '\\' && expr[1] == ']')
	rc = *++expr == c;
      else if (icase)
	rc = mu_toupper(*expr) == c;
      else
	rc = *expr == c;
    }
  *pexpr = *expr ? expr + 1 : expr;
  return rc == res;
}

static int
email_match (char const *pattern, char const *name)
{
  return part_match (pattern, name, part_match_local) == part_match_true;
}

int
mh_is_my_name (const char *name)
{
  static mu_address_t addr;
  mu_address_t p;
  
  if (!addr)
    {
      const char *nlist;
      int rc;
      
      rc = mu_address_create (&addr, mh_my_email ());
      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_address_create", mh_my_email (),
			   rc);
	  return 0;
	}
      
      nlist = mh_global_profile_get ("Alternate-Mailboxes", NULL);
      if (nlist)
	{
	  mu_address_t tmp;
	  struct mu_address hint;

	  hint.domain = NULL;
	  rc = mu_address_create_hint (&tmp, nlist, &hint,
				       MU_ADDR_HINT_DOMAIN);
	  if (rc == 0)
	    {
	      rc = mu_address_union (&addr, tmp);
	      if (rc)
		mu_diag_funcall (MU_DIAG_ERROR, "mu_address_union", NULL, rc);
	      mu_address_destroy (&tmp);
	    }
	  else
	    {
	      mu_error (_("bad Alternate-Mailboxes: %s; please fix"),
			mu_strerror (rc));
	    }
	}
    }

  for (p = addr; p; p = p->next)
    {
      if (email_match (p->email, name))
	return 1;
    }
  return 0;
}

static int
make_dir_hier (const char *p, mode_t perm)
{
  int rc = 0;
  char *dir = mu_strdup (p);
  char *q = dir;

  while (!rc && (q = strchr (q + 1, '/')))
    {
      *q = 0;
      if (access (dir, X_OK))
	{
	  if (errno != ENOENT)
	    {
	      mu_error (_("cannot create directory %s: error accessing name component %s: %s"),

			p, dir, strerror (errno));
	      rc = 1;
	    }
	  else if ((rc = mkdir (dir, perm)))
	    mu_error (_("cannot create directory %s: error creating name component %s: %s"),
		      p, dir, mu_strerror (rc));
	}
      *q = '/';
    }
  free (dir);
  return rc;
}

int
mh_makedir (const char *p)
{
  int rc;
  mode_t save_umask;
  mode_t perm = 0711;
  const char *pb = mh_global_profile_get ("Folder-Protect", NULL);
  if (pb)
    perm = strtoul (pb, NULL, 8);

  save_umask = umask (0);

  if ((rc = make_dir_hier (p, perm)) == 0)
    {
      rc = mkdir (p, perm);
      if (rc)
	mu_error (_("cannot create directory %s: %s"),
		  p, strerror (errno));
    }

  umask (save_umask);
  return rc;
}

int
mh_check_folder (const char *pathname, int confirm)
{
  const char *p;
  struct stat st;
  
  if ((p = strchr (pathname, ':')) != NULL)
    p++;
  else
    p = pathname;
  
  if (stat (p, &st))
    {
      if (errno == ENOENT)
	{
	  /* TRANSLATORS: This is a question and will be followed
	     by question mark on output. */
	  if (!confirm || mh_getyn (_("Create folder \"%s\""), p))
	    return mh_makedir (p);
	  else
	    return 1;
	}
      else
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "stat", p, errno);
	  return 1;
	}
    }
  return 0;
}

int
mh_interactive_mode_p (void)
{
  static int interactive = -1;

  if (interactive < 0)
    interactive = isatty (fileno (stdin)) ? 1 : 0;
  return interactive;
}

int
mh_vgetyn (const char *fmt, va_list ap)
{
  char repl[64];

  while (1)
    {
      char *p;
      int len, rc;
      
      vfprintf (stdout, fmt, ap);
      fprintf (stdout, "? ");
      p = fgets (repl, sizeof repl, stdin);
      if (!p)
	return 0;
      len = strlen (p);
      if (len > 0 && p[len-1] == '\n')
	p[len--] = 0;

      rc = mu_true_answer_p (p);

      if (rc >= 0)
	return rc;

      /* TRANSLATORS: See msgids "nN" and "yY". */
      fprintf (stdout, _("Please answer yes or no: "));
    }
  return 0; /* to pacify gcc */
}

int
mh_getyn (const char *fmt, ...)
{
  va_list ap;
  int rc;
  
  if (!mh_interactive_mode_p ())
      return 1;
  va_start (ap, fmt);
  rc = mh_vgetyn (fmt, ap);
  va_end (ap);
  return rc;
}

int
mh_getyn_interactive (const char *fmt, ...)
{
  va_list ap;
  int rc;
  
  va_start (ap, fmt);
  rc = mh_vgetyn (fmt, ap);
  va_end (ap);
  return rc;
}
	    
mu_stream_t
mh_audit_open (char *name, mu_mailbox_t mbox)
{
  mu_stream_t str;
  char date[64];
  time_t t;
  struct tm *tm;
  mu_url_t url;
  char *namep;
  int rc;
    
  namep = mu_tilde_expansion (name, MU_HIERARCHY_DELIMITER, NULL);
  if (strchr (namep, MU_HIERARCHY_DELIMITER) == NULL)
    {
      char *p = mh_safe_make_file_name (mu_folder_directory (), namep);
      free (namep);
      namep = p;
    }

  rc = mu_file_stream_create (&str, namep, MU_STREAM_CREAT|MU_STREAM_APPEND);
  if (rc)
    {
      mu_error (_("cannot open audit file %s: %s"), namep, strerror (rc));
      free (namep);
      return NULL;
    }
  free (namep);
  
  time (&t);
  tm = localtime (&t);
  mu_strftime (date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", tm);
  mu_mailbox_get_url (mbox, &url);
  
  mu_stream_printf (str, "<<%s>> %s %s\n",
		    mu_program_name,
		    date,
		    mu_url_to_string (url));
  return str;
}

void
mh_audit_close (mu_stream_t str)
{
  mu_stream_close (str);
}

int
mh_message_number (mu_message_t msg, size_t *pnum)
{
  return mu_message_get_uid (msg, pnum);	
}

mu_mailbox_t
mh_open_folder (const char *folder, int flags)
{
  mu_mailbox_t mbox = NULL;
  char *name;
  
  name = mh_expand_name (NULL, folder, NAME_FOLDER);
  if ((flags & MU_STREAM_CREAT) && mh_check_folder (name, 1))
    exit (0);
    
  if (mu_mailbox_create_default (&mbox, name))
    {
      mu_error (_("cannot create mailbox %s: %s"),
		name, strerror (errno));
      exit (1);
    }

  if (mu_mailbox_open (mbox, flags))
    {
      mu_error (_("cannot open mailbox %s: %s"), name, strerror (errno));
      exit (1);
    }

  free (name);

  return mbox;
}

char *
mh_get_dir (void)
{
  const char *mhdir = mh_global_profile_get ("Path", "Mail");
  char *mhcopy;
  
  if (mhdir[0] != '/')
    {
      char *p = mu_get_homedir ();
      mhcopy = mh_safe_make_file_name (p, mhdir);
      free (p);
    }
  else 
    mhcopy = strdup (mhdir);
  if (!mhcopy)
    {
      mu_error (_("not enough memory"));
      abort ();
    }
  return mhcopy;
}

char *
mh_expand_name (const char *base, const char *name, int what)
{
  char *p = NULL;
  char *namep = NULL;
  
  namep = mu_tilde_expansion (name, MU_HIERARCHY_DELIMITER, NULL);
  if (namep[0] == '+')
    memmove (namep, namep + 1, strlen (namep)); /* copy null byte as well */
  else if (strncmp (namep, "../", 3) == 0 || strncmp (namep, "./", 2) == 0)
    {
      char *cwd = mu_getcwd ();
      char *tmp = mh_safe_make_file_name (cwd, namep);
      free (cwd);
      if (what == NAME_FILE)
	return tmp;
      free (namep);
      namep = tmp;
    }
  
  if (what == NAME_FOLDER)
    {
      if (memcmp (namep, "mh:/", 4) == 0)
	return namep;
      else if (namep[0] == '/')
	mu_asprintf (&p, "mh:%s", namep);
      else
	mu_asprintf (&p, "mh:%s/%s", base ? base : mu_folder_directory (), 
                     namep);
    }
  else if (namep[0] != '/')
    {
        if (what == NAME_FILE)
	  {
	    char *cwd = mu_getcwd ();
	    p = mh_safe_make_file_name (cwd, namep);
	    free (cwd);
	  }
	else
	  p = mh_safe_make_file_name (base ? base : mu_folder_directory (),
				      namep);
    }
  else
    return namep;
  
  free (namep);
  return p;
}

int
mh_find_file (const char *name, char **resolved_name)
{
  char *s;
  int rc;
  
  if (name[0] == '/' ||
      (name[0] == '.' && name[1] == '/') ||
      (name[0] == '.' && name[1] == '.' && name[2] == '/'))
    {
      *resolved_name = mu_strdup (name);
      if (access (name, R_OK) == 0)
	return 0;
      return errno;
    }

  if (name[0] == '~')
    {
      s = mu_tilde_expansion (name, MU_HIERARCHY_DELIMITER, NULL);
      *resolved_name = s;
      if (access (s, R_OK) == 0)
	return 0;
      return errno;
    }
  
  s = mh_expand_name (NULL, name, NAME_ANY);
  if (access (s, R_OK) == 0)
    {
      *resolved_name = s;
      return 0;
    }
  if (errno != ENOENT)
    mu_diag_output (MU_DIAG_WARNING,
		    _("cannot access %s: %s"), s, mu_strerror (errno));
  free (s);

  s = mh_expand_name (mh_global_profile_get ("mhetcdir", MHLIBDIR), name,
                      NAME_ANY);
  if (access (s, R_OK) == 0)
    {
      *resolved_name = s;
      return 0;
    }
  if (errno != ENOENT)
    mu_diag_output (MU_DIAG_WARNING,
		    _("cannot access %s: %s"), s, mu_strerror (errno));
  free (s);

  *resolved_name = mu_strdup (name);
  if (access (name, R_OK) == 0)
    return 0;
  rc = errno;
  if (rc != ENOENT)
    mu_diag_output (MU_DIAG_WARNING,
		    _("cannot access %s: %s"), s, mu_strerror (rc));

  return rc;
}

int
mh_spawnp (const char *prog, const char *file)
{
  struct mu_wordsplit ws;
  size_t i;
  int rc, status;
  char **xargv;

  ws.ws_comment = "#";
  if (mu_wordsplit (prog, &ws, MU_WRDSF_DEFFLAGS | MU_WRDSF_COMMENT))
    {
      mu_error (_("cannot split line `%s': %s"), prog,
		mu_wordsplit_strerror (&ws));
      return 1;
    }

  xargv = calloc (ws.ws_wordc + 2, sizeof (*xargv));
  if (!xargv)
    {
      mh_err_memory (0);
      mu_wordsplit_free (&ws);
      return 1;
    }

  for (i = 0; i < ws.ws_wordc; i++)
    xargv[i] = ws.ws_wordv[i];
  xargv[i++] = (char*) file;
  xargv[i++] = NULL;
  
  rc = mu_spawnvp (xargv[0], xargv, &status);

  free (xargv);
  mu_wordsplit_free (&ws);

  return rc;
}

/* Copy data from FROM to TO, creating the latter if necessary.
   FIXME: How about formats?
*/
int
mh_file_copy (const char *from, const char *to)
{
  mu_stream_t in, out, flt;
  int rc;
  
  if ((rc = mu_file_stream_create (&in, from, MU_STREAM_READ)))
    {
      mu_error (_("cannot open input file `%s': %s"),
		from, mu_strerror (rc));
      return 1;
    }

  if ((rc = mu_file_stream_create (&out, to, MU_STREAM_RDWR|MU_STREAM_CREAT)))
    {
      mu_error (_("cannot open output file `%s': %s"),
		to, mu_strerror (rc));
      mu_stream_destroy (&in);
      return 1;
    }

  rc = mu_filter_create (&flt, in, "INLINE-COMMENT", MU_FILTER_DECODE,
			 MU_STREAM_READ);
  mu_stream_unref (in);
  if (rc)
    {
      mu_error (_("cannot open filter stream: %s"), mu_strerror (rc));
      mu_stream_destroy (&out);
      return 1;
    }
  
  rc = mu_stream_copy (out, flt, 0, NULL);
      
  mu_stream_destroy (&flt);
  mu_stream_destroy (&out);

  if (rc)
    mu_error (_("error copying file `%s' to `%s': %s"),
	      from, to, mu_strerror (rc));
  
  return rc;
}

static mu_message_t
_file_to_message (const char *file_name)
{
  struct stat st;
  int rc;
  mu_stream_t instream;

  if (stat (file_name, &st) < 0)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "stat", file_name, errno);
      return NULL;
    }
  
  if ((rc = mu_file_stream_create (&instream, file_name, MU_STREAM_READ)))
    {
      mu_error (_("cannot create input stream (file %s): %s"),
		file_name, mu_strerror (rc));
      return NULL;
    }
  
  return mh_stream_to_message (instream);
}

mu_message_t
mh_file_to_message (const char *folder, const char *file_name)
{
  mu_message_t msg;
  char *tmp_name = NULL;
  
  if (folder)
    {
      tmp_name = mh_expand_name (folder, file_name, NAME_ANY);
      msg = _file_to_message (tmp_name);
      free (tmp_name);
    }
  else
    msg = _file_to_message (file_name);
  
  return msg;
}

void
mh_install_help (char *mhdir)
{
  static char *text = N_(
"Prior to using MH, it is necessary to have a file in your login\n"
"directory (%s) named .mh_profile which contains information\n"
"to direct certain MH operations.  The only item which is required\n"
"is the path to use for all MH folder operations.  The suggested MH\n"
"path for you is %s...\n");

  printf (_(text), mu_get_homedir (), mhdir);
}

void
mh_real_install (char *name, int automode)
{
  char *home = mu_get_homedir ();
  char *mhdir;
  char *ctx;
  int rc;
  mu_stream_t profile;

  mhdir = mh_safe_make_file_name (home, "Mail");
  
  if (!automode)
    {
      /* TRANSLATORS: This is a question and will be followed
	 by question mark on output. */
      if (mh_getyn_interactive (_("Do you need help")))
	mh_install_help (mhdir);

      /* TRANSLATORS: This is a question and will be followed
	 by question mark on output. */
      if (!mh_getyn_interactive (_("Do you want the standard MH path \"%s\""), mhdir))
	{
	  int local;
	  char *p, *buf = NULL;
	  size_t size = 0;
	  
	  /* TRANSLATORS: This is a question and will be followed
	     by question mark on output. */
	  local = mh_getyn_interactive (_("Do you want a path below your login directory"));
	  if (local)
	    mu_printf (_("What is the path? "));
	  else
	    mu_printf (_("What is the full path? "));
	  mu_stream_flush (mu_strin);
	  if (mu_stream_getline (mu_strin, &buf, &size, NULL))
	    exit (1);
	  p = mu_str_stripws (buf);
	  if (p > buf)
	    memmove (buf, p, strlen (p) + 1);
	  
	  free (mhdir);
	  if (local)
	    {
              mhdir = mh_safe_make_file_name (home, p);
	      free (p);
	    }
	  else
	    mhdir = p;
	}
    }

  if (mh_check_folder (mhdir, !automode))
    exit (1);

  rc = mu_file_stream_create (&profile, name,
			      MU_STREAM_WRITE | MU_STREAM_CREAT);
  if (rc)
    {
      mu_error (_("cannot open file %s: %s"), name, mu_strerror (rc));
      exit (1);
    }
  mu_stream_printf (profile, "Path: %s\n", mhdir);
  mu_stream_destroy (&profile);

  ctx = mh_safe_make_file_name (mhdir, MH_CONTEXT_FILE);
  rc = mu_file_stream_create (&profile, ctx,
			      MU_STREAM_WRITE | MU_STREAM_CREAT);
  if (rc)
    {
      mu_stream_printf (profile, "Current-Folder: inbox\n");
      mu_stream_destroy (&profile);
    }
  free (ctx);
  ctx = mh_safe_make_file_name (mhdir, "inbox");
  if (mh_check_folder (ctx, !automode))
    exit (1);
  free (ctx);
  free (mhdir);
}  

void
mh_install (char *name, int automode)
{
  struct stat st;
  
  if (stat (name, &st))
    {
      if (errno == ENOENT)
	{
	  if (automode)
	    printf(_("I'm going to create the standard MH path for you.\n"));
	  mh_real_install (name, automode);
	}
      else
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "stat", name, errno);
	  exit (1);
	}
    }
  else if ((st.st_mode & S_IFREG) || (st.st_mode & S_IFLNK)) 
    {
      mu_error(_("You already have an MH profile, use an editor to modify it"));
      exit (1);
    }
  else
    {
      mu_error (_("You already have file %s which is not a regular file or a symbolic link."), name);
      mu_error (_("Please remove it and try again"));
      exit (1);
    }
}
        
void
mh_annotate (mu_message_t msg, const char *field, const char *text, int date)
{
  mu_header_t hdr;
  mu_attribute_t attr;
  
  if (mu_message_get_header (msg, &hdr))
    return;

  if (date)
    {
      time_t t;
      struct tm *tm;
      char datebuf[80];
      t = time (NULL);
      tm = localtime (&t);
      mu_strftime (datebuf, sizeof datebuf, "%a, %d %b %Y %H:%M:%S %Z", tm);

      mu_header_prepend (hdr, field, datebuf);
    }

  if (text)
    mu_header_prepend (hdr, field, text);
  mu_message_get_attribute (msg, &attr);
  mu_attribute_set_modified (attr);
}

char *
mh_create_message_id (int subpart)
{
  char *p;
  mu_rfc2822_msg_id (subpart, &p);
  return p;
}

void
mh_set_reply_regex (const char *str)
{
  char *err;
  int rc = mu_unre_set_regex (str, 0, &err);
  if (rc)
    mu_error ("reply_regex: %s%s%s", mu_strerror (rc),
	      err ? ": " : "",
	      mu_prstr (err));
}

const char *
mh_charset (const char *dfl)
{
  const char *charset = mh_global_profile_get ("Charset", dfl);

  if (!charset)
    return NULL;
  if (mu_c_strcasecmp (charset, "auto") == 0)
    {
      static char *saved_charset;

      if (!saved_charset)
	{
	  /* Try to deduce the charset from LC_ALL variable */
	  struct mu_lc_all lc_all;
	  if (mu_parse_lc_all (getenv ("LC_ALL"), &lc_all, MU_LC_CSET) == 0)
	    saved_charset = lc_all.charset; /* FIXME: Mild memory leak */
	}
      charset = saved_charset;
    }
  return charset;
}

int
mh_decode_2047 (char const *text, char **decoded_text)
{
  const char *charset = mh_charset (NULL);
  if (!charset)
    return 1;
  
  return mu_rfc2047_decode (charset, text, decoded_text);
}

void
mh_quote (const char *in, char **out)
{
  size_t len = strlen (in);
  if (len && in[0] == '"' && in[len - 1] == '"')
    {
      const char *p;
      char *q;
      
      for (p = in + 1; p < in + len - 1; p++)
        if (*p == '\\' || *p == '"')
	  len++;

      *out = mu_alloc (len + 1);
      q = *out;
      p = in;
      *q++ = *p++;
      while (p[1])
	{
	  if (*p == '\\' || *p == '"')
	    *q++ = '\\';
	  *q++ = *p++;
	}
      *q++ = *p++;
      *q = 0;
    }
  else
    *out = mu_strdup (in);
}

void
mh_expand_aliases (mu_message_t msg,
		   mu_address_t *addr_to,
		   mu_address_t *addr_cc,
		   mu_address_t *addr_bcc)
{
  mu_header_t hdr;
  size_t i, num;
  const char *buf;
  
  mu_message_get_header (msg, &hdr);
  mu_header_get_field_count (hdr, &num);
  for (i = 1; i <= num; i++)
    {
      if (mu_header_sget_field_name (hdr, i, &buf) == 0)
	{
	  if (mu_c_strcasecmp (buf, MU_HEADER_TO) == 0
	      || mu_c_strcasecmp (buf, MU_HEADER_CC) == 0
	      || mu_c_strcasecmp (buf, MU_HEADER_BCC) == 0)
	    {
	      char *value;
	      mu_address_t addr = NULL;
	      int incl;
	      
	      mu_header_aget_field_value_unfold (hdr, i, &value);
	      
	      mh_alias_expand (value, &addr, &incl);
	      free (value);
	      if (mu_c_strcasecmp (buf, MU_HEADER_TO) == 0)
		mu_address_union (addr_to, addr);
	      else if (mu_c_strcasecmp (buf, MU_HEADER_CC) == 0)
		mu_address_union (addr_cc ? addr_cc : addr_to, addr);
	      else if (mu_c_strcasecmp (buf, MU_HEADER_BCC) == 0)
		mu_address_union (addr_bcc ? addr_bcc : addr_to, addr);
	    }
	}
    }
}

int
mh_draft_message (const char *name, const char *msgspec, char **pname)
{
  mu_url_t url;
  size_t uid;
  int rc;
  mu_mailbox_t mbox;
  const char *path;
  
  mbox = mh_open_folder (name, MU_STREAM_RDWR);
  if (!mbox)
    return 1;
  
  mu_mailbox_get_url (mbox, &url);

  if (strcmp (msgspec, "new") == 0)
    {
      mu_property_t prop;
      
      rc = mu_mailbox_uidnext (mbox, &uid);
      if (rc)
	{
	  mu_error (_("cannot obtain sequence number for the new message: %s"),
		    mu_strerror (rc));
	  exit (1);
	}
      mu_mailbox_get_property (mbox, &prop);
      mu_property_set_value (prop, "cur", mu_umaxtostr (0, uid), 1);
    }
  else
    {
      char *argv[2];
      mu_msgset_t msgset;
      
      argv[0] = (char*) msgspec;
      argv[1] = NULL;
      mh_msgset_parse (&msgset, mbox, 1, argv, "cur");
      if (!mh_msgset_single_message (msgset))
	mu_error (_("only one message at a time!"));
      else
	uid = mh_msgset_first (msgset, RET_UID);
      mu_msgset_free (msgset);
    }

  mu_url_sget_path (url, &path);
  rc = mu_asprintf (pname, "%s/%lu", path, (unsigned long) uid);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_asprintf", NULL, rc);
      exit (1);
    }
  mu_mailbox_close (mbox);
  mu_mailbox_destroy (&mbox);
  return rc;
}

char *
mh_safe_make_file_name (const char *dir, const char *file)
{
  char *name = mu_make_file_name (dir, file);
  if (!name)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_make_file_name", NULL, ENOMEM);
      abort ();
    }
  return name;
}

int
mh_width (void)
{
  struct winsize ws;
  ws.ws_col = ws.ws_row = 0;
  if ((ioctl(1, TIOCGWINSZ, (char *) &ws) < 0) || ws.ws_col == 0)
    return 80;  /* FIXME: Should we exit()/abort() if col <= 0 ?  */
  return ws.ws_col;
}
			  
