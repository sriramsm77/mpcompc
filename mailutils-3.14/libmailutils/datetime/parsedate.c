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
#define YYSTYPE         PD_YYSTYPE
/* Substitute the variable and function names.  */
#define yyparse         pd_yyparse
#define yylex           pd_yylex
#define yyerror         pd_yyerror
#define yydebug         pd_yydebug
#define yynerrs         pd_yynerrs

#define yylval          pd_yylval
#define yychar          pd_yychar

/* Copy the first part of user declarations.  */
#line 1 "parsedate.y" /* yacc.c:339  */

/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003-2022 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */
  
/* A heavily modified version of the well-known public domain getdate.y.
   It was originally written by Steven M. Bellovin <smb@research.att.com>
   while at the University of North Carolina at Chapel Hill.  Later tweaked
   by a couple of people on Usenet.  Completely overhauled by Rich $alz
   <rsalz@bbn.com> and Jim Berets <jberets@bbn.com> in August, 1990.
   Rewritten using a proper union by Sergey Poznyakoff <gray@gnu.org> */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/datetime.h>

#define ISSPACE(c) mu_isspace (c)
#define ISALPHA(c) mu_isalpha (c)
#define ISUPPER(c) mu_isupper (c)

static inline int ISDIGIT (unsigned c)
{
  return mu_isdigit (c);
}

static int yyparse (void); 
static int yylex (void);
static int yyerror (char *s);

#define EPOCH		1970
#define HOUR(x)		((x) * 60)

#define MAX_BUFF_LEN    128   /* size of buffer to read the date into */

/*
**  An entry in the lexical lookup table.
*/
typedef struct _lex_tab {
  const char	*name;
  int		type;
  int		value;
} SYMBOL;


/*
**  Meridian:  am, pm, or 24-hour style.
*/
typedef enum meridian {
  MERam,
  MERpm,
  MER24
} MERIDIAN;
 
#define MASK_IS_SET(f,m) (((f)&(m))==(m))
#define MASK_TEST(f,m)   ((f)&(m)) 
struct pd_date
{
  int mask;
  int day;
  int hour;
  int minute;
  int month;
  int second;
  int year;
  int tz;
  char const *tzname;
  enum meridian meridian;
  int number;
  int ordinal;
};

#define DATE_INIT(date) memset(&(date), 0, sizeof(date))
#define DATE_SET(date, memb, m, val, lim, onerror)                        \
 do                                                                       \
   {                                                                      \
     int __x = val;                                                       \
     if (((m) != MU_PD_MASK_TZ && __x < 0) || (lim && __x > lim)) onerror;\
     date . memb = __x; date.mask |= m;                                   \
   }                                                                      \
 while (0)
   
#define __SET_SECOND(d,v,a)   DATE_SET(d,second,MU_PD_MASK_SECOND,v,59,a)
#define __SET_MINUTE(d,v,a)   DATE_SET(d,minute,MU_PD_MASK_MINUTE,v,59,a)  
#define __SET_HOUR(d,v,a)     DATE_SET(d,hour,MU_PD_MASK_HOUR,v,23,a)
#define __SET_DAY(d,v,a)      DATE_SET(d,day,MU_PD_MASK_DAY,v,31,a)   
#define __SET_MONTH(d,v,a)    DATE_SET(d,month,MU_PD_MASK_MONTH,v,12,a)
#define __SET_YEAR(d,v,a)     DATE_SET(d,year,MU_PD_MASK_YEAR,v,0,a)  
#define __SET_TZ(d,v,a)       DATE_SET(d,tz,MU_PD_MASK_TZ,v,0,a)
#define __SET_MERIDIAN(d,v,a) DATE_SET(d,meridian,MU_PD_MASK_MERIDIAN,v,MER24,a)
#define __SET_ORDINAL(d,v,a)  DATE_SET(d,ordinal,MU_PD_MASK_ORDINAL,v,0,a)
#define __SET_NUMBER(d,v,a)   DATE_SET(d,number,MU_PD_MASK_NUMBER,v,0,a) 
 
#define SET_SECOND(d,v)   __SET_SECOND(d,v,YYERROR)   
#define SET_MINUTE(d,v)   __SET_MINUTE(d,v,YYERROR)   
#define SET_HOUR(d,v)     __SET_HOUR(d,v,YYERROR)     
#define SET_DAY(d,v)      __SET_DAY(d,v,YYERROR)      
#define SET_MONTH(d,v)    __SET_MONTH(d,v,YYERROR)    
#define SET_YEAR(d,v)     __SET_YEAR(d,v,YYERROR)     
#define SET_TZ(d,v)       __SET_TZ(d,v,YYERROR)
/* Set timezone from a packed representation (HHMM)

   The proper way of doing so would be:
   
#define SET_TZ_PACK(d,v)				  \
  SET_TZ (d, ((v) < 0 ? -(-(v) % 100 + (-(v) / 100) * 60) \
	              : ((v) % 100 + ((v) / 100) * 60)))

   However, we need to invert the sign in order for mktime
   to work properly (see mu_parse_date_dtl below).  The proper
   sign is then restored upon return from the function.
   
   Once mu_mktime is in place, this can be changed.
*/
#define SET_TZ_PACK(d,v)				  \
  SET_TZ (d, ((v) < 0 ? (-(v) % 100 + (-(v) / 100) * 60)  \
	              : -((v) % 100 + ((v) / 100) * 60)))

#define SET_MERIDIAN(d,v) __SET_MERIDIAN(d,v,YYERROR) 
#define SET_ORDINAL(d,v)  __SET_ORDINAL(d,v,YYERROR)  
#define SET_NUMBER(d,v)   __SET_NUMBER(d,v,YYERROR)   

int
pd_date_union (struct pd_date *a, struct pd_date *b)
{
  int diff = (~a->mask) & b->mask;
  if (!diff)
    return 1;

  a->mask |= diff;
  
  if (diff & MU_PD_MASK_SECOND)
    a->second = b->second;
  
  if (diff & MU_PD_MASK_MINUTE)
    a->minute = b->minute;

  if (diff & MU_PD_MASK_HOUR)
    a->hour = b->hour;

  if (diff & MU_PD_MASK_DAY)
    a->day = b->day;

  if (diff & MU_PD_MASK_MONTH)
    a->month = b->month;

  if (diff & MU_PD_MASK_YEAR)
    a->year = b->year;

  if (diff & MU_PD_MASK_TZ)
    a->tz = b->tz;

  if (diff & MU_PD_MASK_MERIDIAN)
    a->meridian = b->meridian;

  if (diff & MU_PD_MASK_ORDINAL)
    a->ordinal = b->ordinal;

  if (diff & MU_PD_MASK_NUMBER)
    a->number = b->number;

  return 0;
}

struct pd_datespec
{
  struct pd_date date;
  struct pd_date rel;
};

static struct pd_datespec pd;
 
static const char	*yyinput;


#line 269 "parsedate.c" /* yacc.c:339  */

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
#ifndef PD_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define PD_YYDEBUG 1
#  else
#   define PD_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define PD_YYDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined PD_YYDEBUG */
#if PD_YYDEBUG
extern int pd_yydebug;
#endif

/* Token type.  */
#ifndef PD_YYTOKENTYPE
# define PD_YYTOKENTYPE
  enum pd_yytokentype
  {
    T_AGO = 258,
    T_DST = 259,
    T_ID = 260,
    T_DAY = 261,
    T_DAY_UNIT = 262,
    T_HOUR_UNIT = 263,
    T_MINUTE_UNIT = 264,
    T_MONTH = 265,
    T_MONTH_UNIT = 266,
    T_SEC_UNIT = 267,
    T_SNUMBER = 268,
    T_UNUMBER = 269,
    T_YEAR_UNIT = 270,
    T_ZONE = 271,
    T_DAYZONE = 272,
    T_MERIDIAN = 273
  };
#endif

/* Value type.  */
#if ! defined PD_YYSTYPE && ! defined PD_YYSTYPE_IS_DECLARED

union PD_YYSTYPE
{
#line 195 "parsedate.y" /* yacc.c:355  */

  int number;
  enum meridian meridian;
  struct pd_date date;
  struct pd_datespec datespec;
  struct { char const *name; int delta; } tz;

#line 341 "parsedate.c" /* yacc.c:355  */
};

typedef union PD_YYSTYPE PD_YYSTYPE;
# define PD_YYSTYPE_IS_TRIVIAL 1
# define PD_YYSTYPE_IS_DECLARED 1
#endif


extern PD_YYSTYPE pd_yylval;

int pd_yyparse (void);



/* Copy the second part of user declarations.  */

#line 358 "parsedate.c" /* yacc.c:358  */

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
         || (defined PD_YYSTYPE_IS_TRIVIAL && PD_YYSTYPE_IS_TRIVIAL)))

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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   74

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  22
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  54
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  69

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

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
       2,     2,     2,     2,    20,     2,     2,    21,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    19,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18
};

#if PD_YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   220,   220,   227,   231,   237,   243,   286,   287,   288,
     289,   292,   298,   305,   313,   321,   332,   338,   344,   352,
     358,   364,   372,   378,   398,   406,   423,   429,   436,   442,
     449,   464,   474,   478,   484,   492,   497,   502,   507,   512,
     517,   522,   527,   532,   537,   542,   547,   552,   557,   562,
     567,   572,   577,   585,   588
};
#endif

#if PD_YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_AGO", "T_DST", "T_ID", "T_DAY",
  "T_DAY_UNIT", "T_HOUR_UNIT", "T_MINUTE_UNIT", "T_MONTH", "T_MONTH_UNIT",
  "T_SEC_UNIT", "T_SNUMBER", "T_UNUMBER", "T_YEAR_UNIT", "T_ZONE",
  "T_DAYZONE", "T_MERIDIAN", "':'", "','", "'/'", "$accept", "input",
  "spec", "item", "time", "zone", "day", "date", "rel", "relspec",
  "relunit", "o_merid", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,    58,
      44,    47
};
# endif

#define YYPACT_NINF -7

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-7)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -7,     8,    21,    -7,     1,    -7,    -7,    -7,    -4,    -7,
      -7,    32,    -6,    -7,    38,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,    11,    -7,    31,    -7,    34,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,     3,    -7,    -7,
      46,    -7,    -7,    48,    49,    -7,    -7,    41,    -7,    50,
      51,    -7,    -7,    -7,    42,    45,    39,    53,    -7,    -7,
      -7,    54,    -7,    55,    -7,    33,    -7,    -7,    -7
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,    19,    43,    46,    49,     0,    40,
      52,     0,     6,    37,    16,    17,     4,     7,     8,    10,
       9,     5,    32,    33,     0,    20,    26,    42,    45,    48,
      39,    51,    36,    21,    41,    44,    47,    28,    38,    50,
       0,    35,    11,     0,     0,    18,    31,     0,    34,     0,
       0,    25,    29,    24,    53,    22,     0,     0,    27,    13,
      54,     0,    12,     0,    30,    53,    23,    15,    14
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
      -7,    -7,    -7,    -7,    22,    -7,    -7,    -7,    -7,    -7,
      52,     5
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,    16,    17,    18,    19,    20,    21,    22,
      23,    62
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      33,    34,    35,    36,    37,    38,    39,    40,     3,    41,
      26,    24,    42,    43,    46,    44,    51,    52,     5,     6,
       7,    25,     9,    10,    11,    47,    13,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    27,
      28,    29,    45,    30,    31,    49,    67,    32,    34,    35,
      36,    60,    38,    39,    50,    59,    41,    42,    43,    53,
      60,    61,    54,    55,    56,    58,    63,    64,    65,    66,
      68,    57,     0,     0,    48
};

static const yytype_int8 yycheck[] =
{
       6,     7,     8,     9,    10,    11,    12,    13,     0,    15,
      14,    10,    18,    19,     3,    21,    13,    14,     7,     8,
       9,    20,    11,    12,    13,    14,    15,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     7,
       8,     9,     4,    11,    12,    14,    13,    15,     7,     8,
       9,    18,    11,    12,    20,    13,    15,    18,    19,    13,
      18,    19,    14,    14,    14,    14,    21,    14,    14,    14,
      65,    49,    -1,    -1,    22
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    23,    24,     0,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    25,    26,    27,    28,
      29,    30,    31,    32,    10,    20,    14,     7,     8,     9,
      11,    12,    15,     6,     7,     8,     9,    10,    11,    12,
      13,    15,    18,    19,    21,     4,     3,    14,    32,    14,
      20,    13,    14,    13,    14,    14,    14,    26,    14,    13,
      18,    19,    33,    21,    14,    14,    14,    13,    33
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    22,    23,    24,    24,    24,    24,    25,    25,    25,
      25,    26,    26,    26,    26,    26,    27,    27,    27,    28,
      28,    28,    29,    29,    29,    29,    29,    29,    29,    29,
      29,    30,    30,    31,    31,    32,    32,    32,    32,    32,
      32,    32,    32,    32,    32,    32,    32,    32,    32,    32,
      32,    32,    32,    33,    33
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     2,     1,     1,     1,
       1,     2,     4,     4,     6,     6,     1,     1,     2,     1,
       2,     2,     3,     5,     3,     3,     2,     4,     2,     3,
       5,     2,     1,     1,     2,     2,     2,     1,     2,     2,
       1,     2,     2,     1,     2,     2,     1,     2,     2,     1,
       2,     2,     1,     0,     1
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
#if PD_YYDEBUG

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
#else /* !PD_YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !PD_YYDEBUG */


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
#line 221 "parsedate.y" /* yacc.c:1646  */
    {
	    pd = (yyvsp[0].datespec);
	  }
#line 1482 "parsedate.c" /* yacc.c:1646  */
    break;

  case 3:
#line 227 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.datespec).date);
	    DATE_INIT ((yyval.datespec).rel);
	  }
#line 1491 "parsedate.c" /* yacc.c:1646  */
    break;

  case 4:
#line 232 "parsedate.y" /* yacc.c:1646  */
    {
	    if (pd_date_union (&(yyvsp[-1].datespec).date, &(yyvsp[0].date)))
	      YYERROR;
	    (yyval.datespec) = (yyvsp[-1].datespec);
	  }
#line 1501 "parsedate.c" /* yacc.c:1646  */
    break;

  case 5:
#line 238 "parsedate.y" /* yacc.c:1646  */
    {
	    if (pd_date_union (&(yyvsp[-1].datespec).rel, &(yyvsp[0].date)))
	      YYERROR;
	    (yyval.datespec) = (yyvsp[-1].datespec);
	  }
#line 1511 "parsedate.c" /* yacc.c:1646  */
    break;

  case 6:
#line 244 "parsedate.y" /* yacc.c:1646  */
    {
	    if (MASK_IS_SET ((yyvsp[-1].datespec).date.mask, (MU_PD_MASK_TIME|MU_PD_MASK_DATE))
		&& !(yyvsp[-1].datespec).rel.mask)
	      {
		if (MASK_IS_SET ((yyvsp[-1].datespec).date.mask, MU_PD_MASK_YEAR))
		  {
		    if (!MASK_IS_SET ((yyvsp[-1].datespec).date.mask, MU_PD_MASK_TZ))
		      SET_TZ_PACK ((yyvsp[-1].datespec).date, (yyvsp[0].number));
		    else
		      YYERROR;
		  }
		else
		  {
		    SET_YEAR ((yyvsp[-1].datespec).date, (yyvsp[0].number));
		  }
	      }
	    else
	      {
		if ((yyvsp[0].number) > 10000)
		  {
		    SET_DAY ((yyvsp[-1].datespec).date, (yyvsp[0].number) % 100);
		    SET_MONTH ((yyvsp[-1].datespec).date, ((yyvsp[0].number) / 100) %100);
		    SET_YEAR ((yyvsp[-1].datespec).date, (yyvsp[0].number) / 10000);
		  }
		else
		  {
		    if ((yyvsp[0].number) < 100)
		      {
			SET_YEAR ((yyvsp[-1].datespec).date, (yyvsp[0].number));
		      }
		    else
		      {
		    	SET_HOUR ((yyvsp[-1].datespec).date, (yyvsp[0].number) / 100);
		    	SET_MINUTE ((yyvsp[-1].datespec).date, (yyvsp[0].number) % 100);
		      }
		    SET_MERIDIAN ((yyvsp[-1].datespec).date, MER24);
		  }
	      }
	    (yyval.datespec) = (yyvsp[-1].datespec);
	  }
#line 1556 "parsedate.c" /* yacc.c:1646  */
    break;

  case 11:
#line 293 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_HOUR ((yyval.date), (yyvsp[-1].number));
	    SET_MERIDIAN ((yyval.date), (yyvsp[0].meridian));
	  }
#line 1566 "parsedate.c" /* yacc.c:1646  */
    break;

  case 12:
#line 299 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_HOUR ((yyval.date), (yyvsp[-3].number));
	    SET_MINUTE ((yyval.date), (yyvsp[-1].number));
	    SET_MERIDIAN ((yyval.date), (yyvsp[0].meridian));
	  }
#line 1577 "parsedate.c" /* yacc.c:1646  */
    break;

  case 13:
#line 306 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_HOUR ((yyval.date), (yyvsp[-3].number));
	    SET_MINUTE ((yyval.date), (yyvsp[-1].number));
	    SET_MERIDIAN ((yyval.date), MER24);
	    SET_TZ_PACK ((yyval.date), (yyvsp[0].number));
	  }
#line 1589 "parsedate.c" /* yacc.c:1646  */
    break;

  case 14:
#line 314 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_HOUR ((yyval.date), (yyvsp[-5].number));
	    SET_MINUTE ((yyval.date), (yyvsp[-3].number));
	    SET_SECOND ((yyval.date), (yyvsp[-1].number));
	    SET_MERIDIAN ((yyval.date), (yyvsp[0].meridian));
	  }
#line 1601 "parsedate.c" /* yacc.c:1646  */
    break;

  case 15:
#line 322 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_HOUR ((yyval.date), (yyvsp[-5].number));
	    SET_MINUTE ((yyval.date), (yyvsp[-3].number));
	    SET_SECOND ((yyval.date), (yyvsp[-1].number));
	    SET_MERIDIAN ((yyval.date), MER24);
	    SET_TZ_PACK ((yyval.date), (yyvsp[0].number));
	  }
#line 1614 "parsedate.c" /* yacc.c:1646  */
    break;

  case 16:
#line 333 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_TZ ((yyval.date), (yyvsp[0].tz).delta);
	    (yyval.date).tzname = (yyvsp[0].tz).name;
	  }
#line 1624 "parsedate.c" /* yacc.c:1646  */
    break;

  case 17:
#line 339 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_TZ ((yyval.date), (yyvsp[0].tz).delta - 60);
	    (yyval.date).tzname = (yyvsp[0].tz).name;
	  }
#line 1634 "parsedate.c" /* yacc.c:1646  */
    break;

  case 18:
#line 345 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_TZ ((yyval.date), (yyvsp[-1].tz).delta - 60);
	    (yyval.date).tzname = (yyvsp[-1].tz).name;
	  }
#line 1644 "parsedate.c" /* yacc.c:1646  */
    break;

  case 19:
#line 353 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_ORDINAL ((yyval.date), 1);
	    SET_NUMBER ((yyval.date), (yyvsp[0].number));
	  }
#line 1654 "parsedate.c" /* yacc.c:1646  */
    break;

  case 20:
#line 359 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_ORDINAL ((yyval.date), 1);
	    SET_NUMBER ((yyval.date), (yyvsp[-1].number));
	  }
#line 1664 "parsedate.c" /* yacc.c:1646  */
    break;

  case 21:
#line 365 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_ORDINAL ((yyval.date), (yyvsp[-1].number));
	    SET_NUMBER ((yyval.date), (yyvsp[0].number));
	  }
#line 1674 "parsedate.c" /* yacc.c:1646  */
    break;

  case 22:
#line 373 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MONTH ((yyval.date), (yyvsp[-2].number));
	    SET_DAY ((yyval.date), (yyvsp[0].number));
	  }
#line 1684 "parsedate.c" /* yacc.c:1646  */
    break;

  case 23:
#line 379 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    /* Interpret as YYYY/MM/DD if $1 >= 1000, otherwise as MM/DD/YY.
	       The goal in recognizing YYYY/MM/DD is solely to support legacy
	       machine-generated dates like those in an RCS log listing.  If
	       you want portability, use the ISO 8601 format.  */
	    if ((yyvsp[-4].number) >= 1000)
	      {
		SET_YEAR ((yyval.date), (yyvsp[-4].number));
		SET_MONTH ((yyval.date), (yyvsp[-2].number));
		SET_DAY ((yyval.date), (yyvsp[0].number));
	      }
	    else
	      {
		SET_MONTH ((yyval.date), (yyvsp[-4].number));
		SET_DAY ((yyval.date), (yyvsp[-2].number));
		SET_YEAR ((yyval.date), (yyvsp[0].number));
	      }
	  }
#line 1708 "parsedate.c" /* yacc.c:1646  */
    break;

  case 24:
#line 399 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    /* ISO 8601 format.  yyyy-mm-dd.  */
	    SET_YEAR ((yyval.date), (yyvsp[-2].number));
	    SET_MONTH ((yyval.date), -(yyvsp[-1].number));
	    SET_DAY ((yyval.date), -(yyvsp[0].number));
	  }
#line 1720 "parsedate.c" /* yacc.c:1646  */
    break;

  case 25:
#line 407 "parsedate.y" /* yacc.c:1646  */
    {
	    /* either 17-JUN-1992 or 1992-JUN-17 */
	    DATE_INIT ((yyval.date));
	    if ((yyvsp[-2].number) < 32)
	      {
		SET_DAY ((yyval.date), (yyvsp[-2].number));
		SET_MONTH ((yyval.date), (yyvsp[-1].number));
		SET_YEAR ((yyval.date), -(yyvsp[0].number));
	      }
	    else
	      {
		SET_DAY ((yyval.date), -(yyvsp[0].number));
		SET_MONTH ((yyval.date), (yyvsp[-1].number));
		SET_YEAR ((yyval.date), (yyvsp[-2].number));
	      }
	  }
#line 1741 "parsedate.c" /* yacc.c:1646  */
    break;

  case 26:
#line 424 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MONTH ((yyval.date), (yyvsp[-1].number));
	    SET_DAY ((yyval.date), (yyvsp[0].number));
	  }
#line 1751 "parsedate.c" /* yacc.c:1646  */
    break;

  case 27:
#line 430 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MONTH ((yyval.date), (yyvsp[-3].number));
	    SET_DAY ((yyval.date), (yyvsp[-2].number));
	    SET_YEAR ((yyval.date), (yyvsp[0].number));
	  }
#line 1762 "parsedate.c" /* yacc.c:1646  */
    break;

  case 28:
#line 437 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MONTH ((yyval.date), (yyvsp[0].number));
	    SET_DAY ((yyval.date), (yyvsp[-1].number));
	  }
#line 1772 "parsedate.c" /* yacc.c:1646  */
    break;

  case 29:
#line 443 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MONTH ((yyval.date), (yyvsp[-1].number));
	    SET_DAY ((yyval.date), (yyvsp[-2].number));
	    SET_YEAR ((yyval.date), (yyvsp[0].number));
	  }
#line 1783 "parsedate.c" /* yacc.c:1646  */
    break;

  case 30:
#line 450 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));

	    SET_ORDINAL ((yyval.date), 1);
	    SET_NUMBER ((yyval.date), (yyvsp[-4].number));

	    SET_MONTH ((yyval.date), (yyvsp[-3].number));
	    SET_DAY ((yyval.date), (yyvsp[-2].number));
	    SET_YEAR ((yyval.date), (yyvsp[0].number));
	    if (pd_date_union (&(yyval.date), &(yyvsp[-1].date)))
	      YYERROR;
	  }
#line 1800 "parsedate.c" /* yacc.c:1646  */
    break;

  case 31:
#line 465 "parsedate.y" /* yacc.c:1646  */
    {
	    (yyvsp[-1].date).second = - (yyvsp[-1].date).second;
	    (yyvsp[-1].date).minute = - (yyvsp[-1].date).minute;
	    (yyvsp[-1].date).hour = - (yyvsp[-1].date).hour;
	    (yyvsp[-1].date).day = - (yyvsp[-1].date).day;
	    (yyvsp[-1].date).month = - (yyvsp[-1].date).month;
	    (yyvsp[-1].date).year = - (yyvsp[-1].date).year;
	    (yyval.date) = (yyvsp[-1].date);
	  }
#line 1814 "parsedate.c" /* yacc.c:1646  */
    break;

  case 33:
#line 479 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    if (pd_date_union (&(yyval.date), &(yyvsp[0].date)))
	      YYERROR;
	  }
#line 1824 "parsedate.c" /* yacc.c:1646  */
    break;

  case 34:
#line 485 "parsedate.y" /* yacc.c:1646  */
    {
	    if (pd_date_union (&(yyvsp[-1].date), &(yyvsp[0].date)))
	      YYERROR;
	    (yyval.date) = (yyvsp[-1].date);
	  }
#line 1834 "parsedate.c" /* yacc.c:1646  */
    break;

  case 35:
#line 493 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_YEAR ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1843 "parsedate.c" /* yacc.c:1646  */
    break;

  case 36:
#line 498 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_YEAR ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1852 "parsedate.c" /* yacc.c:1646  */
    break;

  case 37:
#line 503 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_YEAR ((yyval.date), (yyvsp[0].number));
	  }
#line 1861 "parsedate.c" /* yacc.c:1646  */
    break;

  case 38:
#line 508 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MONTH ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1870 "parsedate.c" /* yacc.c:1646  */
    break;

  case 39:
#line 513 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MONTH ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1879 "parsedate.c" /* yacc.c:1646  */
    break;

  case 40:
#line 518 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MONTH ((yyval.date), (yyvsp[0].number));
	  }
#line 1888 "parsedate.c" /* yacc.c:1646  */
    break;

  case 41:
#line 523 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_DAY ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1897 "parsedate.c" /* yacc.c:1646  */
    break;

  case 42:
#line 528 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_DAY ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1906 "parsedate.c" /* yacc.c:1646  */
    break;

  case 43:
#line 533 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_DAY ((yyval.date), (yyvsp[0].number));
	  }
#line 1915 "parsedate.c" /* yacc.c:1646  */
    break;

  case 44:
#line 538 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_HOUR ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1924 "parsedate.c" /* yacc.c:1646  */
    break;

  case 45:
#line 543 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_HOUR ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1933 "parsedate.c" /* yacc.c:1646  */
    break;

  case 46:
#line 548 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_HOUR ((yyval.date), (yyvsp[0].number));
	  }
#line 1942 "parsedate.c" /* yacc.c:1646  */
    break;

  case 47:
#line 553 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MINUTE ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1951 "parsedate.c" /* yacc.c:1646  */
    break;

  case 48:
#line 558 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MINUTE ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1960 "parsedate.c" /* yacc.c:1646  */
    break;

  case 49:
#line 563 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_MINUTE ((yyval.date), (yyvsp[0].number));
	  }
#line 1969 "parsedate.c" /* yacc.c:1646  */
    break;

  case 50:
#line 568 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_SECOND ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1978 "parsedate.c" /* yacc.c:1646  */
    break;

  case 51:
#line 573 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_SECOND ((yyval.date), (yyvsp[-1].number) * (yyvsp[0].number));
	  }
#line 1987 "parsedate.c" /* yacc.c:1646  */
    break;

  case 52:
#line 578 "parsedate.y" /* yacc.c:1646  */
    {
	    DATE_INIT ((yyval.date));
	    SET_SECOND ((yyval.date), (yyvsp[0].number));
	  }
#line 1996 "parsedate.c" /* yacc.c:1646  */
    break;

  case 53:
#line 585 "parsedate.y" /* yacc.c:1646  */
    {
	    (yyval.meridian) = MER24;
	  }
#line 2004 "parsedate.c" /* yacc.c:1646  */
    break;

  case 54:
#line 589 "parsedate.y" /* yacc.c:1646  */
    {
	    (yyval.meridian) = (yyvsp[0].meridian);
	  }
#line 2012 "parsedate.c" /* yacc.c:1646  */
    break;


#line 2016 "parsedate.c" /* yacc.c:1646  */
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
#line 594 "parsedate.y" /* yacc.c:1906  */


#include <mailutils/types.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <mailutils/util.h>

/* Month and day table. */
static SYMBOL const month_day_tab[] = {
  { "january",	T_MONTH,  1 },
  { "february",	T_MONTH,  2 },
  { "march",	T_MONTH,  3 },
  { "april",	T_MONTH,  4 },
  { "may",	T_MONTH,  5 },
  { "june",	T_MONTH,  6 },
  { "july",	T_MONTH,  7 },
  { "august",	T_MONTH,  8 },
  { "september",T_MONTH,  9 },
  { "sept",	T_MONTH,  9 },
  { "october",	T_MONTH, 10 },
  { "november",	T_MONTH, 11 },
  { "december",	T_MONTH, 12 },
  { "sunday",	T_DAY,   0 },
  { "monday",	T_DAY,   1 },
  { "tuesday",	T_DAY,   2 },
  { "tues",	T_DAY,   2 },
  { "wednesday",T_DAY,   3 },
  { "wednes",	T_DAY,   3 },
  { "thursday",	T_DAY,   4 },
  { "thur",	T_DAY,   4 },
  { "thurs",	T_DAY,   4 },
  { "friday",	T_DAY,   5 },
  { "saturday",	T_DAY,   6 },
  { NULL, 0, 0 }
};

/* Time units table. */
static SYMBOL const units_tab[] = {
  { "year",	T_YEAR_UNIT,	1 },
  { "month",	T_MONTH_UNIT,	1 },
  { "fortnight",T_DAY_UNIT,	14 },
  { "week",	T_DAY_UNIT,	7 },
  { "day",	T_DAY_UNIT,	1 },
  { "hour",	T_HOUR_UNIT,	1 },
  { "minute",	T_MINUTE_UNIT,	1 },
  { "min",	T_MINUTE_UNIT,	1 },
  { "second",	T_SEC_UNIT,	1 },
  { "sec",	T_SEC_UNIT,	1 },
  { NULL, 0, 0 }
};

/* Assorted relative-time words. */
static SYMBOL const other_tab[] = {
  { "tomorrow",	T_MINUTE_UNIT,	1 * 24 * 60 },
  { "yesterday",T_MINUTE_UNIT,	-1 * 24 * 60 },
  { "today",	T_MINUTE_UNIT,	0 },
  { "now",	T_MINUTE_UNIT,	0 },
  { "last",	T_UNUMBER,	-1 },
  { "this",	T_MINUTE_UNIT,	0 },
  { "next",	T_UNUMBER,	1 },
  { "first",	T_UNUMBER,	1 },
/*  { "second",	T_UNUMBER,	2 }, */
  { "third",	T_UNUMBER,	3 },
  { "fourth",	T_UNUMBER,	4 },
  { "fifth",	T_UNUMBER,	5 },
  { "sixth",	T_UNUMBER,	6 },
  { "seventh",	T_UNUMBER,	7 },
  { "eighth",	T_UNUMBER,	8 },
  { "ninth",	T_UNUMBER,	9 },
  { "tenth",	T_UNUMBER,	10 },
  { "eleventh",	T_UNUMBER,	11 },
  { "twelfth",	T_UNUMBER,	12 },
  { "ago",	T_AGO,	        1 },
  { NULL, 0, 0 }
};

/* The timezone table. */
static SYMBOL const tz_tab[] = {
  { "gmt",	T_ZONE,     HOUR ( 0) },	/* Greenwich Mean */
  { "ut",	T_ZONE,     HOUR ( 0) },	/* Universal (Coordinated) */
  { "utc",	T_ZONE,     HOUR ( 0) },
  { "wet",	T_ZONE,     HOUR ( 0) },	/* Western European */
  { "bst",	T_DAYZONE,  HOUR ( 0) },	/* British Summer */
  { "wat",	T_ZONE,     HOUR ( 1) },	/* West Africa */
  { "at",	T_ZONE,     HOUR ( 2) },	/* Azores */
#if	0
  /* For completeness.  BST is also British Summer, and GST is
   * also Guam Standard. */
  { "bst",	T_ZONE,     HOUR ( 3) },	/* Brazil Standard */
  { "gst",	T_ZONE,     HOUR ( 3) },	/* Greenland Standard */
#endif
#if 0
  { "nft",	T_ZONE,     HOUR (3.5) },	/* Newfoundland */
  { "nst",	T_ZONE,     HOUR (3.5) },	/* Newfoundland Standard */
  { "ndt",	T_DAYZONE,  HOUR (3.5) },	/* Newfoundland Daylight */
#endif
  { "ast",	T_ZONE,     HOUR ( 4) },	/* Atlantic Standard */
  { "adt",	T_DAYZONE,  HOUR ( 4) },	/* Atlantic Daylight */
  { "est",	T_ZONE,     HOUR ( 5) },	/* Eastern Standard */
  { "edt",	T_DAYZONE,  HOUR ( 5) },	/* Eastern Daylight */
  { "cst",	T_ZONE,     HOUR ( 6) },	/* Central Standard */
  { "cdt",	T_DAYZONE,  HOUR ( 6) },	/* Central Daylight */
  { "mst",	T_ZONE,     HOUR ( 7) },	/* Mountain Standard */
  { "mdt",	T_DAYZONE,  HOUR ( 7) },	/* Mountain Daylight */
  { "pst",	T_ZONE,     HOUR ( 8) },	/* Pacific Standard */
  { "pdt",	T_DAYZONE,  HOUR ( 8) },	/* Pacific Daylight */
  { "yst",	T_ZONE,     HOUR ( 9) },	/* Yukon Standard */
  { "ydt",	T_DAYZONE,  HOUR ( 9) },	/* Yukon Daylight */
  { "hst",	T_ZONE,     HOUR (10) },	/* Hawaii Standard */
  { "hdt",	T_DAYZONE,  HOUR (10) },	/* Hawaii Daylight */
  { "cat",	T_ZONE,     HOUR (10) },	/* Central Alaska */
  { "ahst",	T_ZONE,     HOUR (10) },	/* Alaska-Hawaii Standard */
  { "nt",	T_ZONE,     HOUR (11) },	/* Nome */
  { "idlw",	T_ZONE,     HOUR (12) },	/* International Date Line West */
  { "cet",	T_ZONE,     -HOUR (1) },	/* Central European */
  { "met",	T_ZONE,     -HOUR (1) },	/* Middle European */
  { "mewt",	T_ZONE,     -HOUR (1) },	/* Middle European Winter */
  { "mest",	T_DAYZONE,  -HOUR (1) },	/* Middle European Summer */
  { "mesz",	T_DAYZONE,  -HOUR (1) },	/* Middle European Summer */
  { "swt",	T_ZONE,     -HOUR (1) },	/* Swedish Winter */
  { "sst",	T_DAYZONE,  -HOUR (1) },	/* Swedish Summer */
  { "fwt",	T_ZONE,     -HOUR (1) },	/* French Winter */
  { "fst",	T_DAYZONE,  -HOUR (1) },	/* French Summer */
  { "eet",	T_ZONE,     -HOUR (2) },	/* Eastern Europe, USSR Zone 1 */
  { "bt",	T_ZONE,     -HOUR (3) },	/* Baghdad, USSR Zone 2 */
#if 0
  { "it",	T_ZONE,     -HOUR (3.5) },/* Iran */
#endif
  { "zp4",	T_ZONE,     -HOUR (4) },	/* USSR Zone 3 */
  { "zp5",	T_ZONE,     -HOUR (5) },	/* USSR Zone 4 */
#if 0
  { "ist",	T_ZONE,     -HOUR (5.5) },/* Indian Standard */
#endif
  { "zp6",	T_ZONE,     -HOUR (6) },	/* USSR Zone 5 */
#if	0
  /* For completeness.  NST is also Newfoundland Standard, and SST is
   * also Swedish Summer. */
  { "nst",	T_ZONE,     -HOUR (6.5) },/* North Sumatra */
  { "sst",	T_ZONE,     -HOUR (7) },	/* South Sumatra, USSR Zone 6 */
#endif	/* 0 */
  { "wast",	T_ZONE,     -HOUR (7) },	/* West Australian Standard */
  { "wadt",	T_DAYZONE,  -HOUR (7) },	/* West Australian Daylight */
#if 0
  { "jt",	T_ZONE,     -HOUR (7.5) },/* Java (3pm in Cronusland!) */
#endif
  { "cct",	T_ZONE,     -HOUR (8) },	/* China Coast, USSR Zone 7 */
  { "jst",	T_ZONE,     -HOUR (9) },	/* Japan Standard, USSR Zone 8 */
#if 0
  { "cast",	T_ZONE,     -HOUR (9.5) },/* Central Australian Standard */
  { "cadt",	T_DAYZONE,  -HOUR (9.5) },/* Central Australian Daylight */
#endif
  { "east",	T_ZONE,     -HOUR (10) },	/* Eastern Australian Standard */
  { "eadt",	T_DAYZONE,  -HOUR (10) },	/* Eastern Australian Daylight */
  { "gst",	T_ZONE,     -HOUR (10) },	/* Guam Standard, USSR Zone 9 */
  { "nzt",	T_ZONE,     -HOUR (12) },	/* New Zealand */
  { "nzst",	T_ZONE,     -HOUR (12) },	/* New Zealand Standard */
  { "nzdt",	T_DAYZONE,  -HOUR (12) },	/* New Zealand Daylight */
  { "idle",	T_ZONE,     -HOUR (12) },	/* International Date Line
						   East */
  {  NULL, 0, 0  }
};

/* Military timezone table. */
static SYMBOL const mil_tz_tab[] = {
  { "a",	T_ZONE,	HOUR (  1) },
  { "b",	T_ZONE,	HOUR (  2) },
  { "c",	T_ZONE,	HOUR (  3) },
  { "d",	T_ZONE,	HOUR (  4) },
  { "e",	T_ZONE,	HOUR (  5) },
  { "f",	T_ZONE,	HOUR (  6) },
  { "g",	T_ZONE,	HOUR (  7) },
  { "h",	T_ZONE,	HOUR (  8) },
  { "i",	T_ZONE,	HOUR (  9) },
  { "k",	T_ZONE,	HOUR ( 10) },
  { "l",	T_ZONE,	HOUR ( 11) },
  { "m",	T_ZONE,	HOUR ( 12) },
  { "n",	T_ZONE,	HOUR (- 1) },
  { "o",	T_ZONE,	HOUR (- 2) },
  { "p",	T_ZONE,	HOUR (- 3) },
  { "q",	T_ZONE,	HOUR (- 4) },
  { "r",	T_ZONE,	HOUR (- 5) },
  { "s",	T_ZONE,	HOUR (- 6) },
  { "t",	T_ZONE,	HOUR (- 7) },
  { "u",	T_ZONE,	HOUR (- 8) },
  { "v",	T_ZONE,	HOUR (- 9) },
  { "w",	T_ZONE,	HOUR (-10) },
  { "x",	T_ZONE,	HOUR (-11) },
  { "y",	T_ZONE,	HOUR (-12) },
  { "z",	T_ZONE,	HOUR (  0) },
  { NULL, 0, 0 }
};




/* ARGSUSED */
static int
yyerror (char *s MU_ARG_UNUSED)
{
  return 0;
}

static int
norm_hour (int hours, MERIDIAN meridian)
{
  switch (meridian)
    {
    case MER24:
      if (hours < 0 || hours > 23)
	return -1;
      return hours;
      
    case MERam:
      if (hours < 1 || hours > 12)
	return -1;
      if (hours == 12)
	hours = 0;
      return hours;
      
    case MERpm:
      if (hours < 1 || hours > 12)
	return -1;
      if (hours == 12)
	hours = 0;
      return hours + 12;
      
    default:
      abort ();
    }
  /* NOTREACHED */
}

static int
norm_year (int year)
{
  if (year < 0)
    year = -year;
  
  /* XPG4 suggests that years 00-68 map to 2000-2068, and
     years 69-99 map to 1969-1999.  */
  if (year < 69)
    year += 2000;
  else if (year < 100)
    year += 1900;

  return year;
}

static int
sym_lookup (char *buff)
{
  register char *p;
  register char *q;
  register const SYMBOL *tp;
  int i;
  int abbrev;
  
  /* Make it lowercase. */
  mu_strlower (buff);
  
  if (strcmp (buff, "am") == 0 || strcmp (buff, "a.m.") == 0)
    {
      yylval.meridian = MERam;
      return T_MERIDIAN;
    }
  if (strcmp (buff, "pm") == 0 || strcmp (buff, "p.m.") == 0)
    {
      yylval.meridian = MERpm;
      return T_MERIDIAN;
    }
  
  /* See if we have an abbreviation for a month. */
  if (strlen (buff) == 3)
    abbrev = 1;
  else if (strlen (buff) == 4 && buff[3] == '.')
    {
      abbrev = 1;
      buff[3] = '\0';
    }
  else
    abbrev = 0;

  for (tp = month_day_tab; tp->name; tp++)
    {
      if (abbrev)
	{
	  if (strncmp (buff, tp->name, 3) == 0)
	    {
	      yylval.number = tp->value;
	      return tp->type;
	    }
	}
      else if (strcmp (buff, tp->name) == 0)
	{
	  yylval.number = tp->value;
	  return tp->type;
	}
    }

  for (tp = tz_tab; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
        yylval.tz.name = tp->name;
	yylval.tz.delta = tp->value;
	return tp->type;
      }

  if (strcmp (buff, "dst") == 0)
    return T_DST;

  for (tp = units_tab; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
	yylval.number = tp->value;
	return tp->type;
      }

  /* Strip off any plural and try the units table again. */
  i = strlen (buff) - 1;
  if (buff[i] == 's')
    {
      buff[i] = '\0';
      for (tp = units_tab; tp->name; tp++)
	if (strcmp (buff, tp->name) == 0)
	  {
	    yylval.number = tp->value;
	    return tp->type;
	  }
      buff[i] = 's';		/* Put back for "this" in other_tab. */
    }

  for (tp = other_tab; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
	yylval.number = tp->value;
	return tp->type;
      }

  /* Military timezones. */
  if (buff[1] == '\0' && ISALPHA ((unsigned char) *buff))
    {
      for (tp = mil_tz_tab; tp->name; tp++)
	if (strcmp (buff, tp->name) == 0)
	  {
            yylval.tz.name = tp->name;
	    yylval.tz.delta = tp->value;
	    return tp->type;
	  }
    }

  /* Drop out any periods and try the timezone table again. */
  for (i = 0, p = q = buff; *q; q++)
    if (*q != '.')
      *p++ = *q;
    else
      i++;
  *p = '\0';
  if (i)
    for (tp = tz_tab; tp->name; tp++)
      if (strcmp (buff, tp->name) == 0)
	{
	  yylval.number = tp->value;
	  return tp->type;
	}

  return T_ID;
}

static int
yylex (void)
{
  register unsigned char c;
  register char *p;
  char buff[20];
  int count;
  int sign;

  for (;;)
    {
      while (ISSPACE ((unsigned char) *yyinput))
	yyinput++;

      if (ISDIGIT (c = *yyinput) || c == '-' || c == '+')
	{
	  if (c == '-' || c == '+')
	    {
	      sign = c == '-' ? -1 : 1;
	      if (!ISDIGIT (*++yyinput))
		/* skip the '-' sign */
		continue;
	    }
	  else
	    sign = 0;
	  for (yylval.number = 0; ISDIGIT (c = *yyinput++);)
	    yylval.number = 10 * yylval.number + c - '0';
	  yyinput--;
	  if (sign < 0)
	    yylval.number = -yylval.number;
	  return sign ? T_SNUMBER : T_UNUMBER;
	}
      if (ISALPHA (c))
	{
	  for (p = buff; (c = *yyinput++, ISALPHA (c)) || c == '.';)
	    if (p < &buff[sizeof buff - 1])
	      *p++ = c;
	  *p = '\0';
	  yyinput--;
	  return sym_lookup (buff);
	}
      if (c != '(')
	return *yyinput++;
      count = 0;
      do
	{
	  c = *yyinput++;
	  if (c == '\0')
	    return c;
	  if (c == '(')
	    count++;
	  else if (c == ')')
	    count--;
	}
      while (count > 0);
    }
}

#define TM_YEAR_ORIGIN 1900

/* Yield A - B, measured in seconds.  */
static long
difftm (struct tm *a, struct tm *b)
{
  int ay = a->tm_year + (TM_YEAR_ORIGIN - 1);
  int by = b->tm_year + (TM_YEAR_ORIGIN - 1);
  long days = (
  /* difference in day of year */
		a->tm_yday - b->tm_yday
  /* + intervening leap days */
		+ ((ay >> 2) - (by >> 2))
		- (ay / 100 - by / 100)
		+ ((ay / 100 >> 2) - (by / 100 >> 2))
  /* + difference in years * 365 */
		+ (long) (ay - by) * 365
  );
  return (60 * (60 * (24 * days + (a->tm_hour - b->tm_hour))
		+ (a->tm_min - b->tm_min))
	  + (a->tm_sec - b->tm_sec));
}

int
mu_parse_date_dtl (const char *p, const time_t *now, 
		   time_t *rettime,
		   struct tm *rettm,
		   struct mu_timezone *rettz,
		   int *retflags)
{
  struct tm tm, tm0, *tmp;
  time_t start;

  yyinput = p;
  start = now ? *now : time ((time_t *) NULL);
  tmp = localtime (&start);
  if (!tmp)
    return -1;

  memset (&tm, 0, sizeof tm);
  tm.tm_isdst = tmp->tm_isdst;

  if (yyparse ())
    return -1;
  
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_YEAR))
    __SET_YEAR (pd.date, tmp->tm_year + TM_YEAR_ORIGIN, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_MONTH))
    __SET_MONTH (pd.date, tmp->tm_mon + 1, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_DAY))
    __SET_DAY (pd.date, tmp->tm_mday, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_HOUR))
    __SET_HOUR (pd.date, tmp->tm_hour, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_MERIDIAN))
    __SET_MERIDIAN (pd.date, MER24, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_MINUTE))
    __SET_MINUTE (pd.date, tmp->tm_min, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_SECOND))
    __SET_SECOND (pd.date, tmp->tm_sec, return -1);
  
  tm.tm_year = norm_year (pd.date.year) - TM_YEAR_ORIGIN + pd.rel.year;
  tm.tm_mon = pd.date.month - 1 + pd.rel.month;
  tm.tm_mday = pd.date.day + pd.rel.day;
  if (MASK_TEST (pd.date.mask, MU_PD_MASK_TIME)
      || (pd.rel.mask && !MASK_TEST (pd.date.mask, MU_PD_MASK_DATE)
	  && !MASK_TEST (pd.date.mask, MU_PD_MASK_DOW)))
    {
      tm.tm_hour = norm_hour (pd.date.hour, pd.date.meridian);
      if (tm.tm_hour < 0)
	return -1;
      tm.tm_min = pd.date.minute;
      tm.tm_sec = pd.date.second;
    }
  else
    {
      tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
    }
  tm.tm_hour += pd.rel.hour;
  tm.tm_min += pd.rel.minute;
  tm.tm_sec += pd.rel.second;

  /* Let mktime deduce tm_isdst if we have an absolute timestamp,
     or if the relative timestamp mentions days, months, or years.  */
  if (MASK_TEST (pd.date.mask, MU_PD_MASK_DATE | MU_PD_MASK_DOW | MU_PD_MASK_TIME)
      || MASK_TEST (pd.rel.mask, MU_PD_MASK_DOW | MU_PD_MASK_MONTH | MU_PD_MASK_YEAR))
    tm.tm_isdst = -1;

  tm0 = tm;

  start = mktime (&tm);

  if (start == (time_t) -1)
    {

      /* Guard against falsely reporting errors near the time_t boundaries
         when parsing times in other time zones.  For example, if the min
         time_t value is 1970-01-01 00:00:00 UTC and we are 8 hours ahead
         of UTC, then the min localtime value is 1970-01-01 08:00:00; if
         we apply mktime to 1970-01-01 00:00:00 we will get an error, so
         we apply mktime to 1970-01-02 08:00:00 instead and adjust the time
         zone by 24 hours to compensate.  This algorithm assumes that
         there is no DST transition within a day of the time_t boundaries.  */
      if (MASK_TEST (pd.date.mask, MU_PD_MASK_TZ))
	{
	  tm = tm0;
	  if (tm.tm_year <= EPOCH - TM_YEAR_ORIGIN)
	    {
	      tm.tm_mday++;
	      pd.date.tz -= 24 * 60;
	    }
	  else
	    {
	      tm.tm_mday--;
	      pd.date.tz += 24 * 60;
	    }
	  start = mktime (&tm);
	}

      if (start == (time_t) -1)
	return -1;
    }

  if (MASK_TEST (pd.date.mask, MU_PD_MASK_DOW)
      && !MASK_TEST (pd.date.mask, MU_PD_MASK_DATE))
    {
      tm.tm_mday += ((pd.date.number - tm.tm_wday + 7) % 7
		     + 7 * (pd.date.ordinal - (0 < pd.date.ordinal)));
      start = mktime (&tm);
      if (start == (time_t) -1)
	return -1;
    }
  
  if (MASK_TEST (pd.date.mask, MU_PD_MASK_TZ))
    {
      long delta;
      struct tm *gmt = gmtime (&start);
      if (gmt)
	{
	  delta = pd.date.tz * 60L + difftm (&tm, gmt);
	  if ((start + delta < start) != (delta < 0))
	    return -1;		/* time_t overflow */
	  start += delta;
	}
    }

  if (MASK_TEST (pd.date.mask, MU_PD_MASK_TZ))
    {
      pd.date.tz = - pd.date.tz * 60L;
      if (!pd.date.tzname)
	pd.date.tzname = tm.tm_isdst != -1 ? tzname[tm.tm_isdst] : NULL;
#if HAVE_STRUCT_TM_TM_GMTOFF
      tm.tm_gmtoff = pd.date.tz;
#endif
#if HAVE_STRUCT_TM_TM_ZONE 	
      tm.tm_zone = pd.date.tzname;
#endif
    }
  if (rettime)
    *rettime = start;
  if (rettm)
    *rettm = tm;
  if (rettz)
    {
      if (MASK_TEST (pd.date.mask, MU_PD_MASK_TZ))
	{
	  rettz->utc_offset = pd.date.tz;
	  rettz->tz_name = pd.date.tzname;
	}
      else
	{
	  mu_datetime_tz_local (rettz);
	}
    }
  if (retflags)
    *retflags = pd.date.mask;
  return 0;
}

int
mu_parse_date (const char *p, time_t *rettime, const time_t *now)
{
  return mu_parse_date_dtl (p, now, rettime, NULL, NULL, NULL);
}

int
mu_timezone_offset (const char *buf, int *off)
{
  SYMBOL const *tp;
  
  for (tp = tz_tab; tp->name; tp++)
    if (mu_c_strcasecmp (buf, tp->name) == 0)
      {
	*off = - tp->value * 60;
	return 0;
      }
  return -1;
}

#ifdef STANDALONE
int
main (int argc, char *argv[])
{
  char buff[MAX_BUFF_LEN + 1];
  time_t d;

  if (argc > 1 && strcmp (argv[1], "-d") == 0)
    yydebug++;
  printf ("Enter date, or blank line to exit.\n\t> ");
  fflush (stdout);

  buff[MAX_BUFF_LEN] = 0;
  while (fgets (buff, MAX_BUFF_LEN, stdin) && buff[0])
    {
      if (mu_parse_date (buff, &d, NULL))
	printf ("Bad format - couldn't convert.\n");
      else
	printf ("%s", ctime (&d));
      printf ("\t> ");
      fflush (stdout);
    }
  exit (0);
  /* NOTREACHED */
}

#endif
