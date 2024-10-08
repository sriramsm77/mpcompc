%top {
/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

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
}

%{
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <mailutils/cctype.h>
#include <mailutils/wordsplit.h>
#include <sieve-priv.h>
#include <sieve-gram.h>
  
static char *multiline_delimiter;
static int strip_tabs; 

static int number (void);
static int string (void);
static void line_begin (void);
static void line_add (char const *text, size_t len);
static void line_finish (void);
static void multiline_begin (void);
static void multiline_add (void);
static void multiline_finish (void);
static char *multiline_strip_tabs (char *text);
static void ident (const char *text);
static void sieve_include (void);
static void sieve_searchpath (void);
static int isemptystr (char *text);

static mu_linetrack_t trk;
static ino_t sieve_source_inode;
static mu_stream_t input_stream;
 
static int
fillbuf (char *buf, size_t max_size)
{
  int rc;

  if (!input_stream)
    return 0;
  
  rc = mu_stream_read (input_stream, buf, max_size, &max_size);
  if (rc)
    {
      struct mu_locus_point pt;
      mu_linetrack_locus (trk, &pt);
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_read", pt.mu_file, rc);
      mu_locus_point_deinit (&pt);
      return 0;
    }
  return max_size;
}
 
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result = fillbuf (buf, max_size)
#define LEX_BUFFER_STATE YY_BUFFER_STATE
#define SET_BUFFER_STATE(s) do { \
        (s) = YY_CURRENT_BUFFER; \
        yy_switch_to_buffer(yy_create_buffer (yyin, YY_BUF_SIZE)); \
} while (0)
#define RESTORE_BUFFER_STATE(s) do { \
        yy_delete_buffer (YY_CURRENT_BUFFER); \
        yy_switch_to_buffer (s); \
} while (0)

#define YY_USER_ACTION							\
  do									\
    {									\
      mu_linetrack_advance (trk, &yylloc, yytext, yyleng);		\
      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,			\
		       MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &yylloc);	\
    }									\
  while (0);

 
static void
init_locus (char const *name, ino_t ino)
{
  if (name)
    {
      MU_ASSERT (mu_linetrack_create (&trk, name, 2));
    }
  else
    mu_linetrack_destroy (&trk);
  sieve_source_inode = ino;
}
   
struct buffer_ctx
{
  struct buffer_ctx *prev;
  mu_linetrack_t trk;
  struct mu_locus_range incl_range;
  ino_t i_node;
  mu_stream_t input;
  LEX_BUFFER_STATE state;
};

static struct buffer_ctx *context_stack;

static struct buffer_ctx *ctx_lookup (ino_t ino);
static int push_source (const char *name);
static int pop_source (void);

struct buffer_ctx *
ctx_lookup (ino_t ino)
{
  struct buffer_ctx *ctx;

  for (ctx = context_stack; ctx; ctx = ctx->prev)
    if (ctx->i_node == ino)
      break;
  return ctx;
}
        
int
push_source (const char *name)
{
  int rc;
  mu_stream_t stream;
  struct buffer_ctx *ctx;
  struct stat st;
        
  if (stat (name, &st))
    {
      mu_error (_("cannot stat `%s': %s"), name, strerror (errno));
      mu_i_sv_error (mu_sieve_machine);
      return 1;
    }

  if (yylloc.beg.mu_file && st.st_ino == sieve_source_inode)
    {
      mu_sieve_yyerror (_("recursive inclusion"));
      return 1;
    }
  if ((ctx = ctx_lookup (st.st_ino)))
    {
      mu_sieve_yyerror (_("recursive inclusion"));
      if (ctx->prev)
	{
	  mu_diag_at_locus_range (MU_LOG_ERROR, &ctx->incl_range,
				  _("`%s' already included here"),
				  name);
	  mu_i_sv_error (mu_sieve_machine);
	}
      else
	{
	  mu_error (_("`%s' already included at top level"), name);
	  mu_i_sv_error (mu_sieve_machine);
	}	  
      return 1;
    }

  rc = mu_file_stream_create (&stream, name, MU_STREAM_READ);
  if (rc)
    {
      mu_error (_("cannot open file `%s': %s"), name, mu_strerror (rc));
      mu_i_sv_error (mu_sieve_machine);
      return 1;
    }

  /* Push current context */
  if (trk)
    {
      ctx = mu_sieve_malloc (mu_sieve_machine, sizeof (*ctx));
      ctx->trk = trk;
      mu_locus_range_init (&ctx->incl_range);
      mu_locus_range_copy (&ctx->incl_range, &yylloc);
      ctx->i_node = sieve_source_inode;
      ctx->input = input_stream;
      ctx->prev = context_stack;
      context_stack = ctx;

      /* Switch to the new context */
      SET_BUFFER_STATE (ctx->state);
    }
  input_stream = stream;

  init_locus (name, st.st_ino);

  return 0;
}

int
pop_source ()
{
  struct buffer_ctx *ctx;

  mu_stream_destroy (&input_stream);

  if (!context_stack)
    {
      input_stream = NULL;
      init_locus (NULL, 0);
      return 1;
    }
  /* Restore previous context */
  input_stream = context_stack->input;
  mu_linetrack_destroy (&trk);
  trk = context_stack->trk;
  mu_locus_range_deinit (&context_stack->incl_range);
  sieve_source_inode = context_stack->i_node;
  RESTORE_BUFFER_STATE (context_stack->state);
  ctx = context_stack->prev;
  mu_sieve_free (mu_sieve_machine, context_stack);
  context_stack = ctx;

  return 0;
}
%}

%option prefix="mu_sieve_yy"
%option nounput
%option noinput

%x COMMENT ML STR

WS [ \t][ \t]*
IDENT [a-zA-Z_][a-zA-Z_0-9]+
SIZESUF [kKmMgG]

%%
         /* C-style comments */
"/*"                    { BEGIN(COMMENT); }
<COMMENT>[^*\n]*        /* eat anything that's not a '*' */
<COMMENT>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<COMMENT>\n             ;
<COMMENT>"*"+"/"        { BEGIN (INITIAL); }
         /* Preprocessor directives (an extension) */
#[ \t]*include.*\n      { sieve_include (); }
#[ \t]*searchpath.*\n   { sieve_searchpath (); } 
         /* End-of-line comments */
#.*\n   ;
#.*     /* end-of-file comment */;
         /* Reserved words */
require return REQUIRE; 
if	return IF;      
elsif	return ELSIF;  
else	return ELSE;    
anyof	return ANYOF;   
allof	return ALLOF;   
not     return NOT;
false   return FALSE;
true    return TRUE;
         /* Identifiers */
{IDENT}  { ident (yytext); return IDENT; } 
:{IDENT} { ident (yytext + 1); return TAG; }
         /* Numbers */
0[0-7]*{SIZESUF}*                     { return number (); }
0x[0-9a-fA-F][0-9a-fA-F]+{SIZESUF}*   { return number (); }
[1-9][0-9]*{SIZESUF}*                 { return number (); }
         /* Quoted strings */
\"[^\\"]*\"       { return string (); }
\"[^\\"]*\\.      { BEGIN(STR);
                    line_begin ();
  		    line_add (yytext + 1, yyleng - 3);
		    line_add (yytext + yyleng - 1, 1); }
<STR>[^\\"]*\\.   { line_add (yytext, yyleng - 2);
                    line_add (yytext + yyleng - 1, 1); }
<STR>[^\\"]*\"   { BEGIN(INITIAL);
                   if (yyleng > 1) 
                     line_add (yytext, yyleng - 1); 
                   line_finish ();
		   return STRING; }
         /* Multiline strings */
text:-?[ \t]*#.*\n       { BEGIN(ML);
                           multiline_begin (); }
text:-?[ \t]*\n          { BEGIN(ML);
                           multiline_begin (); }
text:-?\\?{IDENT}[ \t]*#.*\n { BEGIN(ML);
                               multiline_begin (); }
text:-?\\?{IDENT}[ \t]*\n    { BEGIN(ML);
                               multiline_begin (); }
<ML>#[ \t]*include.*\n    { if (multiline_delimiter[0] == '\\')
                              multiline_add ();
                            else
                              sieve_include (); }
<ML>.*\n { char *p = multiline_strip_tabs (yytext);
	   
           if (strncmp (p, multiline_delimiter, strlen (multiline_delimiter))
	        == 0
	       && isemptystr (p + strlen (multiline_delimiter)))
	     {
	       free (multiline_delimiter);
	       multiline_delimiter = NULL;
	       BEGIN(INITIAL);
	       multiline_finish ();
	       return MULTILINE;
	     }
	    multiline_add (); } 
{WS}     ;
         /* Other tokens */
\n       ;
. return yytext[0];

%%

int
yywrap ()
{
  return pop_source();
}

static char *
get_file_name (char *p, char *endp, int *usepath)
{
  char exp, *name, *startp;
  int n;
  
  if (usepath)
    *usepath = 0;
  switch (*p)
    {
    case '"':
      exp = '"';
      break;

    case '<':
      exp = '>';
      if (usepath)
	*usepath = 1;
      break;

    default:
      mu_sieve_yyerror (_("preprocessor syntax"));
      return NULL;
    }

  for (startp = ++p; p < endp && *p != exp; p++)
    ;

  if (*p != exp)
    {
      mu_sieve_yyerror (_("missing closing quote in preprocessor statement"));
      return NULL;
    }
  
  n = p - startp;
  name = mu_sieve_malloc (mu_sieve_machine, n + 1);
  memcpy (name, startp, n);
  name[n] = 0;
  return name;
}

static int
_try_include (void *item, void *data)
{
  char **dir = data;
  char *name = mu_make_file_name ((char*) item, *dir);

  if (!name)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_make_file_name", NULL, ENOMEM);
      return 0;
    }
  if (access (name, R_OK) == 0)
    {
      *(char**) data = name;
      return MU_ERR_USER0;
    }
  free (name);
  return 0;
}

static void
sieve_include (void)
{
  char *p, *endp = yytext + yyleng, *name;
  int usepath;
  
  p = strstr (yytext, "include");
  for (p += 7; p < endp && mu_isspace (*p); p++)
    ;

  name = get_file_name (p, endp, &usepath);
  if (!name)
    return;
  
  if (usepath && name[0] != '/' && memcmp (name, "..", 2))
    {
      char *p = name;
      if (mu_sieve_include_path
	  && mu_list_foreach (mu_sieve_include_path, _try_include, &p))
	{
	  push_source (p);
	  mu_sieve_free (mu_sieve_machine, name);
	  free (p);
	  return;
	}
    }

  push_source (name);
  mu_sieve_free (mu_sieve_machine, name);
}

static void
sieve_searchpath (void)
{
  char *p, *endp = yytext + yyleng, *name;
  
  p = strstr (yytext, "searchpath");
  for (p += 10; p < endp && mu_isspace (*p); p++)
    ;
  name = get_file_name (p, endp, NULL);
  if (name)
    {
      mu_i_sv_load_add_dir (mu_sieve_machine, name);
      mu_sieve_free (mu_sieve_machine, name);
    }
}

int
mu_i_sv_lex_begin (const char *name)
{
  return push_source (name);
}

int
mu_i_sv_lex_begin_string (const char *buf, int bufsize,
			  struct mu_locus_point const *pt)
{
  int rc;
  
  yyrestart (NULL);

  rc = mu_static_memory_stream_create (&input_stream, buf, bufsize);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_static_memory_stream_create",
		       NULL, rc);
      return 1;
    }

  init_locus (pt->mu_file, 0);
  mu_linetrack_rebase (trk, pt);

  return 0;
}

void
mu_i_sv_lex_finish (void)
{
  while (pop_source () == 0)
    ;
}
  
static int
number ()
{
  char *p;
  yylval.number = strtoul (yytext, &p, 0);
  switch (*p)
    {
    case 'k':
    case 'K':
      yylval.number *= 1024L;
      break;
      
    case 'm':
    case 'M':
      yylval.number *= 1024*1024L;
      break;
      
    case 'g':
    case 'G':
      yylval.number *= 1024*1024*1024L;
    }
  return NUMBER;
}

static int
string (void)
{
  line_begin ();
  line_add (yytext + 1, yyleng - 2);
  line_finish ();
  return STRING; 
}

static int
isemptystr (char *text)
{
  for (; *text && mu_isspace (*text); text++)
    ;
  return *text == 0;
}

static char *
multiline_strip_tabs (char *text)
{
  if (strip_tabs)
    for (; *text == '\t'; text++)
      ;
  return text;
}

static void
line_add (char const *text, size_t len)
{
  mu_opool_append (mu_sieve_machine->string_pool, text, len);
}

static void
multiline_add (void)
{
  mu_opool_appendz (mu_sieve_machine->string_pool,
		    multiline_strip_tabs (yytext));
}

static void
line_begin (void)
{
  /* nothing */
}

static void
multiline_begin (void)
{
  char *p = yytext + 5; /* past the text: keyword */

  if (*p == '-')
    {
      strip_tabs = 1;
      p++;
    }
  else
    strip_tabs = 0;

  if (!mu_isspace (*p))
    {
      char *endp;
      int len;
      
      for (endp = p; *endp; endp++)
	if (mu_isspace (*endp))
	  break;

      len = endp - p;
      multiline_delimiter = mu_sieve_malloc (mu_sieve_machine, len + 1);
      memcpy (multiline_delimiter, p, len);
      multiline_delimiter[len] = 0;
    }
  else
    {
      multiline_delimiter = strdup (".");
      if (!multiline_delimiter)
	{
	  mu_sieve_yyerror (_("not enough memory"));
	  exit (1);
	}
    }

  line_begin ();
}

static void
multiline_finish (void)
{
  line_finish ();
}

static void
ident (const char *text)
{
  yylval.string = strdup (text);
  if (!yylval.string)
    {
      mu_sieve_yyerror (_("not enough memory"));
      exit (1);
    }
}

static mu_i_sv_interp_t string_interpreter;

static void
line_finish (void)
{
  char *str;
  
  mu_opool_append_char (mu_sieve_machine->string_pool, 0);
  str = mu_opool_finish (mu_sieve_machine->string_pool, NULL);
  if (string_interpreter)
    {
      char *exp;
      int rc = mu_i_sv_string_expand (str, string_interpreter, NULL, &exp);
      if (rc == 0)
	{
	  mu_opool_free (mu_sieve_machine->string_pool, str);
	  mu_opool_appendz (mu_sieve_machine->string_pool, exp);
	  mu_opool_append_char (mu_sieve_machine->string_pool, 0);
	  free (exp);
	  str = mu_opool_finish (mu_sieve_machine->string_pool, NULL);
	}
      else if (rc != MU_ERR_CANCELED)
	{
	  mu_error (_("error expandind string: %s"), mu_strerror (rc));
	}
    }
  yylval.string = str;
}

int
mu_sieve_require_encoded_character (mu_sieve_machine_t mach,
				    const char *name)
{
  string_interpreter = mu_i_sv_expand_encoded_char;
  return 0;
}
