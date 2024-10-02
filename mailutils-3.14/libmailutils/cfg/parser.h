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

#ifndef YY_MU_CFG_YY_PARSER_H_INCLUDED
# define YY_MU_CFG_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef MU_CFG_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define MU_CFG_YYDEBUG 1
#  else
#   define MU_CFG_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define MU_CFG_YYDEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined MU_CFG_YYDEBUG */
#if MU_CFG_YYDEBUG
extern int mu_cfg_yydebug;
#endif
/* "%code requires" blocks.  */
#line 173 "parser.y" /* yacc.c:1909  */

#define MU_CFG_YYLTYPE struct mu_locus_range
#define yylloc mu_cfg_yylloc
#define yylval mu_cfg_yylval

#line 58 "parser.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef MU_CFG_YYTOKENTYPE
# define MU_CFG_YYTOKENTYPE
  enum mu_cfg_yytokentype
  {
    MU_TOK_IDENT = 258,
    MU_TOK_STRING = 259,
    MU_TOK_QSTRING = 260,
    MU_TOK_MSTRING = 261
  };
#endif

/* Value type.  */
#if ! defined MU_CFG_YYSTYPE && ! defined MU_CFG_YYSTYPE_IS_DECLARED

union MU_CFG_YYSTYPE
{
#line 181 "parser.y" /* yacc.c:1909  */

  mu_cfg_node_t node;
  mu_cfg_node_t *pnode;
  mu_list_t /* of mu_cfg_node_t */ nodelist;
  char *string;
  mu_config_value_t value, *pvalue;
  mu_list_t list;

#line 86 "parser.h" /* yacc.c:1909  */
};

typedef union MU_CFG_YYSTYPE MU_CFG_YYSTYPE;
# define MU_CFG_YYSTYPE_IS_TRIVIAL 1
# define MU_CFG_YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined MU_CFG_YYLTYPE && ! defined MU_CFG_YYLTYPE_IS_DECLARED
typedef struct MU_CFG_YYLTYPE MU_CFG_YYLTYPE;
struct MU_CFG_YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define MU_CFG_YYLTYPE_IS_DECLARED 1
# define MU_CFG_YYLTYPE_IS_TRIVIAL 1
#endif


extern MU_CFG_YYSTYPE mu_cfg_yylval;
extern MU_CFG_YYLTYPE mu_cfg_yylloc;
int mu_cfg_yyparse (void);

#endif /* !YY_MU_CFG_YY_PARSER_H_INCLUDED  */
