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




/* Copy the first part of user declarations.  */
#line 17 "msgset.y" /* yacc.c:339  */

#include "mail.h"

#include <stdio.h>
#include <stdlib.h>

/* Defined in <limits.h> on some systems, but redefined in <regex.h>
   if we are using GNU's regex. So, undef it to avoid duplicate definition
   warnings. */

#ifdef RE_DUP_MAX
# undef RE_DUP_MAX
#endif
#include <regex.h>

struct header_data
{
  char *header;
  char *expr;
};

static msgset_t *msgset_select (int (*sel) (mu_message_t, void *),
				     void *closure, int rev,
				     unsigned int max_matches);
static int select_header (mu_message_t msg, void *closure);
static int select_body (mu_message_t msg, void *closure);
static int select_sender (mu_message_t msg, void *closure);
static int select_deleted (mu_message_t msg, void *closure);
static int check_set (msgset_t **pset);

int yyerror (const char *);
int yylex  (void);

static int msgset_flags = MSG_NODELETED;
static size_t message_count;
static msgset_t *result;
static mu_opool_t tokpool;

typedef int (*message_selector_t) (mu_message_t, void *);
static message_selector_t find_type_selector (int type);

#line 108 "msgset.c" /* yacc.c:339  */

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
# define YYERROR_VERBOSE 0
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TYPE = 258,
    IDENT = 259,
    REGEXP = 260,
    HEADER = 261,
    BODY = 262,
    NUMBER = 263
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 59 "msgset.y" /* yacc.c:355  */

  char *string;
  int number;
  int type;
  msgset_t *mset;

#line 161 "msgset.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);



/* Copy the second part of user declarations.  */

#line 178 "msgset.c" /* yacc.c:358  */

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
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
#define YYLAST   102

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  23
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  11
/* YYNRULES -- Number of rules.  */
#define YYNRULES  36
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  52

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   263

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    18,     2,     2,    11,     2,     2,     2,
      21,    22,    12,    14,    15,    13,     9,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    19,     2,    20,    10,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    16,     2,    17,     2,     2,     2,     2,
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
       5,     6,     7,     8
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    75,    75,    78,    82,    86,    90,    94,    98,   102,
     108,   109,   113,   119,   125,   129,   135,   136,   142,   148,
     151,   166,   175,   190,   202,   205,   211,   212,   216,   222,
     223,   237,   243,   246,   252,   260,   264
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TYPE", "IDENT", "REGEXP", "HEADER",
  "BODY", "NUMBER", "'.'", "'^'", "'$'", "'*'", "'-'", "'+'", "','", "'{'",
  "'}'", "'!'", "'['", "']'", "'('", "')'", "$accept", "input", "msgset",
  "msgexpr", "msgspec", "msg", "header", "rangeset", "range", "number",
  "partno", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    46,
      94,    36,    42,    45,    43,    44,   123,   125,    33,    91,
      93,    40,    41
};
# endif

#define YYPACT_NINF -15

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-15)))

#define YYTABLE_NINF -25

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      32,   -15,   -15,   -15,   -15,   -12,   -15,   -15,   -15,   -15,
     -15,   -15,    54,    54,     3,     9,    -1,   -15,   -15,    40,
       5,   -15,   -15,    83,    70,    48,   -15,     8,   -15,   -15,
      54,   -15,     3,     3,   -15,     3,     3,   -15,   -15,   -15,
     -15,     3,   -15,   -15,   -15,    69,    66,    69,    68,   -15,
     -15,   -15
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    22,    23,    25,    21,    35,     3,     5,     6,     7,
       8,     9,    24,    24,     0,     0,     4,    10,    13,    16,
       0,    19,    29,    32,     0,    24,    15,     0,    26,     1,
      24,    12,     0,     0,    20,     0,     0,    35,    31,    30,
      14,     0,    36,    28,    11,    17,     0,    33,     0,    27,
      18,    34
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -15,   -15,    -4,    55,   -15,   -15,   -15,    61,   -14,     1,
     -15
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    15,    16,    17,    18,    19,    20,    27,    21,    22,
      23
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      28,    24,     1,     2,   -24,     3,     4,     5,    25,    29,
      34,     5,     0,    43,    30,    12,     5,    13,    28,    28,
      14,    28,    28,    41,    14,    39,     0,    49,     0,    14,
      42,    43,    43,    43,    43,     1,     2,   -24,     3,     4,
       5,     6,     7,     8,     9,    10,    11,     0,    12,    32,
      13,     1,     2,    14,     3,     4,     5,     1,     2,    33,
       3,     4,     5,    30,    12,    40,    13,     0,    26,    14,
      12,    31,    13,     0,     5,    14,     5,     5,    37,     0,
      31,    41,    38,    41,    41,    44,    50,    14,    51,    14,
      14,    14,    35,    45,    46,     0,    47,    48,     0,     0,
       0,     0,    36
};

static const yytype_int8 yycheck[] =
{
      14,    13,     3,     4,     5,     6,     7,     8,    12,     0,
       5,     8,    -1,    27,    15,    16,     8,    18,    32,    33,
      21,    35,    36,    15,    21,    24,    -1,    41,    -1,    21,
      22,    45,    46,    47,    48,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    -1,    16,     9,
      18,     3,     4,    21,     6,     7,     8,     3,     4,    19,
       6,     7,     8,    15,    16,    17,    18,    -1,    13,    21,
      16,    16,    18,    -1,     8,    21,     8,     8,     8,    -1,
      25,    15,    12,    15,    15,    30,    20,    21,    20,    21,
      21,    21,     9,    32,    33,    -1,    35,    36,    -1,    -1,
      -1,    -1,    19
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    16,    18,    21,    24,    25,    26,    27,    28,
      29,    31,    32,    33,    13,    25,    26,    30,    31,     0,
      15,    26,     9,    19,     5,     9,    19,     8,    12,    32,
      17,    15,    22,    31,    26,    30,    30,    30,    30,    31,
      20,    20
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    23,    24,    24,    24,    24,    24,    24,    24,    24,
      25,    25,    25,    26,    26,    26,    27,    27,    27,    27,
      28,    28,    28,    28,    29,    29,    30,    30,    30,    31,
      31,    31,    32,    32,    32,    33,    33
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     1,     3,     2,     1,     3,     4,     1,
       2,     1,     1,     1,     0,     1,     1,     3,     2,     1,
       3,     3,     1,     3,     4,     1,     3
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



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
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
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
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

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

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
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 75 "msgset.y" /* yacc.c:1646  */
    {
	     result = msgset_make_1 (get_cursor ());
	   }
#line 1297 "msgset.c" /* yacc.c:1646  */
    break;

  case 3:
#line 79 "msgset.y" /* yacc.c:1646  */
    {
	     result = msgset_make_1 (get_cursor ());
	   }
#line 1305 "msgset.c" /* yacc.c:1646  */
    break;

  case 4:
#line 83 "msgset.y" /* yacc.c:1646  */
    {
	     result = (yyvsp[0].mset);
	   }
#line 1313 "msgset.c" /* yacc.c:1646  */
    break;

  case 5:
#line 87 "msgset.y" /* yacc.c:1646  */
    {
	     result = msgset_select (select_deleted, NULL, 0, 1);
	   }
#line 1321 "msgset.c" /* yacc.c:1646  */
    break;

  case 6:
#line 91 "msgset.y" /* yacc.c:1646  */
    {
	     result = msgset_select (select_deleted, NULL, 1, 1);
	   }
#line 1329 "msgset.c" /* yacc.c:1646  */
    break;

  case 7:
#line 95 "msgset.y" /* yacc.c:1646  */
    {
	     result = msgset_select (select_deleted, NULL, 0, total);
	   }
#line 1337 "msgset.c" /* yacc.c:1646  */
    break;

  case 8:
#line 99 "msgset.y" /* yacc.c:1646  */
    {
	     result = msgset_select (select_deleted, NULL, 1, 1);
	   }
#line 1345 "msgset.c" /* yacc.c:1646  */
    break;

  case 9:
#line 103 "msgset.y" /* yacc.c:1646  */
    {
	     result = msgset_select (select_deleted, NULL, 0, 1);
	   }
#line 1353 "msgset.c" /* yacc.c:1646  */
    break;

  case 11:
#line 110 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_append ((yyvsp[-2].mset), (yyvsp[0].mset));
	   }
#line 1361 "msgset.c" /* yacc.c:1646  */
    break;

  case 12:
#line 114 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_append ((yyvsp[-1].mset), (yyvsp[0].mset));
	   }
#line 1369 "msgset.c" /* yacc.c:1646  */
    break;

  case 13:
#line 120 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = (yyvsp[0].mset);
	     if (check_set (&(yyval.mset)))
	       YYABORT;
	   }
#line 1379 "msgset.c" /* yacc.c:1646  */
    break;

  case 14:
#line 126 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = (yyvsp[-1].mset);
	   }
#line 1387 "msgset.c" /* yacc.c:1646  */
    break;

  case 15:
#line 130 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_negate ((yyvsp[0].mset));
	   }
#line 1395 "msgset.c" /* yacc.c:1646  */
    break;

  case 17:
#line 137 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_expand ((yyvsp[-2].mset), (yyvsp[0].mset));
	     msgset_free ((yyvsp[-2].mset));
	     msgset_free ((yyvsp[0].mset));
	   }
#line 1405 "msgset.c" /* yacc.c:1646  */
    break;

  case 18:
#line 143 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_expand ((yyvsp[-3].mset), (yyvsp[-1].mset));
	     msgset_free ((yyvsp[-3].mset));
	     msgset_free ((yyvsp[-1].mset));
	   }
#line 1415 "msgset.c" /* yacc.c:1646  */
    break;

  case 20:
#line 152 "msgset.y" /* yacc.c:1646  */
    {
	     struct header_data hd;
	     hd.header = (yyvsp[-1].string);
	     hd.expr   = (yyvsp[0].string);
	     (yyval.mset) = msgset_select (select_header, &hd, 0, 0);
	     if (!(yyval.mset))
	       {
		 if ((yyvsp[-1].string))
		   mu_error (_("No applicable messages from {%s:/%s}"), (yyvsp[-1].string), (yyvsp[0].string));
		 else
		   mu_error (_("No applicable messages from {/%s}"), (yyvsp[0].string));
		 YYERROR;
	       }
	   }
#line 1434 "msgset.c" /* yacc.c:1646  */
    break;

  case 21:
#line 167 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_select (select_body, (yyvsp[0].string), 0, 0);
	     if (!(yyval.mset))
	       {
		 mu_error (_("No applicable messages from {:/%s}"), (yyvsp[0].string));
		 YYERROR;
	       }
	   }
#line 1447 "msgset.c" /* yacc.c:1646  */
    break;

  case 22:
#line 176 "msgset.y" /* yacc.c:1646  */
    {
	     message_selector_t sel = find_type_selector ((yyvsp[0].type));
	     if (!sel)
	       {
		 yyerror (_("unknown message type"));
		 YYERROR;
	       }
	     (yyval.mset) = msgset_select (sel, NULL, 0, 0);
	     if (!(yyval.mset))
	       {
		 mu_error (_("No messages satisfy :%c"), (yyvsp[0].type));
		 YYERROR;
	       }
	   }
#line 1466 "msgset.c" /* yacc.c:1646  */
    break;

  case 23:
#line 191 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_select (select_sender, (void *)(yyvsp[0].string), 0, 0);
	     if (!(yyval.mset))
	       {
		 mu_error (_("No applicable messages from {%s}"), (yyvsp[0].string));
		 YYERROR;
	       }
	   }
#line 1479 "msgset.c" /* yacc.c:1646  */
    break;

  case 24:
#line 202 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.string) = NULL;
	   }
#line 1487 "msgset.c" /* yacc.c:1646  */
    break;

  case 25:
#line 206 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.string) = (yyvsp[0].string);
	   }
#line 1495 "msgset.c" /* yacc.c:1646  */
    break;

  case 27:
#line 213 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_append ((yyvsp[-2].mset), (yyvsp[0].mset));
	   }
#line 1503 "msgset.c" /* yacc.c:1646  */
    break;

  case 28:
#line 217 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_append ((yyvsp[-1].mset), (yyvsp[0].mset));
	   }
#line 1511 "msgset.c" /* yacc.c:1646  */
    break;

  case 30:
#line 224 "msgset.y" /* yacc.c:1646  */
    {
	     if (msgset_length ((yyvsp[0].mset)) == 1)
	       {
		 (yyval.mset) = msgset_range ((yyvsp[-2].number), msgset_msgno ((yyvsp[0].mset)));
	       }
	     else
	       {
		 (yyval.mset) = msgset_range ((yyvsp[-2].number), msgset_msgno ((yyvsp[0].mset)) - 1);
		 if (!(yyval.mset))
		   YYERROR;
		 msgset_append ((yyval.mset), (yyvsp[0].mset));
	       }
	   }
#line 1529 "msgset.c" /* yacc.c:1646  */
    break;

  case 31:
#line 238 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_range ((yyvsp[-2].number), total);
	   }
#line 1537 "msgset.c" /* yacc.c:1646  */
    break;

  case 32:
#line 244 "msgset.y" /* yacc.c:1646  */
    {
	   }
#line 1544 "msgset.c" /* yacc.c:1646  */
    break;

  case 33:
#line 247 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_expand ((yyvsp[-2].mset), (yyvsp[0].mset));
	     msgset_free ((yyvsp[-2].mset));
	     msgset_free ((yyvsp[0].mset));
	   }
#line 1554 "msgset.c" /* yacc.c:1646  */
    break;

  case 34:
#line 253 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_expand ((yyvsp[-3].mset), (yyvsp[-1].mset));
	     msgset_free ((yyvsp[-3].mset));
	     msgset_free ((yyvsp[-1].mset));
	   }
#line 1564 "msgset.c" /* yacc.c:1646  */
    break;

  case 35:
#line 261 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = msgset_make_1 ((yyvsp[0].number));
	   }
#line 1572 "msgset.c" /* yacc.c:1646  */
    break;

  case 36:
#line 265 "msgset.y" /* yacc.c:1646  */
    {
	     (yyval.mset) = (yyvsp[-1].mset);
	   }
#line 1580 "msgset.c" /* yacc.c:1646  */
    break;


#line 1584 "msgset.c" /* yacc.c:1646  */
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
                      yytoken, &yylval);
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


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
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
#line 269 "msgset.y" /* yacc.c:1906  */


static int xargc;
static char **xargv;
static int cur_ind;
static char *cur_p;

int
yyerror (const char *s)
{
  mu_stream_printf (mu_strerr, "%s: ", xargv[0]);
  mu_stream_printf (mu_strerr, "%s", s);
  if (!cur_p)
    mu_stream_printf (mu_strerr, _(" near end"));
  else if (*cur_p == 0)
    {
      int i =  (*cur_p == 0) ? cur_ind + 1 : cur_ind;
      if (i == xargc)
	mu_stream_printf (mu_strerr, _(" near end"));
      else
	mu_stream_printf (mu_strerr, _(" near %s"), xargv[i]);
    }
  else
    mu_stream_printf (mu_strerr, _(" near %s"), cur_p);
  mu_stream_printf (mu_strerr, "\n");
  return 0;
}

int
yylex (void)
{
  if (cur_ind == xargc)
    return 0;
  if (!cur_p)
    cur_p = xargv[cur_ind];
  if (*cur_p == 0)
    {
      cur_ind++;
      cur_p = NULL;
      return yylex ();
    }

  if (mu_isdigit (*cur_p))
    {
      yylval.number = strtoul (cur_p, &cur_p, 10);
      return NUMBER;
    }

  if (mu_isalpha (*cur_p))
    {
      char *p = cur_p;

      while (*cur_p && *cur_p != ',' && *cur_p != ':') 
	cur_p++;
      mu_opool_append (tokpool, p, cur_p - p);
      mu_opool_append_char (tokpool, 0);
      yylval.string = mu_opool_finish (tokpool, NULL);
      if (*cur_p == ':')
	{
	  ++cur_p;
	  return HEADER;
	}
      return IDENT;
    }

  if (*cur_p == '/')
    {
      char *p = ++cur_p;

      while (*cur_p && *cur_p != '/')
	cur_p++;

      mu_opool_append (tokpool, p, cur_p - p);
      mu_opool_append_char (tokpool, 0);
      yylval.string = mu_opool_finish (tokpool, NULL);

      if (*cur_p)
	cur_p++;

      return REGEXP;
    }

  if (*cur_p == ':')
    {
      cur_p++;
      if (*cur_p == '/')
	{
	  char *p = ++cur_p;
	  
	  while (*cur_p && *cur_p != '/')
	    cur_p++;

	  mu_opool_append (tokpool, p, cur_p - p);
	  mu_opool_append_char (tokpool, 0);
	  yylval.string = mu_opool_finish (tokpool, NULL);

	  if (*cur_p)
	    cur_p++;

	  return BODY;
	}
      if (*cur_p == 0)
	return 0;
      yylval.type = *cur_p++;
      return TYPE;
    }

  return *cur_p++;
}

int
msgset_parse (const int argc, char **argv, int flags, msgset_t **mset)
{
  int rc;
  xargc = argc;
  xargv = argv;
  msgset_flags = flags;
  cur_ind = 1;
  cur_p = NULL;
  result = NULL;

  mu_opool_create (&tokpool, MU_OPOOL_ENOMEMABRT);
  mu_mailbox_messages_count (mbox, &message_count);
  rc = yyparse ();
  if (rc == 0)
    {
      if (!result)
	{
	  util_noapp ();
	  rc = 1;
	}
      else
	{
	  if (result->crd[1] > message_count)
	    {
	      util_error_range (result->crd[1]);
	      msgset_free (result);
	      return 1;
	    }
	  *mset = result;
	}
    }
  mu_opool_destroy (&tokpool);
  return rc;
}

void
msgset_free (msgset_t *msg_set)
{
  msgset_t *next;

  while (msg_set)
    {
      next = msg_set->next;
      free (msg_set->crd);
      free (msg_set);
      msg_set = next;
    }
}

size_t
msgset_count (msgset_t *set)
{
  size_t count = 0;
  for (; set; set = set->next)
    count++;
  return count;
}

/* Create a message set consisting of a single msg_num and no subparts */
msgset_t *
msgset_make_1 (size_t number)
{
  msgset_t *mp;

  if (number == 0)
    return NULL;
  mp = mu_alloc (sizeof (*mp));
  mp->next = NULL;
  if (mu_coord_alloc (&mp->crd, 1))
    mu_alloc_die ();
  mp->crd[1] = number;
  return mp;
}

msgset_t *
msgset_dup (const msgset_t *set)
{
  msgset_t *mp;
  mp = mu_alloc (sizeof (*mp));
  mp->next = NULL;
  if (mu_coord_dup (set->crd, &mp->crd))
    mu_alloc_die ();
  return mp;
}

/* Append message set TWO to the end of message set ONE. Take care to
   eliminate duplicates. Preserve the ordering of both lists. Return
   the resulting set.

   The function is destructive: the set TWO is attached to ONE and
   eventually modified to avoid duplicates.
*/
msgset_t *
msgset_append (msgset_t *one, msgset_t *two)
{
  msgset_t *last;

  if (!one)
    return two;
  for (last = one; last->next; last = last->next)
    {
      msgset_remove (&two, msgset_msgno (last));
    }
  last->next = two;
  return one;
}

int
msgset_member (msgset_t *set, size_t n)
{
  for (; set; set = set->next)
    if (msgset_msgno (set) == n)
      return 1;
  return 0;
}

void
msgset_remove (msgset_t **pset, size_t n)
{
  msgset_t *cur = *pset, **pnext = pset;

  while (1)
    {
      if (cur == NULL)
	return;
      if (msgset_msgno (cur) == n)
	break;
      pnext = &cur->next;
      cur = cur->next;
    }
  *pnext = cur->next;
  cur->next = NULL;
  msgset_free (cur);
}

msgset_t *
msgset_negate (msgset_t *set)
{
  size_t i;
  msgset_t *first = NULL, *last = NULL;

  for (i = 1; i <= total; i++)
    {
      if (!msgset_member (set, i))
	{
	  msgset_t *mp = msgset_make_1 (i);
	  if (!first)
	    first = mp;
	  else
	    last->next = mp;
	  last = mp;
	}
    }
  return first;
}

msgset_t *
msgset_range (int low, int high)
{
  int i;
  msgset_t *mp, *first = NULL, *last = NULL;

  if (low == high)
    return msgset_make_1 (low);

  if (low >= high)
    {
      yyerror (_("range error"));
      return NULL;
    }

  for (i = 0; low <= high; i++, low++)
    {
      mp = msgset_make_1 (low);
      if (!first)
	first = mp;
      else
	last->next = mp;
      last = mp;
    }
  return first;
}

msgset_t *
msgset_expand (msgset_t *set, msgset_t *expand_by)
{
  msgset_t *i, *j;
  msgset_t *first = NULL, *last = NULL, *mp;

  for (i = set; i; i = i->next)
    for (j = expand_by; j; j = j->next)
      {
	mp = mu_alloc (sizeof *mp);
	mp->next = NULL;
	if (mu_coord_alloc (&mp->crd,
			    mu_coord_length (i->crd) + mu_coord_length (j->crd)))
	  mu_alloc_die ();
	memcpy (&mp->crd[1], &i->crd[1],
		mu_coord_length (i->crd) * sizeof i->crd[0]);
	memcpy (&mp->crd[1] + mu_coord_length (i->crd), &j->crd[1],
		mu_coord_length (j->crd) * sizeof j->crd[0]);

	if (!first)
	  first = mp;
	else
	  last->next = mp;
	last = mp;
      }
  return first;
}

msgset_t *
msgset_select (message_selector_t sel, void *closure, int rev,
	       unsigned int max_matches)
{
  size_t i, match_count = 0;
  msgset_t *first = NULL, *last = NULL, *mp;
  mu_message_t msg = NULL;

  if (max_matches == 0)
    max_matches = total;

  if (rev)
    {
      for (i = total; i > 0; i--)
	{
	  mu_mailbox_get_message (mbox, i, &msg);
	  if ((*sel) (msg, closure))
	    {
	      mp = msgset_make_1 (i);
	      if (!first)
		first = mp;
	      else
		last->next = mp;
	      last = mp;
	      if (++match_count == max_matches)
		break;
	    }
	}
    }
  else
    {
      for (i = 1; i <= total; i++)
	{
	  mu_mailbox_get_message (mbox, i, &msg);
	  if ((*sel) (msg, closure))
	    {
	      mp = msgset_make_1 (i);
	      if (!first)
		first = mp;
	      else
		last->next = mp;
	      last = mp;
	      if (++match_count == max_matches)
		break;
	    }
	}
    }
  return first;
}

int
select_header (mu_message_t msg, void *closure)
{
  struct header_data *hd = (struct header_data *)closure;
  mu_header_t hdr;
  char *contents;
  const char *header = hd->header ? hd->header : MU_HEADER_SUBJECT;

  mu_message_get_header (msg, &hdr);
  if (mu_header_aget_value (hdr, header, &contents) == 0)
    {
      if (mailvar_get (NULL, "regex", mailvar_type_boolean, 0) == 0)
	{
	  /* Match string against the extended regular expression(ignoring
	     case) in pattern, treating errors as no match.
	     Return 1 for match, 0 for no match.
	  */
          regex_t re;
          int status;
	  int flags = REG_EXTENDED;

	  if (mu_islower (header[0]))
	    flags |= REG_ICASE;
          if (regcomp (&re, hd->expr, flags) != 0)
	    {
	      free (contents);
	      return 0;
	    }
          status = regexec (&re, contents, 0, NULL, 0);
          free (contents);
	  regfree (&re);
          return status == 0;
	}
      else
	{
	  int rc;
	  mu_strupper (contents);
	  rc = strstr (contents, hd->expr) != NULL;
	  free (contents);
	  return rc;
	}
    }
  return 0;
}

int
select_body (mu_message_t msg, void *closure)
{
  char *expr = closure;
  int noregex = mailvar_get (NULL, "regex", mailvar_type_boolean, 0);
  regex_t re;
  int status = 0;
  mu_body_t body = NULL;
  mu_stream_t stream = NULL;
  int rc;
  
  if (noregex)
    mu_strupper (expr);
  else if (regcomp (&re, expr, REG_EXTENDED | REG_ICASE) != 0)
    return 0;

  mu_message_get_body (msg, &body);
  rc = mu_body_get_streamref (body, &stream);
  if (rc == 0)
    {
      char *buffer = NULL;
      size_t size = 0;
      size_t n = 0;
      
      while (status == 0
	     && (rc = mu_stream_getline (stream, &buffer, &size, &n)) == 0
	     && n > 0)
	{
	  if (noregex)
	    {
	      /* FIXME: charset */
	      mu_strupper (buffer);
	      status = strstr (buffer, expr) != NULL;
	    }
	  else
	    status = regexec (&re, buffer, 0, NULL, 0) == 0;
	}
      mu_stream_destroy (&stream);
      free (buffer);
      if (rc)
	mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_getline", NULL, rc);
    }
  else
    mu_diag_funcall (MU_DIAG_ERROR, "mu_body_get_streamref", NULL, rc);
  
  if (!noregex)
    regfree (&re);

  return status;
}

int
select_sender (mu_message_t msg, void *closure)
{
  char *needle = (char*) closure;
  char *sender = sender_string (msg);
  int status = strcmp (sender, needle) == 0;
  free (sender);
  return status;
}

static int
select_type_d (mu_message_t msg, void *unused MU_ARG_UNUSED)
{
  mu_attribute_t attr;

  if (mu_message_get_attribute (msg, &attr) == 0)
    return mu_attribute_is_deleted (attr);
  return 0;
}

static int
select_type_n (mu_message_t msg, void *unused MU_ARG_UNUSED)
{
  mu_attribute_t attr;

  if (mu_message_get_attribute (msg, &attr) == 0)
    return mu_attribute_is_recent (attr);
  return 0;
}

static int
select_type_o (mu_message_t msg, void *unused MU_ARG_UNUSED)
{
  mu_attribute_t attr;

  if (mu_message_get_attribute (msg, &attr) == 0)
    return mu_attribute_is_seen (attr);
  return 0;
}

static int
select_type_r (mu_message_t msg, void *unused MU_ARG_UNUSED)
{
  mu_attribute_t attr;

  if (mu_message_get_attribute (msg, &attr) == 0)
    return mu_attribute_is_read (attr);
  return 0;
}

static int
select_type_s (mu_message_t msg, void *unused MU_ARG_UNUSED)
{
  mu_attribute_t attr;

  if (mu_message_get_attribute (msg, &attr) == 0)
    return mu_attribute_is_userflag (attr, MAIL_ATTRIBUTE_SAVED);
  return 0;
}

static int
select_type_t (mu_message_t msg, void *unused MU_ARG_UNUSED)
{
  mu_attribute_t attr;

  if (mu_message_get_attribute (msg, &attr) == 0)
    return mu_attribute_is_userflag (attr, MAIL_ATTRIBUTE_TAGGED);
  return 0;
}

static int
select_type_T (mu_message_t msg, void *unused MU_ARG_UNUSED)
{
  mu_attribute_t attr;

  if (mu_message_get_attribute (msg, &attr) == 0)
    return !mu_attribute_is_userflag (attr, MAIL_ATTRIBUTE_TAGGED);
  return 0;
}

static int
select_type_u (mu_message_t msg, void *unused MU_ARG_UNUSED)
{
  mu_attribute_t attr;

  if (mu_message_get_attribute (msg, &attr) == 0)
    return !mu_attribute_is_read (attr);
  return 0;
}

struct type_selector
{
  int letter;
  message_selector_t func;
};

static struct type_selector type_selector[] = {
  { 'd', select_type_d },
  { 'n', select_type_n },
  { 'o', select_type_o },
  { 'r', select_type_r },
  { 's', select_type_s },
  { 't', select_type_t },
  { 'T', select_type_T },
  { 'u', select_type_u },
  { '/', NULL }, /* A pseudo-entry needed for msgtype_generator only */
  { 0 }
};

static message_selector_t
find_type_selector (int type)
{
  struct type_selector *p;
  for (p = type_selector; p->func; p++)
    {
      if (p->letter == type)
	return p->func;
    }
  return NULL;
}

#ifdef WITH_READLINE
char *
msgtype_generator (const char *text, int state)
{
  /* Allowed message types, plus '/'. The latter can folow a colon,
     meaning body lookup */
  static int i;
  char c;

  if (!state)
    {
      i = 0;
    }
  while ((c = type_selector[i].letter))
    {
      i++;
      if (!text[1] || text[1] == c)
	{
	  char *s = mu_alloc (3);
	  s[0] = ':';
	  s[1] = c;
	  s[2] = 0;
	  return s;
	}
    }
  return NULL;
}
#endif

int
select_deleted (mu_message_t msg, void *closure MU_ARG_UNUSED)
{
  mu_attribute_t attr= NULL;
  int rc;

  mu_message_get_attribute (msg, &attr);
  rc = mu_attribute_is_deleted (attr);
  return strcmp (xargv[0], "undelete") == 0 ? rc : !rc;
}

int
check_set (msgset_t **pset)
{
  int flags = msgset_flags;
  int rc = 0;

  if (!*pset)
    {
      util_noapp ();
      return 1;
    }
  if (msgset_count (*pset) == 1)
    flags ^= MSG_SILENT;
  if (flags & MSG_NODELETED)
    {
      msgset_t *p = *pset, *prev = NULL;
      msgset_t *delset = NULL;

      while (p)
	{
	  msgset_t *next = p->next;
	  if (util_isdeleted (msgset_msgno (p)))
	    {
	      if ((flags & MSG_SILENT) && (prev || next))
		{
		  /* Mark subset as deleted */
		  p->next = delset;
		  delset = p;
		  /* Remove it from the set */
		  if (prev)
		    prev->next = next;
		  else
		    *pset = next;
		}
	      else
		{
		  mu_error (_("%lu: Inappropriate message (has been deleted)"),
			    (unsigned long) msgset_msgno (p));
		  /* Delete entire set */
		  delset = *pset;
		  *pset = NULL;
		  rc = 1;
		  break;
		}
	    }
	  else
	    prev = p;
	  p = next;
	}

      if (delset)
	msgset_free (delset);

      if (!*pset)
	rc = 1;
    }

  return rc;
}


#if 0
void
msgset_print (msgset_t *mset)
{
  int i;
  mu_printf ("(");
  mu_printf ("%d .", mset->msg_part[0]);
  for (i = 1; i < mset->npart; i++)
    {
      mu_printf (" %d", mset->msg_part[i]);
    }
  mu_printf (")\n");
}

int
main(int argc, char **argv)
{
  msgset_t *mset = NULL;
  int rc = msgset_parse (argc, argv, &mset);

  for (; mset; mset = mset->next)
    msgset_print (mset);
  return 0;
}
#endif
