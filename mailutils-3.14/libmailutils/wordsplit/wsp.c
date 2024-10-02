/* wsp - test program for wordsplit
   Copyright (C) 2014-2019 Sergey Poznyakoff

   Wordsplit is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   Wordsplit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with wordsplit. If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "wordsplit-version.h"
#include "wordsplit.h"

extern char **environ;

char *progname;

/* Global options */
enum
  {
    TRIMNL_OPTION    = 0x01,   /* Remove trailing newline */
    PLAINTEXT_OPTION = 0x02    /* Print intput verbatim (no escapes) */
  };

/* Environment types */
enum env_type
  {
    env_none,                  /* No environment */
    env_null,                  /* Null environment */
    env_sys,                   /* Use system environment */
    env_extra                  /* Use small built-in "extra" environment */
  };

enum
  {
    MAX_F_ENV = 16,
    MAX_X_ENV = 16
  };

struct wsclosure
{
  int options;            /* Global options */
  struct wordsplit ws;    /* The wordsplit structure */
  int wsflags;            /* Wordsplit flags */
  enum env_type env_type; /* Environment type */
  int offarg;             /* Index of the first of the initial words in
			     the argv array. The ws.ws_dooffs field gives
			     the number of such variables. Forces the
			     WRDSF_DOOFFS flag. */
  char *fenvbase[MAX_F_ENV+1];
                          /* Environment for testing the ws_getenv function */
  int fenvidx;            /* Number of variables in fenvbase */

  char *xenvbase[MAX_X_ENV+1];
                          /* Extra environment variables */
  int xenvidx;            /* Number of variables in xenvbase */
  
  int append_start;       /* First argument to append (index in argv) */
  int append_count;       /* Number of arguments to append */
};

/* Command line option types */
enum
  {
    ws_no_argument,       /* Option requires no arguments */
    ws_boolean,           /* Option is boolean (can be prefixed with -no) */
    ws_required_argument, /* Option requires one argument */
    ws_multiple_arguments /* Option takes multiple arguments, terminated with
			     "--" or end of argument list */
  };

/* Structure describing a single command-line option */
struct wsopt
{
  const char *name;    /* Option name */
  int tok;             /* Corresponding flag */
  int arg;             /* Option type (see the enum above) */
  void (*setfn) (int tok, int neg, char *arg, struct wsclosure *wsc);
                       /* Setter function */
};

/* Index of the next argument in the argv */
static int wsoptind = -1;

void
print_version (void)
{
  printf ("wsp (wordsplit %s)\n", WORDSPLIT_VERSION);
}

/* Parse next argument from the command line. Return EOF on end of arguments
   or when the "--" argument is seen. */
static int
getwsopt (int argc, char **argv, struct wsopt *wso, struct wsclosure *wsc)
{
  int negate = 0;
  char *opt;

  if (wsoptind == -1)
    wsoptind = 1;
  if (wsoptind == argc)
    return EOF;

  opt = argv[wsoptind++];
  if (strcmp (opt, "--") == 0)
    return EOF;
  if (*opt != '-')
    {
      if (strchr (opt, '='))
	{
	  assert (wsc->fenvidx < MAX_F_ENV);
	  wsc->fenvbase[wsc->fenvidx++] = opt;
	  return 0;
	}
      wsoptind--;
      return EOF;
    }

  if (strncmp (opt, "-D", 2) == 0)
    {
      char *asgn;
      
      if (opt[2])
	asgn = opt + 2;
      else if (wsoptind == argc)
	{
	  fprintf (stderr, "%s: missing arguments for -D\n",
		   progname);
	  exit (1);
	}
      else
	asgn = argv[wsoptind++];

      if (strchr (asgn, '='))
	{
	  assert (wsc->xenvidx < MAX_F_ENV);
	  wsc->xenvbase[wsc->xenvidx++] = asgn;
	  return 0;
	}
      wsoptind--;
      return EOF;
    }	  
  
  if (strcmp (opt, "--version") == 0)
    {
      print_version ();
      exit (0);
    }

  opt++; /* skip past initial dash */
  
  if (strncmp (opt, "no-", 3) == 0)
    {
      negate = 1;
      opt += 3;
    }
  else if (strncmp (opt, "no", 2) == 0)
    {
      negate = 1;
      opt += 2;
    }
  
  for (; wso->name; wso++)
    {
      if (wso->arg == ws_boolean && wso->name[0] == 'n' && wso->name[1] == 'o'
	  && strcmp (wso->name + 2, opt) == 0)
	{
	  negate ^= 1;
	  break;
	}
      if (strcmp (wso->name, opt) == 0)
	break;
    }

  if (wso->name)
    {
      char *arg;
      if (wso->arg == ws_multiple_arguments)
	{
	  while (1)
	    {
	      if (wsoptind == argc)
		break;
	      arg = argv[wsoptind++];
	      if (strcmp (arg, "--") == 0)
		break;
	      wso->setfn (wso->tok, negate, arg, wsc);
	    }
	}
      else
	{
	  if (wso->arg == ws_required_argument)
	    {
	      if (wsoptind == argc)
		{
		  fprintf (stderr, "%s: missing arguments for -%s\n",
			   progname, opt);
		  exit (1);
		}
	      arg = argv[wsoptind++];
	    }
	  wso->setfn (wso->tok, negate, arg, wsc);
	}
      return 0;
    }

  fprintf (stderr, "%s: unrecognized option: -%s\n",
	   progname, opt);
  fprintf (stderr, "%s: try %s -help for more detail\n",
	   progname, progname);
  exit (1);
}

/* Setter functions for various options */

static void
setfn_flag (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  if (neg)
    wsc->wsflags &= ~flag;
  else
    wsc->wsflags |= flag;
}

static void
setfn_option (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  wsc->wsflags |= WRDSF_OPTIONS;
  if (neg)
    wsc->ws.ws_options &= ~flag;
  else
    wsc->ws.ws_options |= flag;
}

static void
setfn_delim (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  wsc->wsflags |= flag;
  wsc->ws.ws_delim = arg;
}

static void
setfn_comment (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  wsc->wsflags |= flag;
  wsc->ws.ws_comment = arg;
}

static void
set_escape_string (wordsplit_t *ws, int *wsflags, int q, const char *str)
{
  if (*str == ':')
    {
      while (*++str != ':')
	{
	  int f;
	  switch (*str)
	    {
	    case '+':
	      f = WRDSO_BSKEEP;
	      break;

	    case '0':
	      f = WRDSO_OESC;
	      break;

	    case 'x':
	      f = WRDSO_XESC;
	      break;

	    default:
	      fprintf (stderr, "%s: invalid escape flag near %s\n",
		       progname, str);
	      abort ();
	    }
	  WRDSO_ESC_SET (ws, q, f);
	}
      *wsflags |= WRDSF_OPTIONS;
      ++str;
    }
  ws->ws_escape[q] = str;
}

static void
setfn_escape (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  wsc->wsflags |= flag;
  set_escape_string (&wsc->ws, &wsc->wsflags, 0, arg);
  set_escape_string (&wsc->ws, &wsc->wsflags, 1, arg);
}

static void
setfn_escape_qw (char *arg, int quote, struct wsclosure *wsc)
{
  if (!(wsc->wsflags & WRDSF_ESCAPE))
    {
      wsc->wsflags |= WRDSF_ESCAPE;
      wsc->ws.ws_escape[!quote] = NULL;
    }
  set_escape_string (&wsc->ws, &wsc->wsflags, quote, arg);
}

static void
setfn_escape_word (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  setfn_escape_qw (arg, 0, wsc);
}
  
static void
setfn_escape_quote (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  setfn_escape_qw (arg, 1, wsc);
}

static void
setfn_maxwords (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  char *p;

  wsc->wsflags |= WRDSF_OPTIONS;
  wsc->ws.ws_options |= WRDSO_MAXWORDS;

  wsc->ws.ws_maxwords = strtoul (arg, &p, 10);
  if (*p)
    {
      fprintf (stderr, "%s: invalid number: %s\n", progname, arg);
      exit (1);
    }
}

static void
setfn_namechar (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  wsc->wsflags |= WRDSF_OPTIONS;
  wsc->ws.ws_options |= WRDSO_NAMECHAR;
  wsc->ws.ws_namechar = arg;
}

static void
setfn_global (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  if (neg)
    wsc->options &= ~flag;
  else
    wsc->options |= flag;
}

static void
setfn_env (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  if (strcmp (arg, "none") == 0)
    wsc->env_type = env_none;
  else if (strcmp (arg, "null") == 0)
    wsc->env_type = env_null;
  else if (strcmp (arg, "sys") == 0)
    wsc->env_type = env_sys;
  else if (strcmp (arg, "extra") == 0)
    wsc->env_type = env_extra;
  else
    {
      fprintf (stderr, "%s: environment flag: %s\n", progname, arg);
      exit (1);
    }
}

static void
setfn_dooffs (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  if (!(wsc->wsflags & flag))
    {
      wsc->wsflags |= flag;
      wsc->offarg = wsoptind - 1;
      wsc->ws.ws_offs = 0;
    }
  wsc->ws.ws_offs++;
}

static void
setfn_append (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  if (wsc->append_count == 0)
    wsc->append_start = wsoptind - 1;
  wsc->append_count++;
}

static void help (void);

static void
setfn_help (int flag, int neg, char *arg, struct wsclosure *wsc)
{
  help ();
  exit (0);
}

/* Available options: */
struct wsopt opttab[] = {
  /* Global options */
  { "trimnl",         TRIMNL_OPTION,        ws_boolean, setfn_global },
  { "plaintext",      PLAINTEXT_OPTION,     ws_boolean, setfn_global },
  { "env",            0,          ws_required_argument, setfn_env },
  
  /* Wordsplit flags */
  { "append",         WRDSF_APPEND,         ws_boolean, setfn_flag },
  /*{ "reuse",  WRDSF_REUSE, ws_boolean, setfn_flag },*/
  { "undef",          WRDSF_UNDEF,          ws_boolean, setfn_flag },
  { "novar",          WRDSF_NOVAR,          ws_boolean, setfn_flag },
  { "nocmd",          WRDSF_NOCMD,          ws_boolean, setfn_flag },
  { "ws",             WRDSF_WS,             ws_boolean, setfn_flag },
  { "quote",          WRDSF_QUOTE,          ws_boolean, setfn_flag },
  { "squote",         WRDSF_SQUOTE,         ws_boolean, setfn_flag },
  { "dquote",         WRDSF_DQUOTE,         ws_boolean, setfn_flag },
  { "squeeze_delims", WRDSF_SQUEEZE_DELIMS, ws_boolean, setfn_flag },
  { "return_delims",  WRDSF_RETURN_DELIMS,  ws_boolean, setfn_flag },
  { "sed",            WRDSF_SED_EXPR,       ws_boolean, setfn_flag },
  { "debug",          WRDSF_SHOWDBG,        ws_boolean, setfn_flag },
  { "nosplit",        WRDSF_NOSPLIT,        ws_boolean, setfn_flag },
  { "keepundef",      WRDSF_KEEPUNDEF,      ws_boolean, setfn_flag },
  { "warnundef",      WRDSF_WARNUNDEF,      ws_boolean, setfn_flag },
  { "cescapes",       WRDSF_CESCAPES,       ws_boolean, setfn_flag },
  { "default",        WRDSF_DEFFLAGS,       ws_boolean, setfn_flag },
  { "env_kv",         WRDSF_ENV_KV,         ws_boolean, setfn_flag },
  { "incremental",    WRDSF_INCREMENTAL,    ws_boolean, setfn_flag },
  { "pathexpand",     WRDSF_PATHEXPAND,     ws_boolean, setfn_flag },
  { "default",        WRDSF_DEFFLAGS,       ws_boolean, setfn_flag },
  /* Wordsplit options */
  { "nullglob",       WRDSO_NULLGLOB,       ws_boolean, setfn_option },
  { "failglob",       WRDSO_FAILGLOB,       ws_boolean, setfn_option },
  { "dotglob",        WRDSO_DOTGLOB,        ws_boolean, setfn_option },
  { "bskeep_words",   WRDSO_BSKEEP_WORD,    ws_boolean, setfn_option },
  { "bskeep_quote",   WRDSO_BSKEEP_QUOTE,   ws_boolean, setfn_option },
  { "bskeep",         WRDSO_BSKEEP_WORD|WRDSO_BSKEEP_QUOTE,
                                            ws_boolean, setfn_option },
  { "novarsplit",     WRDSO_NOVARSPLIT,     ws_boolean, setfn_option },
  { "nocmdsplit",     WRDSO_NOCMDSPLIT,     ws_boolean, setfn_option },
  { "maxwords",       WRDSO_MAXWORDS, ws_required_argument, setfn_maxwords },
  { "namechar",       WRDSO_NAMECHAR, ws_required_argument, setfn_namechar },
  /* String options */
  { "delim",          WRDSF_DELIM,  ws_required_argument, setfn_delim },
  { "comment",        WRDSF_COMMENT,ws_required_argument, setfn_comment },
  { "escape",         WRDSF_ESCAPE, ws_required_argument, setfn_escape },
  { "escape-word",    WRDSF_ESCAPE, ws_required_argument, setfn_escape_word },
  { "escape-quote",   WRDSF_ESCAPE, ws_required_argument, setfn_escape_quote },

  { "dooffs",         WRDSF_DOOFFS, ws_multiple_arguments, setfn_dooffs },  
  { "append-args",    0,            ws_multiple_arguments, setfn_append },

  { "help",           0,                   ws_no_argument, setfn_help },
  
  { NULL, 0 }
};

static void
help (void)
{
  size_t i;
  
  printf ("usage: %s [options] [-D VAR=VALUE ...] [VAR=VALUE...] [-- EXTRA...]\n", progname);
  printf ("options are:\n");
  for (i = 0; opttab[i].name; i++)
    {
      printf (" -");
      if (opttab[i].arg == ws_boolean)
	printf ("[no]");
      if (strncmp (opttab[i].name, "no", 2) == 0)
	printf ("%s", opttab[i].name + 2);
      else
	printf ("%s", opttab[i].name);
      switch (opttab[i].arg)
	{
	case ws_no_argument:
	case ws_boolean:
	  break;
	case ws_required_argument:
	  printf(" ARG");
	  break;
	case ws_multiple_arguments:
	  printf(" ARGS... --");
	}
      putchar ('\n');
    }
  putchar ('\n');
}

void
print_qword (const char *word, int plaintext)
{
  static char *qbuf = NULL;
  static size_t qlen = 0;
  int quote;
  size_t size = wordsplit_c_quoted_length (word, 0, &quote);

  if (plaintext)
    {
      printf ("%s", word);
      return;
    }

  if (*word == 0)
    quote = 1;
  
  if (size >= qlen)
    {
      qlen = size + 1;
      qbuf = realloc (qbuf, qlen);
      assert (qbuf != NULL);
    }
  wordsplit_c_quote_copy (qbuf, word, 0);
  qbuf[size] = 0;
  if (quote)
    printf ("\"%s\"", qbuf);
  else
    printf ("%s", qbuf);
}

/* Convert environment to K/V form */
static char **
make_env_kv (char **origenv)
{
  size_t i, j, size;
  char **newenv;
  
  /* Count the number of entries */
  for (i = 0; origenv[i]; i++)
    ;

  size = i * 2 + 1;
  newenv = calloc (size, sizeof (newenv[0]));
  assert (newenv != NULL);

  for (i = j = 0; origenv[i]; i++)
    {
      size_t len = strcspn (origenv[i], "=");
      char *p = malloc (len+1);
      assert (p != NULL);
      memcpy (p, origenv[i], len);
      p[len] = 0;
      newenv[j++] = p;
      p = strdup (origenv[i] + len + 1);
      assert (p != NULL);
      newenv[j++] = p;
    }
  newenv[j] = NULL;
  return newenv;
}

static int
wsp_getvar (char **ret, const char *vptr, size_t vlen, void *data)
{
  char **base = data;
  int i;

  for (i = 0; base[i]; i++)
    {
      size_t l = strcspn (base[i], "=");
      if (l == vlen && memcmp (base[i], vptr, vlen) == 0)
	{
	  char *p = strdup (base[i] + vlen + 1);
	  if (p == NULL)
	    return WRDSE_NOSPACE;
	  *ret = p;
	  return WRDSE_OK;
	}
    }
  return WRDSE_UNDEF;
}

static int
cmd_quote (char **ret, const char *str, size_t len, char **argv)
{
  int alen;
  for (alen = 0; alen < len && !(str[alen] == ' ' || str[alen] == '\t'); alen++)
    ;
  for (; alen < len && (str[alen] == ' ' || str[alen] == '\t'); alen++)
    ;
  len -= alen;
  *ret = malloc (len + 1);
  if (!*ret)
    return WRDSE_NOSPACE;
  memcpy (*ret, str + alen, len);
  (*ret)[len] = 0;
  return WRDSE_OK;
}
  
static int
cmd_words (char **ret, const char *str, size_t len, char **argv)
{
  char *p;
  int i;
  
  p = malloc (len + 1);
  if (!p)
    return WRDSE_NOSPACE;
  *ret = p;
  for (i = 1; argv[i]; i++)
    {
      size_t s = strlen (argv[i]);
      if (i > 1)
	*p++ = ' ';
      memcpy (p, argv[i], s);
      p += s;
    }
  *p = 0;
  return WRDSE_OK;
}

static int
cmd_lines (char **ret, const char *str, size_t len, char **argv)
{
  char *p;
  int i;
  
  p = malloc (len + 1);
  if (!p)
    return WRDSE_NOSPACE;
  *ret = p;
  for (i = 1; argv[i]; i++)
    {
      size_t s = strlen (argv[i]);
      if (i > 1)
	*p++ = '\n';
      memcpy (p, argv[i], s);
      p += s;
    }
  *p = 0;
  return WRDSE_OK;
}

static struct command
{
  char const *name;
  int (*cmd)(char **ret, const char *str, size_t len, char **argv);
} comtab[] = {
  { "quote", cmd_quote },
  { "words", cmd_words },
  { "lines", cmd_lines }
};

static int
wsp_runcmd (char **ret, const char *str, size_t len, char **argv, void *closure)
{
  int i;
  char const msg[] = "unknown command: ";
  
  for (i = 0; ; i++)
    {
      if (i == sizeof (comtab) / sizeof (comtab[0]))
	break;
      if (strcmp (comtab[i].name, argv[0]) == 0)
	return comtab[i].cmd (ret, str, len, argv);
    }

  *ret = malloc (sizeof (msg) + strlen (argv[0]));
  if (!*ret)
    return WRDSE_NOSPACE;
  strcat (strcpy (*ret, msg), argv[0]);
  return WRDSE_USERERR;
}

int
main (int argc, char **argv)
{
  struct wsclosure wsc;
  char buf[1024], *ptr, *saved_ptr;
  int next_call = 0;

  wsc.options = 0;
  wsc.wsflags = 0;
  wsc.env_type = env_sys;
  wsc.offarg = 0;
  wsc.fenvidx = 0;
  wsc.xenvidx = 0;
  wsc.ws.ws_options = 0;
  wsc.wsflags = (WRDSF_DEFFLAGS & ~WRDSF_NOVAR) |
                 WRDSF_ENOMEMABRT |
                 WRDSF_SHOWERR;
  wsc.append_count = 0;
  
  progname = argv[0];
  while (getwsopt (argc, argv, opttab, &wsc) != EOF)
    ;

  wsc.fenvbase[wsc.fenvidx] = NULL;
  wsc.xenvbase[wsc.xenvidx] = NULL;
  
  if (wsc.fenvidx > 0)
    {
      wsc.wsflags |= WRDSF_GETVAR | WRDSF_CLOSURE;
      wsc.ws.ws_getvar = wsp_getvar;
      wsc.ws.ws_closure = wsc.fenvbase;
    }

  if (wsoptind < argc)
    {
      wsc.ws.ws_paramc = argc - wsoptind;
      wsc.ws.ws_paramv = (char const **) (argv + wsoptind);
      wsc.ws.ws_options |= WRDSO_PARAMV|WRDSO_PARAM_NEGIDX;
      wsc.wsflags |= WRDSF_OPTIONS;
    }

  switch (wsc.env_type)
    {
    case env_null:
      wsc.wsflags |= WRDSF_ENV;
      wsc.ws.ws_env = NULL;
      break;

    case env_none:
      break;

    case env_sys:
      {
	char **newenv;
	
	if (wsc.xenvidx)
	  {
	    size_t i, j;
	    for (i = 0; environ[i]; i++)
	      ;
	    newenv = calloc (i + wsc.xenvidx + 1, sizeof (*newenv));
	    assert (newenv != NULL);
	    for (i = 0; environ[i]; i++)
	      {
		newenv[i] = strdup (environ[i]);
		assert (newenv[i] != NULL);
	      }
	    for (j = 0; j < wsc.xenvidx; j++, i++)
	      {
		newenv[i] = strdup (wsc.xenvbase[j]);
		assert (newenv[i] != NULL);
	      }
	    newenv[i] = NULL;
	  }
	else
	  newenv = environ;

	wsc.wsflags |= WRDSF_ENV;
	if (wsc.wsflags & WRDSF_ENV_KV)
	  wsc.ws.ws_env = (const char **) make_env_kv (newenv);
	else
	  wsc.ws.ws_env = (const char **) newenv;
      }
      break;

    case env_extra:
      wsc.wsflags |= WRDSF_ENV;
      if (wsc.wsflags & WRDSF_ENV_KV)
	wsc.ws.ws_env = (const char **) make_env_kv (wsc.xenvbase);
      else
	wsc.ws.ws_env = (const char **) wsc.xenvbase;
      break;
    }
  
  if (!(wsc.wsflags & WRDSF_NOCMD))
    wsc.ws.ws_command = wsp_runcmd;
  
  if (wsc.wsflags & WRDSF_INCREMENTAL)
    wsc.options |= TRIMNL_OPTION;
  
  next_call = 0;
  while ((ptr = fgets (buf, sizeof (buf), stdin)))
    {
      int rc;
      size_t i;
      
      if (wsc.options & TRIMNL_OPTION)
	{
	  size_t len = strlen (ptr);
	  if (len && ptr[len-1] == '\n')
	    ptr[len-1] = 0;
	}
      
      if (wsc.wsflags & WRDSF_INCREMENTAL)
	{
	  if (next_call)
	    {
	      if (*ptr == 0)
		ptr = NULL;
	      else
		free (saved_ptr);
	    }
	  else
	    next_call = 1;
	  if (ptr)
	    {
	      ptr = saved_ptr = strdup (ptr);
	      assert (ptr != NULL);
	    }
	}
	
      rc = wordsplit (ptr, &wsc.ws, wsc.wsflags);
      if (rc)
	{
	  if (!(wsc.wsflags & WRDSF_SHOWERR))
	    wordsplit_perror (&wsc.ws);
	  continue;
	}
	  
      if (wsc.offarg)
	{
	  size_t i;
	  for (i = 0; i < wsc.ws.ws_offs; i++)
	    wsc.ws.ws_wordv[i] = argv[wsc.offarg + i];
	  wsc.offarg = 0;
	}

      if (wsc.append_count)
	{
	  rc = wordsplit_append (&wsc.ws, wsc.append_count,
				 argv + wsc.append_start);
	  if (rc)
	    {
	      if (!(wsc.wsflags & WRDSF_SHOWERR))
		wordsplit_perror (&wsc.ws);
	      continue;
	    }
	}
      
      wsc.wsflags |= WRDSF_REUSE;
      printf ("NF: %lu", (unsigned long) wsc.ws.ws_wordc);
      if (wsc.wsflags & WRDSF_DOOFFS)
	printf (" (%lu)", (unsigned long) wsc.ws.ws_offs);
      putchar ('\n');
      for (i = 0; i < wsc.ws.ws_offs; i++)
	{
	  printf ("(%lu): ", (unsigned long) i);
	  print_qword (wsc.ws.ws_wordv[i], wsc.options & PLAINTEXT_OPTION);
	  putchar ('\n');
	}
      for (; i < wsc.ws.ws_offs + wsc.ws.ws_wordc; i++)
	{
	  printf ("%lu: ", (unsigned long) i);
	  print_qword (wsc.ws.ws_wordv[i], wsc.options & PLAINTEXT_OPTION);
	  putchar ('\n');
	}
      printf ("TOTAL: %lu\n", (unsigned long) wsc.ws.ws_wordi);
    }
  return 0;
}
