%top {
/* cfg_lexer.l -- default lexer for Mailutils configuration files
   Copyright (C) 2007-2022 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3, or (at
   your option) any later version.

   GNU Mailutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
}

%{
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <mailutils/cctype.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/debug.h>  
#include <mailutils/wordsplit.h>
#include <mailutils/alloc.h>  
#include <mailutils/nls.h>
#include <mailutils/cfg.h>
#include <mailutils/list.h>
#include <mailutils/util.h>
#include <mailutils/locus.h>
#include <mailutils/stream.h>
#include <mailutils/stdstream.h>  
#include <mailutils/yyloc.h>
#include "parser.h"

void _mu_line_begin (void);
void _mu_line_add (char *text, size_t len);
char *_mu_line_finish (void);

extern void mu_cfg_set_debug (void);
static void
mu_cfg_set_lex_debug (void)
{
  yy_flex_debug = mu_debug_level_p (MU_DEBCAT_CONFIG, MU_DEBUG_TRACE2);
}
 
static void _mu_line_add_unescape_last (char *text, size_t len);
static void multiline_begin (char *p);
static char *multiline_strip_tabs (char *text);
static void multiline_add (char *s);
static char *multiline_finish (void);
 
static char *multiline_delimiter;
static size_t multiline_delimiter_len;
static int multiline_unescape;         /* Unescape here-document contents */
static int (*char_to_strip)(char);     /* Strip matching characters of each
					  here-document line */
static int isemptystr(int off);

static mu_opool_t pool;
static mu_linetrack_t trk;
static struct mu_locus_point string_beg;
#define YY_USER_ACTION							\
  do									\
    {									\
      mu_linetrack_advance (trk, &yylloc, yytext, yyleng);		\
      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,			\
		       MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &yylloc);	\
    }									\
  while (0); 
%}

%option prefix="mu_cfg_yy"
%option nounput
%option noinput

%x COMMENT ML STR

WS [ \t\f][ \t\f]*
ID [a-zA-Z_][a-zA-Z_0-9-]+
P [1-9][0-9]*

%%
         /* C-style comments */
"/*"         BEGIN(COMMENT);
<COMMENT>[^*\n]*        /* eat anything that's not a '*' */
<COMMENT>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<COMMENT>\n             ;
<COMMENT>"*"+"/"        BEGIN (INITIAL);
         /* End-of-line comments */
#debug=.*\n {
          mu_debug_parse_spec (yytext + 7); 
          mu_cfg_set_debug ();
          mu_cfg_set_lex_debug ();
      }
#.*\n   ;
#.*     /* end-of-file comment */;
"//".*\n  ;
"//".*    /* end-of-file comment */;
        /* Identifiers */
<INITIAL>{ID}         {
                        _mu_line_begin ();
			_mu_line_add (yytext, yyleng);
			yylval.string = _mu_line_finish ();
			return MU_TOK_IDENT; }
         /* Strings */
[a-zA-Z0-9_\./:\*=@-]+ { _mu_line_begin ();
                        _mu_line_add (yytext, yyleng);
                        yylval.string = _mu_line_finish ();
                        return MU_TOK_STRING; }
         /* Quoted strings */
\"[^\\"\n]*\"         { _mu_line_begin ();
                        _mu_line_add (yytext + 1, yyleng - 2);
                        yylval.string = _mu_line_finish ();
                        return MU_TOK_QSTRING; }
\"[^\\"\n]*\\. |
\"[^\\"\n]*\\\n        { BEGIN (STR);
                         mu_locus_point_copy (&string_beg, &yylloc.beg);
                        _mu_line_begin ();
		        _mu_line_add_unescape_last (yytext + 1, yyleng - 1); }
<STR>[^\\"\n]*\\. |
<STR>[^\\"\n]*\\\n    { _mu_line_add_unescape_last (yytext, yyleng); }
<STR>[^\\"\n]*\"      { BEGIN (INITIAL);
                        if (yyleng > 1) 
                          _mu_line_add (yytext, yyleng - 1); 
                        yylval.string = _mu_line_finish ();

			mu_locus_point_copy (&yylloc.beg, &string_beg);
			mu_locus_point_deinit (&string_beg);
			
		        return MU_TOK_QSTRING; }
         /* Multiline strings */
"<<"(-" "?)?\\?{ID}[ \t]*#.*\n |
"<<"(-" "?)?\\?{ID}[ \t]*"//".*\n |
"<<"(-" "?)?\\?{ID}[ \t]*\n |
"<<"(-" "?)?\"{ID}\"[ \t]*#.*\n |
"<<"(-" "?)?\"{ID}\"[ \t]*"//".*\n |
"<<"(-" "?)?\"{ID}\"[ \t]*\n {
                        BEGIN (ML);
			mu_locus_point_copy (&string_beg, &yylloc.beg);
			multiline_begin (yytext+2);
		  }
<ML>.*\n { char *p = multiline_strip_tabs (yytext);
	   
           if (!strncmp (p, multiline_delimiter, multiline_delimiter_len)
	       && isemptystr (p + multiline_delimiter_len - yytext))
	     {
	       free (multiline_delimiter);
	       multiline_delimiter = NULL;
	       BEGIN (INITIAL);
	       yylval.string = multiline_finish ();

	       mu_locus_point_copy (&yylloc.beg, &string_beg);
	       mu_locus_point_deinit (&string_beg);
	       /* FIXME: Adjust yylloc.end without retreating trk */
	       return MU_TOK_MSTRING;
	     }
	   multiline_add (p); } 
{WS}     ;
         /* Other tokens */
\n       ; 
[,;{}()] return yytext[0];
.        { if (mu_isprint (yytext[0]))
              mu_error (_("stray character %c"), yytext[0]);
           else 
              mu_error (_("stray character \\%03o"), (unsigned char) yytext[0]);
           mu_cfg_error_count++;  
         }
%%

int
yywrap ()
{
  return 1;
}

static void
unescape_to_line (int c)
{
  if (c != '\n')
    {
      char t = mu_wordsplit_c_unquote_char (c);
      if (t == c && t != '\\' && t != '\"')
        {
          mu_error (_("unknown escape sequence '\\%c'"), c);
          mu_cfg_error_count++;
	}
      mu_opool_append_char (pool, t);
    }
}

void
_mu_line_add (char *text, size_t len)
{
  mu_opool_append (pool, text, len);
}

void
_mu_line_add_unescape_last (char *text, size_t len)
{
  mu_opool_append (pool, text, len - 2);
  unescape_to_line (text[len - 1]);
}

void
_mu_line_begin ()
{
  if (!pool)
    mu_opool_create (&pool, MU_OPOOL_ENOMEMABRT);
  else
    mu_opool_clear (pool);
}

char *
_mu_line_finish ()
{
  mu_opool_append_char (pool, 0);
  return mu_opool_finish (pool, NULL);
}



static int
is_tab (char c)
{
    return c == '\t';
}
 
static int
is_ws (char c)
{
    return c == '\t' || c == ' ';
}

static int
isemptystr (int off)
{
  for (; yytext[off] && mu_isspace (yytext[off]); off++)
    ;
  if (yytext[off] == ';')
    {
      int i;
      for (i = off + 1; yytext[i]; i++) 
	if (!mu_isspace (yytext[i]))
	  return 0;
      yyless (off);
      return 1;
    }
  return yytext[off] == 0;
}

static void
multiline_begin (char *p)
{
  if (*p == '-')
    {
      if (*++p == ' ')
	{
	  char_to_strip = is_ws;
	  p++;
	}
      else
	char_to_strip = is_tab;
    }
  else
    char_to_strip = NULL;
  if (*p == '\\')
    {
      p++;
      multiline_unescape = 0;
    }
  else if (*p == '"')
    {
      char *q;
      
      p++;
      multiline_unescape = 0;
      q = strchr (p, '"');
      multiline_delimiter_len = q - p;
    }
  else
    {
	multiline_delimiter_len = strcspn (p, " \t");
	multiline_unescape = 1;
    }

  /* Remove trailing newline */
  multiline_delimiter_len--;
  multiline_delimiter = mu_alloc (multiline_delimiter_len + 1);
  memcpy (multiline_delimiter, p, multiline_delimiter_len);
  multiline_delimiter[multiline_delimiter_len] = 0;
  _mu_line_begin ();
}

static char *
multiline_strip_tabs (char *text)
{
  if (char_to_strip)
    for (; *text && char_to_strip (*text); text++)
      ;
  return text;
}

static void
multiline_add (char *s)
{
  if (multiline_unescape)
    {
      for (; *s; s++)
	{
	  if (*s == '\\')
	    {
	      unescape_to_line (s[1]);
	      ++s;
	    }
	  else
	    _mu_line_add (s, 1);
	}
    }
  else
    _mu_line_add (s, strlen (s));
}

static char *
multiline_finish ()
{
  return _mu_line_finish ();
}


int
mu_cfg_parse_file (mu_cfg_tree_t **return_tree, const char *file, int flags)
{
  struct stat st;
  FILE *fp;
  int rc;
  char *full_name = mu_tilde_expansion (file, MU_HIERARCHY_DELIMITER, NULL);

  if (flags & MU_CF_VERBOSE)
    mu_diag_output (MU_DIAG_INFO, _("opening configuration file %s"),
		    full_name);
  if (stat (full_name, &st))
    {
      if (errno != ENOENT)
	mu_error (_("cannot stat `%s': %s"), full_name, mu_strerror (errno));
      else if (flags & MU_CF_VERBOSE)
	mu_diag_output (MU_DIAG_INFO, _("configuration file %s doesn't exist"),
			full_name);
      free (full_name);
      return ENOENT;
    }
  else if (!S_ISREG (st.st_mode))
    {
      if (flags & MU_CF_VERBOSE)
	mu_diag_output (MU_DIAG_INFO, _("%s: not a regular file"), full_name);
      free (full_name);
      return ENOENT;
    } 
      
  fp = fopen (full_name, "r");
  if (!fp)
    {
      mu_error (_("cannot open config file `%s': %s"), full_name,
		mu_strerror (errno));
      free (full_name);
      return errno;
    }

  if (flags & MU_CF_VERBOSE)
    mu_diag_output (MU_DIAG_INFO, _("parsing file `%s'"), full_name);

  mu_cfg_set_lex_debug ();

  /* Initialize tracker */
  mu_linetrack_create (&trk, full_name, 2);
  memset (&string_beg, 0, sizeof string_beg);
  /* Parse configuration */
  yyrestart (fp);
  rc = mu_cfg_parse (return_tree);
  fclose (fp);
  if (flags & MU_CF_VERBOSE)
    mu_diag_output (MU_DIAG_INFO, _("finished parsing file `%s'"), full_name);
  free (full_name); 
  mu_linetrack_destroy (&trk);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, NULL);
  return rc == 0 ? 0 : MU_ERR_FAILURE;
}

mu_opool_t 
mu_cfg_lexer_pool (void)
{
  mu_opool_t p = pool;
  pool = NULL;
  return p;
}
