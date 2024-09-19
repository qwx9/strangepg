/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 25 "awkgram.y"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "awk.h"

void checkdup(Node *list, Cell *item);
int yywrap(void) { return(1); }

Node	*beginloc = 0;
Node	*endloc = 0;
bool	infunc	= false;	/* = true if in arglist or body of func */
int	inloop	= 0;	/* >= 1 if in while, for, do; can't be bool, since loops can next */
char	*curfname = 0;	/* current function name */
Node	*arglist = 0;	/* list of args for current function */

#line 88 "awkgram.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "awkgram.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_FIRSTTOKEN = 3,                 /* FIRSTTOKEN  */
  YYSYMBOL_PROGRAM = 4,                    /* PROGRAM  */
  YYSYMBOL_PASTAT = 5,                     /* PASTAT  */
  YYSYMBOL_PASTAT2 = 6,                    /* PASTAT2  */
  YYSYMBOL_XBEGIN = 7,                     /* XBEGIN  */
  YYSYMBOL_XEND = 8,                       /* XEND  */
  YYSYMBOL_NL = 9,                         /* NL  */
  YYSYMBOL_10_ = 10,                       /* ','  */
  YYSYMBOL_11_ = 11,                       /* '{'  */
  YYSYMBOL_12_ = 12,                       /* '('  */
  YYSYMBOL_13_ = 13,                       /* ';'  */
  YYSYMBOL_14_ = 14,                       /* '/'  */
  YYSYMBOL_15_ = 15,                       /* ')'  */
  YYSYMBOL_16_ = 16,                       /* '}'  */
  YYSYMBOL_17_ = 17,                       /* '['  */
  YYSYMBOL_18_ = 18,                       /* ']'  */
  YYSYMBOL_ARRAY = 19,                     /* ARRAY  */
  YYSYMBOL_MATCH = 20,                     /* MATCH  */
  YYSYMBOL_NOTMATCH = 21,                  /* NOTMATCH  */
  YYSYMBOL_MATCHOP = 22,                   /* MATCHOP  */
  YYSYMBOL_FINAL = 23,                     /* FINAL  */
  YYSYMBOL_DOT = 24,                       /* DOT  */
  YYSYMBOL_ALL = 25,                       /* ALL  */
  YYSYMBOL_CCL = 26,                       /* CCL  */
  YYSYMBOL_NCCL = 27,                      /* NCCL  */
  YYSYMBOL_CHAR = 28,                      /* CHAR  */
  YYSYMBOL_OR = 29,                        /* OR  */
  YYSYMBOL_STAR = 30,                      /* STAR  */
  YYSYMBOL_QUEST = 31,                     /* QUEST  */
  YYSYMBOL_PLUS = 32,                      /* PLUS  */
  YYSYMBOL_EMPTYRE = 33,                   /* EMPTYRE  */
  YYSYMBOL_ZERO = 34,                      /* ZERO  */
  YYSYMBOL_LAND = 35,                      /* LAND  */
  YYSYMBOL_LOR = 36,                       /* LOR  */
  YYSYMBOL_EQ = 37,                        /* EQ  */
  YYSYMBOL_GE = 38,                        /* GE  */
  YYSYMBOL_GT = 39,                        /* GT  */
  YYSYMBOL_LE = 40,                        /* LE  */
  YYSYMBOL_LT = 41,                        /* LT  */
  YYSYMBOL_NE = 42,                        /* NE  */
  YYSYMBOL_IN = 43,                        /* IN  */
  YYSYMBOL_ARG = 44,                       /* ARG  */
  YYSYMBOL_BLTIN = 45,                     /* BLTIN  */
  YYSYMBOL_BREAK = 46,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 47,                  /* CONTINUE  */
  YYSYMBOL_DELETE = 48,                    /* DELETE  */
  YYSYMBOL_DO = 49,                        /* DO  */
  YYSYMBOL_EXIT = 50,                      /* EXIT  */
  YYSYMBOL_FOR = 51,                       /* FOR  */
  YYSYMBOL_FUNC = 52,                      /* FUNC  */
  YYSYMBOL_SUB = 53,                       /* SUB  */
  YYSYMBOL_GSUB = 54,                      /* GSUB  */
  YYSYMBOL_IF = 55,                        /* IF  */
  YYSYMBOL_INDEX = 56,                     /* INDEX  */
  YYSYMBOL_LSUBSTR = 57,                   /* LSUBSTR  */
  YYSYMBOL_MATCHFCN = 58,                  /* MATCHFCN  */
  YYSYMBOL_NEXT = 59,                      /* NEXT  */
  YYSYMBOL_ADD = 60,                       /* ADD  */
  YYSYMBOL_MINUS = 61,                     /* MINUS  */
  YYSYMBOL_MULT = 62,                      /* MULT  */
  YYSYMBOL_DIVIDE = 63,                    /* DIVIDE  */
  YYSYMBOL_MOD = 64,                       /* MOD  */
  YYSYMBOL_LSHIFT = 65,                    /* LSHIFT  */
  YYSYMBOL_RSHIFT = 66,                    /* RSHIFT  */
  YYSYMBOL_XOR = 67,                       /* XOR  */
  YYSYMBOL_BAND = 68,                      /* BAND  */
  YYSYMBOL_BOR = 69,                       /* BOR  */
  YYSYMBOL_CMPL = 70,                      /* CMPL  */
  YYSYMBOL_ASSIGN = 71,                    /* ASSIGN  */
  YYSYMBOL_ASGNOP = 72,                    /* ASGNOP  */
  YYSYMBOL_ADDEQ = 73,                     /* ADDEQ  */
  YYSYMBOL_SUBEQ = 74,                     /* SUBEQ  */
  YYSYMBOL_MULTEQ = 75,                    /* MULTEQ  */
  YYSYMBOL_DIVEQ = 76,                     /* DIVEQ  */
  YYSYMBOL_MODEQ = 77,                     /* MODEQ  */
  YYSYMBOL_POWEQ = 78,                     /* POWEQ  */
  YYSYMBOL_BANDEQ = 79,                    /* BANDEQ  */
  YYSYMBOL_BOREQ = 80,                     /* BOREQ  */
  YYSYMBOL_XOREQ = 81,                     /* XOREQ  */
  YYSYMBOL_SHLEQ = 82,                     /* SHLEQ  */
  YYSYMBOL_SHREQ = 83,                     /* SHREQ  */
  YYSYMBOL_PRINT = 84,                     /* PRINT  */
  YYSYMBOL_PRINTF = 85,                    /* PRINTF  */
  YYSYMBOL_SPRINTF = 86,                   /* SPRINTF  */
  YYSYMBOL_ELSE = 87,                      /* ELSE  */
  YYSYMBOL_INTEST = 88,                    /* INTEST  */
  YYSYMBOL_CONDEXPR = 89,                  /* CONDEXPR  */
  YYSYMBOL_POSTINCR = 90,                  /* POSTINCR  */
  YYSYMBOL_PREINCR = 91,                   /* PREINCR  */
  YYSYMBOL_POSTDECR = 92,                  /* POSTDECR  */
  YYSYMBOL_PREDECR = 93,                   /* PREDECR  */
  YYSYMBOL_VAR = 94,                       /* VAR  */
  YYSYMBOL_IVAR = 95,                      /* IVAR  */
  YYSYMBOL_VARNF = 96,                     /* VARNF  */
  YYSYMBOL_CALL = 97,                      /* CALL  */
  YYSYMBOL_NUMBER = 98,                    /* NUMBER  */
  YYSYMBOL_STRING = 99,                    /* STRING  */
  YYSYMBOL_REGEXPR = 100,                  /* REGEXPR  */
  YYSYMBOL_101_ = 101,                     /* '?'  */
  YYSYMBOL_102_ = 102,                     /* ':'  */
  YYSYMBOL_RETURN = 103,                   /* RETURN  */
  YYSYMBOL_SPLIT = 104,                    /* SPLIT  */
  YYSYMBOL_SUBSTR = 105,                   /* SUBSTR  */
  YYSYMBOL_WHILE = 106,                    /* WHILE  */
  YYSYMBOL_CAT = 107,                      /* CAT  */
  YYSYMBOL_108_ = 108,                     /* '+'  */
  YYSYMBOL_109_ = 109,                     /* '-'  */
  YYSYMBOL_110_ = 110,                     /* '*'  */
  YYSYMBOL_111_ = 111,                     /* '%'  */
  YYSYMBOL_NOT = 112,                      /* NOT  */
  YYSYMBOL_UMINUS = 113,                   /* UMINUS  */
  YYSYMBOL_UPLUS = 114,                    /* UPLUS  */
  YYSYMBOL_POWER = 115,                    /* POWER  */
  YYSYMBOL_DECR = 116,                     /* DECR  */
  YYSYMBOL_INCR = 117,                     /* INCR  */
  YYSYMBOL_INDIRECT = 118,                 /* INDIRECT  */
  YYSYMBOL_LASTTOKEN = 119,                /* LASTTOKEN  */
  YYSYMBOL_120_ = 120,                     /* '^'  */
  YYSYMBOL_121_ = 121,                     /* '|'  */
  YYSYMBOL_122_ = 122,                     /* '&'  */
  YYSYMBOL_YYACCEPT = 123,                 /* $accept  */
  YYSYMBOL_program = 124,                  /* program  */
  YYSYMBOL_land = 125,                     /* land  */
  YYSYMBOL_lor = 126,                      /* lor  */
  YYSYMBOL_comma = 127,                    /* comma  */
  YYSYMBOL_do = 128,                       /* do  */
  YYSYMBOL_else = 129,                     /* else  */
  YYSYMBOL_for = 130,                      /* for  */
  YYSYMBOL_131_1 = 131,                    /* $@1  */
  YYSYMBOL_132_2 = 132,                    /* $@2  */
  YYSYMBOL_133_3 = 133,                    /* $@3  */
  YYSYMBOL_funcname = 134,                 /* funcname  */
  YYSYMBOL_if = 135,                       /* if  */
  YYSYMBOL_lbrace = 136,                   /* lbrace  */
  YYSYMBOL_nl = 137,                       /* nl  */
  YYSYMBOL_opt_nl = 138,                   /* opt_nl  */
  YYSYMBOL_opt_pst = 139,                  /* opt_pst  */
  YYSYMBOL_opt_simple_stmt = 140,          /* opt_simple_stmt  */
  YYSYMBOL_pas = 141,                      /* pas  */
  YYSYMBOL_pa_pat = 142,                   /* pa_pat  */
  YYSYMBOL_pa_stat = 143,                  /* pa_stat  */
  YYSYMBOL_144_4 = 144,                    /* $@4  */
  YYSYMBOL_pa_stats = 145,                 /* pa_stats  */
  YYSYMBOL_patlist = 146,                  /* patlist  */
  YYSYMBOL_ppattern = 147,                 /* ppattern  */
  YYSYMBOL_pattern = 148,                  /* pattern  */
  YYSYMBOL_plist = 149,                    /* plist  */
  YYSYMBOL_pplist = 150,                   /* pplist  */
  YYSYMBOL_prarg = 151,                    /* prarg  */
  YYSYMBOL_print = 152,                    /* print  */
  YYSYMBOL_pst = 153,                      /* pst  */
  YYSYMBOL_rbrace = 154,                   /* rbrace  */
  YYSYMBOL_re = 155,                       /* re  */
  YYSYMBOL_reg_expr = 156,                 /* reg_expr  */
  YYSYMBOL_157_5 = 157,                    /* $@5  */
  YYSYMBOL_rparen = 158,                   /* rparen  */
  YYSYMBOL_simple_stmt = 159,              /* simple_stmt  */
  YYSYMBOL_st = 160,                       /* st  */
  YYSYMBOL_stmt = 161,                     /* stmt  */
  YYSYMBOL_162_6 = 162,                    /* $@6  */
  YYSYMBOL_163_7 = 163,                    /* $@7  */
  YYSYMBOL_164_8 = 164,                    /* $@8  */
  YYSYMBOL_stmtlist = 165,                 /* stmtlist  */
  YYSYMBOL_subop = 166,                    /* subop  */
  YYSYMBOL_string = 167,                   /* string  */
  YYSYMBOL_term = 168,                     /* term  */
  YYSYMBOL_var = 169,                      /* var  */
  YYSYMBOL_varlist = 170,                  /* varlist  */
  YYSYMBOL_varname = 171,                  /* varname  */
  YYSYMBOL_while = 172                     /* while  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
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
#define YYLAST   4740

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  123
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  182
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  363

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   359


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   111,   122,     2,
      12,    15,   110,   108,    10,   109,     2,    14,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   102,    13,
       2,     2,     2,   101,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    17,     2,    18,   120,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,   121,    16,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   103,   104,   105,
     106,   107,   112,   113,   114,   115,   116,   117,   118,   119
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   105,   105,   113,   117,   117,   121,   121,   125,   125,
     129,   129,   133,   133,   137,   137,   139,   139,   141,   141,
     146,   147,   151,   155,   155,   159,   159,   163,   164,   168,
     169,   174,   175,   179,   180,   184,   188,   189,   190,   191,
     192,   193,   195,   197,   197,   202,   203,   207,   208,   212,
     213,   215,   217,   219,   220,   225,   226,   227,   228,   229,
     233,   234,   236,   238,   240,   241,   242,   243,   244,   245,
     246,   247,   252,   253,   254,   255,   256,   260,   261,   265,
     266,   270,   271,   272,   276,   276,   280,   280,   280,   280,
     284,   284,   288,   290,   294,   294,   298,   298,   302,   303,
     304,   305,   306,   310,   311,   315,   317,   319,   319,   319,
     321,   322,   323,   324,   325,   326,   327,   330,   331,   332,
     333,   333,   334,   338,   339,   343,   343,   347,   348,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   379,   382,   383,   385,   390,
     391,   393,   395,   397,   398,   399,   401,   406,   408,   413,
     415,   417,   421,   422,   423,   424,   428,   429,   430,   436,
     437,   438,   443
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "FIRSTTOKEN",
  "PROGRAM", "PASTAT", "PASTAT2", "XBEGIN", "XEND", "NL", "','", "'{'",
  "'('", "';'", "'/'", "')'", "'}'", "'['", "']'", "ARRAY", "MATCH",
  "NOTMATCH", "MATCHOP", "FINAL", "DOT", "ALL", "CCL", "NCCL", "CHAR",
  "OR", "STAR", "QUEST", "PLUS", "EMPTYRE", "ZERO", "LAND", "LOR", "EQ",
  "GE", "GT", "LE", "LT", "NE", "IN", "ARG", "BLTIN", "BREAK", "CONTINUE",
  "DELETE", "DO", "EXIT", "FOR", "FUNC", "SUB", "GSUB", "IF", "INDEX",
  "LSUBSTR", "MATCHFCN", "NEXT", "ADD", "MINUS", "MULT", "DIVIDE", "MOD",
  "LSHIFT", "RSHIFT", "XOR", "BAND", "BOR", "CMPL", "ASSIGN", "ASGNOP",
  "ADDEQ", "SUBEQ", "MULTEQ", "DIVEQ", "MODEQ", "POWEQ", "BANDEQ", "BOREQ",
  "XOREQ", "SHLEQ", "SHREQ", "PRINT", "PRINTF", "SPRINTF", "ELSE",
  "INTEST", "CONDEXPR", "POSTINCR", "PREINCR", "POSTDECR", "PREDECR",
  "VAR", "IVAR", "VARNF", "CALL", "NUMBER", "STRING", "REGEXPR", "'?'",
  "':'", "RETURN", "SPLIT", "SUBSTR", "WHILE", "CAT", "'+'", "'-'", "'*'",
  "'%'", "NOT", "UMINUS", "UPLUS", "POWER", "DECR", "INCR", "INDIRECT",
  "LASTTOKEN", "'^'", "'|'", "'&'", "$accept", "program", "land", "lor",
  "comma", "do", "else", "for", "$@1", "$@2", "$@3", "funcname", "if",
  "lbrace", "nl", "opt_nl", "opt_pst", "opt_simple_stmt", "pas", "pa_pat",
  "pa_stat", "$@4", "pa_stats", "patlist", "ppattern", "pattern", "plist",
  "pplist", "prarg", "print", "pst", "rbrace", "re", "reg_expr", "$@5",
  "rparen", "simple_stmt", "st", "stmt", "$@6", "$@7", "$@8", "stmtlist",
  "subop", "string", "term", "var", "varlist", "varname", "while", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-322)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-32)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     697,  -322,  -322,  -322,    34,  1660,  -322,    18,  -322,     0,
       0,  -322,  4337,  -322,  -322,    40,    71,  -322,  -322,    48,
      62,  4622,    72,  -322,  -322,  -322,    97,  -322,  -322,   112,
     114,  4622,  4622,  4394,   -25,   -25,  4622,   773,    57,  -322,
      32,  3575,  -322,  -322,   138,   -55,   388,   -35,   126,  -322,
    -322,   773,   773,  2257,     7,    63,  4109,  -322,  -322,   145,
    4337,  4337,  4337,  4622,   -72,   -22,  4337,  4166,  4337,  4337,
     -72,   -72,  -322,   -72,  -322,  -322,    12,  -322,  -322,   155,
      67,    67,   -21,  -322,  1824,   158,   165,    67,  -322,  -322,
    1824,   168,   163,  -322,  1459,   773,  3575,  4451,    67,  -322,
     849,  -322,   155,   773,  1660,  4337,  -322,  -322,  4337,  4337,
    4337,  4337,  4337,  4337,   -21,  4337,  1881,  1938,   388,  4337,
    -322,  4508,  4622,  4622,  4622,  4622,  4622,  4622,  4622,  4622,
    4622,  4622,  4337,  -322,  -322,  4337,   925,  1001,  -322,  -322,
    1995,   147,  1995,   167,  -322,    44,  3575,    98,  2685,  2685,
    2773,    73,  -322,    86,  2685,  2685,  -322,   182,  -322,   155,
     182,  -322,  -322,   176,  1758,  -322,  1525,  4337,  -322,  1758,
    -322,  4337,  -322,  1459,   107,  1077,  4337,  3974,   186,  -322,
    -322,   388,   -30,  -322,  -322,  -322,  1459,  4337,  1153,  -322,
    3751,  -322,  3751,  3751,  3751,  3751,  3751,  3751,  -322,  2861,
    -322,  3839,  -322,  3663,  2685,   186,  4622,   -72,   354,   354,
      64,    64,   -72,   -72,   -72,   388,   388,   388,  3575,    11,
    -322,  -322,  -322,  3575,   -21,  3575,  -322,  -322,  1995,  -322,
      87,  1995,  1995,  -322,  -322,    -4,  1995,  -322,  -322,  4337,
    -322,   187,  -322,    -8,  2949,  -322,  2949,  -322,  -322,  1231,
    -322,   190,   110,  4565,   -21,  4565,  2052,  2109,   388,  2166,
    4565,  -322,     0,  -322,  4337,  1995,  1995,   388,  -322,  -322,
    3575,  -322,    -3,   192,  3037,   188,  3125,   191,   113,  2366,
      28,   131,   -21,   192,   192,    96,  -322,  -322,  -322,   162,
    4337,  4223,  -322,  -322,  3896,  4280,  4052,  3974,  3974,   773,
    3575,  2475,  2584,  -322,  -322,     0,  -322,  -322,  -322,  -322,
    -322,  1995,  -322,  1995,  -322,   155,  4337,   194,   195,   -21,
     121,  4565,  1307,  -322,   160,  -322,   160,   773,  3213,   196,
    3301,  1591,  3399,   192,  4337,  -322,   162,  3974,  -322,   197,
     198,  1383,  -322,  -322,  -322,   194,   155,  1459,  3487,  -322,
    -322,  -322,   192,  1591,  -322,    67,  1459,   194,  -322,  -322,
     192,  1459,  -322
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     3,    86,    87,     0,    33,     2,    30,     1,     0,
       0,    23,     0,    94,   180,   147,     0,   125,   126,     0,
       0,     0,     0,   179,   174,   181,     0,   159,   127,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    45,
      29,    35,    75,    92,     0,   164,    76,   171,   172,    88,
      89,     0,     0,     0,     0,     0,     0,    20,    21,     0,
       0,     0,     0,     0,   141,   171,     0,     0,     0,     0,
     143,   142,    93,   144,   150,   151,   175,   102,    24,    27,
       0,     0,     0,    10,     0,     0,     0,     0,    84,    85,
       0,     0,   107,   112,     0,     0,   101,    81,     0,   123,
       0,   120,    27,     0,    34,     0,     4,     6,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,     0,
     128,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   152,   153,     0,     0,     0,     8,   156,
       0,     0,     0,     0,   145,     0,    47,   176,     0,     0,
       0,     0,   148,     0,     0,     0,    25,    28,   122,    27,
     103,   105,   106,   100,     0,   111,     0,     0,   116,     0,
     118,     0,    11,     0,   114,     0,     0,    79,    82,    98,
      58,    59,   171,   119,    40,   124,     0,     0,     0,    46,
      71,    70,    64,    65,    66,    67,    68,    69,    72,     0,
       5,    63,     7,    62,     0,    92,     0,   133,   139,   140,
     130,   131,   132,   134,   138,   135,   136,   137,    60,     0,
      41,    42,     9,    77,     0,    78,    95,   146,     0,   177,
       0,     0,     0,   163,   149,     0,     0,    26,   104,     0,
     110,     0,    32,   172,     0,   117,     0,   108,    12,     0,
      90,   115,     0,     0,     0,     0,     0,     0,    57,     0,
       0,   121,    38,    37,     0,     0,     0,   129,   173,    73,
      48,    96,     0,    43,     0,    92,     0,    92,     0,     0,
       0,    27,     0,    22,   182,     0,    13,   113,    91,    83,
       0,    54,    53,    55,     0,    52,    51,    80,    49,     0,
      61,     0,     0,   178,    97,     0,   154,   155,   158,   157,
     162,     0,   170,     0,    99,    27,     0,     0,     0,     0,
       0,     0,     0,   166,     0,   165,     0,     0,     0,    92,
       0,     0,     0,    18,     0,    56,     0,    50,    39,     0,
       0,     0,   160,   161,   169,     0,    27,     0,     0,   168,
     167,    44,    16,     0,    19,     0,     0,     0,   109,    17,
      14,     0,    15
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -322,  -322,  -149,    90,   204,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,    15,    19,   -94,   174,  -321,  -322,    33,
     111,  -322,  -322,   -63,    39,    -5,  -175,  -322,  -322,  -322,
    -322,  -322,   -20,   -93,  -322,  -228,  -164,   -51,    22,  -322,
    -322,  -322,   -37,  -322,  -322,   392,   120,  -322,   116,  -322
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     4,   116,   117,   228,    92,   249,    93,   361,   356,
     347,    59,    94,    95,   160,   158,     5,   241,     6,    38,
      39,   305,    40,   145,   177,    96,    54,   178,   179,    97,
       7,   251,    42,    43,    55,   273,    98,   161,    99,   173,
     285,   186,   100,    44,    45,    46,    47,   230,    48,   101
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      41,   252,   242,   151,   153,   222,   222,    53,   187,   135,
     345,    11,   191,    72,   136,   137,   283,   138,   284,    14,
      37,   138,   141,    14,    51,    52,   205,    49,   256,   268,
     162,    50,   357,   165,     8,   282,   168,   132,   138,   170,
      14,     2,   260,   128,   120,     3,   314,   183,   129,   130,
     131,   146,    56,   103,   138,   148,   149,   150,   175,   227,
      60,   146,   146,   154,   155,   238,   188,   102,    11,    23,
      24,    25,   219,    23,    61,    25,   156,   180,   121,   164,
     159,   133,   134,   138,    66,   169,   133,   134,   233,   333,
      23,   303,    25,    36,   133,   134,   138,   138,   157,    41,
     190,   234,   271,   192,   193,   194,   195,   196,   197,    67,
     199,   201,   203,   240,   204,   320,   174,   352,   245,    37,
     138,   157,   185,   138,    68,   289,    69,   218,   310,   360,
     146,   138,   129,   130,   131,   223,   336,   225,   275,   277,
     156,    65,   256,   135,   315,   256,   256,   256,   256,   256,
     119,    65,    65,    65,    74,    75,    65,   147,   185,   185,
     292,    65,   244,   143,   156,    57,   246,   242,    58,   222,
     166,    53,   172,    65,   126,   127,   280,   167,   157,   128,
     171,   226,    41,    65,   129,   130,   131,   316,   256,   242,
     224,   237,   229,   239,   248,   247,   138,   185,   163,   288,
     281,   304,   318,   307,    14,   319,   309,   334,   261,   271,
     185,   343,   349,   350,   104,   189,    65,   182,   329,     0,
     262,   331,     0,   270,     0,     0,   274,   276,     0,     0,
     198,   279,     0,   180,   146,   180,   180,   180,     0,   180,
     180,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,   353,     0,    23,    24,    25,   140,   142,   300,
     301,   302,   322,     0,     0,     0,    65,   257,    65,    65,
      65,   287,     0,     0,    65,    65,     0,   299,    36,     0,
       0,     0,   243,     0,    65,    53,     0,     0,     0,    65,
     341,     0,   291,     0,   294,   295,   296,    65,   297,   298,
     157,   180,     0,     0,   358,     0,   328,     0,   330,     0,
      65,   332,    65,    65,    65,    65,    65,    65,     0,    65,
     327,    65,     0,    65,    65,     0,    65,     0,     0,   348,
       0,     0,     0,     0,   157,     0,     0,     0,    65,     0,
     269,     0,     0,    65,   185,    65,     0,     0,     0,     0,
       0,   278,   231,   232,     0,     0,     0,     0,   235,   236,
     337,     0,     0,   185,    65,   157,    65,     0,   121,   354,
     293,     0,     0,   182,     0,   182,   182,   182,   359,   182,
     182,   257,   259,   362,   257,   257,   257,   257,   257,     0,
      65,     0,     0,     0,    65,     0,    65,     0,   317,    65,
       0,     0,   121,     0,     0,     0,     0,     0,   265,   266,
       0,    65,     0,    64,    65,    65,    65,    65,    65,     0,
      65,    65,    65,    70,    71,    73,     0,   257,    76,     0,
       0,     0,     0,   118,   272,   335,     0,     0,     0,     0,
       0,   182,     0,     0,   339,   118,   340,     0,    65,     0,
      65,     0,    65,   122,   123,    73,   142,    65,     0,     0,
       0,     0,   124,   125,   126,   127,     0,     0,    65,   128,
       0,     0,     0,     0,   129,   130,   131,     0,     0,     0,
       0,     0,   311,   313,     0,     0,     0,     0,   118,   181,
       0,     0,     0,     0,     0,     0,   124,   125,   126,   127,
       0,     0,     0,   128,     0,   324,   326,     0,   129,   130,
     131,     0,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   118,     0,
     118,   118,   118,     0,     0,     0,   118,   118,     0,     0,
       0,     0,     0,     0,     0,     0,   118,     0,     0,     0,
       0,   118,     0,     0,     0,     0,     0,     0,     0,   258,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   118,     0,   118,   118,   118,   118,   118,   118,
       0,   118,     0,   118,     0,   118,   118,     0,   267,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     118,     0,     0,     0,     0,   118,     0,   118,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   118,     0,   118,     0,
       0,     0,     0,     0,     0,   181,     0,   181,   181,   181,
       0,   181,   181,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   118,     0,     0,     0,   118,     0,   118,     0,
       0,   118,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   258,     0,     0,   258,   258,   258,   258,
     258,     0,   118,   118,   118,     0,     0,   -29,     1,     0,
       0,     0,     0,     0,   -29,   -29,     2,     0,   -29,   -29,
       3,   -29,     0,   181,     0,     0,     0,     0,     0,     0,
     118,     0,   118,     0,   118,     0,     0,     0,     0,   258,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     118,   -29,   -29,     0,     0,     0,     0,     0,     0,   -29,
     -29,   -29,     0,   -29,     0,   -29,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   -29,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,     0,
       0,     0,    78,   -29,    11,    12,    79,    13,     0,     0,
       0,   -29,   -29,   -29,   -29,   -29,   -29,     0,     0,     0,
       0,   -29,   -29,     0,     0,   -29,   -29,     0,     0,   -29,
       0,     0,     0,   -29,   -29,   -29,     0,    14,    15,    80,
      81,    82,    83,    84,    85,     0,    17,    18,    86,    19,
       0,    20,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,     0,     0,    88,    89,    22,
      11,    12,    79,    13,     0,   184,     0,    23,    24,    25,
      26,    27,    28,     0,     0,     0,    90,    29,    30,    91,
       0,    31,    32,     0,     0,    33,     0,     0,     0,    34,
      35,    36,     0,    14,    15,    80,    81,    82,    83,    84,
      85,     0,    17,    18,    86,    19,     0,    20,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,     0,     0,    88,    89,    22,    11,    12,    79,    13,
       0,   220,     0,    23,    24,    25,    26,    27,    28,     0,
       0,     0,    90,    29,    30,    91,     0,    31,    32,     0,
       0,    33,     0,     0,     0,    34,    35,    36,     0,    14,
      15,    80,    81,    82,    83,    84,    85,     0,    17,    18,
      86,    19,     0,    20,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,     0,     0,    88,
      89,    22,    11,    12,    79,    13,     0,   221,     0,    23,
      24,    25,    26,    27,    28,     0,     0,     0,    90,    29,
      30,    91,     0,    31,    32,     0,     0,    33,     0,     0,
       0,    34,    35,    36,     0,    14,    15,    80,    81,    82,
      83,    84,    85,     0,    17,    18,    86,    19,     0,    20,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,     0,     0,    88,    89,    22,    11,    12,
      79,    13,     0,   250,     0,    23,    24,    25,    26,    27,
      28,     0,     0,     0,    90,    29,    30,    91,     0,    31,
      32,     0,     0,    33,     0,     0,     0,    34,    35,    36,
       0,    14,    15,    80,    81,    82,    83,    84,    85,     0,
      17,    18,    86,    19,     0,    20,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,     0,
       0,    88,    89,    22,    11,    12,    79,    13,     0,   263,
       0,    23,    24,    25,    26,    27,    28,     0,     0,     0,
      90,    29,    30,    91,     0,    31,    32,     0,     0,    33,
       0,     0,     0,    34,    35,    36,     0,    14,    15,    80,
      81,    82,    83,    84,    85,     0,    17,    18,    86,    19,
       0,    20,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,    88,    89,    22,
     286,     0,    11,    12,    79,    13,     0,    23,    24,    25,
      26,    27,    28,     0,     0,     0,    90,    29,    30,    91,
       0,    31,    32,     0,     0,    33,     0,     0,     0,    34,
      35,    36,     0,     0,     0,    14,    15,    80,    81,    82,
      83,    84,    85,     0,    17,    18,    86,    19,     0,    20,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,     0,     0,    88,    89,    22,    11,    12,
      79,    13,     0,   338,     0,    23,    24,    25,    26,    27,
      28,     0,     0,     0,    90,    29,    30,    91,     0,    31,
      32,     0,     0,    33,     0,     0,     0,    34,    35,    36,
       0,    14,    15,    80,    81,    82,    83,    84,    85,     0,
      17,    18,    86,    19,     0,    20,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,     0,
       0,    88,    89,    22,    11,    12,    79,    13,     0,   351,
       0,    23,    24,    25,    26,    27,    28,     0,     0,     0,
      90,    29,    30,    91,     0,    31,    32,     0,     0,    33,
       0,     0,     0,    34,    35,    36,     0,    14,    15,    80,
      81,    82,    83,    84,    85,     0,    17,    18,    86,    19,
       0,    20,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,     0,     0,    88,    89,    22,
      11,    12,    79,    13,     0,     0,     0,    23,    24,    25,
      26,    27,    28,     0,     0,     0,    90,    29,    30,    91,
       0,    31,    32,     0,     0,    33,     0,     0,     0,    34,
      35,    36,     0,    14,    15,    80,    81,    82,    83,    84,
      85,     0,    17,    18,    86,    19,     0,    20,    87,     0,
       0,     0,     0,     0,     0,     0,    77,     0,     0,    21,
       0,     0,     0,     0,     0,     0,     0,    12,   -31,    13,
       0,     0,     0,    88,    89,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
       0,     0,    90,    29,    30,    91,     0,    31,    32,    14,
      15,    33,     0,    82,     0,    34,    35,    36,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,    77,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,    12,     0,    13,   -31,     0,     0,    88,
      89,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,     0,     0,     0,    29,
      30,     0,     0,    31,    32,    14,    15,    33,     0,    82,
       0,    34,    35,    36,    17,    18,     0,    19,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     9,    10,     0,
       0,    11,    12,     0,    13,    88,    89,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,     0,     0,     0,     0,    29,    30,     0,     0,    31,
      32,     0,     0,    33,    14,    15,     0,    34,    35,    36,
       0,     0,    16,    17,    18,     0,    19,     0,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    29,    30,     0,   156,    31,    32,
      62,   159,    33,     0,     0,     0,    34,    35,    36,     0,
     105,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    14,    15,     0,     0,     0,     0,     0,     0,
       0,    17,    18,     0,    19,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,     0,
       0,     0,     0,   156,     0,     0,    12,   159,    13,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,     0,   115,
       0,     0,    29,    30,     0,     0,    31,    32,    14,    15,
      63,     0,     0,     0,    34,    35,    36,    17,    18,     0,
      19,     0,    20,     0,     0,     0,     0,     0,     0,     0,
     200,     0,     0,    12,    21,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,     0,    14,    15,     0,    29,    30,
       0,     0,    31,    32,    17,    18,    33,    19,     0,    20,
      34,    35,    36,     0,     0,     0,     0,   202,     0,     0,
      12,    21,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,     0,    14,    15,     0,    29,    30,     0,     0,    31,
      32,    17,    18,    33,    19,     0,    20,    34,    35,    36,
       0,     0,     0,     0,   222,     0,     0,    12,    21,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,     0,    14,
      15,     0,    29,    30,     0,     0,    31,    32,    17,    18,
      33,    19,     0,    20,    34,    35,    36,     0,     0,     0,
       0,   200,     0,     0,   290,    21,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,    14,    15,     0,    29,
      30,     0,     0,    31,    32,    17,    18,    33,    19,     0,
      20,    34,    35,    36,     0,     0,     0,     0,   202,     0,
       0,   290,    21,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,    14,    15,     0,    29,    30,     0,     0,
      31,    32,    17,    18,    33,    19,     0,    20,    34,    35,
      36,     0,     0,     0,     0,   222,     0,     0,   290,    21,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
      14,    15,     0,    29,    30,     0,     0,    31,    32,    17,
      18,    33,    19,     0,    20,    34,    35,    36,     0,     0,
       0,     0,     0,     0,     0,     0,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,     0,   138,     0,    62,
      29,    30,   139,     0,    31,    32,     0,     0,    33,   105,
       0,     0,    34,    35,    36,     0,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    14,    15,     0,     0,     0,     0,     0,     0,     0,
      17,    18,     0,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,     0,   115,     0,
       0,    29,    30,     0,     0,    31,    32,     0,     0,    63,
       0,     0,     0,    34,    35,    36,   138,     0,    62,     0,
       0,   312,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   106,   107,   108,   109,   110,   111,   112,   113,   114,
      14,    15,     0,     0,     0,     0,     0,     0,     0,    17,
      18,     0,    19,     0,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,     0,   115,     0,     0,
      29,    30,     0,     0,    31,    32,     0,     0,    63,     0,
       0,     0,    34,    35,    36,   138,     0,    62,     0,     0,
     323,     0,     0,     0,     0,     0,     0,   105,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,   115,     0,     0,    29,
      30,     0,     0,    31,    32,     0,     0,    63,     0,     0,
       0,    34,    35,    36,   138,     0,    62,     0,     0,   325,
       0,     0,     0,     0,     0,     0,   105,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    14,    15,
       0,     0,     0,     0,     0,     0,     0,    17,    18,     0,
      19,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,     0,   115,     0,     0,    29,    30,
       0,     0,    31,    32,     0,   138,    63,    62,     0,     0,
      34,    35,    36,     0,     0,     0,     0,   105,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,    62,   115,     0,   139,    29,
      30,     0,     0,    31,    32,   105,     0,    63,     0,     0,
       0,    34,    35,    36,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    17,    18,     0,    19,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,    62,   115,     0,     0,    29,    30,     0,
       0,    31,    32,   105,     0,    63,     0,     0,     0,    34,
      35,    36,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    14,    15,     0,     0,     0,
       0,     0,     0,     0,    17,    18,     0,    19,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,    62,   115,   264,   271,    29,    30,     0,     0,    31,
      32,   105,     0,    63,     0,     0,     0,    34,    35,    36,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    14,    15,     0,     0,     0,     0,     0,
       0,     0,    17,    18,     0,    19,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,    62,
     115,     0,   306,    29,    30,     0,     0,    31,    32,   105,
       0,    63,     0,     0,     0,    34,    35,    36,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    14,    15,     0,     0,     0,     0,     0,     0,     0,
      17,    18,     0,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    62,   115,     0,
     308,    29,    30,     0,     0,    31,    32,   105,     0,    63,
       0,     0,     0,    34,    35,    36,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,    62,   115,     0,   342,    29,
      30,     0,     0,    31,    32,   105,     0,    63,     0,     0,
       0,    34,    35,    36,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    17,    18,     0,    19,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,    62,   115,     0,   344,    29,    30,     0,
       0,    31,    32,   105,     0,    63,     0,     0,     0,    34,
      35,    36,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    14,    15,     0,     0,     0,
       0,     0,     0,     0,    17,    18,     0,    19,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,     0,   115,     0,     0,    29,    30,     0,     0,    31,
      32,    62,   346,    63,     0,     0,     0,    34,    35,    36,
       0,   105,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    14,    15,     0,     0,     0,     0,     0,
       0,     0,    17,    18,     0,    19,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,    62,
     115,     0,   355,    29,    30,     0,     0,    31,    32,   105,
       0,    63,     0,     0,     0,    34,    35,    36,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    14,    15,     0,     0,     0,     0,     0,     0,     0,
      17,    18,     0,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    62,   115,     0,
       0,    29,    30,     0,     0,    31,    32,   105,     0,    63,
       0,     0,     0,    34,    35,    36,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,    62,   115,     0,     0,    29,
      30,     0,     0,    31,    32,   105,     0,    63,     0,     0,
       0,    34,    35,    36,     0,     0,     0,     0,   106,     0,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    17,    18,     0,    19,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,    62,     0,     0,     0,    29,    30,     0,
       0,    31,    32,   -32,     0,    63,     0,     0,     0,    34,
      35,    36,     0,     0,     0,     0,     0,     0,   -32,   -32,
     -32,   -32,   -32,   -32,   -32,    14,    15,     0,     0,     0,
       0,     0,     0,     0,    17,    18,     0,    19,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,    62,     0,     0,     0,    29,    30,     0,     0,    31,
      32,   105,     0,    63,     0,     0,     0,    34,    35,    36,
       0,     0,     0,     0,     0,     0,   108,   109,   110,   111,
     112,   113,   114,    14,    15,     0,     0,     0,     0,     0,
       0,     0,    17,    18,     0,    19,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    62,    21,
       0,     0,     0,     0,     0,     0,     0,     0,   253,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,   106,   107,    23,    24,    25,    26,    27,    28,   254,
      14,    15,     0,    29,    30,     0,     0,    31,    32,    17,
      18,    63,    19,     0,    20,    34,    35,    36,     0,     0,
       0,     0,     0,     0,     0,     0,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,    62,     0,     0,     0,
      23,    24,    25,    26,    27,    28,   253,   255,   321,     0,
      29,    30,     0,     0,    31,    32,     0,     0,    63,   106,
     107,     0,    34,    35,    36,     0,     0,   254,    14,    15,
       0,     0,     0,     0,     0,     0,     0,    17,    18,     0,
      19,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,    62,     0,     0,     0,    23,    24,
      25,    26,    27,    28,   253,   255,     0,     0,    29,    30,
       0,     0,    31,    32,     0,     0,    63,   106,     0,     0,
      34,    35,    36,     0,     0,   254,    14,    15,     0,     0,
       0,     0,     0,     0,     0,    17,    18,     0,    19,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    21,    13,   144,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,    14,    15,     0,    29,    30,     0,     0,
      31,    32,    17,    18,    63,    19,     0,    20,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,    12,    21,
      13,   152,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
      14,    15,     0,    29,    30,     0,     0,    31,    32,    17,
      18,    33,    19,     0,    20,    34,    35,    36,     0,     0,
       0,     0,     0,     0,     0,    62,    21,     0,     0,     0,
       0,     0,     0,     0,     0,   -32,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,   -32,    14,    15,     0,
      29,    30,     0,     0,    31,    32,    17,    18,    33,    19,
       0,    20,    34,    35,    36,     0,     0,     0,     0,     0,
       0,     0,    62,    21,     0,     0,     0,     0,     0,     0,
       0,     0,   253,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,   254,    14,    15,     0,    29,    30,     0,
       0,    31,    32,    17,    18,    63,    19,     0,    20,    34,
      35,    36,     0,     0,     0,     0,     0,     0,     0,    12,
      21,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,    14,    15,     0,    29,    30,     0,     0,    31,    32,
      17,    18,    63,    19,     0,    20,    34,    35,    36,     0,
       0,     0,     0,     0,     0,     0,    62,    21,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,     0,    14,    15,
       0,    29,    30,     0,     0,    31,    32,    17,    18,    33,
      19,     0,    20,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,   176,    21,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,     0,    14,    15,     0,    29,    30,
       0,     0,    31,    32,    17,    18,    33,    19,     0,    20,
      34,    35,    36,     0,     0,     0,     0,     0,     0,     0,
      62,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,     0,    14,    15,     0,    29,    30,     0,     0,    31,
      32,    17,    18,    33,    19,     0,    20,    34,    35,    36,
       0,     0,     0,     0,     0,     0,     0,   290,    21,    13,
     206,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,     0,    14,
      15,     0,    29,    30,     0,     0,    31,    32,    17,    18,
      63,    19,     0,    20,    34,    35,    36,     0,     0,     0,
       0,     0,     0,     0,    62,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,    14,    15,     0,    29,
      30,     0,     0,    31,    32,    17,    18,    33,    19,     0,
      20,    34,    35,    36,     0,     0,     0,     0,     0,     0,
       0,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,     0,     0,     0,    29,    30,     0,     0,
      31,    32,     0,     0,    63,     0,     0,     0,    34,    35,
      36
};

static const yytype_int16 yycheck[] =
{
       5,   176,   166,    66,    67,     9,     9,    12,   102,    17,
     331,    11,   105,    33,    51,    52,   244,    10,   246,    44,
       5,    10,    15,    44,     9,    10,   119,     9,   177,    18,
      81,    13,   353,    84,     0,    43,    87,    72,    10,    90,
      44,     9,    72,   115,    99,    13,    18,    98,   120,   121,
     122,    56,    12,    38,    10,    60,    61,    62,    95,    15,
      12,    66,    67,    68,    69,   159,   103,    10,    11,    94,
      95,    96,   135,    94,    12,    96,     9,    97,    14,    84,
      13,   116,   117,    10,    12,    90,   116,   117,    15,   317,
      94,    94,    96,   118,   116,   117,    10,    10,    79,   104,
     105,    15,    15,   108,   109,   110,   111,   112,   113,    12,
     115,   116,   117,   164,   119,   290,    94,   345,   169,   104,
      10,   102,   100,    10,    12,    15,    12,   132,    15,   357,
     135,    10,   120,   121,   122,   140,    15,   142,   231,   232,
       9,    21,   291,    17,    13,   294,   295,   296,   297,   298,
      12,    31,    32,    33,    34,    35,    36,    12,   136,   137,
     253,    41,   167,   100,     9,    94,   171,   331,    97,     9,
      12,   176,     9,    53,   110,   111,   239,    12,   159,   115,
      12,    14,   187,    63,   120,   121,   122,   281,   337,   353,
      43,     9,    94,    17,    87,   173,    10,   175,    82,     9,
      13,     9,   106,    15,    44,    43,    15,    12,   186,    15,
     188,    15,    15,    15,    40,   104,    96,    97,   311,    -1,
     187,   315,    -1,   228,    -1,    -1,   231,   232,    -1,    -1,
     114,   236,    -1,   253,   239,   255,   256,   257,    -1,   259,
     260,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   346,    -1,    94,    95,    96,    53,    54,   264,
     265,   266,   299,    -1,    -1,    -1,   146,   177,   148,   149,
     150,   249,    -1,    -1,   154,   155,    -1,   262,   118,    -1,
      -1,    -1,   166,    -1,   164,   290,    -1,    -1,    -1,   169,
     327,    -1,   253,    -1,   255,   256,   257,   177,   259,   260,
     281,   321,    -1,    -1,   355,    -1,   311,    -1,   313,    -1,
     190,   316,   192,   193,   194,   195,   196,   197,    -1,   199,
     305,   201,    -1,   203,   204,    -1,   206,    -1,    -1,   334,
      -1,    -1,    -1,    -1,   315,    -1,    -1,    -1,   218,    -1,
     224,    -1,    -1,   223,   322,   225,    -1,    -1,    -1,    -1,
      -1,   235,   148,   149,    -1,    -1,    -1,    -1,   154,   155,
     321,    -1,    -1,   341,   244,   346,   246,    -1,    14,   347,
     254,    -1,    -1,   253,    -1,   255,   256,   257,   356,   259,
     260,   291,   178,   361,   294,   295,   296,   297,   298,    -1,
     270,    -1,    -1,    -1,   274,    -1,   276,    -1,   282,   279,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,   204,   205,
      -1,   291,    -1,    21,   294,   295,   296,   297,   298,    -1,
     300,   301,   302,    31,    32,    33,    -1,   337,    36,    -1,
      -1,    -1,    -1,    41,   230,   319,    -1,    -1,    -1,    -1,
      -1,   321,    -1,    -1,   324,    53,   326,    -1,   328,    -1,
     330,    -1,   332,    65,    66,    63,   252,   337,    -1,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,    -1,   348,   115,
      -1,    -1,    -1,    -1,   120,   121,   122,    -1,    -1,    -1,
      -1,    -1,   278,   279,    -1,    -1,    -1,    -1,    96,    97,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,    -1,    -1,   115,    -1,   301,   302,    -1,   120,   121,
     122,    -1,    -1,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   320,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
     148,   149,   150,    -1,    -1,    -1,   154,   155,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   164,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   190,    -1,   192,   193,   194,   195,   196,   197,
      -1,   199,    -1,   201,    -1,   203,   204,    -1,   206,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     218,    -1,    -1,    -1,    -1,   223,    -1,   225,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   244,    -1,   246,    -1,
      -1,    -1,    -1,    -1,    -1,   253,    -1,   255,   256,   257,
      -1,   259,   260,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   270,    -1,    -1,    -1,   274,    -1,   276,    -1,
      -1,   279,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   291,    -1,    -1,   294,   295,   296,   297,
     298,    -1,   300,   301,   302,    -1,    -1,     0,     1,    -1,
      -1,    -1,    -1,    -1,     7,     8,     9,    -1,    11,    12,
      13,    14,    -1,   321,    -1,    -1,    -1,    -1,    -1,    -1,
     328,    -1,   330,    -1,   332,    -1,    -1,    -1,    -1,   337,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     348,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    86,    11,    12,    13,    14,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    -1,    -1,    -1,
      -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,
      -1,    -1,    -1,   116,   117,   118,    -1,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,    86,
      11,    12,    13,    14,    -1,    16,    -1,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,    -1,   112,    -1,    -1,    -1,   116,
     117,   118,    -1,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    -1,    58,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    84,    85,    86,    11,    12,    13,    14,
      -1,    16,    -1,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
      -1,   112,    -1,    -1,    -1,   116,   117,   118,    -1,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    -1,    58,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    84,
      85,    86,    11,    12,    13,    14,    -1,    16,    -1,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,    -1,    -1,   112,    -1,    -1,
      -1,   116,   117,   118,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    -1,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    -1,    -1,    -1,    -1,    84,    85,    86,    11,    12,
      13,    14,    -1,    16,    -1,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,    -1,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    -1,    58,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    84,    85,    86,    11,    12,    13,    14,    -1,    16,
      -1,    94,    95,    96,    97,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,    -1,   112,
      -1,    -1,    -1,   116,   117,   118,    -1,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    84,    85,    86,
       9,    -1,    11,    12,    13,    14,    -1,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,    -1,   112,    -1,    -1,    -1,   116,
     117,   118,    -1,    -1,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    -1,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    -1,    -1,    -1,    -1,    84,    85,    86,    11,    12,
      13,    14,    -1,    16,    -1,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,    -1,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    -1,    58,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    84,    85,    86,    11,    12,    13,    14,    -1,    16,
      -1,    94,    95,    96,    97,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,    -1,   112,
      -1,    -1,    -1,   116,   117,   118,    -1,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,    86,
      11,    12,    13,    14,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,    -1,   112,    -1,    -1,    -1,   116,
     117,   118,    -1,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    -1,    58,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    13,    14,
      -1,    -1,    -1,    84,    85,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    44,
      45,   112,    -1,    48,    -1,   116,   117,   118,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    12,    -1,    14,    15,    -1,    -1,    84,
      85,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    44,    45,   112,    -1,    48,
      -1,   116,   117,   118,    53,    54,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    -1,     7,     8,    -1,
      -1,    11,    12,    -1,    14,    84,    85,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,    -1,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,   112,    44,    45,    -1,   116,   117,   118,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
      -1,    -1,    -1,    -1,   104,   105,    -1,     9,   108,   109,
      12,    13,   112,    -1,    -1,    -1,   116,   117,   118,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,     9,    -1,    -1,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,   108,   109,    44,    45,
     112,    -1,    -1,    -1,   116,   117,   118,    53,    54,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       9,    -1,    -1,    12,    70,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    -1,    44,    45,    -1,   104,   105,
      -1,    -1,   108,   109,    53,    54,   112,    56,    -1,    58,
     116,   117,   118,    -1,    -1,    -1,    -1,     9,    -1,    -1,
      12,    70,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    -1,    44,    45,    -1,   104,   105,    -1,    -1,   108,
     109,    53,    54,   112,    56,    -1,    58,   116,   117,   118,
      -1,    -1,    -1,    -1,     9,    -1,    -1,    12,    70,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,    44,
      45,    -1,   104,   105,    -1,    -1,   108,   109,    53,    54,
     112,    56,    -1,    58,   116,   117,   118,    -1,    -1,    -1,
      -1,     9,    -1,    -1,    12,    70,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    44,    45,    -1,   104,
     105,    -1,    -1,   108,   109,    53,    54,   112,    56,    -1,
      58,   116,   117,   118,    -1,    -1,    -1,    -1,     9,    -1,
      -1,    12,    70,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,    -1,    44,    45,    -1,   104,   105,    -1,    -1,
     108,   109,    53,    54,   112,    56,    -1,    58,   116,   117,
     118,    -1,    -1,    -1,    -1,     9,    -1,    -1,    12,    70,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    -1,
      44,    45,    -1,   104,   105,    -1,    -1,   108,   109,    53,
      54,   112,    56,    -1,    58,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    10,    -1,    12,
     104,   105,    15,    -1,   108,   109,    -1,    -1,   112,    22,
      -1,    -1,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,
      -1,    -1,    -1,   116,   117,   118,    10,    -1,    12,    -1,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,   101,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,    -1,
      -1,    -1,   116,   117,   118,    10,    -1,    12,    -1,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,   112,    -1,    -1,
      -1,   116,   117,   118,    10,    -1,    12,    -1,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    -1,   101,    -1,    -1,   104,   105,
      -1,    -1,   108,   109,    -1,    10,   112,    12,    -1,    -1,
     116,   117,   118,    -1,    -1,    -1,    -1,    22,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    12,   101,    -1,    15,   104,
     105,    -1,    -1,   108,   109,    22,    -1,   112,    -1,    -1,
      -1,   116,   117,   118,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    12,   101,    -1,    -1,   104,   105,    -1,
      -1,   108,   109,    22,    -1,   112,    -1,    -1,    -1,   116,
     117,   118,    -1,    -1,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    12,   101,   102,    15,   104,   105,    -1,    -1,   108,
     109,    22,    -1,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    12,
     101,    -1,    15,   104,   105,    -1,    -1,   108,   109,    22,
      -1,   112,    -1,    -1,    -1,   116,   117,   118,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    12,   101,    -1,
      15,   104,   105,    -1,    -1,   108,   109,    22,    -1,   112,
      -1,    -1,    -1,   116,   117,   118,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    12,   101,    -1,    15,   104,
     105,    -1,    -1,   108,   109,    22,    -1,   112,    -1,    -1,
      -1,   116,   117,   118,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    12,   101,    -1,    15,   104,   105,    -1,
      -1,   108,   109,    22,    -1,   112,    -1,    -1,    -1,   116,
     117,   118,    -1,    -1,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,   108,
     109,    12,    13,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    12,
     101,    -1,    15,   104,   105,    -1,    -1,   108,   109,    22,
      -1,   112,    -1,    -1,    -1,   116,   117,   118,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    12,   101,    -1,
      -1,   104,   105,    -1,    -1,   108,   109,    22,    -1,   112,
      -1,    -1,    -1,   116,   117,   118,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    12,   101,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    22,    -1,   112,    -1,    -1,
      -1,   116,   117,   118,    -1,    -1,    -1,    -1,    35,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    12,    -1,    -1,    -1,   104,   105,    -1,
      -1,   108,   109,    22,    -1,   112,    -1,    -1,    -1,   116,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    12,    -1,    -1,    -1,   104,   105,    -1,    -1,   108,
     109,    22,    -1,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    35,    36,    94,    95,    96,    97,    98,    99,    43,
      44,    45,    -1,   104,   105,    -1,    -1,   108,   109,    53,
      54,   112,    56,    -1,    58,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    22,   101,   102,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,    35,
      36,    -1,   116,   117,   118,    -1,    -1,    43,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    12,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    22,   101,    -1,    -1,   104,   105,
      -1,    -1,   108,   109,    -1,    -1,   112,    35,    -1,    -1,
     116,   117,   118,    -1,    -1,    43,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    12,    70,    14,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,    -1,    44,    45,    -1,   104,   105,    -1,    -1,
     108,   109,    53,    54,   112,    56,    -1,    58,   116,   117,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    70,
      14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    -1,
      44,    45,    -1,   104,   105,    -1,    -1,   108,   109,    53,
      54,   112,    56,    -1,    58,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    12,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    43,    44,    45,    -1,
     104,   105,    -1,    -1,   108,   109,    53,    54,   112,    56,
      -1,    58,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    12,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    43,    44,    45,    -1,   104,   105,    -1,
      -1,   108,   109,    53,    54,   112,    56,    -1,    58,   116,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,
      70,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
      -1,    44,    45,    -1,   104,   105,    -1,    -1,   108,   109,
      53,    54,   112,    56,    -1,    58,   116,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    70,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    -1,    44,    45,
      -1,   104,   105,    -1,    -1,   108,   109,    53,    54,   112,
      56,    -1,    58,   116,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    12,    70,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    -1,    44,    45,    -1,   104,   105,
      -1,    -1,   108,   109,    53,    54,   112,    56,    -1,    58,
     116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      12,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    -1,    44,    45,    -1,   104,   105,    -1,    -1,   108,
     109,    53,    54,   112,    56,    -1,    58,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    70,    14,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,    44,
      45,    -1,   104,   105,    -1,    -1,   108,   109,    53,    54,
     112,    56,    -1,    58,   116,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    12,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    44,    45,    -1,   104,
     105,    -1,    -1,   108,   109,    53,    54,   112,    56,    -1,
      58,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,    -1,    -1,    -1,    -1,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,   112,    -1,    -1,    -1,   116,   117,
     118
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     9,    13,   124,   139,   141,   153,     0,     7,
       8,    11,    12,    14,    44,    45,    52,    53,    54,    56,
      58,    70,    86,    94,    95,    96,    97,    98,    99,   104,
     105,   108,   109,   112,   116,   117,   118,   136,   142,   143,
     145,   148,   155,   156,   166,   167,   168,   169,   171,     9,
      13,   136,   136,   148,   149,   157,    12,    94,    97,   134,
      12,    12,    12,   112,   168,   169,    12,    12,    12,    12,
     168,   168,   155,   168,   169,   169,   168,     1,     9,    13,
      46,    47,    48,    49,    50,    51,    55,    59,    84,    85,
     103,   106,   128,   130,   135,   136,   148,   152,   159,   161,
     165,   172,    10,   136,   139,    22,    35,    36,    37,    38,
      39,    40,    41,    42,    43,   101,   125,   126,   168,    12,
      99,    14,    65,    66,   108,   109,   110,   111,   115,   120,
     121,   122,    72,   116,   117,    17,   165,   165,    10,    15,
     127,    15,   127,   100,    15,   146,   148,    12,   148,   148,
     148,   146,    15,   146,   148,   148,     9,   137,   138,    13,
     137,   160,   160,   171,   148,   160,    12,    12,   160,   148,
     160,    12,     9,   162,   161,   165,    12,   147,   150,   151,
     155,   168,   169,   160,    16,   161,   164,   138,   165,   143,
     148,   156,   148,   148,   148,   148,   148,   148,   171,   148,
       9,   148,     9,   148,   148,   156,    72,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   148,   146,
      16,    16,     9,   148,    43,   148,    14,    15,   127,    94,
     170,   127,   127,    15,    15,   127,   127,     9,   138,    17,
     160,   140,   159,   171,   148,   160,   148,   161,    87,   129,
      16,   154,   149,    22,    43,   101,   125,   126,   168,   127,
      72,   161,   142,    16,   102,   127,   127,   168,    18,   171,
     148,    15,   127,   158,   148,   156,   148,   156,   171,   148,
     146,    13,    43,   158,   158,   163,     9,   161,     9,    15,
      12,   147,   156,   171,   147,   147,   147,   147,   147,   136,
     148,   148,   148,    94,     9,   144,    15,    15,    15,    15,
      15,   127,    15,   127,    18,    13,   138,   171,   106,    43,
     149,   102,   165,    15,   127,    15,   127,   136,   148,   156,
     148,   138,   148,   158,    12,   171,    15,   147,    16,   169,
     169,   165,    15,    15,    15,   140,    13,   133,   148,    15,
      15,    16,   158,   138,   161,    15,   132,   140,   160,   161,
     158,   131,   161
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   123,   124,   124,   125,   125,   126,   126,   127,   127,
     128,   128,   129,   129,   131,   130,   132,   130,   133,   130,
     134,   134,   135,   136,   136,   137,   137,   138,   138,   139,
     139,   140,   140,   141,   141,   142,   143,   143,   143,   143,
     143,   143,   143,   144,   143,   145,   145,   146,   146,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   149,   149,   150,
     150,   151,   151,   151,   152,   152,   153,   153,   153,   153,
     154,   154,   155,   155,   157,   156,   158,   158,   159,   159,
     159,   159,   159,   160,   160,   161,   161,   162,   163,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     164,   161,   161,   165,   165,   166,   166,   167,   167,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   169,   169,   169,   169,   170,   170,   170,   171,
     171,   171,   172
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     1,     2,     1,     2,
       1,     2,     1,     2,     0,    12,     0,    10,     0,     8,
       1,     1,     4,     1,     2,     1,     2,     0,     1,     0,
       1,     0,     1,     1,     3,     1,     1,     4,     4,     7,
       3,     4,     4,     0,     9,     1,     3,     1,     3,     3,
       5,     3,     3,     3,     3,     3,     5,     2,     1,     1,
       3,     5,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     2,     1,     1,     3,     3,     1,
       3,     0,     1,     3,     1,     1,     1,     1,     2,     2,
       1,     2,     1,     2,     0,     4,     1,     2,     2,     5,
       2,     1,     1,     1,     2,     2,     2,     0,     0,     9,
       3,     2,     1,     4,     2,     3,     2,     3,     2,     2,
       0,     3,     2,     1,     2,     1,     1,     1,     2,     4,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     4,     1,     3,     4,
       2,     2,     2,     2,     6,     6,     3,     6,     6,     1,
       8,     8,     6,     4,     1,     6,     6,     8,     8,     8,
       6,     1,     1,     4,     1,     2,     0,     1,     3,     1,
       1,     1,     4
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
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
  case 2: /* program: pas  */
#line 106 "awkgram.y"
          { if (errorflag == 0){
			if(winner == NULL)
				winner = (Node *)stat3(PROGRAM, beginloc, (yyvsp[0].p), endloc);
			else
				runnerup = (Node *)stat3(PROGRAM, (yyvsp[0].p), NIL, NIL);
		}
	  }
#line 2387 "awkgram.tab.c"
    break;

  case 3: /* program: error  */
#line 113 "awkgram.y"
                { yyclearin; bracecheck(); SYNTAX("bailing out"); }
#line 2393 "awkgram.tab.c"
    break;

  case 14: /* $@1: %empty  */
#line 137 "awkgram.y"
                                                                                       {inloop++;}
#line 2399 "awkgram.tab.c"
    break;

  case 15: /* for: FOR '(' opt_simple_stmt ';' opt_nl pattern ';' opt_nl opt_simple_stmt rparen $@1 stmt  */
#line 138 "awkgram.y"
                { --inloop; (yyval.p) = stat4(FOR, (yyvsp[-9].p), notnull((yyvsp[-6].p)), (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2405 "awkgram.tab.c"
    break;

  case 16: /* $@2: %empty  */
#line 139 "awkgram.y"
                                                                         {inloop++;}
#line 2411 "awkgram.tab.c"
    break;

  case 17: /* for: FOR '(' opt_simple_stmt ';' ';' opt_nl opt_simple_stmt rparen $@2 stmt  */
#line 140 "awkgram.y"
                { --inloop; (yyval.p) = stat4(FOR, (yyvsp[-7].p), NIL, (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2417 "awkgram.tab.c"
    break;

  case 18: /* $@3: %empty  */
#line 141 "awkgram.y"
                                            {inloop++;}
#line 2423 "awkgram.tab.c"
    break;

  case 19: /* for: FOR '(' varname IN varname rparen $@3 stmt  */
#line 142 "awkgram.y"
                { --inloop; (yyval.p) = stat3(IN, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (yyvsp[0].p)); }
#line 2429 "awkgram.tab.c"
    break;

  case 20: /* funcname: VAR  */
#line 146 "awkgram.y"
                { setfname((yyvsp[0].cp)); }
#line 2435 "awkgram.tab.c"
    break;

  case 21: /* funcname: CALL  */
#line 147 "awkgram.y"
                { setfname((yyvsp[0].cp)); }
#line 2441 "awkgram.tab.c"
    break;

  case 22: /* if: IF '(' pattern rparen  */
#line 151 "awkgram.y"
                                        { (yyval.p) = notnull((yyvsp[-1].p)); }
#line 2447 "awkgram.tab.c"
    break;

  case 27: /* opt_nl: %empty  */
#line 163 "awkgram.y"
                        { (yyval.i) = 0; }
#line 2453 "awkgram.tab.c"
    break;

  case 29: /* opt_pst: %empty  */
#line 168 "awkgram.y"
                        { (yyval.i) = 0; }
#line 2459 "awkgram.tab.c"
    break;

  case 31: /* opt_simple_stmt: %empty  */
#line 174 "awkgram.y"
                                        { (yyval.p) = 0; }
#line 2465 "awkgram.tab.c"
    break;

  case 33: /* pas: opt_pst  */
#line 179 "awkgram.y"
                                        { (yyval.p) = 0; }
#line 2471 "awkgram.tab.c"
    break;

  case 34: /* pas: opt_pst pa_stats opt_pst  */
#line 180 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2477 "awkgram.tab.c"
    break;

  case 35: /* pa_pat: pattern  */
#line 184 "awkgram.y"
                        { (yyval.p) = notnull((yyvsp[0].p)); }
#line 2483 "awkgram.tab.c"
    break;

  case 36: /* pa_stat: pa_pat  */
#line 188 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, (yyvsp[0].p), stat2(PRINT, rectonode(), NIL)); }
#line 2489 "awkgram.tab.c"
    break;

  case 37: /* pa_stat: pa_pat lbrace stmtlist '}'  */
#line 189 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2495 "awkgram.tab.c"
    break;

  case 38: /* pa_stat: pa_pat ',' opt_nl pa_pat  */
#line 190 "awkgram.y"
                                                { (yyval.p) = pa2stat((yyvsp[-3].p), (yyvsp[0].p), stat2(PRINT, rectonode(), NIL)); }
#line 2501 "awkgram.tab.c"
    break;

  case 39: /* pa_stat: pa_pat ',' opt_nl pa_pat lbrace stmtlist '}'  */
#line 191 "awkgram.y"
                                                        { (yyval.p) = pa2stat((yyvsp[-6].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2507 "awkgram.tab.c"
    break;

  case 40: /* pa_stat: lbrace stmtlist '}'  */
#line 192 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, NIL, (yyvsp[-1].p)); }
#line 2513 "awkgram.tab.c"
    break;

  case 41: /* pa_stat: XBEGIN lbrace stmtlist '}'  */
#line 194 "awkgram.y"
                { beginloc = linkum(beginloc, (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2519 "awkgram.tab.c"
    break;

  case 42: /* pa_stat: XEND lbrace stmtlist '}'  */
#line 196 "awkgram.y"
                { endloc = linkum(endloc, (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2525 "awkgram.tab.c"
    break;

  case 43: /* $@4: %empty  */
#line 197 "awkgram.y"
                                           {infunc = true;}
#line 2531 "awkgram.tab.c"
    break;

  case 44: /* pa_stat: FUNC funcname '(' varlist rparen $@4 lbrace stmtlist '}'  */
#line 198 "awkgram.y"
                { infunc = false; curfname=0; defn((Cell *)(yyvsp[-7].p), (yyvsp[-5].p), (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2537 "awkgram.tab.c"
    break;

  case 46: /* pa_stats: pa_stats opt_pst pa_stat  */
#line 203 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2543 "awkgram.tab.c"
    break;

  case 48: /* patlist: patlist comma pattern  */
#line 208 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2549 "awkgram.tab.c"
    break;

  case 49: /* ppattern: var ASGNOP ppattern  */
#line 212 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2555 "awkgram.tab.c"
    break;

  case 50: /* ppattern: ppattern '?' ppattern ':' ppattern  */
#line 214 "awkgram.y"
                { (yyval.p) = op3(CONDEXPR, notnull((yyvsp[-4].p)), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2561 "awkgram.tab.c"
    break;

  case 51: /* ppattern: ppattern lor ppattern  */
#line 216 "awkgram.y"
                { (yyval.p) = op2(LOR, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2567 "awkgram.tab.c"
    break;

  case 52: /* ppattern: ppattern land ppattern  */
#line 218 "awkgram.y"
                { (yyval.p) = op2(LAND, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2573 "awkgram.tab.c"
    break;

  case 53: /* ppattern: ppattern MATCHOP reg_expr  */
#line 219 "awkgram.y"
                                        { (yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2579 "awkgram.tab.c"
    break;

  case 54: /* ppattern: ppattern MATCHOP ppattern  */
#line 221 "awkgram.y"
                { if (constnode((yyvsp[0].p)))
			(yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa(strnode((yyvsp[0].p)), 0));
		  else
			(yyval.p) = op3((yyvsp[-1].i), (Node *)1, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2588 "awkgram.tab.c"
    break;

  case 55: /* ppattern: ppattern IN varname  */
#line 225 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-2].p), makearr((yyvsp[0].p))); }
#line 2594 "awkgram.tab.c"
    break;

  case 56: /* ppattern: '(' plist ')' IN varname  */
#line 226 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-3].p), makearr((yyvsp[0].p))); }
#line 2600 "awkgram.tab.c"
    break;

  case 57: /* ppattern: ppattern term  */
#line 227 "awkgram.y"
                                        { (yyval.p) = op2(CAT, (yyvsp[-1].p), (yyvsp[0].p)); }
#line 2606 "awkgram.tab.c"
    break;

  case 60: /* pattern: var ASGNOP pattern  */
#line 233 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2612 "awkgram.tab.c"
    break;

  case 61: /* pattern: pattern '?' pattern ':' pattern  */
#line 235 "awkgram.y"
                { (yyval.p) = op3(CONDEXPR, notnull((yyvsp[-4].p)), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2618 "awkgram.tab.c"
    break;

  case 62: /* pattern: pattern lor pattern  */
#line 237 "awkgram.y"
                { (yyval.p) = op2(LOR, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2624 "awkgram.tab.c"
    break;

  case 63: /* pattern: pattern land pattern  */
#line 239 "awkgram.y"
                { (yyval.p) = op2(LAND, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2630 "awkgram.tab.c"
    break;

  case 64: /* pattern: pattern EQ pattern  */
#line 240 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2636 "awkgram.tab.c"
    break;

  case 65: /* pattern: pattern GE pattern  */
#line 241 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2642 "awkgram.tab.c"
    break;

  case 66: /* pattern: pattern GT pattern  */
#line 242 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2648 "awkgram.tab.c"
    break;

  case 67: /* pattern: pattern LE pattern  */
#line 243 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2654 "awkgram.tab.c"
    break;

  case 68: /* pattern: pattern LT pattern  */
#line 244 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2660 "awkgram.tab.c"
    break;

  case 69: /* pattern: pattern NE pattern  */
#line 245 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2666 "awkgram.tab.c"
    break;

  case 70: /* pattern: pattern MATCHOP reg_expr  */
#line 246 "awkgram.y"
                                        { (yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2672 "awkgram.tab.c"
    break;

  case 71: /* pattern: pattern MATCHOP pattern  */
#line 248 "awkgram.y"
                { if (constnode((yyvsp[0].p)))
			(yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa(strnode((yyvsp[0].p)), 0));
		  else
			(yyval.p) = op3((yyvsp[-1].i), (Node *)1, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2681 "awkgram.tab.c"
    break;

  case 72: /* pattern: pattern IN varname  */
#line 252 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-2].p), makearr((yyvsp[0].p))); }
#line 2687 "awkgram.tab.c"
    break;

  case 73: /* pattern: '(' plist ')' IN varname  */
#line 253 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-3].p), makearr((yyvsp[0].p))); }
#line 2693 "awkgram.tab.c"
    break;

  case 74: /* pattern: pattern term  */
#line 254 "awkgram.y"
                                        { (yyval.p) = op2(CAT, (yyvsp[-1].p), (yyvsp[0].p)); }
#line 2699 "awkgram.tab.c"
    break;

  case 77: /* plist: pattern comma pattern  */
#line 260 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2705 "awkgram.tab.c"
    break;

  case 78: /* plist: plist comma pattern  */
#line 261 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2711 "awkgram.tab.c"
    break;

  case 80: /* pplist: pplist comma ppattern  */
#line 266 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2717 "awkgram.tab.c"
    break;

  case 81: /* prarg: %empty  */
#line 270 "awkgram.y"
                                        { (yyval.p) = rectonode(); }
#line 2723 "awkgram.tab.c"
    break;

  case 83: /* prarg: '(' plist ')'  */
#line 272 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2729 "awkgram.tab.c"
    break;

  case 92: /* re: reg_expr  */
#line 289 "awkgram.y"
                { (yyval.p) = op3(MATCH, NIL, rectonode(), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2735 "awkgram.tab.c"
    break;

  case 93: /* re: NOT re  */
#line 290 "awkgram.y"
                        { (yyval.p) = op1(NOT, notnull((yyvsp[0].p))); }
#line 2741 "awkgram.tab.c"
    break;

  case 94: /* $@5: %empty  */
#line 294 "awkgram.y"
              {startreg();}
#line 2747 "awkgram.tab.c"
    break;

  case 95: /* reg_expr: '/' $@5 REGEXPR '/'  */
#line 294 "awkgram.y"
                                                { (yyval.s) = (yyvsp[-1].s); }
#line 2753 "awkgram.tab.c"
    break;

  case 98: /* simple_stmt: print prarg  */
#line 302 "awkgram.y"
                                        { (yyval.p) = stat3((yyvsp[-1].i), (yyvsp[0].p), NIL, NIL); }
#line 2759 "awkgram.tab.c"
    break;

  case 99: /* simple_stmt: DELETE varname '[' patlist ']'  */
#line 303 "awkgram.y"
                                         { (yyval.p) = stat2(DELETE, makearr((yyvsp[-3].p)), (yyvsp[-1].p)); }
#line 2765 "awkgram.tab.c"
    break;

  case 100: /* simple_stmt: DELETE varname  */
#line 304 "awkgram.y"
                                         { (yyval.p) = stat2(DELETE, makearr((yyvsp[0].p)), 0); }
#line 2771 "awkgram.tab.c"
    break;

  case 101: /* simple_stmt: pattern  */
#line 305 "awkgram.y"
                                        { (yyval.p) = exptostat((yyvsp[0].p)); }
#line 2777 "awkgram.tab.c"
    break;

  case 102: /* simple_stmt: error  */
#line 306 "awkgram.y"
                                        { yyclearin; SYNTAX("illegal statement"); }
#line 2783 "awkgram.tab.c"
    break;

  case 105: /* stmt: BREAK st  */
#line 315 "awkgram.y"
                                { if (!inloop) SYNTAX("break illegal outside of loops");
				  (yyval.p) = stat1(BREAK, NIL); }
#line 2790 "awkgram.tab.c"
    break;

  case 106: /* stmt: CONTINUE st  */
#line 317 "awkgram.y"
                                {  if (!inloop) SYNTAX("continue illegal outside of loops");
				  (yyval.p) = stat1(CONTINUE, NIL); }
#line 2797 "awkgram.tab.c"
    break;

  case 107: /* $@6: %empty  */
#line 319 "awkgram.y"
             {inloop++;}
#line 2803 "awkgram.tab.c"
    break;

  case 108: /* $@7: %empty  */
#line 319 "awkgram.y"
                              {--inloop;}
#line 2809 "awkgram.tab.c"
    break;

  case 109: /* stmt: do $@6 stmt $@7 WHILE '(' pattern ')' st  */
#line 320 "awkgram.y"
                { (yyval.p) = stat2(DO, (yyvsp[-6].p), notnull((yyvsp[-2].p))); }
#line 2815 "awkgram.tab.c"
    break;

  case 110: /* stmt: EXIT pattern st  */
#line 321 "awkgram.y"
                                { (yyval.p) = stat1(EXIT, (yyvsp[-1].p)); }
#line 2821 "awkgram.tab.c"
    break;

  case 111: /* stmt: EXIT st  */
#line 322 "awkgram.y"
                                { (yyval.p) = stat1(EXIT, NIL); }
#line 2827 "awkgram.tab.c"
    break;

  case 113: /* stmt: if stmt else stmt  */
#line 324 "awkgram.y"
                                { (yyval.p) = stat3(IF, (yyvsp[-3].p), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2833 "awkgram.tab.c"
    break;

  case 114: /* stmt: if stmt  */
#line 325 "awkgram.y"
                                { (yyval.p) = stat3(IF, (yyvsp[-1].p), (yyvsp[0].p), NIL); }
#line 2839 "awkgram.tab.c"
    break;

  case 115: /* stmt: lbrace stmtlist rbrace  */
#line 326 "awkgram.y"
                                 { (yyval.p) = (yyvsp[-1].p); }
#line 2845 "awkgram.tab.c"
    break;

  case 116: /* stmt: NEXT st  */
#line 327 "awkgram.y"
                        { if (infunc)
				SYNTAX("next is illegal inside a function");
			  (yyval.p) = stat1(NEXT, NIL); }
#line 2853 "awkgram.tab.c"
    break;

  case 117: /* stmt: RETURN pattern st  */
#line 330 "awkgram.y"
                                { (yyval.p) = stat1(RETURN, (yyvsp[-1].p)); }
#line 2859 "awkgram.tab.c"
    break;

  case 118: /* stmt: RETURN st  */
#line 331 "awkgram.y"
                                { (yyval.p) = stat1(RETURN, NIL); }
#line 2865 "awkgram.tab.c"
    break;

  case 120: /* $@8: %empty  */
#line 333 "awkgram.y"
                {inloop++;}
#line 2871 "awkgram.tab.c"
    break;

  case 121: /* stmt: while $@8 stmt  */
#line 333 "awkgram.y"
                                        { --inloop; (yyval.p) = stat2(WHILE, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2877 "awkgram.tab.c"
    break;

  case 122: /* stmt: ';' opt_nl  */
#line 334 "awkgram.y"
                                { (yyval.p) = 0; }
#line 2883 "awkgram.tab.c"
    break;

  case 124: /* stmtlist: stmtlist stmt  */
#line 339 "awkgram.y"
                                { (yyval.p) = linkum((yyvsp[-1].p), (yyvsp[0].p)); }
#line 2889 "awkgram.tab.c"
    break;

  case 128: /* string: string STRING  */
#line 348 "awkgram.y"
                                { (yyval.cp) = catstr((yyvsp[-1].cp), (yyvsp[0].cp)); }
#line 2895 "awkgram.tab.c"
    break;

  case 129: /* term: term '/' ASGNOP term  */
#line 352 "awkgram.y"
                                        { (yyval.p) = op2(DIVEQ, (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2901 "awkgram.tab.c"
    break;

  case 130: /* term: term '+' term  */
#line 353 "awkgram.y"
                                        { (yyval.p) = op2(ADD, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2907 "awkgram.tab.c"
    break;

  case 131: /* term: term '-' term  */
#line 354 "awkgram.y"
                                        { (yyval.p) = op2(MINUS, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2913 "awkgram.tab.c"
    break;

  case 132: /* term: term '*' term  */
#line 355 "awkgram.y"
                                        { (yyval.p) = op2(MULT, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2919 "awkgram.tab.c"
    break;

  case 133: /* term: term '/' term  */
#line 356 "awkgram.y"
                                        { (yyval.p) = op2(DIVIDE, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2925 "awkgram.tab.c"
    break;

  case 134: /* term: term '%' term  */
#line 357 "awkgram.y"
                                        { (yyval.p) = op2(MOD, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2931 "awkgram.tab.c"
    break;

  case 135: /* term: term '^' term  */
#line 358 "awkgram.y"
                                        { (yyval.p) = op2(XOR, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2937 "awkgram.tab.c"
    break;

  case 136: /* term: term '|' term  */
#line 359 "awkgram.y"
                                        { (yyval.p) = op2(BOR, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2943 "awkgram.tab.c"
    break;

  case 137: /* term: term '&' term  */
#line 360 "awkgram.y"
                                        { (yyval.p) = op2(BAND, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2949 "awkgram.tab.c"
    break;

  case 138: /* term: term POWER term  */
#line 361 "awkgram.y"
                                        { (yyval.p) = op2(POWER, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2955 "awkgram.tab.c"
    break;

  case 139: /* term: term LSHIFT term  */
#line 362 "awkgram.y"
                                        { (yyval.p) = op2(LSHIFT, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2961 "awkgram.tab.c"
    break;

  case 140: /* term: term RSHIFT term  */
#line 363 "awkgram.y"
                                        { (yyval.p) = op2(RSHIFT, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2967 "awkgram.tab.c"
    break;

  case 141: /* term: CMPL term  */
#line 364 "awkgram.y"
                                        { (yyval.p) = op1(CMPL, (yyvsp[0].p)); }
#line 2973 "awkgram.tab.c"
    break;

  case 142: /* term: '-' term  */
#line 365 "awkgram.y"
                                        { (yyval.p) = op1(UMINUS, (yyvsp[0].p)); }
#line 2979 "awkgram.tab.c"
    break;

  case 143: /* term: '+' term  */
#line 366 "awkgram.y"
                                        { (yyval.p) = op1(UPLUS, (yyvsp[0].p)); }
#line 2985 "awkgram.tab.c"
    break;

  case 144: /* term: NOT term  */
#line 367 "awkgram.y"
                                        { (yyval.p) = op1(NOT, notnull((yyvsp[0].p))); }
#line 2991 "awkgram.tab.c"
    break;

  case 145: /* term: BLTIN '(' ')'  */
#line 368 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[-2].i)), rectonode()); }
#line 2997 "awkgram.tab.c"
    break;

  case 146: /* term: BLTIN '(' patlist ')'  */
#line 369 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[-3].i)), (yyvsp[-1].p)); }
#line 3003 "awkgram.tab.c"
    break;

  case 147: /* term: BLTIN  */
#line 370 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[0].i)), rectonode()); }
#line 3009 "awkgram.tab.c"
    break;

  case 148: /* term: CALL '(' ')'  */
#line 371 "awkgram.y"
                                        { (yyval.p) = op2(CALL, celltonode((yyvsp[-2].cp),CVAR), NIL); }
#line 3015 "awkgram.tab.c"
    break;

  case 149: /* term: CALL '(' patlist ')'  */
#line 372 "awkgram.y"
                                        { (yyval.p) = op2(CALL, celltonode((yyvsp[-3].cp),CVAR), (yyvsp[-1].p)); }
#line 3021 "awkgram.tab.c"
    break;

  case 150: /* term: DECR var  */
#line 373 "awkgram.y"
                                        { (yyval.p) = op1(PREDECR, (yyvsp[0].p)); }
#line 3027 "awkgram.tab.c"
    break;

  case 151: /* term: INCR var  */
#line 374 "awkgram.y"
                                        { (yyval.p) = op1(PREINCR, (yyvsp[0].p)); }
#line 3033 "awkgram.tab.c"
    break;

  case 152: /* term: var DECR  */
#line 375 "awkgram.y"
                                        { (yyval.p) = op1(POSTDECR, (yyvsp[-1].p)); }
#line 3039 "awkgram.tab.c"
    break;

  case 153: /* term: var INCR  */
#line 376 "awkgram.y"
                                        { (yyval.p) = op1(POSTINCR, (yyvsp[-1].p)); }
#line 3045 "awkgram.tab.c"
    break;

  case 154: /* term: INDEX '(' pattern comma pattern ')'  */
#line 378 "awkgram.y"
                { (yyval.p) = op2(INDEX, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3051 "awkgram.tab.c"
    break;

  case 155: /* term: INDEX '(' pattern comma reg_expr ')'  */
#line 380 "awkgram.y"
                { SYNTAX("index() doesn't permit regular expressions");
		  (yyval.p) = op2(INDEX, (yyvsp[-3].p), (Node*)(yyvsp[-1].s)); }
#line 3058 "awkgram.tab.c"
    break;

  case 156: /* term: '(' pattern ')'  */
#line 382 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 3064 "awkgram.tab.c"
    break;

  case 157: /* term: MATCHFCN '(' pattern comma reg_expr ')'  */
#line 384 "awkgram.y"
                { (yyval.p) = op3(MATCHFCN, NIL, (yyvsp[-3].p), (Node*)makedfa((yyvsp[-1].s), 1)); free((yyvsp[-1].s)); }
#line 3070 "awkgram.tab.c"
    break;

  case 158: /* term: MATCHFCN '(' pattern comma pattern ')'  */
#line 386 "awkgram.y"
                { if (constnode((yyvsp[-1].p)))
			(yyval.p) = op3(MATCHFCN, NIL, (yyvsp[-3].p), (Node*)makedfa(strnode((yyvsp[-1].p)), 1));
		  else
			(yyval.p) = op3(MATCHFCN, (Node *)1, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3079 "awkgram.tab.c"
    break;

  case 159: /* term: NUMBER  */
#line 390 "awkgram.y"
                                        { (yyval.p) = celltonode((yyvsp[0].cp), CCON); }
#line 3085 "awkgram.tab.c"
    break;

  case 160: /* term: SPLIT '(' pattern comma varname comma pattern ')'  */
#line 392 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (yyvsp[-1].p), (Node*)STRING); }
#line 3091 "awkgram.tab.c"
    break;

  case 161: /* term: SPLIT '(' pattern comma varname comma reg_expr ')'  */
#line 394 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (Node*)makedfa((yyvsp[-1].s), 1), (Node *)REGEXPR); free((yyvsp[-1].s)); }
#line 3097 "awkgram.tab.c"
    break;

  case 162: /* term: SPLIT '(' pattern comma varname ')'  */
#line 396 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-3].p), makearr((yyvsp[-1].p)), NIL, (Node*)STRING); }
#line 3103 "awkgram.tab.c"
    break;

  case 163: /* term: SPRINTF '(' patlist ')'  */
#line 397 "awkgram.y"
                                        { (yyval.p) = op1((yyvsp[-3].i), (yyvsp[-1].p)); }
#line 3109 "awkgram.tab.c"
    break;

  case 164: /* term: string  */
#line 398 "awkgram.y"
                                        { (yyval.p) = celltonode((yyvsp[0].cp), CCON); }
#line 3115 "awkgram.tab.c"
    break;

  case 165: /* term: subop '(' reg_expr comma pattern ')'  */
#line 400 "awkgram.y"
                { (yyval.p) = op4((yyvsp[-5].i), NIL, (Node*)makedfa((yyvsp[-3].s), 1), (yyvsp[-1].p), rectonode()); free((yyvsp[-3].s)); }
#line 3121 "awkgram.tab.c"
    break;

  case 166: /* term: subop '(' pattern comma pattern ')'  */
#line 402 "awkgram.y"
                { if (constnode((yyvsp[-3].p)))
			(yyval.p) = op4((yyvsp[-5].i), NIL, (Node*)makedfa(strnode((yyvsp[-3].p)), 1), (yyvsp[-1].p), rectonode());
		  else
			(yyval.p) = op4((yyvsp[-5].i), (Node *)1, (yyvsp[-3].p), (yyvsp[-1].p), rectonode()); }
#line 3130 "awkgram.tab.c"
    break;

  case 167: /* term: subop '(' reg_expr comma pattern comma var ')'  */
#line 407 "awkgram.y"
                { (yyval.p) = op4((yyvsp[-7].i), NIL, (Node*)makedfa((yyvsp[-5].s), 1), (yyvsp[-3].p), (yyvsp[-1].p)); free((yyvsp[-5].s)); }
#line 3136 "awkgram.tab.c"
    break;

  case 168: /* term: subop '(' pattern comma pattern comma var ')'  */
#line 409 "awkgram.y"
                { if (constnode((yyvsp[-5].p)))
			(yyval.p) = op4((yyvsp[-7].i), NIL, (Node*)makedfa(strnode((yyvsp[-5].p)), 1), (yyvsp[-3].p), (yyvsp[-1].p));
		  else
			(yyval.p) = op4((yyvsp[-7].i), (Node *)1, (yyvsp[-5].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3145 "awkgram.tab.c"
    break;

  case 169: /* term: SUBSTR '(' pattern comma pattern comma pattern ')'  */
#line 414 "awkgram.y"
                { (yyval.p) = op3(SUBSTR, (yyvsp[-5].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3151 "awkgram.tab.c"
    break;

  case 170: /* term: SUBSTR '(' pattern comma pattern ')'  */
#line 416 "awkgram.y"
                { (yyval.p) = op3(SUBSTR, (yyvsp[-3].p), (yyvsp[-1].p), NIL); }
#line 3157 "awkgram.tab.c"
    break;

  case 173: /* var: varname '[' patlist ']'  */
#line 422 "awkgram.y"
                                        { (yyval.p) = op2(ARRAY, makearr((yyvsp[-3].p)), (yyvsp[-1].p)); }
#line 3163 "awkgram.tab.c"
    break;

  case 174: /* var: IVAR  */
#line 423 "awkgram.y"
                                        { (yyval.p) = op1(INDIRECT, celltonode((yyvsp[0].cp), CVAR)); }
#line 3169 "awkgram.tab.c"
    break;

  case 175: /* var: INDIRECT term  */
#line 424 "awkgram.y"
                                        { (yyval.p) = op1(INDIRECT, (yyvsp[0].p)); }
#line 3175 "awkgram.tab.c"
    break;

  case 176: /* varlist: %empty  */
#line 428 "awkgram.y"
                                { arglist = (yyval.p) = 0; }
#line 3181 "awkgram.tab.c"
    break;

  case 177: /* varlist: VAR  */
#line 429 "awkgram.y"
                                { arglist = (yyval.p) = celltonode((yyvsp[0].cp),CVAR); }
#line 3187 "awkgram.tab.c"
    break;

  case 178: /* varlist: varlist comma VAR  */
#line 430 "awkgram.y"
                                {
			checkdup((yyvsp[-2].p), (yyvsp[0].cp));
			arglist = (yyval.p) = linkum((yyvsp[-2].p),celltonode((yyvsp[0].cp),CVAR)); }
#line 3195 "awkgram.tab.c"
    break;

  case 179: /* varname: VAR  */
#line 436 "awkgram.y"
                                { (yyval.p) = celltonode((yyvsp[0].cp), CVAR); }
#line 3201 "awkgram.tab.c"
    break;

  case 180: /* varname: ARG  */
#line 437 "awkgram.y"
                                { (yyval.p) = op1(ARG, itonp((yyvsp[0].i))); }
#line 3207 "awkgram.tab.c"
    break;

  case 181: /* varname: VARNF  */
#line 438 "awkgram.y"
                                { (yyval.p) = op1(VARNF, (Node *) (yyvsp[0].cp)); }
#line 3213 "awkgram.tab.c"
    break;

  case 182: /* while: WHILE '(' pattern rparen  */
#line 443 "awkgram.y"
                                        { (yyval.p) = notnull((yyvsp[-1].p)); }
#line 3219 "awkgram.tab.c"
    break;


#line 3223 "awkgram.tab.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 446 "awkgram.y"


void setfname(Cell *p)
{
	if (isarr(p))
		SYNTAX("%s is an array, not a function", p->nval);
	else if (isfcn(p))
		SYNTAX("you can't define function %s more than once", p->nval);
	curfname = p->nval;
}

int constnode(Node *p)
{
	return isvalue(p) && ((Cell *) (p->narg[0]))->csub == CCON;
}

char *strnode(Node *p)
{
	return ((Cell *)(p->narg[0]))->sval;
}

Node *notnull(Node *n)
{
	switch (n->nobj) {
	case LE: case LT: case EQ: case NE: case GT: case GE:
	case LOR: case LAND: case NOT:
		return n;
	default:
		return op2(NE, n, nullnode);
	}
}

void checkdup(Node *vl, Cell *cp)	/* check if name already in list */
{
	char *s = cp->nval;
	for ( ; vl; vl = vl->nnext) {
		if (strcmp(s, ((Cell *)(vl->narg[0]))->nval) == 0) {
			SYNTAX("duplicate argument %s", s);
			break;
		}
	}
}
