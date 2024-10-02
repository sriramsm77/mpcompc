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

#ifndef YY_PICK_YY_PICK_GRAM_H_INCLUDED
# define YY_PICK_YY_PICK_GRAM_H_INCLUDED
/* Debug traces.  */
#ifndef PICK_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define PICK_YYDEBUG 1
#  else
#   define PICK_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define PICK_YYDEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined PICK_YYDEBUG */
#if PICK_YYDEBUG
extern int pick_yydebug;
#endif
/* "%code requires" blocks.  */
#line 42 "pick-gram.y" /* yacc.c:1909  */

#define PICK_YYLTYPE struct mu_locus_range
#define yylloc pick_yylloc
#define yylval pick_yylval

#line 58 "pick-gram.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef PICK_YYTOKENTYPE
# define PICK_YYTOKENTYPE
  enum pick_yytokentype
  {
    T_COMP = 258,
    T_DATEFIELD = 259,
    T_STRING = 260,
    T_CFLAGS = 261,
    T_LBRACE = 262,
    T_RBRACE = 263,
    T_BEFORE = 264,
    T_AFTER = 265,
    T_OR = 266,
    T_AND = 267,
    T_NOT = 268
  };
#endif

/* Value type.  */
#if ! defined PICK_YYSTYPE && ! defined PICK_YYSTYPE_IS_DECLARED

union PICK_YYSTYPE
{
#line 54 "pick-gram.y" /* yacc.c:1909  */

  char *string;
  node_t *node;
  regex_t regex;

#line 90 "pick-gram.h" /* yacc.c:1909  */
};

typedef union PICK_YYSTYPE PICK_YYSTYPE;
# define PICK_YYSTYPE_IS_TRIVIAL 1
# define PICK_YYSTYPE_IS_DECLARED 1
#endif


extern PICK_YYSTYPE pick_yylval;

int pick_yyparse (void);

#endif /* !YY_PICK_YY_PICK_GRAM_H_INCLUDED  */
