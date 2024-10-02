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

#ifndef YY_ALI_YY_MH_ALIAS_GRAM_H_INCLUDED
# define YY_ALI_YY_MH_ALIAS_GRAM_H_INCLUDED
/* Debug traces.  */
#ifndef ALI_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ALI_YYDEBUG 1
#  else
#   define ALI_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ALI_YYDEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ALI_YYDEBUG */
#if ALI_YYDEBUG
extern int ali_yydebug;
#endif
/* "%code requires" blocks.  */
#line 123 "mh_alias_gram.y" /* yacc.c:1909  */

#define ALI_YYLTYPE struct mu_locus_range
#define yylloc ali_yylloc
#define yylval ali_yylval

#line 58 "mh_alias_gram.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef ALI_YYTOKENTYPE
# define ALI_YYTOKENTYPE
  enum ali_yytokentype
  {
    EOL = 258,
    STRING = 259
  };
#endif

/* Value type.  */
#if ! defined ALI_YYSTYPE && ! defined ALI_YYSTYPE_IS_DECLARED

union ALI_YYSTYPE
{
#line 110 "mh_alias_gram.y" /* yacc.c:1909  */

  char *string;
  mu_list_t list;
  struct mh_alias *alias;

#line 81 "mh_alias_gram.h" /* yacc.c:1909  */
};

typedef union ALI_YYSTYPE ALI_YYSTYPE;
# define ALI_YYSTYPE_IS_TRIVIAL 1
# define ALI_YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined ALI_YYLTYPE && ! defined ALI_YYLTYPE_IS_DECLARED
typedef struct ALI_YYLTYPE ALI_YYLTYPE;
struct ALI_YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define ALI_YYLTYPE_IS_DECLARED 1
# define ALI_YYLTYPE_IS_TRIVIAL 1
#endif


extern ALI_YYSTYPE ali_yylval;
extern ALI_YYLTYPE ali_yylloc;
int ali_yyparse (void);

#endif /* !YY_ALI_YY_MH_ALIAS_GRAM_H_INCLUDED  */
