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

#ifndef YY_MU_SIEVE_YY_SIEVE_GRAM_H_INCLUDED
# define YY_MU_SIEVE_YY_SIEVE_GRAM_H_INCLUDED
/* Debug traces.  */
#ifndef MU_SIEVE_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define MU_SIEVE_YYDEBUG 1
#  else
#   define MU_SIEVE_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define MU_SIEVE_YYDEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined MU_SIEVE_YYDEBUG */
#if MU_SIEVE_YYDEBUG
extern int mu_sieve_yydebug;
#endif
/* "%code requires" blocks.  */
#line 41 "sieve-gram.y" /* yacc.c:1909  */

#define MU_SIEVE_YYLTYPE struct mu_locus_range
#define yylloc mu_sieve_yylloc
#define yylval mu_sieve_yylval

#line 58 "sieve-gram.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef MU_SIEVE_YYTOKENTYPE
# define MU_SIEVE_YYTOKENTYPE
  enum mu_sieve_yytokentype
  {
    IDENT = 258,
    TAG = 259,
    NUMBER = 260,
    STRING = 261,
    MULTILINE = 262,
    REQUIRE = 263,
    IF = 264,
    ELSIF = 265,
    ELSE = 266,
    ANYOF = 267,
    ALLOF = 268,
    NOT = 269,
    FALSE = 270,
    TRUE = 271
  };
#endif

/* Value type.  */
#if ! defined MU_SIEVE_YYSTYPE && ! defined MU_SIEVE_YYSTYPE_IS_DECLARED

union MU_SIEVE_YYSTYPE
{
#line 49 "sieve-gram.y" /* yacc.c:1909  */

  char *string;
  size_t number;
  size_t idx;
  struct mu_sieve_slice slice;
  struct
  {
    char *ident;
    struct mu_locus_range idloc;
    size_t first;
    size_t count;
  } command;
  struct mu_sieve_node_list node_list;
  struct mu_sieve_node *node;

#line 103 "sieve-gram.h" /* yacc.c:1909  */
};

typedef union MU_SIEVE_YYSTYPE MU_SIEVE_YYSTYPE;
# define MU_SIEVE_YYSTYPE_IS_TRIVIAL 1
# define MU_SIEVE_YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined MU_SIEVE_YYLTYPE && ! defined MU_SIEVE_YYLTYPE_IS_DECLARED
typedef struct MU_SIEVE_YYLTYPE MU_SIEVE_YYLTYPE;
struct MU_SIEVE_YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define MU_SIEVE_YYLTYPE_IS_DECLARED 1
# define MU_SIEVE_YYLTYPE_IS_TRIVIAL 1
#endif


extern MU_SIEVE_YYSTYPE mu_sieve_yylval;
extern MU_SIEVE_YYLTYPE mu_sieve_yylloc;
int mu_sieve_yyparse (void);

#endif /* !YY_MU_SIEVE_YY_SIEVE_GRAM_H_INCLUDED  */
