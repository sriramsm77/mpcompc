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
#define YYSTYPE         MU_CFG_YYSTYPE
#define YYLTYPE         MU_CFG_YYLTYPE
/* Substitute the variable and function names.  */
#define yyparse         mu_cfg_yyparse
#define yylex           mu_cfg_yylex
#define yyerror         mu_cfg_yyerror
#define yydebug         mu_cfg_yydebug
#define yynerrs         mu_cfg_yynerrs

#define yylval          mu_cfg_yylval
#define yychar          mu_cfg_yychar
#define yylloc          mu_cfg_yylloc

/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

/* cfg_parser.y -- general-purpose configuration file parser
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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <mailutils/argcv.h>
#include <mailutils/wordsplit.h>
#include <mailutils/nls.h>
#include <mailutils/cfg.h>
#include <mailutils/alloc.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/list.h>
#include <mailutils/iterator.h>
#include <mailutils/debug.h>
#include <mailutils/util.h>
#include <mailutils/cctype.h>
#include <mailutils/stream.h>
#include <mailutils/stdstream.h>
#include <mailutils/cidr.h>
#include <mailutils/yyloc.h>

int mu_cfg_parser_verbose;
static mu_list_t /* of mu_cfg_node_t */ parse_node_list; 
size_t mu_cfg_error_count;

static int _mu_cfg_errcnt;

int yylex ();

void _mu_line_begin (void);
void _mu_line_add (char *text, size_t len);
char *_mu_line_finish (void);

static int
yyerror (char *s)
{
  mu_error ("%s", s);
  mu_cfg_error_count++;
  return 0;
}

static mu_config_value_t *
config_value_dup (mu_config_value_t *src)
{
  if (!src)
    return NULL;
  else
    {
      /* FIXME: Use mu_opool_alloc */
      mu_config_value_t *val = mu_alloc (sizeof (*val));
      *val = *src;
      return val;
    }
}

static int
_node_set_parent (void *item, void *data)
{
  struct mu_cfg_node *node = item;
  node->parent = data;
  return 0;
}

static mu_cfg_node_t *
mu_cfg_alloc_node (enum mu_cfg_node_type type, struct mu_locus_range *loc,
		   const char *tag, mu_config_value_t *label,
		   mu_list_t nodelist)
{
  char *p;
  mu_cfg_node_t *np;
  size_t size = sizeof *np + strlen (tag) + 1;
  np = mu_alloc (size);
  np->type = type;
  mu_locus_range_init (&np->locus);
  mu_locus_range_copy (&np->locus, loc);
  p = (char*) (np + 1);
  np->tag = p;
  strcpy (p, tag);
  np->label = label;
  np->nodes = nodelist;
  np->parent = NULL;
  return np;
}

void
mu_cfg_free_node (mu_cfg_node_t *node)
{
  free (node->label);
  free (node);
}

#define node_type_str(t) (((t) == mu_cfg_node_statement) ? "stmt" : "param")

static void
debug_print_node (mu_cfg_node_t *node)
{
  if (mu_debug_level_p (MU_DEBCAT_CONFIG, MU_DEBUG_TRACE0))
    {
      if (node->type == mu_cfg_node_undefined)
	{
	  /* Stay on the safe side */
	  mu_error (_("unknown statement type!"));
	  mu_cfg_error_count++;
	}
      else
	{
	  /* FIXME: How to print label? */
	  mu_error ("statement: %s, id: %s",
		    node_type_str (node->type),
		    node->tag ? node->tag : "(null)");
	}
    }
}

static void
free_node_item (void *item)
{
  mu_cfg_node_t *node = item;

  switch (node->type)
    {
    case mu_cfg_node_statement:
      mu_list_destroy (&node->nodes);
      break;
      
    case mu_cfg_node_undefined: /* hmm... */
    case mu_cfg_node_param:
      break;
    }
  mu_cfg_free_node (node);
}

int
mu_cfg_create_node_list (mu_list_t *plist)
{
  int rc;
  mu_list_t list;

  rc = mu_list_create (&list);
  if (rc)
    return rc;
  mu_list_set_destroy_item (list, free_node_item);
  *plist = list;
  return 0;
}


#line 248 "parser.c" /* yacc.c:339  */

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
   by #include "parser.h".  */
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
#line 173 "parser.y" /* yacc.c:355  */

#define MU_CFG_YYLTYPE struct mu_locus_range
#define yylloc mu_cfg_yylloc
#define yylval mu_cfg_yylval

#line 292 "parser.c" /* yacc.c:355  */

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
#line 181 "parser.y" /* yacc.c:355  */

  mu_cfg_node_t node;
  mu_cfg_node_t *pnode;
  mu_list_t /* of mu_cfg_node_t */ nodelist;
  char *string;
  mu_config_value_t value, *pvalue;
  mu_list_t list;

#line 320 "parser.c" /* yacc.c:355  */
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

/* Copy the second part of user declarations.  */

#line 351 "parser.c" /* yacc.c:358  */

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
         || (defined MU_CFG_YYLTYPE_IS_TRIVIAL && MU_CFG_YYLTYPE_IS_TRIVIAL \
             && defined MU_CFG_YYSTYPE_IS_TRIVIAL && MU_CFG_YYSTYPE_IS_TRIVIAL)))

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
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   29

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  13
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  30
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  39

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   261

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
      10,    11,     2,     2,    12,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     7,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     8,     2,     9,     2,     2,     2,     2,
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
       5,     6
};

#if MU_CFG_YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   202,   202,   208,   213,   221,   222,   225,   234,   241,
     251,   255,   258,   261,   296,   306,   312,   317,   322,   329,
     330,   331,   334,   353,   358,   365,   369,   375,   380,   387,
     388
};
#endif

#if MU_CFG_YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "MU_TOK_IDENT", "MU_TOK_STRING",
  "MU_TOK_QSTRING", "MU_TOK_MSTRING", "';'", "'{'", "'}'", "'('", "')'",
  "','", "$accept", "input", "stmtlist", "stmt", "simple", "block",
  "ident", "tag", "vallist", "vlist", "value", "string", "slist", "slist0",
  "list", "values", "opt_sc", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,    59,   123,   125,
      40,    41,    44
};
# endif

#define YYPACT_NINF -14

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-14)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -1,   -14,    11,    -1,   -14,   -14,   -14,    15,   -14,   -14,
     -14,   -14,   -14,   -14,    15,    14,    16,    15,   -14,   -14,
     -14,    19,   -14,   -14,     3,     0,   -14,   -14,   -14,   -14,
       2,    20,     7,   -14,   -14,   -14,   -14,    20,   -14
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    10,     0,     2,     3,     5,     6,    11,     1,     4,
      20,    19,    23,    18,     0,     0,    12,    13,    14,    16,
      21,    22,    17,    27,     0,     0,     7,    15,    24,    25,
       0,    29,     0,    26,    28,    30,     8,    29,     9
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -14,   -14,     1,    -3,   -14,   -14,   -14,   -14,   -14,   -14,
     -13,   -14,   -14,   -14,   -14,   -14,    -9
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     2,     3,     4,     5,     6,     7,    15,    16,    17,
      18,    19,    20,    21,    22,    24,    36
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
       9,    23,     1,     1,    27,    10,    11,    12,    13,    31,
       1,     8,    14,    33,    29,    30,    37,    34,    10,    11,
      12,    13,    25,    26,    28,    14,    32,    35,    38,     9
};

static const yytype_uint8 yycheck[] =
{
       3,    14,     3,     3,    17,     3,     4,     5,     6,     9,
       3,     0,    10,    11,    11,    12,     9,    30,     3,     4,
       5,     6,     8,     7,     5,    10,    25,     7,    37,    32
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    14,    15,    16,    17,    18,    19,     0,    16,
       3,     4,     5,     6,    10,    20,    21,    22,    23,    24,
      25,    26,    27,    23,    28,     8,     7,    23,     5,    11,
      12,     9,    15,    11,    23,     7,    29,     9,    29
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    13,    14,    15,    15,    16,    16,    17,    18,    18,
      19,    20,    20,    21,    22,    22,    23,    23,    23,    24,
      24,    24,    25,    26,    26,    27,    27,    28,    28,    29,
      29
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     3,     5,     6,
       1,     0,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     4,     1,     3,     0,
       1
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
#if MU_CFG_YYDEBUG

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
# if defined MU_CFG_YYLTYPE_IS_TRIVIAL && MU_CFG_YYLTYPE_IS_TRIVIAL

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
#else /* !MU_CFG_YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !MU_CFG_YYDEBUG */


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
# if defined MU_CFG_YYLTYPE_IS_TRIVIAL && MU_CFG_YYLTYPE_IS_TRIVIAL
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
#line 203 "parser.y" /* yacc.c:1646  */
    {
	    parse_node_list = (yyvsp[0].nodelist);
	  }
#line 1542 "parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 209 "parser.y" /* yacc.c:1646  */
    {
	    mu_cfg_create_node_list (&(yyval.nodelist));
	    mu_list_append ((yyval.nodelist), (yyvsp[0].pnode));
	  }
#line 1551 "parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 214 "parser.y" /* yacc.c:1646  */
    {
	    mu_list_append ((yyvsp[-1].nodelist), (yyvsp[0].pnode));
	    (yyval.nodelist) = (yyvsp[-1].nodelist);
	    debug_print_node ((yyvsp[0].pnode));
	  }
#line 1561 "parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 226 "parser.y" /* yacc.c:1646  */
    {
	    struct mu_locus_range lr;
	    lr.beg = (yylsp[-2]).beg;
	    lr.end = (yylsp[0]).end;
	    (yyval.pnode) = mu_cfg_alloc_node (mu_cfg_node_param, &lr, (yyvsp[-2].string), (yyvsp[-1].pvalue), NULL);
	  }
#line 1572 "parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 235 "parser.y" /* yacc.c:1646  */
    {
	    struct mu_locus_range lr;
	    lr.beg = (yylsp[-4]).beg;
	    lr.end = (yylsp[0]).end;
	    (yyval.pnode) = mu_cfg_alloc_node (mu_cfg_node_statement, &lr, (yyvsp[-4].string), (yyvsp[-3].pvalue), NULL);
	  }
#line 1583 "parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 242 "parser.y" /* yacc.c:1646  */
    {
	    struct mu_locus_range lr;
	    lr.beg = (yylsp[-5]).beg;
	    lr.end = (yylsp[0]).end;
	    (yyval.pnode) = mu_cfg_alloc_node (mu_cfg_node_statement, &lr, (yyvsp[-5].string), (yyvsp[-4].pvalue), (yyvsp[-2].nodelist));
	    mu_list_foreach ((yyvsp[-2].nodelist), _node_set_parent, (yyval.pnode));
	  }
#line 1595 "parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 255 "parser.y" /* yacc.c:1646  */
    {
	    (yyval.pvalue) = NULL;
	  }
#line 1603 "parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 262 "parser.y" /* yacc.c:1646  */
    {
	    size_t n = 0;
	    mu_list_count((yyvsp[0].list), &n);
	    if (n == 1)
	      {
		mu_list_get ((yyvsp[0].list), 0, (void**) &(yyval.pvalue));
	      }
	    else
	      {
		size_t i;
		mu_config_value_t val;

		val.type = MU_CFG_ARRAY;
		val.v.arg.c = n;
		/* FIXME: Use mu_opool_alloc */
		val.v.arg.v = mu_alloc (n * sizeof (val.v.arg.v[0]));
		if (!val.v.arg.v)
		  {
		    mu_error (_("not enough memory"));
		    abort();
		  }

		for (i = 0; i < n; i++)
		  {
		    mu_config_value_t *v;
		    mu_list_get ((yyvsp[0].list), i, (void **) &v);
		    val.v.arg.v[i] = *v;
		  }
		(yyval.pvalue) = config_value_dup (&val);
	      }
	    mu_list_destroy (&(yyvsp[0].list));
	  }
#line 1640 "parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 297 "parser.y" /* yacc.c:1646  */
    {
	    int rc = mu_list_create (&(yyval.list));
	    if (rc)
	      {
		mu_error (_("cannot create list: %s"), mu_strerror (rc));
		abort ();
	      }
	    mu_list_append ((yyval.list), config_value_dup (&(yyvsp[0].value))); /* FIXME */
	  }
#line 1654 "parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 307 "parser.y" /* yacc.c:1646  */
    {
	    mu_list_append ((yyvsp[-1].list), config_value_dup (&(yyvsp[0].value)));
	  }
#line 1662 "parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 313 "parser.y" /* yacc.c:1646  */
    {
	      (yyval.value).type = MU_CFG_STRING;
	      (yyval.value).v.string = (yyvsp[0].string);
	  }
#line 1671 "parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 318 "parser.y" /* yacc.c:1646  */
    {
	      (yyval.value).type = MU_CFG_LIST;
	      (yyval.value).v.list = (yyvsp[0].list);
	  }
#line 1680 "parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 323 "parser.y" /* yacc.c:1646  */
    {
	      (yyval.value).type = MU_CFG_STRING;
	      (yyval.value).v.string = (yyvsp[0].string);
	  }
#line 1689 "parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 335 "parser.y" /* yacc.c:1646  */
    {
	    mu_iterator_t itr;
	    mu_list_get_iterator ((yyvsp[0].list), &itr);

	    _mu_line_begin ();
	    for (mu_iterator_first (itr);
		 !mu_iterator_is_done (itr); mu_iterator_next (itr))
	      {
		char *p;
		mu_iterator_current (itr, (void**)&p);
		_mu_line_add (p, strlen (p));
	      }
	    (yyval.string) = _mu_line_finish ();
	    mu_iterator_destroy (&itr);
	    mu_list_destroy(&(yyvsp[0].list));
	  }
#line 1710 "parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 354 "parser.y" /* yacc.c:1646  */
    {
	    mu_list_create (&(yyval.list));
	    mu_list_append ((yyval.list), (yyvsp[0].string));
	  }
#line 1719 "parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 359 "parser.y" /* yacc.c:1646  */
    {
	    mu_list_append ((yyvsp[-1].list), (yyvsp[0].string));
	    (yyval.list) = (yyvsp[-1].list);
	  }
#line 1728 "parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 366 "parser.y" /* yacc.c:1646  */
    {
	      (yyval.list) = (yyvsp[-1].list);
	  }
#line 1736 "parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 370 "parser.y" /* yacc.c:1646  */
    {
	      (yyval.list) = (yyvsp[-2].list);
	  }
#line 1744 "parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 376 "parser.y" /* yacc.c:1646  */
    {
	    mu_list_create (&(yyval.list));
	    mu_list_append ((yyval.list), config_value_dup (&(yyvsp[0].value)));
	  }
#line 1753 "parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 381 "parser.y" /* yacc.c:1646  */
    {
	    mu_list_append ((yyvsp[-2].list), config_value_dup (&(yyvsp[0].value)));
	    (yyval.list) = (yyvsp[-2].list);
	  }
#line 1762 "parser.c" /* yacc.c:1646  */
    break;


#line 1766 "parser.c" /* yacc.c:1646  */
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
#line 392 "parser.y" /* yacc.c:1906  */


void
mu_cfg_set_debug ()
{
  if (mu_debug_level_p (MU_DEBCAT_CONFIG, MU_DEBUG_TRACE7))
    yydebug = 1;
}

int
mu_cfg_parse (mu_cfg_tree_t **ptree)
{
  int rc;
  mu_cfg_tree_t *tree;
  mu_opool_t pool;
  int save_mode = 0, mode;
  struct mu_locus_range save_locus = MU_LOCUS_RANGE_INITIALIZER;

  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, MU_IOCTL_LOGSTREAM_GET_MODE, 
                   &save_mode);
  mode = save_mode | MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, MU_IOCTL_LOGSTREAM_SET_MODE,
                   &mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_GET_LOCUS_RANGE,
                   &save_locus);
  
  mu_cfg_set_debug ();
  _mu_cfg_errcnt = 0;

  rc = yyparse ();
  pool = mu_cfg_lexer_pool ();
  if (rc == 0 && _mu_cfg_errcnt)
    {
      mu_opool_destroy (&pool);
      rc = 1;
    }
  else
    {
      tree = mu_alloc (sizeof (*tree));
      tree->nodes = parse_node_list;
      tree->pool = pool;
      parse_node_list = NULL;
      *ptree = tree;
    }

  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, MU_IOCTL_LOGSTREAM_SET_MODE,
                   &save_mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE,
                   &save_locus);
  mu_locus_range_deinit (&save_locus); /* FIXME: refcount? */

  return rc;
}

int
mu_cfg_tree_union (mu_cfg_tree_t **pa, mu_cfg_tree_t **pb)
{
  mu_cfg_tree_t *a, *b;
  int rc;
  
  if (!pb)
    return EINVAL;
  if (!*pb)
    return 0;
  b = *pb;
  if (!pa)
    return EINVAL;
  if (!*pa)
    {
      *pa = b;
      *pb = NULL;
      return 0;
    }
  else
    a = *pa;
  
  /* Merge opools */
  rc = mu_opool_union (&b->pool, &a->pool);
  if (rc)
    return rc;
    
  /* Link node lists */
  if (b->nodes)
    {
      mu_list_append_list (a->nodes, b->nodes);
      mu_list_destroy (&b->nodes);
    }
  
  free (b);
  *pb = NULL;
  return 0;
}

static mu_cfg_tree_t *
do_include (const char *name, struct mu_cfg_parse_hints *hints,
	    struct mu_locus_range const *loc)
{
  struct stat sb;
  char *tmpname = NULL;
  mu_cfg_tree_t *tree = NULL;
  
  if (name[0] != '/')
    {
      name = tmpname = mu_make_file_name (SYSCONFDIR, name);
      if (!name)
        {
          mu_error ("%s", mu_strerror (errno));
          return NULL;
        }
    }
  if (stat (name, &sb) == 0)
    {
      int rc = 0;
      
      if (S_ISDIR (sb.st_mode))
	{
	  if (hints->flags & MU_CFHINT_PROGRAM)
	    {
	      char *file = mu_make_file_name (name, hints->program);
	      rc = mu_cfg_parse_file (&tree, file, hints->flags);
	      free (file);
	    }
	  else
	    {
	      mu_diag_at_locus_range (MU_LOG_WARNING, loc,
				      _("ignoring `include': directory argument is allowed only from the top-level configuration file"));

	    }
	}
      else
	rc = mu_cfg_parse_file (&tree, name, hints->flags);
      
      if (rc == 0 && tree)
	{
	  struct mu_cfg_parse_hints xhints = *hints;
	  xhints.flags &= ~MU_CFHINT_PROGRAM;
	  mu_cfg_tree_postprocess (tree, &xhints);
	}
    }
  else if (errno == ENOENT)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, loc,
			      _("include file or directory does not exist"));
      mu_cfg_error_count++;
    }		   
  else
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, loc,
			      _("cannot stat include file or directory: %s"),
			      mu_strerror (errno));
      mu_cfg_error_count++;
    }		   
  
  free (tmpname);
  return tree;
}

int
mu_cfg_tree_postprocess (mu_cfg_tree_t *tree, struct mu_cfg_parse_hints *hints)
{
  int rc;
  mu_iterator_t itr;
  
  if (!tree->nodes)
    return 0;
  rc = mu_list_get_iterator (tree->nodes, &itr);
  if (rc)
    return rc;
  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr))
    {
      mu_cfg_node_t *node;
      
      mu_iterator_current (itr, (void**) &node);
      
      if (node->type == mu_cfg_node_statement)
	{
	  if (strcmp (node->tag, "program") == 0)
	    {
	      if (hints->flags & MU_CFHINT_PROGRAM)
		{
		  if (node->label->type == MU_CFG_STRING)
		    {
		      if (strcmp (node->label->v.string, hints->program) == 0)
			{
			  /* Reset the parent node */
			  mu_list_foreach (node->nodes, _node_set_parent,
					   node->parent);
			  /* Move all nodes from this block to the topmost
			     level */
			  mu_iterator_ctl (itr, mu_itrctl_insert_list,
					   node->nodes);
			  mu_iterator_ctl (itr, mu_itrctl_delete, NULL);
			  /*FIXME:mu_cfg_free_node (node);*/
			}
		    }
		  else
		    {
		      mu_diag_at_locus_range (MU_LOG_ERROR, &node->locus,
					      _("argument to `program' is not a string"));
		      mu_cfg_error_count++;
		      mu_iterator_ctl (itr, mu_itrctl_delete, NULL);
		    }
		}
	      else
		{
		  mu_diag_at_locus_range (MU_LOG_WARNING, &node->locus,
					  _("ignoring `program' block: not located in top-level configuration file"));
		}
	    }
	}
      else if (node->type == mu_cfg_node_param &&
	       strcmp (node->tag, "include") == 0)
	{
	  if (node->label->type == MU_CFG_STRING)
	    {
	      mu_cfg_tree_t *t = do_include (node->label->v.string,
					     hints,
					     &node->locus);
	      if (t)
		{
		  /* Merge the new tree into the current point and
		     destroy the rest of it */
		  mu_iterator_ctl (itr, mu_itrctl_insert_list, t->nodes);
		  mu_opool_union (&tree->pool, &t->pool);
		  mu_cfg_destroy_tree (&t);
		}		      
	    }
	  else
	    {
	      mu_diag_at_locus_range (MU_LOG_ERROR, &node->locus,
				_("argument to `include' is not a string"));
	      mu_cfg_error_count++;
	    }
	  
	  /* Remove node from the list */
	  mu_iterator_ctl (itr, mu_itrctl_delete, NULL);
	}
    }
  mu_iterator_destroy (&itr);
  return 0;
}

static int
_mu_cfg_preorder_recursive (void *item, void *cbdata)
{
  mu_cfg_node_t *node = item;
  struct mu_cfg_iter_closure *clos = cbdata;
  int rc;
  
  switch (node->type)
    {
    case mu_cfg_node_undefined:
      abort ();
      
    case mu_cfg_node_statement:
      switch (clos->beg (node, clos->data))
	{
	case MU_CFG_ITER_OK:
	  rc = mu_cfg_preorder (node->nodes, clos);
	  if (rc)
	    return rc;
	  if (clos->end && clos->end (node, clos->data) == MU_CFG_ITER_STOP)
	    return MU_ERR_USER0;
	  break;
	  
	case MU_CFG_ITER_SKIP:
	  break;
	  
	case MU_CFG_ITER_STOP:
	  return MU_ERR_USER0;
	}
      break;
      
    case mu_cfg_node_param:
      if (clos->beg (node, clos->data) == MU_CFG_ITER_STOP)
	return MU_ERR_USER0;
    }
  return 0;
}

int
mu_cfg_preorder (mu_list_t nodelist, struct mu_cfg_iter_closure *clos)
{
  if (!nodelist)
    return 0;
  return mu_list_foreach (nodelist, _mu_cfg_preorder_recursive, clos);
}



void
mu_cfg_destroy_tree (mu_cfg_tree_t **ptree)
{
  if (ptree && *ptree)
    {
      mu_cfg_tree_t *tree = *ptree;
      mu_list_destroy (&tree->nodes);
      mu_opool_destroy (&tree->pool);
      free (tree);
      *ptree = NULL;
    }
}



struct mu_cfg_section_list
{
  struct mu_cfg_section_list *next;
  struct mu_cfg_section *sec;
};

struct scan_tree_data
{
  struct mu_cfg_section_list *list;
  void *target;
  void *call_data;
  mu_cfg_tree_t *tree;
  int error;
};

static struct mu_cfg_cont *
find_container (mu_list_t list, enum mu_cfg_cont_type type,
		const char *ident, size_t len)
{
  mu_iterator_t iter;
  struct mu_cfg_cont *ret = NULL;
  
  if (len == 0)
    len = strlen (ident);
  
  mu_list_get_iterator (list, &iter);
  for (mu_iterator_first (iter); !mu_iterator_is_done (iter);
       mu_iterator_next (iter))
    {
      struct mu_cfg_cont *cont;
      mu_iterator_current (iter, (void**) &cont);
      
      if (cont->type == type
	  && strlen (cont->v.ident) == len
	  && memcmp (cont->v.ident, ident, len) == 0)
	{
	  ret = cont;
	  break;
	}
    }
  mu_iterator_destroy (&iter);
  return ret;
}

static struct mu_cfg_section *
find_subsection (struct mu_cfg_section *sec, const char *ident, size_t len)
{
  if (sec)
    {
      if (sec->children)
	{
	  struct mu_cfg_cont *cont = find_container (sec->children,
						     mu_cfg_cont_section,
						     ident, len);
	  if (cont)
	    return &cont->v.section;
	}
    }
  return NULL;
}

static struct mu_cfg_param *
find_param (struct mu_cfg_section *sec, const char *ident, size_t len)
{
  if (sec)
    {
      if (sec->children)
	{
	  struct mu_cfg_cont *cont = find_container (sec->children,
						     mu_cfg_cont_param,
						     ident, len);
	  if (cont)
	    return &cont->v.param;
	}
    }
  return NULL;
}

static int
push_section (struct scan_tree_data *dat, struct mu_cfg_section *sec)
{
  struct mu_cfg_section_list *p = mu_alloc (sizeof *p);
  if (!p)
    {
      mu_error (_("not enough memory"));
      mu_cfg_error_count++;
      return 1;
    }
  p->sec = sec;
  p->next = dat->list;
  dat->list = p;
  return 0;
}

static struct mu_cfg_section *
pop_section (struct scan_tree_data *dat)
{
  struct mu_cfg_section_list *p = dat->list;
  struct mu_cfg_section *sec = p->sec;
  dat->list = p->next;
  free (p);
  return sec;
}

static int
valcvt (const struct mu_locus_range *locus,
	void *tgt, mu_c_type_t type, mu_config_value_t *val)
{
  int rc;
  char *errmsg;
  
  if (val->type != MU_CFG_STRING)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, locus, _("expected string value"));
      mu_cfg_error_count++;
      return 1;
    }

  rc = mu_str_to_c (val->v.string, type, tgt, &errmsg);
  if (rc)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, locus, "%s",
			      errmsg ? errmsg : mu_strerror (rc));
      free (errmsg);
    }
  return rc;
}

struct set_closure
{
  mu_list_t list;
  int type;
  struct scan_tree_data *sdata;
  const struct mu_locus_range *locus;
};

static size_t config_type_size[] = {
  [mu_c_string] = sizeof (char*),          
  [mu_c_short]  = sizeof (short),          
  [mu_c_ushort] = sizeof (unsigned short), 
  [mu_c_int]    = sizeof (int),            
  [mu_c_uint]   = sizeof (unsigned),       
  [mu_c_long]   = sizeof (long),           
  [mu_c_ulong]  = sizeof (unsigned long),  
  [mu_c_size]   = sizeof (size_t),         
  [mu_c_time]   = sizeof (time_t),         
  [mu_c_bool]   = sizeof (int),            
  [mu_c_ipv4]   = sizeof (struct in_addr), 
  [mu_c_cidr]   = sizeof (struct mu_cidr), 
  [mu_c_host]   = sizeof (struct in_addr), 
};

static int
_set_fun (void *item, void *data)
{
  mu_config_value_t *val = item;
  struct set_closure *clos = data;
  void *tgt;
  size_t size;
  
  if ((size_t) clos->type >= MU_ARRAY_SIZE(config_type_size)
      || (size = config_type_size[clos->type]) == 0)
    {
      mu_diag_at_locus_range (MU_LOG_EMERG, clos->locus,
			_("INTERNAL ERROR at %s:%d: unhandled data type %d"),
			__FILE__, __LINE__, clos->type);
      mu_cfg_error_count++;
      return 1;
    }
  
  tgt = mu_alloc (size);
  if (!tgt)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, clos->locus,
			      _("not enough memory"));
      mu_cfg_error_count++;
      return 1;
    }
  
  if (valcvt (clos->locus, &tgt, clos->type, val) == 0)
    mu_list_append (clos->list, tgt);
  return 0;
}

static int
parse_param (struct scan_tree_data *sdata, const mu_cfg_node_t *node)
{
  void *tgt;
  struct set_closure clos;
  struct mu_cfg_param *param = find_param (sdata->list->sec, node->tag,
					   0);
  
  if (!param)
    {
      mu_diag_at_locus_range (MU_LOG_ERROR, &node->locus,
			      _("unknown keyword `%s'"),
			      node->tag);
      mu_cfg_error_count++;
      return 1;
    }
  
  if (param->data)
    tgt = param->data;
  else if (sdata->list->sec->target)
    tgt = (char*)sdata->list->sec->target + param->offset;
  else if (sdata->target)
    tgt = (char*)sdata->target + param->offset;
  else if (param->type == mu_cfg_callback)
    tgt = NULL;
  else
    {
      mu_diag_at_locus_range (MU_LOG_EMERG, &node->locus,
			_("INTERNAL ERROR: cannot determine target offset for "
			  "%s"), param->ident);
      abort ();
    }
  
  memset (&clos, 0, sizeof clos);
  clos.type = MU_CFG_TYPE (param->type);
  if (MU_CFG_IS_LIST (param->type))
    {
      clos.sdata = sdata;
      clos.locus = &node->locus;
      switch (node->label->type)
	{
	case MU_CFG_LIST:
	  break;
	  
	case MU_CFG_STRING:
	  {
	    mu_list_t list;
	    mu_list_create (&list);
	    mu_list_append (list, config_value_dup (node->label));
	    node->label->type = MU_CFG_LIST;
	    node->label->v.list = list;
	  }
	  break;
	  
	case MU_CFG_ARRAY:
	  mu_diag_at_locus_range (MU_LOG_ERROR, &node->locus,
			    _("expected list, but found array"));
	  mu_cfg_error_count++;
	  return 1;
	}
      
      mu_list_create (&clos.list);
      mu_list_foreach (node->label->v.list, _set_fun, &clos);
      *(mu_list_t*)tgt = clos.list;
    }
  else if (clos.type == mu_cfg_callback)
    {
      if (!param->callback)
	{
	  mu_diag_at_locus_range (MU_LOG_EMERG, &node->locus,
			    _("INTERNAL ERROR: %s: callback not defined"),
			    node->tag);
	  abort ();
	}
      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		       MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE,
		       (void*) &node->locus);
      if (param->callback (tgt, node->label))
	return 1;
    }
  else
    return valcvt (&node->locus, tgt, clos.type, node->label);
  
  return 0;
}


static int
_scan_tree_helper (const mu_cfg_node_t *node, void *data)
{
  struct scan_tree_data *sdata = data;
  struct mu_cfg_section *sec;
  
  switch (node->type)
    {
    case mu_cfg_node_undefined:
      abort ();
      
    case mu_cfg_node_statement:
      sec = find_subsection (sdata->list->sec, node->tag, 0);
      if (!sec)
	{
	  if (mu_cfg_parser_verbose)
	    {
	      mu_diag_at_locus_range (MU_LOG_WARNING, &node->locus,
				      _("unknown section `%s'"),
				      node->tag);
	    }
	  return MU_CFG_ITER_SKIP;
	}
      if (!sec->children)
	return MU_CFG_ITER_SKIP;
      if (sec->data)
	sec->target = sec->data;
      else if (sdata->list->sec->target)
	sec->target = (char*)sdata->list->sec->target + sec->offset;
      else if (sdata->target)
	sec->target = (char*)sdata->target + sec->offset;
      else
	sec->target = NULL;
      if (sec->parser)
	{
	  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
			   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE,
			   (void*) &node->locus);
	  if (sec->parser (mu_cfg_section_start, node,
			   sec->label, &sec->target,
			   sdata->call_data, sdata->tree))
	    {
	      sdata->error++;
	      return MU_CFG_ITER_SKIP;
	    }
	}
      push_section(sdata, sec);
      break;
      
    case mu_cfg_node_param:
      if (parse_param (sdata, node))
	{
	  sdata->error++;
	  return MU_CFG_ITER_SKIP;
	}
      break;
    }
  return MU_CFG_ITER_OK;
}

static int
_scan_tree_end_helper (const mu_cfg_node_t *node, void *data)
{
  struct scan_tree_data *sdata = data;
  struct mu_cfg_section *sec;
  
  switch (node->type)
    {
    default:
      abort ();
      
    case mu_cfg_node_statement:
      sec = pop_section (sdata);
      if (sec && sec->parser)
	{
	  if (sec->parser (mu_cfg_section_end, node,
			   sec->label, &sec->target,
			   sdata->call_data, sdata->tree))
	    {
	      sdata->error++;
	      return MU_CFG_ITER_SKIP;
	    }
	}
    }
  return MU_CFG_ITER_OK;
}

int
mu_cfg_scan_tree (mu_cfg_tree_t *tree, struct mu_cfg_section *sections,
		   void *target, void *data)
{
  struct scan_tree_data dat;
  struct mu_cfg_iter_closure clos;
  int save_mode = 0, mode;
  struct mu_locus_range save_locus = MU_LOCUS_RANGE_INITIALIZER;
  int rc;
  
  dat.tree = tree;
  dat.list = NULL;
  dat.error = 0;
  dat.call_data = data;
  dat.target = target;
  
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_GET_MODE, &save_mode);
  mode = save_mode | MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_GET_LOCUS_RANGE, &save_locus);

  if (push_section (&dat, sections))
    return 1;
  clos.beg = _scan_tree_helper;
  clos.end = _scan_tree_end_helper;
  clos.data = &dat;
  rc = mu_cfg_preorder (tree->nodes, &clos);
  pop_section (&dat);
  if (rc && rc != MU_ERR_USER0)
    dat.error++;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_SET_MODE, &save_mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM, 
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &save_locus);
  
  return dat.error;
}

int
mu_cfg_find_section (struct mu_cfg_section *root_sec,
		     const char *path, struct mu_cfg_section **retval)
{
  while (path[0])
    {
      struct mu_cfg_section *sec;
      size_t len;
      const char *p;
      
      while (*path == MU_CFG_PATH_DELIM)
	path++;
      
      if (*path == 0)
	return MU_ERR_NOENT;
      
      p = strchr (path, MU_CFG_PATH_DELIM);
      if (p)
	len = p - path;
      else
	len = strlen (path);
      
      sec = find_subsection (root_sec, path, len);
      if (!sec)
	return MU_ERR_NOENT;
      root_sec = sec;
      path += len;
    }
  if (retval)
    *retval = root_sec;
  return 0;
}


int
mu_cfg_tree_create (struct mu_cfg_tree **ptree)
{
  struct mu_cfg_tree *tree = calloc (1, sizeof *tree);
  if (!tree)
    return errno;
  mu_opool_create (&tree->pool, MU_OPOOL_ENOMEMABRT);
  *ptree = tree;
  return 0;
}

mu_cfg_node_t *
mu_cfg_tree_create_node (struct mu_cfg_tree *tree,
			 enum mu_cfg_node_type type,
			 const struct mu_locus_range *loc,
			 const char *tag, const char *label,
			 mu_list_t nodelist)
{
  char *p;
  mu_cfg_node_t *np;
  size_t size = sizeof *np + strlen (tag) + 1;
  mu_config_value_t val;
  
  np = mu_alloc (size);
  np->type = type;
  mu_locus_range_init (&np->locus);
  if (loc)
    mu_locus_range_copy (&np->locus, loc);
  p = (char*) (np + 1);
  np->tag = p;
  strcpy (p, tag);
  p += strlen (p) + 1;
  val.type = MU_CFG_STRING;
  if (label)
    {
      mu_opool_clear (tree->pool);
      mu_opool_appendz (tree->pool, label);
      mu_opool_append_char (tree->pool, 0);
      val.v.string = mu_opool_finish (tree->pool, NULL);
      np->label = config_value_dup (&val);
    }
  else
    np->label = NULL;
  np->nodes = nodelist;
  return np;
}

void
mu_cfg_tree_add_node (mu_cfg_tree_t *tree, mu_cfg_node_t *node)
{
  if (!node)
    return;
  if (!tree->nodes)
    /* FIXME: return code? */
    mu_cfg_create_node_list (&tree->nodes);
  mu_list_append (tree->nodes, node);
}

void
mu_cfg_tree_add_nodelist (mu_cfg_tree_t *tree, mu_list_t nodelist)
{
  if (!nodelist)
    return;
  if (!tree->nodes)
    /* FIXME: return code? */
    mu_cfg_create_node_list (&tree->nodes);
  mu_list_append_list (tree->nodes, nodelist);
}


/* Return 1 if configuration value A equals B */
int
mu_cfg_value_eq (mu_config_value_t *a, mu_config_value_t *b)
{
  if (a->type != b->type)
    return 0;
  switch (a->type)
    {
    case MU_CFG_STRING:
      if (a->v.string == NULL)
	return b->v.string == NULL;
      return strcmp (a->v.string, b->v.string) == 0;
      
    case MU_CFG_LIST:
      {
	int ret = 1;
	size_t cnt;
	size_t i;
	mu_iterator_t aitr, bitr;
	
	mu_list_count (a->v.list, &cnt);
	mu_list_count (b->v.list, &i);
	if (i != cnt)
	  return 1;
	
	mu_list_get_iterator (a->v.list, &aitr);
	mu_list_get_iterator (b->v.list, &bitr);
	for (i = 0,
	       mu_iterator_first (aitr),
	       mu_iterator_first (bitr);
	     !mu_iterator_is_done (aitr) && !mu_iterator_is_done (bitr);
	     mu_iterator_next (aitr),
	       mu_iterator_next (bitr),
	       i++)
	  {
	    mu_config_value_t *ap, *bp;
	    mu_iterator_current (aitr, (void**)&ap);
	    mu_iterator_current (bitr, (void**)&bp);
	    ret = mu_cfg_value_eq (ap, bp);
	    if (!ret)
	      break;
	  }
	mu_iterator_destroy (&aitr);
	mu_iterator_destroy (&bitr);
	return ret && i == cnt;
      }
      
    case MU_CFG_ARRAY:
      if (a->v.arg.c == b->v.arg.c)
	{
	  size_t i;
	  for (i = 0; i < a->v.arg.c; i++)
	    if (!mu_cfg_value_eq (&a->v.arg.v[i], &b->v.arg.v[i]))
	      return 0;
	  return 1;
	}
    }
  return 0;
}


static int
split_cfg_path (const char *path, int *pargc, char ***pargv)
{
  int argc;
  char **argv;
  char *delim = MU_CFG_PATH_DELIM_STR;
  char static_delim[2] = { 0, 0 };
  
  if (path[0] == '\\')
    {
      argv = calloc (2, sizeof (*argv));
      if (!argv)
	return ENOMEM;
      argv[0] = strdup (path + 1);
      if (!argv[0])
	{
	  free (argv);
	  return ENOMEM;
	}
      argv[1] = NULL;
      argc = 1;
    }
  else
    {
      struct mu_wordsplit ws;
      
      if (mu_ispunct (path[0]))
	{
	  delim = static_delim;
	  delim[0] = path[0];
	  path++;
	}
      ws.ws_delim = delim;
      
      if (mu_wordsplit (path, &ws, MU_WRDSF_DEFFLAGS|MU_WRDSF_DELIM))
	{
	  mu_error (_("cannot split line `%s': %s"), path,
		    mu_wordsplit_strerror (&ws));
	  mu_wordsplit_free (&ws);
	  return errno;
	}
      argc = ws.ws_wordc;
      argv = ws.ws_wordv;
      ws.ws_wordc = 0;
      ws.ws_wordv = NULL;
      mu_wordsplit_free (&ws);
    }
  
  *pargc = argc;
  *pargv = argv;
  
  return 0;
}

struct find_data
{
  int argc;
  char **argv;
  int tag;
  mu_config_value_t *label;
  const mu_cfg_node_t *node;
};

static void
free_value_mem (mu_config_value_t *p)
{
  switch (p->type)
    {
    case MU_CFG_STRING:
      free ((char*)p->v.string);
      break;
      
    case MU_CFG_LIST:
      /* FIXME */
      break;
      
    case MU_CFG_ARRAY:
      {
	size_t i;
	for (i = 0; i < p->v.arg.c; i++)
	  free_value_mem (&p->v.arg.v[i]);
      }
    }
}

static void
destroy_value (void *p)
{
  mu_config_value_t *val = p;
  if (val)
    {
      free_value_mem (val);
      free (val);
    }
}

static mu_config_value_t *
parse_label (const char *str)
{
  mu_config_value_t *val = NULL;
  size_t i;
  struct mu_wordsplit ws;
  size_t len = strlen (str);
  
  if (len > 1 && str[0] == '(' && str[len-1] == ')')
    {
      mu_list_t lst;
      
      ws.ws_delim = ",";
      if (mu_wordsplit_len (str + 1, len - 2, &ws,
			    MU_WRDSF_DEFFLAGS|MU_WRDSF_DELIM|MU_WRDSF_WS))
	{
	  mu_error (_("cannot split line `%s': %s"), str,
		    mu_wordsplit_strerror (&ws));
	  return NULL;
	}
      
      mu_list_create (&lst);
      mu_list_set_destroy_item (lst, destroy_value);
      for (i = 0; i < ws.ws_wordc; i++)
	{
	  mu_config_value_t *p = mu_alloc (sizeof (*p));
	  p->type = MU_CFG_STRING;
	  p->v.string = ws.ws_wordv[i];
	  mu_list_append (lst, p);
	}
      val = mu_alloc (sizeof (*val));
      val->type = MU_CFG_LIST;
      val->v.list = lst;
    }
  else
    {      
      if (mu_wordsplit (str, &ws, MU_WRDSF_DEFFLAGS))
	{
	  mu_error (_("cannot split line `%s': %s"), str,
		    mu_wordsplit_strerror (&ws));
	  return NULL;
	}
      val = mu_alloc (sizeof (*val));
      if (ws.ws_wordc == 1)
	{
	  val->type = MU_CFG_STRING;
	  val->v.string = ws.ws_wordv[0];
	}
      else
	{
	  val->type = MU_CFG_ARRAY;
	  val->v.arg.c = ws.ws_wordc;
	  val->v.arg.v = mu_alloc (ws.ws_wordc * sizeof (val->v.arg.v[0]));
	  for (i = 0; i < ws.ws_wordc; i++)
	    {
	      val->v.arg.v[i].type = MU_CFG_STRING;
	      val->v.arg.v[i].v.string = ws.ws_wordv[i];
	    }
	}
      ws.ws_wordc = 0;
      mu_wordsplit_free (&ws);
    }
  return val;
}

static void
parse_tag (struct find_data *fptr)
{
  char *p = strchr (fptr->argv[fptr->tag], '=');
  if (p)
    {
      *p++ = 0;
      fptr->label = parse_label (p);
    }
  else
    fptr->label = NULL;
}

static int
node_finder (const mu_cfg_node_t *node, void *data)
{
  struct find_data *fdptr = data;
  if (strcmp (fdptr->argv[fdptr->tag], node->tag) == 0
      && (!fdptr->label || mu_cfg_value_eq (fdptr->label, node->label)))
    {
      fdptr->tag++;
      if (fdptr->tag == fdptr->argc)
	{
	  fdptr->node = node;
	  return MU_CFG_ITER_STOP;
	}
      parse_tag (fdptr);
      return MU_CFG_ITER_OK;
    }
  
  return node->type == mu_cfg_node_statement ?
               MU_CFG_ITER_SKIP : MU_CFG_ITER_OK;
}

int	    
mu_cfg_find_node (mu_cfg_tree_t *tree, const char *path, mu_cfg_node_t **pval)
{
  int rc;
  struct find_data data;
  
  rc = split_cfg_path (path, &data.argc, &data.argv);
  if (rc)
    return rc;
  data.tag = 0;
  if (data.argc)
    {
      struct mu_cfg_iter_closure clos;
      
      parse_tag (&data);
      
      clos.beg = node_finder;
      clos.end = NULL;
      clos.data = &data;
      rc = mu_cfg_preorder (tree->nodes, &clos);
      destroy_value (data.label);
      if (rc == MU_ERR_USER0)
	{
	  *pval = (mu_cfg_node_t *) data.node;
	  return 0;
	}
      else if (rc != 0)
	mu_diag_funcall (MU_DIAG_ERR, "mu_cfg_preorder", NULL, rc);
    }
  return MU_ERR_NOENT;
}



int
mu_cfg_create_subtree (const char *path, mu_cfg_node_t **pnode)
{
  int rc;
  int argc, i;
  char **argv;
  enum mu_cfg_node_type type;
  mu_cfg_node_t *node = NULL;
  struct mu_locus_range locus = MU_LOCUS_RANGE_INITIALIZER;
  
  rc = split_cfg_path (path, &argc, &argv);
  if (rc)
    return rc;
  
  for (i = argc - 1; i >= 0; i--)
    {
      mu_list_t nodelist = NULL;
      mu_config_value_t *label = NULL;
      char *q = argv[i], *p;
      mu_cfg_node_t *parent;
      
      type = mu_cfg_node_statement;
      do
	{
	  p = strchr (q, '=');
	  if (p && p > argv[i] && p[-1] != '\\')
	    {
	      *p++ = 0;
	      label = parse_label (p);
	      if (i == argc - 1)
		type = mu_cfg_node_param;
	      break;
	    }
	  else if (p)
	    q = p + 1;
	  else
	    break;
	}
      while (*q);
      
      if (node)
	{
	  mu_cfg_create_node_list (&nodelist);
	  mu_list_append (nodelist, node);
	}
      parent = mu_cfg_alloc_node (type, &locus, argv[i], label, nodelist);
      if (node)
	node->parent = parent;
      node = parent;
    }
  
  mu_argcv_free (argc, argv);
  *pnode = node;
  return 0;
}

int
mu_cfg_parse_config (mu_cfg_tree_t **ptree, struct mu_cfg_parse_hints *hints)
{
  int rc = 0;
  mu_cfg_tree_t *tree = NULL, *tmp;
  struct mu_cfg_parse_hints xhints;
  
  if ((hints->flags & MU_CFHINT_SITE_FILE) && hints->site_file)
    {
      rc = mu_cfg_parse_file (&tmp, hints->site_file, hints->flags);
      
      switch (rc)
	{
	case 0:
	  mu_cfg_tree_postprocess (tmp, hints);
	  mu_cfg_tree_union (&tree, &tmp);
	  
	case ENOENT:
	  rc = 0;
	  break;

	default:
	  mu_error ("%s", mu_strerror (rc));
	  return rc;
	}
    }

  xhints = *hints;
  xhints.flags &= ~MU_CFHINT_PROGRAM;
  
  if ((hints->flags & MU_CFHINT_PER_USER_FILE)
      && (hints->flags & MU_CFHINT_PROGRAM))
    {
      size_t size = 3 + strlen (hints->program) + 1;
      char *file_name = malloc (size);
      if (file_name)
	{
	  strcpy (file_name, "~/.");
	  strcat (file_name, hints->program);
	  
	  rc = mu_cfg_parse_file (&tmp, file_name, xhints.flags);
	  switch (rc)
	    {
	    case 0:
	      mu_cfg_tree_postprocess (tmp, &xhints);
	      mu_cfg_tree_union (&tree, &tmp);
	      break;

	    case ENOENT:
	      rc = 0;
	      break;
	      
	    default:
	      mu_error ("%s", mu_strerror (rc));
	      mu_cfg_destroy_tree (&tree);
	      return rc;
	    }
	  free (file_name);
	}
    }
  
  if ((hints->flags & MU_CFHINT_CUSTOM_FILE) && hints->custom_file)
    {
      rc = mu_cfg_parse_file (&tmp, hints->custom_file, xhints.flags);
      if (rc)
	{
	  mu_error (_("errors parsing file %s: %s"), hints->custom_file,
		    mu_strerror (rc));
	  mu_cfg_destroy_tree (&tree);
	  return rc;
	}
      else
	{
	  mu_cfg_tree_postprocess (tmp, &xhints);
	  mu_cfg_tree_union (&tree, &tmp);
	}
    }

  *ptree = tree;
  return rc;
}

  
	  
        
  
