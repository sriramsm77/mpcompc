/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_MIMETYPES_YY_GRAMMAR_H_INCLUDED
# define YY_MIMETYPES_YY_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef MIMETYPES_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define MIMETYPES_YYDEBUG 1
#  else
#   define MIMETYPES_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define MIMETYPES_YYDEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined MIMETYPES_YYDEBUG */
#if MIMETYPES_YYDEBUG
extern int mimetypes_yydebug;
#endif
/* "%code requires" blocks.  */
#line 91 "grammar.y" /* yacc.c:1909  */

#include <stdio.h>
#include <regex.h>  
#include <mailutils/stream.h>  
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>  
#include <mailutils/locus.h>
#include <mailutils/yyloc.h>
#include <mailutils/opool.h>
#include <mailutils/list.h>
#include <mailutils/nls.h>
#include <mailutils/diag.h>
#include <mailutils/stdstream.h>
#include <mailutils/iterator.h>
#include <mailutils/util.h>  
#include <mailutils/mimetypes.h>
#include <mailutils/sys/mimetypes.h>

#define MIMETYPES_YYLTYPE struct mu_locus_range

typedef void *yyscan_t;  

struct parser_control
{
  mu_linetrack_t trk;
  struct mu_locus_point string_beg; 
  size_t errors;
  mu_mimetypes_t mth;
};


#line 84 "grammar.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef MIMETYPES_YYTOKENTYPE
# define MIMETYPES_YYTOKENTYPE
  enum mimetypes_yytokentype
  {
    TYPE = 258,
    IDENT = 259,
    STRING = 260,
    EOL = 261,
    BOGUS = 262,
    PRIORITY = 263
  };
#endif

/* Value type.  */
#if ! defined MIMETYPES_YYSTYPE && ! defined MIMETYPES_YYSTYPE_IS_DECLARED

union MIMETYPES_YYSTYPE
{
#line 163 "grammar.y" /* yacc.c:1909  */

  struct mimetypes_string string;
  char *s;
  mu_list_t list;
  int result;
  struct node *node;

#line 113 "grammar.h" /* yacc.c:1909  */
};

typedef union MIMETYPES_YYSTYPE MIMETYPES_YYSTYPE;
# define MIMETYPES_YYSTYPE_IS_TRIVIAL 1
# define MIMETYPES_YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined MIMETYPES_YYLTYPE && ! defined MIMETYPES_YYLTYPE_IS_DECLARED
typedef struct MIMETYPES_YYLTYPE MIMETYPES_YYLTYPE;
struct MIMETYPES_YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define MIMETYPES_YYLTYPE_IS_DECLARED 1
# define MIMETYPES_YYLTYPE_IS_TRIVIAL 1
#endif



int mimetypes_yyparse (struct parser_control *pctl, void *yyscanner);
/* "%code provides" blocks.  */
#line 123 "grammar.y" /* yacc.c:1909  */

int mimetypes_yylex (MIMETYPES_YYSTYPE *lvalp, MIMETYPES_YYLTYPE *llocp,
		     yyscan_t yyscanner);
int mimetypes_yylex_init_extra (struct parser_control *, yyscan_t *);
int mimetypes_yylex_destroy (yyscan_t);
void mimetypes_yyerror (MIMETYPES_YYLTYPE const *llocp,
			struct parser_control *pctl, yyscan_t scanner,
			char const *fmt, ...)
  MU_PRINTFLIKE(4,5);
int mimetypes_scanner_open (yyscan_t scanner, const char *name);
 
void lex_next_rule (MIMETYPES_YYLTYPE *llocp, yyscan_t scanner);

#line 153 "grammar.h" /* yacc.c:1909  */

#endif /* !YY_MIMETYPES_YY_GRAMMAR_H_INCLUDED  */
