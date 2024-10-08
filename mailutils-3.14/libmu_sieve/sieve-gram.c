/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         MU_SIEVE_YYSTYPE
#define YYLTYPE         MU_SIEVE_YYLTYPE
/* Substitute the variable and function names.  */
#define yyparse         mu_sieve_yyparse
#define yylex           mu_sieve_yylex
#define yyerror         mu_sieve_yyerror
#define yydebug         mu_sieve_yydebug
#define yynerrs         mu_sieve_yynerrs

#define yylval          mu_sieve_yylval
#define yychar          mu_sieve_yychar
#define yylloc          mu_sieve_yylloc

/* Copy the first part of user declarations.  */
#line 1 "sieve-gram.y" /* yacc.c:339  */

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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sieve-priv.h>
#include <mailutils/stdstream.h>

mu_sieve_machine_t mu_sieve_machine;
int mu_sieve_error_count;
static struct mu_sieve_node *sieve_tree;

static struct mu_sieve_node *node_alloc (enum mu_sieve_node_type,
					 struct mu_locus_range *);

static void node_list_add (struct mu_sieve_node_list *list,
			   struct mu_sieve_node *node);


#line 116 "sieve-gram.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "sieve-gram.h".  */
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
#line 41 "sieve-gram.y" /* yacc.c:355  */

#define MU_SIEVE_YYLTYPE struct mu_locus_range
#define yylloc mu_sieve_yylloc
#define yylval mu_sieve_yylval

#line 160 "sieve-gram.c" /* yacc.c:355  */

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
#line 49 "sieve-gram.y" /* yacc.c:355  */

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

#line 205 "sieve-gram.c" /* yacc.c:355  */
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

/* Copy the second part of user declarations.  */

#line 236 "sieve-gram.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined MU_SIEVE_YYLTYPE_IS_TRIVIAL && MU_SIEVE_YYLTYPE_IS_TRIVIAL \
             && defined MU_SIEVE_YYSTYPE_IS_TRIVIAL && MU_SIEVE_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  29
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   55

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  25
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  19
/* YYNRULES -- Number of rules.  */
#define YYNRULES  40
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  64

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   271

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      21,    22,     2,     2,    20,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    17,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    23,     2,    24,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    18,     2,    19,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16
};

#if MU_SIEVE_YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    80,    80,    83,    94,    98,   105,   113,   115,   124,
     128,   141,   144,   149,   158,   173,   179,   183,   192,   193,
     198,   203,   210,   240,   244,   250,   259,   292,   296,   299,
     304,   311,   316,   321,   326,   331,   338,   343,   346,   352,
     357
};
#endif

#if MU_SIEVE_YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENT", "TAG", "NUMBER", "STRING",
  "MULTILINE", "REQUIRE", "IF", "ELSIF", "ELSE", "ANYOF", "ALLOF", "NOT",
  "FALSE", "TRUE", "';'", "'{'", "'}'", "','", "'('", "')'", "'['", "']'",
  "$accept", "input", "list", "statement", "else_part", "maybe_elsif",
  "elsif_branch", "block", "testlist", "cond", "test", "command", "action",
  "maybe_arglist", "arglist", "arg", "stringorlist", "stringlist", "slist", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,    59,   123,   125,
      44,    40,    41,    91,    93
};
# endif

#define YYPACT_NINF -28

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-28)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      25,     3,    -4,     9,    31,    25,   -28,   -28,    15,   -28,
     -28,   -28,   -28,    30,   -28,     3,   -28,   -28,   -28,    20,
     -28,    17,    18,     9,   -28,   -28,    23,   -28,   -28,   -28,
     -28,   -28,   -28,    -6,   -28,   -28,     9,     9,   -28,    25,
      32,    37,   -28,   -17,   -28,    -7,     8,     9,   -28,    35,
      38,   -28,     9,   -28,   -28,   -28,    23,    23,     9,   -28,
     -28,   -28,    23,   -28
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    27,     0,     0,     0,     3,     4,    26,     0,    35,
      34,    32,    33,     0,    25,    28,    29,    31,    36,     0,
      37,     0,     0,     0,    24,    23,     0,    18,    22,     1,
       5,     7,    39,     0,    30,     6,     0,     0,    21,     0,
      11,     0,    38,     0,    16,     0,     0,     0,     8,     9,
      12,    40,     0,    19,    20,    15,     0,     0,     0,    17,
      13,    10,     0,    14
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -28,   -28,    11,    -1,   -28,   -28,   -28,   -27,    14,    -3,
     -28,     1,   -28,   -28,   -28,    39,   -28,    50,   -28
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,     5,     6,    48,    49,    50,    40,    43,    44,
      27,    28,     8,    14,    15,    16,    19,    17,    33
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      26,     7,    18,    52,    30,    53,     7,     9,    10,    11,
      12,     1,     1,    52,    41,    54,     2,     3,    42,    13,
      38,    21,    22,    23,    24,    25,    13,    55,     1,    60,
      61,    29,    31,     2,     3,    63,    32,    35,    36,    37,
       7,    39,    47,    51,    56,    30,    57,     7,    58,    59,
      46,    45,    20,     0,    34,    62
};

static const yytype_int8 yycheck[] =
{
       3,     0,     6,    20,     5,    22,     5,     4,     5,     6,
       7,     3,     3,    20,    20,    22,     8,     9,    24,    23,
      23,    12,    13,    14,    15,    16,    23,    19,     3,    56,
      57,     0,    17,     8,     9,    62,     6,    17,    21,    21,
      39,    18,    10,     6,    47,    46,    11,    46,    10,    52,
      39,    37,     2,    -1,    15,    58
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     8,     9,    26,    27,    28,    36,    37,     4,
       5,     6,     7,    23,    38,    39,    40,    42,     6,    41,
      42,    12,    13,    14,    15,    16,    34,    35,    36,     0,
      28,    17,     6,    43,    40,    17,    21,    21,    34,    18,
      32,    20,    24,    33,    34,    33,    27,    10,    29,    30,
      31,     6,    20,    22,    22,    19,    34,    11,    10,    34,
      32,    32,    34,    32
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    25,    26,    26,    27,    27,    28,    28,    28,    29,
      29,    30,    30,    31,    31,    32,    33,    33,    34,    34,
      34,    34,    35,    35,    35,    36,    37,    38,    38,    39,
      39,    40,    40,    40,    40,    40,    41,    41,    42,    43,
      43
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     3,     2,     4,     1,
       3,     0,     1,     3,     4,     3,     1,     3,     1,     4,
       4,     2,     1,     1,     1,     2,     1,     0,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if MU_SIEVE_YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined MU_SIEVE_YYLTYPE_IS_TRIVIAL && MU_SIEVE_YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !MU_SIEVE_YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !MU_SIEVE_YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined MU_SIEVE_YYLTYPE_IS_TRIVIAL && MU_SIEVE_YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 80 "sieve-gram.y" /* yacc.c:1646  */
    {
		 sieve_tree = NULL;
	       }
#line 1448 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 3:
#line 84 "sieve-gram.y" /* yacc.c:1646  */
    {
		 struct mu_locus_range lr;

		 lr.beg = lr.end = (yylsp[0]).end;
		 
		 node_list_add (&(yyvsp[0].node_list), node_alloc (mu_sieve_node_end, &lr));
		 sieve_tree = (yyvsp[0].node_list).head;
	       }
#line 1461 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 4:
#line 95 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node_list).head = (yyval.node_list).tail = (yyvsp[0].node);
	       }
#line 1469 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 5:
#line 99 "sieve-gram.y" /* yacc.c:1646  */
    {
		 node_list_add (&(yyvsp[-1].node_list), (yyvsp[0].node));
		 (yyval.node_list) = (yyvsp[-1].node_list);
	       }
#line 1478 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 6:
#line 106 "sieve-gram.y" /* yacc.c:1646  */
    {
		 mu_sieve_require (mu_sieve_machine, &(yyvsp[-1].slice));
		 /* Reclaim string slots.  The string data referred to by
		    $2 are registered in memory_pool, so we don't free them */
		 mu_sieve_machine->stringcount -= (yyvsp[-1].slice).count;
		 (yyval.node) = NULL;
	       }
#line 1490 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 8:
#line 116 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node) = node_alloc (mu_sieve_node_cond, &(yylsp[-3]));
		 (yyval.node)->v.cond.expr = (yyvsp[-2].node);
		 (yyval.node)->v.cond.iftrue = (yyvsp[-1].node);
		 (yyval.node)->v.cond.iffalse = (yyvsp[0].node);
	       }
#line 1501 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 9:
#line 125 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node) = (yyvsp[0].node_list).head;
	       }
#line 1509 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 10:
#line 129 "sieve-gram.y" /* yacc.c:1646  */
    {
		 if ((yyvsp[-2].node_list).tail)
		   {
		     (yyvsp[-2].node_list).tail->v.cond.iffalse = (yyvsp[0].node);
		     (yyval.node) = (yyvsp[-2].node_list).head;
		   }
		 else
		   (yyval.node) = (yyvsp[0].node);
	       }
#line 1523 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 11:
#line 141 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node_list).head = (yyval.node_list).tail = NULL;
	       }
#line 1531 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 13:
#line 150 "sieve-gram.y" /* yacc.c:1646  */
    {
		 struct mu_sieve_node *node =
		   node_alloc (mu_sieve_node_cond, &(yylsp[-2]));
		 node->v.cond.expr = (yyvsp[-1].node);
		 node->v.cond.iftrue = (yyvsp[0].node);
		 node->v.cond.iffalse = NULL;
		 (yyval.node_list).head = (yyval.node_list).tail = node;
	       }
#line 1544 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 14:
#line 159 "sieve-gram.y" /* yacc.c:1646  */
    {
		 struct mu_sieve_node *node =
		   node_alloc (mu_sieve_node_cond, &(yylsp[-2]));
		 node->v.cond.expr = (yyvsp[-1].node);
		 node->v.cond.iftrue = (yyvsp[0].node);
		 node->v.cond.iffalse = NULL;
		 
		 (yyvsp[-3].node_list).tail->v.cond.iffalse = node;
		 (yyvsp[-3].node_list).tail = node;

		 (yyval.node_list) = (yyvsp[-3].node_list);
	       }
#line 1561 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 15:
#line 174 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node) = (yyvsp[-1].node_list).head;
	       }
#line 1569 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 16:
#line 180 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node_list).head = (yyval.node_list).tail = (yyvsp[0].node);
	       }
#line 1577 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 17:
#line 184 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyvsp[0].node)->prev = (yyvsp[-2].node_list).tail;
		 (yyvsp[-2].node_list).tail->next = (yyvsp[0].node);
		 (yyvsp[-2].node_list).tail = (yyvsp[0].node);
		 (yyval.node_list) = (yyvsp[-2].node_list);
	       }
#line 1588 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 19:
#line 194 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node) = node_alloc (mu_sieve_node_anyof, &(yylsp[-3]));
		 (yyval.node)->v.node = (yyvsp[-1].node_list).head;
	       }
#line 1597 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 20:
#line 199 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node) = node_alloc (mu_sieve_node_allof, &(yylsp[-3]));
		 (yyval.node)->v.node = (yyvsp[-1].node_list).head;
	       }
#line 1606 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 21:
#line 204 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node) = node_alloc (mu_sieve_node_not, &(yylsp[-1]));
		 (yyval.node)->v.node = (yyvsp[0].node);
	       }
#line 1615 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 22:
#line 211 "sieve-gram.y" /* yacc.c:1646  */
    {
		 mu_sieve_registry_t *reg;

		 mu_locus_range_copy (&mu_sieve_machine->locus, &(yylsp[0]));
		 reg = mu_sieve_registry_lookup (mu_sieve_machine, (yyvsp[0].command).ident,
						 mu_sieve_record_test);
		 if (!reg)
		   {
		     mu_diag_at_locus_range (MU_LOG_ERROR, &(yyvsp[0].command).idloc,
					     _("unknown test: %s"),
					     (yyvsp[0].command).ident);
		     mu_i_sv_error (mu_sieve_machine);
		   }
		 else if (!reg->required)
		   {
		     mu_diag_at_locus_range (MU_LOG_ERROR, &(yyvsp[0].command).idloc,
					     _("test `%s' has not been required"),
					     (yyvsp[0].command).ident);
		     mu_i_sv_error (mu_sieve_machine);
		   }
		 
		 (yyval.node) = node_alloc (mu_sieve_node_test, &(yylsp[0]));
		 (yyval.node)->v.command.reg = reg;
		 (yyval.node)->v.command.argstart = (yyvsp[0].command).first;
		 (yyval.node)->v.command.argcount = (yyvsp[0].command).count;
		 (yyval.node)->v.command.tagcount = 0;
		 (yyval.node)->v.command.comparator = NULL;
		 mu_i_sv_lint_command (mu_sieve_machine, (yyval.node));
	       }
#line 1649 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 23:
#line 241 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node) = node_alloc (mu_sieve_node_true, &(yylsp[0]));
	       }
#line 1657 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 24:
#line 245 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.node) = node_alloc (mu_sieve_node_false, &(yylsp[0]));
	       }
#line 1665 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 25:
#line 251 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.command).ident = (yyvsp[-1].string);
		 (yyval.command).idloc = (yylsp[-1]);
		 (yyval.command).first = (yyvsp[0].slice).first;
		 (yyval.command).count = (yyvsp[0].slice).count;
	       }
#line 1676 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 26:
#line 260 "sieve-gram.y" /* yacc.c:1646  */
    {
		 mu_sieve_registry_t *reg;

		 mu_locus_range_copy (&mu_sieve_machine->locus, &(yylsp[0]));
		 reg = mu_sieve_registry_lookup (mu_sieve_machine, (yyvsp[0].command).ident,
						 mu_sieve_record_action);
		 
		 if (!reg)
		   {
		     mu_diag_at_locus_range (MU_LOG_ERROR, &(yyvsp[0].command).idloc,
					     _("unknown action: %s"),
					     (yyvsp[0].command).ident);
		     mu_i_sv_error (mu_sieve_machine);
		   }
		 else if (!reg->required)
		   {
		     mu_diag_at_locus_range (MU_LOG_ERROR, &(yyvsp[0].command).idloc,
					     _("action `%s' has not been required"),
					     (yyvsp[0].command).ident);
		     mu_i_sv_error (mu_sieve_machine);
		   }
		 
		 (yyval.node) = node_alloc(mu_sieve_node_action, &(yylsp[0]));
		 (yyval.node)->v.command.reg = reg;
		 (yyval.node)->v.command.argstart = (yyvsp[0].command).first;
		 (yyval.node)->v.command.argcount = (yyvsp[0].command).count;
		 (yyval.node)->v.command.tagcount = 0;
		 mu_i_sv_lint_command (mu_sieve_machine, (yyval.node));		 
	       }
#line 1710 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 27:
#line 292 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.slice).first = 0;
		 (yyval.slice).count = 0;
	       }
#line 1719 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 29:
#line 300 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.slice).first = (yyvsp[0].idx);
		 (yyval.slice).count = 1;
	       }
#line 1728 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 30:
#line 305 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyvsp[-1].slice).count++;
		 (yyval.slice) = (yyvsp[-1].slice);
	       }
#line 1737 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 31:
#line 312 "sieve-gram.y" /* yacc.c:1646  */
    {		 
		 (yyval.idx) = mu_sieve_value_create (mu_sieve_machine,
					     SVT_STRING_LIST, &(yylsp[0]), &(yyvsp[0].slice));
	       }
#line 1746 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 32:
#line 317 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.idx) = mu_sieve_value_create (mu_sieve_machine, SVT_STRING,
					     &(yylsp[0]), (yyvsp[0].string));
               }
#line 1755 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 33:
#line 322 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.idx) = mu_sieve_value_create (mu_sieve_machine, SVT_STRING,
					     &(yylsp[0]), (yyvsp[0].string));
	       }
#line 1764 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 34:
#line 327 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.idx) = mu_sieve_value_create (mu_sieve_machine, SVT_NUMBER,
					     &(yylsp[0]), &(yyvsp[0].number));
	       }
#line 1773 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 35:
#line 332 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.idx) = mu_sieve_value_create (mu_sieve_machine, SVT_TAG,
					     &(yylsp[0]), (yyvsp[0].string));
	       }
#line 1782 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 36:
#line 339 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.slice).first = mu_i_sv_string_create (mu_sieve_machine, (yyvsp[0].string));
		 (yyval.slice).count = 1;
	       }
#line 1791 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 38:
#line 347 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.slice) = (yyvsp[-1].slice);
	       }
#line 1799 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 39:
#line 353 "sieve-gram.y" /* yacc.c:1646  */
    {
		 (yyval.slice).first = mu_i_sv_string_create (mu_sieve_machine, (yyvsp[0].string));
		 (yyval.slice).count = 1;
	       }
#line 1808 "sieve-gram.c" /* yacc.c:1646  */
    break;

  case 40:
#line 358 "sieve-gram.y" /* yacc.c:1646  */
    {
		 mu_i_sv_string_create (mu_sieve_machine, (yyvsp[0].string));
		 (yyvsp[-2].slice).count++;
		 (yyval.slice) = (yyvsp[-2].slice);
	       }
#line 1818 "sieve-gram.c" /* yacc.c:1646  */
    break;


#line 1822 "sieve-gram.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 365 "sieve-gram.y" /* yacc.c:1906  */


int
yyerror (const char *s)
{
  mu_error ("%s", s);
  mu_i_sv_error (mu_sieve_machine);
  return 0;
}

static void
node_list_add (struct mu_sieve_node_list *list, struct mu_sieve_node *node)
{
  if (!node)
    return;

  node->prev = list->tail;
  if (list->tail)
    list->tail->next = node;
  else
    list->head = node;
  list->tail = node;
}

static struct mu_sieve_node *
node_alloc (enum mu_sieve_node_type type, struct mu_locus_range *lr)
{
  struct mu_sieve_node *node = malloc (sizeof (*node));
  if (node)
    {
      node->prev = node->next = NULL;
      node->type = type;
      mu_locus_range_init (&node->locus);
      mu_locus_range_copy (&node->locus, lr);
    }
  return node;
}

static void node_optimize (struct mu_sieve_node *node);
static void node_free (struct mu_sieve_node *node);
static void node_replace (struct mu_sieve_node *node,
			  struct mu_sieve_node *repl);
static void node_code (struct mu_sieve_machine *mach,
		       struct mu_sieve_node *node);
static void node_dump (mu_stream_t str, struct mu_sieve_node *node,
		       unsigned level, struct mu_sieve_machine *mach);

static void tree_free (struct mu_sieve_node **tree);
static void tree_optimize (struct mu_sieve_node *tree);
static void tree_code (struct mu_sieve_machine *mach,
		       struct mu_sieve_node *tree);
static void tree_dump (mu_stream_t str,
		       struct mu_sieve_node *tree, unsigned level,
		       struct mu_sieve_machine *mach);

static void
indent (mu_stream_t str, unsigned level)
{
#define tab "  "
#define tablen (sizeof (tab) - 1)
  while (level--)
    mu_stream_write (str, tab, tablen, NULL);
}

/* mu_sieve_node_noop */
static void
dump_node_noop (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
		struct mu_sieve_machine *mach)
{
  indent (str, level);
  mu_stream_printf (str, "NOOP\n");  
}

/* mu_sieve_node_false */
static void
dump_node_false (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
		 struct mu_sieve_machine *mach)
{
  indent (str, level);
  mu_stream_printf (str, "FALSE\n");
}

/* mu_sieve_node_true */
static void
dump_node_true (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
		struct mu_sieve_machine *mach)
{
  indent (str, level);
  mu_stream_printf (str, "TRUE\n");
}

/* mu_sieve_node_test & mu_sieve_node_action */
static void
free_node_command (struct mu_sieve_node *node)
{
  /* nothing */
}

static void
code_node_test (struct mu_sieve_machine *mach, struct mu_sieve_node *node)
{
  mu_i_sv_code_test (mach, node);
}

static void
code_node_action (struct mu_sieve_machine *mach, struct mu_sieve_node *node)
{
  mu_i_sv_code_action (mach, node);
}

void
mu_i_sv_valf (mu_sieve_machine_t mach, mu_stream_t str, mu_sieve_value_t *val)
{
  mu_stream_printf (str, " ");
  if (val->tag)
    {
      mu_stream_printf (str, ":%s", val->tag);
      if (val->type == SVT_VOID)
	return;
      mu_stream_printf (str, " ");
    }
  switch (val->type)
    {
    case SVT_VOID:
      mu_stream_printf (str, "(void)");
      break;
      
    case SVT_NUMBER:
      mu_stream_printf (str, "%zu", val->v.number);
      break;
      
    case SVT_STRING:
      mu_stream_printf (str, "\"%s\"",
			mu_sieve_string_raw (mach, &val->v.list, 0)->orig);
      break;
      
    case SVT_STRING_LIST:
      {
	size_t i;
	
	mu_stream_printf (str, "[");
	for (i = 0; i < val->v.list.count; i++)
	  {
	    if (i)
	      mu_stream_printf (str, ", ");
	    mu_stream_printf (str, "\"%s\"",
			      mu_sieve_string_raw (mach, &val->v.list, i)->orig);
	  }
	mu_stream_printf (str, "]");
      }
      break;
      
    case SVT_TAG:
      mu_stream_printf (str, ":%s", val->v.string);
      break;
      
    default:
      abort ();
    }
}
  
static void
dump_node_command (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
		   struct mu_sieve_machine *mach)
{
  size_t i;
  
  indent (str, level);
  mu_stream_printf (str, "COMMAND %s", node->v.command.reg->name);
  for (i = 0; i < node->v.command.argcount + node->v.command.tagcount; i++)
    mu_i_sv_valf (mach, str, &mach->valspace[node->v.command.argstart + i]);
  mu_stream_printf (str, "\n");
}

/* mu_sieve_node_cond */
static void
free_node_cond (struct mu_sieve_node *node)
{
  tree_free (&node->v.cond.expr);
  tree_free (&node->v.cond.iftrue);
  tree_free (&node->v.cond.iffalse);	     
}

static void
optimize_node_cond (struct mu_sieve_node *node)
{
  tree_optimize (node->v.cond.expr);
  switch (node->v.cond.expr->type)
    {
    case mu_sieve_node_true:
      tree_optimize (node->v.cond.iftrue);
      node_replace (node, node->v.cond.iftrue);
      break;

    case mu_sieve_node_false:
      tree_optimize (node->v.cond.iffalse);
      node_replace (node, node->v.cond.iffalse);
      break;

    default:
      tree_optimize (node->v.cond.iftrue);
      tree_optimize (node->v.cond.iffalse);
    }
}

static void
code_node_cond (struct mu_sieve_machine *mach, struct mu_sieve_node *node)
{
  size_t br1;
  
  tree_code (mach, node->v.cond.expr);
  mu_i_sv_code (mach, (sieve_op_t) _mu_i_sv_instr_brz);
  br1 = mach->pc;
  mu_i_sv_code (mach, (sieve_op_t) 0);
  tree_code (mach, node->v.cond.iftrue);
  
  if (node->v.cond.iffalse)
    {
      size_t br2;

      mu_i_sv_code (mach, (sieve_op_t) _mu_i_sv_instr_branch);
      br2 = mach->pc;
      mu_i_sv_code (mach, (sieve_op_t) 0);
      
      mach->prog[br1].pc = mach->pc - br1 - 1;

      tree_code (mach, node->v.cond.iffalse);
      mach->prog[br2].pc = mach->pc - br2 - 1;
    }
  else
    mach->prog[br1].pc = mach->pc - br1 - 1;
}
  
static void
dump_node_cond (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
		struct mu_sieve_machine *mach)
{
  indent (str, level);
  mu_stream_printf (str, "COND\n");

  ++level;

  indent (str, level);
  mu_stream_printf (str, "EXPR:\n");
  tree_dump (str, node->v.cond.expr, level + 1, mach);

  indent (str, level);
  mu_stream_printf (str, "IFTRUE:\n");
  tree_dump (str, node->v.cond.iftrue, level + 1, mach);

  indent (str, level);
  mu_stream_printf (str, "IFFALSE:\n");
  tree_dump (str, node->v.cond.iffalse, level + 1, mach);
}

/* mu_sieve_node_anyof & mu_sieve_node_allof */
static void
free_node_x_of (struct mu_sieve_node *node)
{
  tree_free (&node->v.node);
}

static void
optimize_x_of (struct mu_sieve_node *node, enum mu_sieve_node_type solve)
{
  struct mu_sieve_node *cur;
  tree_optimize (node->v.node);
  cur = node->v.node;
  while (cur)
    {
      struct mu_sieve_node *next = cur->next;
      switch (cur->type)
	{
	case mu_sieve_node_false:
	case mu_sieve_node_true:
	  if (cur->type == solve)
	    {
	      tree_free (&node->v.node);
	      node->type = solve;
	      return;
	    }
	  else
	    {
	      if (cur->prev)
		cur->prev->next = next;
	      else
		node->v.node = next;
	      if (next)
		next->prev = cur->prev;
	      node_free (cur);
	    }
	  break;

	default:
	  break;
	}
      
      cur = next;
    }
  
  if (!node->v.node)
    node->type = solve == mu_sieve_node_false ? mu_sieve_node_true : mu_sieve_node_false;
}

static void
code_node_x_of (struct mu_sieve_machine *mach, struct mu_sieve_node *node,
		sieve_op_t op)
{
  struct mu_sieve_node *cur = node->v.node;
  size_t pc = 0;
  size_t end;
  
  while (cur)
    {
      node_code (mach, cur);
      if (cur->next)
	{
	  mu_i_sv_code (mach, op);
	  mu_i_sv_code (mach, (sieve_op_t) pc);
	  pc = mach->pc - 1;
	}
      cur = cur->next;
    }

  /* Fix-up locations */
  end = mach->pc;
  while (pc != 0)
    {
      size_t prev = mach->prog[pc].pc;
      mach->prog[pc].pc = end - pc - 1;
      pc = prev;
    }
}

static void
dump_node_x_of (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
		mu_sieve_machine_t mach)
{
  indent (str, level);
  mu_stream_printf (str, "%s:\n",
		    node->type == mu_sieve_node_allof ? "ALLOF" : "ANYOF");

  ++level;
  node = node->v.node;
  while (node)
    {
      node_dump (str, node, level + 1, mach);
      node = node->next;
      if (node)
	{
	  indent (str, level);
	  mu_stream_printf (str, "%s:\n",
			    node->type == mu_sieve_node_allof ? "AND" : "OR");
	}
    }
}
  
/* mu_sieve_node_anyof */
static void
optimize_node_anyof (struct mu_sieve_node *node)
{
  optimize_x_of (node, mu_sieve_node_true);
}

static void
code_node_anyof (struct mu_sieve_machine *mach, struct mu_sieve_node *node)
{
  code_node_x_of (mach, node, (sieve_op_t) _mu_i_sv_instr_brnz);
}

/* mu_sieve_node_allof */
static void
optimize_node_allof (struct mu_sieve_node *node)
{
  return optimize_x_of (node, mu_sieve_node_false);
}

static void
code_node_allof (struct mu_sieve_machine *mach, struct mu_sieve_node *node)
{
  code_node_x_of (mach, node, (sieve_op_t) _mu_i_sv_instr_brz);
}

/* mu_sieve_node_not */
static void
free_node_not (struct mu_sieve_node *node)
{
  tree_free (&node->v.node);
}

static void
optimize_node_not (struct mu_sieve_node *node)
{
  tree_optimize (node->v.node);
  switch (node->v.node->type)
    {
    case mu_sieve_node_false:
      tree_free (&node->v.node);
      node->type = mu_sieve_node_true;
      break;
      
    case mu_sieve_node_true:
      tree_free (&node->v.node);
      node->type = mu_sieve_node_false;
      break;

    default:
      break;
    }
}

static void
code_node_not (struct mu_sieve_machine *mach, struct mu_sieve_node *node)
{
  node_code (mach, node->v.node);
  mu_i_sv_code (mach, (sieve_op_t) _mu_i_sv_instr_not);
}

static void
dump_node_not (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
	       struct mu_sieve_machine *mach)
{
  indent (str, level);
  mu_stream_printf (str, "NOT\n");
  node_dump (str, node->v.node, level + 1, mach);
}

/* mu_sieve_node_end */
static void
code_node_end (struct mu_sieve_machine *mach, struct mu_sieve_node *node)
{
  mu_i_sv_code (mach, (sieve_op_t) (sieve_instr_t) 0);
}

static void
dump_node_end (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
	       struct mu_sieve_machine *mach)
{
  indent (str, level);
  mu_stream_printf (str, "END\n");
}

struct node_descr
{
  void (*code_fn) (struct mu_sieve_machine *mach, struct mu_sieve_node *node);
  void (*optimize_fn) (struct mu_sieve_node *node);
  void (*free_fn) (struct mu_sieve_node *node);
  void (*dump_fn) (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
		   mu_sieve_machine_t);

};

static struct node_descr node_descr[] = {
  [mu_sieve_node_noop]  = { NULL, NULL, NULL, dump_node_noop },
  [mu_sieve_node_false]  = { NULL, NULL, NULL, dump_node_false },
  [mu_sieve_node_true]   = { NULL, NULL, NULL, dump_node_true },
  [mu_sieve_node_test]   = { code_node_test, NULL,
			     free_node_command, dump_node_command },
  [mu_sieve_node_action] = { code_node_action, NULL,
			     free_node_command, dump_node_command },
  [mu_sieve_node_cond]   = { code_node_cond, optimize_node_cond,
			     free_node_cond, dump_node_cond },
  [mu_sieve_node_anyof]  = { code_node_anyof, optimize_node_anyof,
			     free_node_x_of, dump_node_x_of },
  [mu_sieve_node_allof]  = { code_node_allof, optimize_node_allof,
			     free_node_x_of, dump_node_x_of },
  [mu_sieve_node_not]    = { code_node_not, optimize_node_not,
			     free_node_not, dump_node_not },
  [mu_sieve_node_end]    = { code_node_end, NULL, NULL, dump_node_end }
};

static void
node_optimize (struct mu_sieve_node *node)
{
  if ((int)node->type >= MU_ARRAY_SIZE (node_descr))
    abort ();
  if (node_descr[node->type].optimize_fn)
    node_descr[node->type].optimize_fn (node);
}

static void
node_free (struct mu_sieve_node *node)
{
  if ((int)node->type >= MU_ARRAY_SIZE (node_descr))
    abort ();
  if (node_descr[node->type].free_fn)
    node_descr[node->type].free_fn (node);
  free (node);
}

static void
node_replace (struct mu_sieve_node *node, struct mu_sieve_node *repl)
{
  struct mu_sieve_node copy;

  if ((int)node->type >= MU_ARRAY_SIZE (node_descr))
    abort ();
  
  copy = *node;
  if (repl)
    {
      node->type = repl->type;
      node->v = repl->v;

      switch (copy.type)
	{
	case mu_sieve_node_cond:
	  if (repl == copy.v.cond.expr)
	    copy.v.cond.expr = NULL;
	  else if (repl == copy.v.cond.iftrue)
	    copy.v.cond.iftrue = NULL;
	  else if (repl == copy.v.cond.iffalse)
	    copy.v.cond.iffalse = NULL;
	  break;
	  
	case mu_sieve_node_not:
	  if (repl == copy.v.node)
	    copy.v.node = NULL;
	  break;

	default:
	  break;
	}
    }
  else
    node->type = mu_sieve_node_noop;

  if (node_descr[node->type].free_fn)
    node_descr[node->type].free_fn (&copy);
}

static void
node_code (struct mu_sieve_machine *mach, struct mu_sieve_node *node)
{
  if ((int)node->type >= MU_ARRAY_SIZE (node_descr))
    abort ();

  if (node_descr[node->type].code_fn)
    {
      mu_i_sv_locus (mach, &node->locus);
      node_descr[node->type].code_fn (mach, node);
    }
}

static void
node_dump (mu_stream_t str, struct mu_sieve_node *node, unsigned level,
	   struct mu_sieve_machine *mach)
{
  if ((int)node->type >= MU_ARRAY_SIZE (node_descr)
      || !node_descr[node->type].dump_fn) 
    abort ();
  node_descr[node->type].dump_fn (str, node, level, mach);
}


static void
tree_free (struct mu_sieve_node **tree)
{
  struct mu_sieve_node *cur = *tree;
  while (cur)
    {
      struct mu_sieve_node *next = cur->next;
      node_free (cur);
      cur = next;
    }
}

static void
tree_optimize (struct mu_sieve_node *tree)
{
  while (tree)
    {
      node_optimize (tree);
      tree = tree->next;
    }
}

static void
tree_code (struct mu_sieve_machine *mach, struct mu_sieve_node *tree)
{
  while (tree)
    {
      node_code (mach, tree);
      tree = tree->next;
    }
}

static void
tree_dump (mu_stream_t str, struct mu_sieve_node *tree, unsigned level,
	   struct mu_sieve_machine *mach)
{
  while (tree)
    {
      node_dump (str, tree, level, mach);
      tree = tree->next;
    }
}  

void
mu_i_sv_error (mu_sieve_machine_t mach)
{
  mach->state = mu_sieve_state_error;
}

int
mu_sieve_machine_create (mu_sieve_machine_t *pmach)
{
  int rc;
  mu_sieve_machine_t mach;

  mu_sieve_debug_init ();
  mach = malloc (sizeof (*mach));
  if (!mach)
    return ENOMEM;
  memset (mach, 0, sizeof (*mach));
  mach->memory_pool = NULL;
  rc = mu_opool_create (&mach->string_pool, MU_OPOOL_DEFAULT);
  if (rc)
    {
      mu_list_destroy (&mach->memory_pool);
      free (mach);
      return rc;
    }
  
  mach->data = NULL;

  mu_sieve_set_diag_stream (mach, mu_strerr);
  mu_sieve_set_dbg_stream (mach, mu_strerr);
  
  *pmach = mach;
  return 0;
}

void
mu_i_sv_free_stringspace (mu_sieve_machine_t mach)
{
  size_t i;
  
  for (i = 0; i < mach->stringcount; i++)
    {
      if (mach->stringspace[i].rx)
	{
	  regex_t *rx = mach->stringspace[i].rx;
	  regfree (rx);
	}
      /* There's no need to free mach->stringspace[i].exp, because
	 it is allocated in mach's memory pool */
    }
}  

int
mu_sieve_machine_reset (mu_sieve_machine_t mach)
{
  switch (mach->state)
    {
    case mu_sieve_state_init:
      /* Nothing to do */
      return 0;
      
    case mu_sieve_state_error:
    case mu_sieve_state_compiled:
      /* Do the right thing */
      break;
      
    case mu_sieve_state_running:
    case mu_sieve_state_disass:
      /* Can't reset a running machine */
      return MU_ERR_FAILURE;
    }

  mu_i_sv_free_stringspace (mach);
  mu_list_clear (mach->memory_pool);
  mu_list_clear (mach->destr_list);
  mu_opool_free (mach->string_pool, NULL);
  mu_i_sv_free_idspace (mach);
  mu_list_clear (mach->registry);

  mach->stringspace = NULL;
  mach->stringcount = 0;
  mach->stringmax = 0;

  mach->valspace = NULL;
  mach->valcount = 0;
  mach->valmax = 0;

  mach->progsize = 0;
  mach->prog = NULL;

  mu_assoc_destroy (&mach->vartab);
  mach->match_string = NULL;
  mach->match_buf = NULL;
  mach->match_count = 0;
  mach->match_max = 0;
  
  mach->state = mu_sieve_state_init;

  return 0;
}

static int
regdup (void *item, void *data)
{
  mu_sieve_registry_t *reg = item;
  mu_sieve_machine_t mach = data;

  mu_sieve_registry_require (mach, reg->name, reg->type);
  return 0;
}

static void
copy_stream_state (mu_sieve_machine_t child, mu_sieve_machine_t parent)
{
  child->state_flags = parent->state_flags;
  child->err_mode    = parent->err_mode;
  mu_locus_range_copy (&child->err_locus, &parent->err_locus);
  child->dbg_mode    = parent->dbg_mode;
  mu_locus_range_copy (&child->dbg_locus, &parent->dbg_locus);
  child->errstream = parent->errstream;
  mu_stream_ref (child->errstream);
  child->dbgstream = parent->dbgstream;
  mu_stream_ref (child->dbgstream);
}

int
mu_sieve_machine_clone (mu_sieve_machine_t const parent,
			mu_sieve_machine_t *pmach)
{
  size_t i;
  mu_sieve_machine_t child;
  int rc;
  
  if (!parent || parent->state == mu_sieve_state_error)
    return EINVAL;
  
  rc = mu_sieve_machine_create (&child);
  if (rc)
    return rc;

  rc = setjmp (child->errbuf);

  if (rc == 0)
    {
      child->state = mu_sieve_state_init;
      mu_i_sv_register_standard_actions (child);
      mu_i_sv_register_standard_tests (child);
      mu_i_sv_register_standard_comparators (child);

      /* Load necessary modules */
      mu_list_foreach (parent->registry, regdup, child);
  
      /* Copy identifiers */
      child->idspace = mu_sieve_calloc (child, parent->idcount,
					sizeof (child->idspace[0]));
      child->idcount = child->idmax = parent->idcount;
      for (i = 0; i < child->idcount; i++)
	child->idspace[i] = mu_sieve_strdup (parent, parent->idspace[i]);
      
      /* Copy string constants */
      child->stringspace = mu_sieve_calloc (child, parent->stringcount,
					    sizeof (child->stringspace[0]));
      child->stringcount = child->stringmax = parent->stringcount;
      for (i = 0; i < parent->stringcount; i++)
	{
	  memset (&child->stringspace[i], 0, sizeof (child->stringspace[0]));
	  child->stringspace[i].orig =
	    mu_sieve_strdup (parent, parent->stringspace[i].orig);
	}

      /* Copy value space */
      child->valspace = mu_sieve_calloc (child, parent->valcount,
					 sizeof child->valspace[0]);
      child->valcount = child->valmax = parent->valcount;
      for (i = 0; i < child->valcount; i++)
	{
	  child->valspace[i].type = parent->valspace[i].type;
	  child->valspace[i].tag =
	    mu_sieve_strdup (child, parent->valspace[i].tag);
	  switch (child->valspace[i].type)
	    {
	    case SVT_TAG:
	      child->valspace[i].v.string =
		mu_sieve_strdup (child, parent->valspace[i].v.string);
	      break;
	      
	    default:
	      child->valspace[i].v = parent->valspace[i].v;
	    }
	}
      
      /* Copy progspace */
      child->progsize = parent->progsize;
      child->prog = mu_sieve_calloc (child, parent->progsize,
				     sizeof child->prog[0]);
      memcpy (child->prog, parent->prog,
	      parent->progsize * sizeof (child->prog[0]));

      /* Copy variables */
      if (mu_sieve_has_variables (parent))
	{
	  mu_i_sv_copy_variables (child, parent);
	  child->match_string = NULL;
	  child->match_buf = NULL;
	  child->match_count = 0;
	  child->match_max = 0;
	}
      
      /* Copy user-defined settings */
      
      child->dry_run     = parent->dry_run;
      
      copy_stream_state (child, parent);
      
      child->data = parent->data;
      child->logger = parent->logger;
      child->daemon_email = parent->daemon_email;
      
      *pmach = child;
    }
  else
    mu_sieve_machine_destroy (&child);
  
  return rc;
}

int
mu_sieve_machine_dup (mu_sieve_machine_t const in, mu_sieve_machine_t *out)
{
  int rc;
  mu_sieve_machine_t mach;

  if (!in || in->state == mu_sieve_state_error)
    return EINVAL; 
  mach = malloc (sizeof (*mach));
  if (!mach)
    return ENOMEM;
  memset (mach, 0, sizeof (*mach));
  rc = mu_list_create (&mach->memory_pool);
  if (rc)
    {
      free (mach);
      return rc;
    }
  mach->destr_list = NULL;
  mach->registry = NULL;

  mach->progsize = in->progsize;
  mach->prog = in->prog;

  switch (in->state)
    {
    case mu_sieve_state_running:
    case mu_sieve_state_disass:
      mach->state = mu_sieve_state_compiled;
      break;

    default:
      mach->state = in->state;
    }

  rc = setjmp (mach->errbuf);

  if (rc == 0)
    {
      mach->pc = 0;
      mach->reg = 0;

      mach->dry_run = in->dry_run;
      
      mach->state_flags = in->state_flags;
      mach->err_mode    = in->err_mode;
      mu_locus_range_copy (&mach->err_locus, &in->err_locus);
      mach->dbg_mode    = in->dbg_mode;
      mu_locus_range_copy (&mach->dbg_locus, &in->dbg_locus);  
      
      copy_stream_state (mach, in);

      mu_i_sv_copy_variables (mach, in);
      
      mach->data = in->data;
      mach->logger = in->logger;
      mach->daemon_email = in->daemon_email;

      *out = mach;
    }
  else
    mu_sieve_machine_destroy (&mach);
  
  return rc;
}

void
mu_sieve_get_diag_stream (mu_sieve_machine_t mach, mu_stream_t *pstr)
{
  *pstr = mach->errstream;
  mu_stream_ref (*pstr);
}

void
mu_sieve_set_diag_stream (mu_sieve_machine_t mach, mu_stream_t str)
{
  mu_stream_unref (mach->errstream);
  mach->errstream = str;
  mu_stream_ref (mach->errstream);
}

void
mu_sieve_set_dbg_stream (mu_sieve_machine_t mach, mu_stream_t str)
{
  mu_stream_unref (mach->dbgstream);
  mach->dbgstream = str;
  mu_stream_ref (mach->dbgstream);
}

void
mu_sieve_get_dbg_stream (mu_sieve_machine_t mach, mu_stream_t *pstr)
{
  *pstr = mach->dbgstream;
  mu_stream_ref (*pstr);
}

void
mu_sieve_set_logger (mu_sieve_machine_t mach, mu_sieve_action_log_t logger)
{
  mach->logger = logger;
}

mu_mailer_t
mu_sieve_get_mailer (mu_sieve_machine_t mach)
{
  if (!mach->mailer)
    {
      int rc;

      rc = mu_mailer_create (&mach->mailer, NULL);
      if (rc)
	{
	  mu_sieve_error (mach,
			  _("%lu: cannot create mailer: %s"),
			  (unsigned long) mu_sieve_get_message_num (mach),
			  mu_strerror (rc));
	  return NULL;
	}
      rc = mu_mailer_open (mach->mailer, 0);
      if (rc)
	{
	  mu_url_t url = NULL;
	  mu_mailer_get_url (mach->mailer, &url);
	  mu_sieve_error (mach,
			  _("%lu: cannot open mailer %s: %s"),
			  (unsigned long) mu_sieve_get_message_num (mach),
			  mu_url_to_string (url), mu_strerror (rc));
	  mu_mailer_destroy (&mach->mailer);
	  return NULL;
	}
    }
  return mach->mailer;
}

void
mu_sieve_set_mailer (mu_sieve_machine_t mach, mu_mailer_t mailer)
{
  mu_mailer_destroy (&mach->mailer);
  mach->mailer = mailer;
}

#define MAILER_DAEMON_PFX "MAILER-DAEMON@"

char *
mu_sieve_get_daemon_email (mu_sieve_machine_t mach)
{
  if (!mach->daemon_email)
    {
      const char *domain = NULL;
      
      mu_get_user_email_domain (&domain);
      mach->daemon_email = mu_sieve_malloc (mach,
					    sizeof(MAILER_DAEMON_PFX) +
					    strlen (domain));
      sprintf (mach->daemon_email, "%s%s", MAILER_DAEMON_PFX, domain);
    }
  return mach->daemon_email;
}

void
mu_sieve_set_daemon_email (mu_sieve_machine_t mach, const char *email)
{
  mu_sieve_free (mach, (void *)mach->daemon_email);
  mach->daemon_email = mu_sieve_strdup (mach, email);
}

struct sieve_destr_record
{
  mu_sieve_destructor_t destr;
  void *ptr;
};

static void
run_destructor (void *data)
{
  struct sieve_destr_record *p = data;
  p->destr (p->ptr);
  free (data);
}

void
mu_sieve_machine_add_destructor (mu_sieve_machine_t mach,
				 mu_sieve_destructor_t destr,
				 void *ptr)
{
  int rc;
  struct sieve_destr_record *p;
  
  if (!mach->destr_list)
    {
      rc = mu_list_create (&mach->destr_list);
      if (rc)
	{
	  mu_sieve_error (mach, "mu_list_create: %s", mu_strerror (rc));
	  destr (ptr);
	  mu_sieve_abort (mach);
	}
      mu_list_set_destroy_item (mach->destr_list, run_destructor);
    }
  p = malloc (sizeof (*p));
  if (!p)
    {
      mu_sieve_error (mach, "%s", mu_strerror (errno));
      destr (ptr);
      mu_sieve_abort (mach);
    }
  p->destr = destr;
  p->ptr = ptr;
  rc = mu_list_prepend (mach->destr_list, p);
  if (rc)
    {
      mu_sieve_error (mach, "mu_list_prepend: %s", mu_strerror (rc));
      destr (ptr);
      free (p);
      mu_sieve_abort (mach);
    }
}

void
mu_sieve_machine_destroy (mu_sieve_machine_t *pmach)
{
  mu_sieve_machine_t mach = *pmach;

  mu_i_sv_free_stringspace (mach);
  mu_sieve_free (mach, mach->stringspace);
  mu_stream_destroy (&mach->errstream);
  mu_stream_destroy (&mach->dbgstream);
  mu_mailer_destroy (&mach->mailer);
  mu_list_destroy (&mach->destr_list);
  mu_list_destroy (&mach->registry);
  mu_sieve_free (mach, mach->idspace);
  mu_opool_destroy (&mach->string_pool);
  mu_list_destroy (&mach->memory_pool);
  mu_assoc_destroy (&mach->vartab);
  mu_list_destroy (&mach->init_var);

  free (mach);
  *pmach = NULL;
}

int
with_machine (mu_sieve_machine_t mach, int (*thunk) (void *), void *data)
{
  int rc = 0;
  mu_stream_t save_errstr;

  rc = mu_sieve_machine_reset (mach);
  if (rc)
    return rc;
  
  save_errstr = mu_strerr;  
  mu_stream_ref (save_errstr);
  mu_strerr = mach->errstream;
  mu_stream_ref (mu_strerr);

  mu_sieve_machine = mach;
  rc = setjmp (mach->errbuf);

  if (rc == 0)
    {
      mach->state = mu_sieve_state_init;
      mu_i_sv_register_standard_actions (mach);
      mu_i_sv_register_standard_tests (mach);
      mu_i_sv_register_standard_comparators (mach);

      mu_sieve_stream_save (mach);
      rc = thunk (data);
      mu_sieve_stream_restore (mach);

      mu_stream_unref (save_errstr);
      mu_strerr = save_errstr;
      mu_stream_unref (mu_strerr);
    }
  else
    mach->state = mu_sieve_state_error;
  
  return rc;
}

/* Rescan all registered strings to determine their properties */
static void
string_rescan (mu_sieve_machine_t mach)
{
  size_t i;
  int hasvar = mu_sieve_has_variables (mach);
  
  for (i = 0; i < mach->stringcount; i++)
    {
      mach->stringspace[i].changed = 0;
      if (hasvar)
	{
	  mach->stringspace[i].constant = 0;
	  mu_sieve_string_get (mach, &mach->stringspace[i]);
	  mu_sieve_free (mach, mach->stringspace[i].exp);
	  mach->stringspace[i].exp = NULL;
	  mach->stringspace[i].constant = !mach->stringspace[i].changed;
	  mach->stringspace[i].changed = 0;
	}
      else
	mach->stringspace[i].constant = 1;
    }
}

static int
sieve_parse (void)
{
  int rc;
  int old_mode, mode;

  sieve_tree = NULL;
  yydebug = mu_debug_level_p (mu_sieve_debug_handle, MU_DEBUG_TRACE3);

  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_GET_MODE, &old_mode);
  mode = old_mode | MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);

  rc = yyparse ();
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &old_mode);
  
  mu_i_sv_lex_finish ();
  if (rc)
    mu_i_sv_error (mu_sieve_machine);
  if (mu_sieve_machine->state == mu_sieve_state_init)
    {
      if (mu_debug_level_p (mu_sieve_debug_handle, MU_DEBUG_TRACE1))
	{
	  mu_error (_("Unoptimized parse tree"));
	  tree_dump (mu_strerr, sieve_tree, 0, mu_sieve_machine);
	}
      tree_optimize (sieve_tree);
      if (mu_debug_level_p (mu_sieve_debug_handle, MU_DEBUG_TRACE2))
	{
	  mu_error (_("Optimized parse tree"));
	  tree_dump (mu_strerr, sieve_tree, 0, mu_sieve_machine);
	}
      mu_i_sv_code (mu_sieve_machine, (sieve_op_t) (sieve_instr_t) 0);

      /* Clear location, so that mu_i_sv_locus will do its job. */
      /* FIXME: is it still needed? */
      mu_locus_range_deinit (&mu_sieve_machine->locus);
      
      tree_code (mu_sieve_machine, sieve_tree);
      mu_i_sv_code (mu_sieve_machine, (sieve_op_t) (sieve_instr_t) 0);
    }
  
  if (rc == 0)
    {
      if (mu_sieve_machine->state == mu_sieve_state_error)
	rc = MU_ERR_PARSE;
      else
	{
	  string_rescan (mu_sieve_machine);
	  mu_sieve_machine->state = mu_sieve_state_compiled;
	}
    }

  tree_free (&sieve_tree);
  return rc;
}

static int
sieve_compile_file (void *name)
{
  if (mu_i_sv_lex_begin (name) == 0)
    return sieve_parse ();
  return MU_ERR_FAILURE;
}

int
mu_sieve_compile (mu_sieve_machine_t mach, const char *name)
{
  return with_machine (mach, sieve_compile_file, (void *) name);
}

struct strbuf
{
  const char *ptr;
  size_t size;
  struct mu_locus_point const *pt;
};

static int
sieve_compile_strbuf (void *name)
{
  struct strbuf *buf = name;
  if (mu_i_sv_lex_begin_string (buf->ptr, buf->size, buf->pt) == 0)
    return sieve_parse ();
  return MU_ERR_FAILURE;
} 

int
mu_sieve_compile_text (mu_sieve_machine_t mach,
		       const char *str, size_t strsize,
		       struct mu_locus_point const *loc)
{
  struct strbuf buf;
  buf.ptr = str;
  buf.size = strsize;
  buf.pt = loc;
  return with_machine (mach, sieve_compile_strbuf, &buf);
}


