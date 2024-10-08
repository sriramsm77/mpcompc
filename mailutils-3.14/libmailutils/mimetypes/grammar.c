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
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         MIMETYPES_YYSTYPE
#define YYLTYPE         MIMETYPES_YYLTYPE
/* Substitute the variable and function names.  */
#define yyparse         mimetypes_yyparse
#define yylex           mimetypes_yylex
#define yyerror         mimetypes_yyerror
#define yydebug         mimetypes_yydebug
#define yynerrs         mimetypes_yynerrs


/* Copy the first part of user declarations.  */
#line 1 "grammar.y" /* yacc.c:339  */

/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2005-2022 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <mailutils/mimetypes.h>
#include <grammar.h>

static void
yyprint (FILE *output, unsigned short toknum, YYSTYPE val)
{
  switch (toknum)
    {
    case TYPE:
    case IDENT:
    case STRING:
      fprintf (output, "[%lu] %s", (unsigned long) val.string.len,
	       val.string.ptr);
      break;

    case EOL:
      fprintf (output, "\\n");
      break;
      
    default:
      if (mu_isprint (toknum))
	fprintf (output, "'%c'", toknum);
      else
	fprintf (output, "tok(%d)", toknum);
      break;
    }
}

#define YYPRINT yyprint
  
static struct node *make_node (mu_mimetypes_t mth,
                               enum node_type type,
			       struct mu_locus_range const *loc); 
static struct node *make_binary_node (mu_mimetypes_t mth,
                                      int op,
				      struct node *left, struct node *rigth,
				      struct mu_locus_range const *loc);
static struct node *make_negation_node (mu_mimetypes_t mth,
                                        struct node *p,
					struct mu_locus_range const *loc);

static struct node *make_suffix_node (mu_mimetypes_t mth,
                                      struct mimetypes_string *suffix,
				      struct mu_locus_range const *loc);
static struct node *make_functional_node (mu_mimetypes_t mth,
                                          char *ident, mu_list_t list,
					  struct mu_locus_range const *loc);

static void *
mimetypes_malloc (mu_mimetypes_t mth, size_t size)
{
  mu_opool_alloc (mth->pool, size);
  return mu_opool_finish (mth->pool, NULL);
}

static struct mimetypes_string *     
mimetypes_string_dup (mu_mimetypes_t mth, struct mimetypes_string *s)
{
  mu_opool_append (mth->pool, s, sizeof *s);
  return mu_opool_finish (mth->pool, NULL);
}

static void rule_destroy_item (void *ptr);
 

#line 161 "grammar.c" /* yacc.c:339  */

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

/* In a future release of Bison, this section will be replaced
   by #include "grammar.h".  */
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
#line 91 "grammar.y" /* yacc.c:355  */

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


#line 231 "grammar.c" /* yacc.c:355  */

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
#line 163 "grammar.y" /* yacc.c:355  */

  struct mimetypes_string string;
  char *s;
  mu_list_t list;
  int result;
  struct node *node;

#line 260 "grammar.c" /* yacc.c:355  */
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
#line 123 "grammar.y" /* yacc.c:355  */

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

#line 300 "grammar.c" /* yacc.c:355  */

#endif /* !YY_MIMETYPES_YY_GRAMMAR_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 306 "grammar.c" /* yacc.c:358  */

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
         || (defined MIMETYPES_YYLTYPE_IS_TRIVIAL && MIMETYPES_YYLTYPE_IS_TRIVIAL \
             && defined MIMETYPES_YYSTYPE_IS_TRIVIAL && MIMETYPES_YYSTYPE_IS_TRIVIAL)))

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
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   58

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  14
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  27
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  41

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
       2,     2,     2,    11,     2,     2,     2,     2,     2,     2,
      12,    13,     2,    10,     9,     2,     2,     2,     2,     2,
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

#if MIMETYPES_YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   175,   175,   178,   179,   182,   183,   198,   202,   212,
     215,   218,   219,   226,   233,   242,   246,   250,   254,   255,
     261,   280,   283,   286,   299,   304,   311,   312
};
#endif

#if MIMETYPES_YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TYPE", "IDENT", "STRING", "EOL",
  "BOGUS", "PRIORITY", "','", "'+'", "'!'", "'('", "')'", "$accept",
  "input", "list", "rule_line", "maybe_rule", "rule", "stmt", "priority",
  "maybe_priority", "function", "arglist", "arg", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    44,
      43,    33,    40,    41
};
# endif

#define YYPACT_NINF -8

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-8)))

#define YYTABLE_NINF -6

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      51,    -8,    38,    -8,    16,    -5,    -8,     5,    -8,    -8,
      38,    38,    10,    20,    -8,    -8,    -8,    51,    -3,    -8,
       1,     8,    -8,    -8,    38,    38,    29,    -8,    -8,    -8,
      -6,    -8,    -8,    -3,    29,    38,    -3,    -8,     6,    -8,
      -8
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     8,     9,     7,     0,     2,     3,     0,    17,    19,
       0,     0,    21,    10,    11,    18,     1,     0,     0,    15,
       0,     0,    22,     6,     0,     0,    12,     4,    26,    27,
       0,    24,    16,     0,    13,    14,     0,    23,     0,    25,
      20
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
      -8,    -8,    -8,     4,    -8,    -2,    18,    -8,    -8,    -8,
      -7,    -1
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,     5,     6,    12,    26,    14,    22,    23,    15,
      30,    31
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      13,    17,    28,    36,    29,     7,     8,    37,     9,    20,
      24,    25,    10,    11,    32,    36,    16,    18,    21,    40,
      33,    27,    34,    35,     7,     8,    38,     9,    19,    24,
      25,    10,    11,     7,     8,    39,     9,     0,     0,    25,
      10,    11,     7,     8,     0,     9,     0,     0,     0,    10,
      11,    -5,     1,     0,     2,     0,     0,    -5,     3
};

static const yytype_int8 yycheck[] =
{
       2,     6,     5,     9,     7,     4,     5,    13,     7,    11,
       9,    10,    11,    12,    13,     9,     0,    12,     8,    13,
      12,    17,    24,    25,     4,     5,    33,     7,    10,     9,
      10,    11,    12,     4,     5,    36,     7,    -1,    -1,    10,
      11,    12,     4,     5,    -1,     7,    -1,    -1,    -1,    11,
      12,     0,     1,    -1,     3,    -1,    -1,     6,     7
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     7,    15,    16,    17,     4,     5,     7,
      11,    12,    18,    19,    20,    23,     0,     6,    12,    20,
      19,     8,    21,    22,     9,    10,    19,    17,     5,     7,
      24,    25,    13,    12,    19,    19,     9,    13,    24,    25,
      13
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    14,    15,    16,    16,    17,    17,    17,    17,    18,
      18,    19,    19,    19,    19,    20,    20,    20,    20,    20,
      21,    22,    22,    23,    24,    24,    25,    25
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     3,     0,     3,     1,     1,     0,
       1,     1,     2,     3,     3,     2,     3,     1,     1,     1,
       4,     0,     1,     4,     1,     3,     1,     1
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
      yyerror (&yylloc, pctl, yyscanner, YY_("syntax error: cannot back up")); \
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
#if MIMETYPES_YYDEBUG

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
# if defined MIMETYPES_YYLTYPE_IS_TRIVIAL && MIMETYPES_YYLTYPE_IS_TRIVIAL

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
                  Type, Value, Location, pctl, yyscanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct parser_control *pctl, void *yyscanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (pctl);
  YYUSE (yyscanner);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct parser_control *pctl, void *yyscanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, pctl, yyscanner);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, struct parser_control *pctl, void *yyscanner)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , pctl, yyscanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, pctl, yyscanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !MIMETYPES_YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !MIMETYPES_YYDEBUG */


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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, struct parser_control *pctl, void *yyscanner)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (pctl);
  YYUSE (yyscanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 24: /* arglist  */
#line 171 "grammar.y" /* yacc.c:1257  */
      { mu_list_destroy (&((*yyvaluep).list)); }
#line 1227 "grammar.c" /* yacc.c:1257  */
        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (struct parser_control *pctl, void *yyscanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined MIMETYPES_YYLTYPE_IS_TRIVIAL && MIMETYPES_YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

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

/* User initialization code.  */
#line 139 "grammar.y" /* yacc.c:1429  */
{
  /*
   * yylloc is a local variable of yyparse.  It is maintained in sync with
   * the trk member of struct parser_control, and need to be deallocated
   * before returning from yyparse.  Since Bison does not provide any
   * %final-action, the variable is deinited in the <<EOF>> scanner rule.
   */
  mu_locus_range_init (&yylloc);
}

#line 1346 "grammar.c" /* yacc.c:1429  */
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
      yychar = yylex (&yylval, &yylloc, yyscanner);
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
        case 6:
#line 184 "grammar.y" /* yacc.c:1646  */
    {
	     struct rule_tab *p = mimetypes_malloc (pctl->mth, sizeof (*p));
	     p->type = (yyvsp[-2].string).ptr;
	     p->node = (yyvsp[-1].node);
	     p->priority = (yyvsp[0].result);
	     mu_locus_range_init (&p->loc);
	     mu_locus_point_copy (&p->loc.beg, &(yylsp[-2]).beg);
	     mu_locus_point_copy (&p->loc.end, &(yylsp[0]).end);
#if 0
	     YY_LOCATION_PRINT (stderr, p->loc);
	     fprintf (stderr, ": rule %s\n", p->type);
#endif
	     mu_list_append (pctl->mth->rule_list, p);
	   }
#line 1548 "grammar.c" /* yacc.c:1646  */
    break;

  case 7:
#line 199 "grammar.y" /* yacc.c:1646  */
    {
	     YYERROR;
	   }
#line 1556 "grammar.c" /* yacc.c:1646  */
    break;

  case 8:
#line 203 "grammar.y" /* yacc.c:1646  */
    {
	     pctl->errors++;
	     lex_next_rule (&(yylsp[0]), yyscanner);
	     yyerrok;
	     yyclearin;
	   }
#line 1567 "grammar.c" /* yacc.c:1646  */
    break;

  case 9:
#line 212 "grammar.y" /* yacc.c:1646  */
    {
	     (yyval.node) = make_node (pctl->mth, true_node, &yylloc);
	   }
#line 1575 "grammar.c" /* yacc.c:1646  */
    break;

  case 12:
#line 220 "grammar.y" /* yacc.c:1646  */
    {
	     struct mu_locus_range lr;
	     lr.beg = (yylsp[-1]).beg;
	     lr.end = (yylsp[0]).end;
	     (yyval.node) = make_binary_node (pctl->mth, L_OR, (yyvsp[-1].node), (yyvsp[0].node), &lr);
	   }
#line 1586 "grammar.c" /* yacc.c:1646  */
    break;

  case 13:
#line 227 "grammar.y" /* yacc.c:1646  */
    {
	     struct mu_locus_range lr;
	     lr.beg = (yylsp[-2]).beg;
	     lr.end = (yylsp[0]).end;
	     (yyval.node) = make_binary_node (pctl->mth, L_OR, (yyvsp[-2].node), (yyvsp[0].node), &lr);
	   }
#line 1597 "grammar.c" /* yacc.c:1646  */
    break;

  case 14:
#line 234 "grammar.y" /* yacc.c:1646  */
    {
	     struct mu_locus_range lr;
	     lr.beg = (yylsp[-2]).beg;
	     lr.end = (yylsp[0]).end;
	     (yyval.node) = make_binary_node (pctl->mth, L_AND, (yyvsp[-2].node), (yyvsp[0].node), &lr);
	   }
#line 1608 "grammar.c" /* yacc.c:1646  */
    break;

  case 15:
#line 243 "grammar.y" /* yacc.c:1646  */
    {
	     (yyval.node) = make_negation_node (pctl->mth, (yyvsp[0].node), &(yylsp[0]));
	   }
#line 1616 "grammar.c" /* yacc.c:1646  */
    break;

  case 16:
#line 247 "grammar.y" /* yacc.c:1646  */
    {
	     (yyval.node) = (yyvsp[-1].node);
	   }
#line 1624 "grammar.c" /* yacc.c:1646  */
    break;

  case 17:
#line 251 "grammar.y" /* yacc.c:1646  */
    {
	     (yyval.node) = make_suffix_node (pctl->mth, &(yyvsp[0].string), &(yylsp[0]));
	   }
#line 1632 "grammar.c" /* yacc.c:1646  */
    break;

  case 19:
#line 256 "grammar.y" /* yacc.c:1646  */
    {
	     YYERROR;
	   }
#line 1640 "grammar.c" /* yacc.c:1646  */
    break;

  case 20:
#line 262 "grammar.y" /* yacc.c:1646  */
    {
	     size_t count = 0;
	     struct mimetypes_string *arg;
	     
	     mu_list_count ((yyvsp[-1].list), &count);
	     if (count != 1)
	       {
		 yyerror (&(yylsp[-1]), pctl, yyscanner,
			  "%s", _("priority takes single numberic argument"));
		 YYERROR;
	       }
	     mu_list_head ((yyvsp[-1].list), (void**) &arg);
	     (yyval.result) = atoi (arg->ptr);
	     mu_list_destroy (&(yyvsp[-1].list));
	   }
#line 1660 "grammar.c" /* yacc.c:1646  */
    break;

  case 21:
#line 280 "grammar.y" /* yacc.c:1646  */
    {
	     (yyval.result) = 100;
	   }
#line 1668 "grammar.c" /* yacc.c:1646  */
    break;

  case 23:
#line 287 "grammar.y" /* yacc.c:1646  */
    {
	     struct mu_locus_range lr;
	     lr.beg = (yylsp[-3]).beg;
	     lr.end = (yylsp[0]).end;
	     
	     (yyval.node) = make_functional_node (pctl->mth, (yyvsp[-3].string).ptr, (yyvsp[-1].list), &lr);
	     if (!(yyval.node))
	       YYERROR;
	     mu_list_destroy (&(yyvsp[-1].list));
	   }
#line 1683 "grammar.c" /* yacc.c:1646  */
    break;

  case 24:
#line 300 "grammar.y" /* yacc.c:1646  */
    {
	     mu_list_create (&(yyval.list));
	     mu_list_append ((yyval.list), mimetypes_string_dup (pctl->mth, &(yyvsp[0].string)));
	   }
#line 1692 "grammar.c" /* yacc.c:1646  */
    break;

  case 25:
#line 305 "grammar.y" /* yacc.c:1646  */
    {
	     mu_list_append ((yyvsp[-2].list), mimetypes_string_dup (pctl->mth, &(yyvsp[0].string)));
	     (yyval.list) = (yyvsp[-2].list);
	   }
#line 1701 "grammar.c" /* yacc.c:1646  */
    break;

  case 27:
#line 313 "grammar.y" /* yacc.c:1646  */
    {
	     YYERROR;
	   }
#line 1709 "grammar.c" /* yacc.c:1646  */
    break;


#line 1713 "grammar.c" /* yacc.c:1646  */
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
      yyerror (&yylloc, pctl, yyscanner, YY_("syntax error"));
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
        yyerror (&yylloc, pctl, yyscanner, yymsgp);
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
                      yytoken, &yylval, &yylloc, pctl, yyscanner);
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
                  yystos[yystate], yyvsp, yylsp, pctl, yyscanner);
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
  yyerror (&yylloc, pctl, yyscanner, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, pctl, yyscanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, pctl, yyscanner);
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
#line 318 "grammar.y" /* yacc.c:1906  */


void
yyerror (MIMETYPES_YYLTYPE const *loc, struct parser_control *pctl,
	 yyscan_t scanner, char const *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  mu_vdiag_at_locus_range (MU_DIAG_ERROR, loc, fmt, ap);
  va_end (ap);
}

static void
parser_control_init (struct parser_control *ctl,
		     char const *filename, struct mu_mimetypes *mth)
{
  memset (ctl, 0, sizeof *ctl);
  mu_linetrack_create (&ctl->trk, filename, 3);
  ctl->mth = mth;
}

static void
parser_control_destroy (struct parser_control *ctl)
{
  mu_linetrack_destroy (&ctl->trk);
  mu_locus_point_deinit (&ctl->string_beg);
}

static void
locus_on (void)
{
  int mode;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_GET_MODE, &mode);
  mode |= MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
}

static void
locus_off (void)
{
  int mode;

  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_GET_MODE, &mode);
  mode &= ~MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, NULL);
}

mu_mimetypes_t 
mu_mimetypes_open (const char *name)
{
  int rc;
  struct mu_mimetypes *mtp;
  struct parser_control ctl;
  void *scanner;

  mtp = calloc (1, sizeof *mtp);
  if (!mtp)
    return NULL;

  //FIXME: install destroy_item ?
  if (mu_list_create (&mtp->rule_list)
      || mu_opool_create (&mtp->pool, MU_OPOOL_DEFAULT))
    rc = 1;
  else
    {
      mu_list_set_destroy_item (mtp->rule_list, rule_destroy_item);
      parser_control_init (&ctl, name, mtp);  

      mimetypes_yylex_init_extra (&ctl, &scanner);
      if (mimetypes_scanner_open (scanner, name))
	rc = 1;
      else
	{
	  yydebug = mu_debug_level_p (MU_DEBCAT_MIMETYPES, MU_DEBUG_TRACE3);
	  locus_on ();
	  rc = yyparse (&ctl, scanner);
	  locus_off ();
	}
      mimetypes_yylex_destroy (scanner);
    }
  
  if (rc || ctl.errors)
    {
      mu_mimetypes_close (mtp);
      mtp = NULL;
    }
  parser_control_destroy (&ctl);

  return mtp;
}

void
mu_mimetypes_close (mu_mimetypes_t mt)
{
  if (mt)
    {
      mu_list_destroy (&mt->rule_list);
      mu_opool_destroy (&mt->pool);
      free (mt);
    }
}

static struct node *
make_node (mu_mimetypes_t mth,
	   enum node_type type, struct mu_locus_range const *loc)
{
  struct node *p = mimetypes_malloc (mth, sizeof *p);
  p->type = type;
  mu_locus_range_init (&p->loc);
  mu_locus_range_copy (&p->loc, loc);
  return p;
}

static struct node *
make_binary_node (mu_mimetypes_t mth,
		  int op, struct node *left, struct node *right,
		  struct mu_locus_range const *loc)
{
  struct node *node = make_node (mth, binary_node, loc);

  node->v.bin.op = op;
  node->v.bin.arg1 = left;
  node->v.bin.arg2 = right;
  return node;
}

static struct node *
make_negation_node (mu_mimetypes_t mth,
		    struct node *p, struct mu_locus_range const *loc)
{
  struct node *node = make_node (mth, negation_node, loc);
  node->v.arg = p;
  return node;
}

static struct node *
make_suffix_node (mu_mimetypes_t mth,
		  struct mimetypes_string *suffix,
		  struct mu_locus_range const *loc)
{
  struct node *node = make_node (mth, suffix_node, loc);
  node->v.suffix = *suffix;
  return node;
}

static struct node *
make_functional_node (mu_mimetypes_t mth,
		      char *ident, mu_list_t list,
		      struct mu_locus_range const *loc)
{
  size_t count, i;
  struct builtin_tab const *p;
  struct node *node;
  union argument *args;
  mu_iterator_t itr;
  int rc;
  
  p = mu_mimetypes_builtin (ident);
  if (!p)
    {
      yyerror (loc, NULL, NULL, _("unknown builtin: %s"), ident);
      return NULL;
    }

  mu_list_count (list, &count);
  i = strlen (p->args);

  if (count < i)
    {
      yyerror (loc, NULL, NULL, _("too few arguments in call to `%s'"), ident);
      return NULL;
    }
  else if (count > i)
    {
      yyerror (loc, NULL, NULL, _("too many arguments in call to `%s'"), ident);
      return NULL;
    }

  args = mimetypes_malloc (mth, count * sizeof *args);
  
  mu_list_get_iterator (list, &itr);
  for (i = 0, mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr), i++)
    {
      struct mimetypes_string *data;
      char *tmp;
      
      mu_iterator_current (itr, (void **)&data);
      switch (p->args[i])
	{
	case 'd':
	  args[i].number = strtoul (data->ptr, &tmp, 0);
	  if (*tmp)
	    goto err;
	  break;
	  
	case 's':
	  args[i].string = data;
	  break;

	case 'x':
	  {
	    char *s;
	    
	    rc = mu_c_str_unescape_trans (data->ptr,
					  "\\\\\"\"a\ab\bf\fn\nr\rt\tv\v", &s);
	    if (rc)
	      {
		mu_diag_funcall (MU_DIAG_ERROR, "mu_c_str_unescape_trans",
				 data->ptr, rc);
		return NULL;
	      }
	    rc = regcomp (&args[i].rx, s, REG_EXTENDED|REG_NOSUB);
	    free (s);
	    if (rc)
	      {
		char errbuf[512];
		regerror (rc, &args[i].rx, errbuf, sizeof errbuf);
		yyerror (loc, NULL, NULL, "%s", errbuf);
		return NULL;
	      }
	  }
	  break;
	  
	case 'c':
	  args[i].c = strtoul (data->ptr, &tmp, 0);
	  if (*tmp)
	    goto err;
	  break;
	  
	default:
	  abort ();
	}
    }
  mu_iterator_destroy (&itr);
  node = make_node (mth, functional_node, loc);
  node->v.function.builtin = p;
  node->v.function.args = args;
  return node;
  
 err:
  {
    yyerror (loc, NULL, NULL,
	     _("argument %lu has wrong type in call to `%s'"),
	     (unsigned long) i, ident); 
    return NULL;
  }
}

static void
free_node (struct node *node)
{
  switch (node->type)
    {
    case functional_node:
      {
	char const *p;

	for (p = node->v.function.builtin->args; *p; p++)
	  {
	    switch (*p)
	      {
	      case 'd':
		break;
		
	      case 'x':
		regfree (&node->v.function.args[0].rx);
		break;
		
	      case 's':
		break;
	      }
	  }
	//	free (node->v.function.args);
      }
      break;

    case binary_node:
      free_node (node->v.bin.arg1);
      free_node (node->v.bin.arg2);
      break;

    case negation_node:
      free_node (node->v.arg);
      break;

    default:
      break;
    }

  mu_locus_range_deinit (&node->loc);
}

static void
rule_destroy_item (void *ptr)
{
  struct rule_tab *rt = ptr;
  mu_locus_range_deinit (&rt->loc);  
  free_node (rt->node);
}

  

