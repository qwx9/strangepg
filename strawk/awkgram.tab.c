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
#define YYLAST   4758

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  123
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  180
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  359

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
     213,   215,   217,   219,   220,   226,   227,   228,   229,   230,
     234,   235,   237,   239,   241,   242,   243,   244,   245,   246,
     247,   248,   254,   255,   256,   257,   258,   262,   263,   267,
     268,   272,   273,   274,   278,   278,   282,   282,   282,   282,
     286,   286,   290,   292,   296,   296,   300,   300,   304,   305,
     306,   307,   308,   312,   313,   317,   319,   321,   321,   321,
     323,   324,   325,   326,   327,   328,   329,   332,   333,   334,
     335,   335,   336,   340,   341,   345,   345,   349,   350,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   379,   382,   383,   385,   391,   392,   394,
     396,   398,   399,   400,   402,   408,   410,   416,   418,   420,
     424,   425,   426,   427,   431,   432,   433,   439,   440,   441,
     446
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

#define YYPACT_NINF (-306)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-32)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     715,  -306,  -306,  -306,    31,  1678,  -306,   164,  -306,    25,
      25,  -306,  4355,  -306,  -306,    36,    -4,  -306,  -306,    57,
      90,  4640,    97,  -306,  -306,  -306,   128,  -306,  -306,   134,
     135,  4640,  4640,  4412,     2,     2,  4640,   791,    49,  -306,
     173,  3593,  -306,  -306,   143,    19,   234,   -35,   140,  -306,
    -306,   791,   791,  2275,    23,    58,  4127,  -306,  -306,   156,
    4355,  4355,  4355,  4640,    41,   -25,  4355,  4184,  4355,  4355,
      41,    41,  -306,    41,  -306,  -306,     9,  -306,  -306,   158,
     176,   176,   -16,  -306,  1842,   171,   182,   176,  -306,  -306,
    1842,   183,   190,  -306,  1477,   791,  3593,  4469,   176,  -306,
     867,  -306,   158,   791,  1678,  4355,  -306,  -306,  4355,  4355,
    4355,  4355,  4355,  4355,   -16,  4355,  1899,  1956,   234,  4355,
    -306,  4526,  4640,  4640,  4640,  4640,  4640,  4640,  4640,  4640,
    4355,  -306,  -306,  4355,   943,  1019,  -306,  -306,  2013,   157,
    2013,   191,  -306,    24,  3593,   113,  2703,  2703,  2791,    35,
    -306,    37,  2703,  2703,  -306,   199,  -306,   158,   199,  -306,
    -306,   193,  1776,  -306,  1543,  4355,  -306,  1776,  -306,  4355,
    -306,  1477,   124,  1095,  4355,  3992,   202,  -306,  -306,   234,
     -30,  -306,  -306,  -306,  1477,  4355,  1171,  -306,  3769,  -306,
    3769,  3769,  3769,  3769,  3769,  3769,  -306,  2879,  -306,  3857,
    -306,  3681,  2703,   202,  4640,    41,    81,    81,    41,    41,
      41,   234,   234,   234,  3593,    17,  -306,  -306,  -306,  3593,
     -16,  3593,  -306,  -306,  2013,  -306,    39,  2013,  2013,  -306,
    -306,    21,  2013,  -306,  -306,  4355,  -306,   203,  -306,     0,
    2967,  -306,  2967,  -306,  -306,  1249,  -306,   204,   106,  4583,
     -16,  4583,  2070,  2127,   234,  2184,  4583,  -306,    25,  -306,
    4355,  2013,  2013,   234,  -306,  -306,  3593,  -306,     7,   208,
    3055,   205,  3143,   209,   122,  2384,    22,   184,   -16,   208,
     208,   112,  -306,  -306,  -306,   185,  4355,  4241,  -306,  -306,
    3914,  4298,  4070,  3992,  3992,   791,  3593,  2493,  2602,  -306,
    -306,    25,  -306,  -306,  -306,  -306,  -306,  2013,  -306,  2013,
    -306,   158,  4355,   210,   214,   -16,   155,  4583,  1325,  -306,
      80,  -306,    80,   791,  3231,   219,  3319,  1609,  3417,   208,
    4355,  -306,   185,  3992,  -306,   235,   236,  1401,  -306,  -306,
    -306,   210,   158,  1477,  3505,  -306,  -306,  -306,   208,  1609,
    -306,   176,  1477,   210,  -306,  -306,   208,  1477,  -306
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     3,    86,    87,     0,    33,     2,    30,     1,     0,
       0,    23,     0,    94,   178,   145,     0,   125,   126,     0,
       0,     0,     0,   177,   172,   179,     0,   157,   127,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    45,
      29,    35,    75,    92,     0,   162,    76,   169,   170,    88,
      89,     0,     0,     0,     0,     0,     0,    20,    21,     0,
       0,     0,     0,     0,   139,   169,     0,     0,     0,     0,
     141,   140,    93,   142,   148,   149,   173,   102,    24,    27,
       0,     0,     0,    10,     0,     0,     0,     0,    84,    85,
       0,     0,   107,   112,     0,     0,   101,    81,     0,   123,
       0,   120,    27,     0,    34,     0,     4,     6,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,     0,
     128,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   150,   151,     0,     0,     0,     8,   154,     0,     0,
       0,     0,   143,     0,    47,   174,     0,     0,     0,     0,
     146,     0,     0,     0,    25,    28,   122,    27,   103,   105,
     106,   100,     0,   111,     0,     0,   116,     0,   118,     0,
      11,     0,   114,     0,     0,    79,    82,    98,    58,    59,
     169,   119,    40,   124,     0,     0,     0,    46,    71,    70,
      64,    65,    66,    67,    68,    69,    72,     0,     5,    63,
       7,    62,     0,    92,     0,   133,   130,   131,   132,   134,
     138,   135,   136,   137,    60,     0,    41,    42,     9,    77,
       0,    78,    95,   144,     0,   175,     0,     0,     0,   161,
     147,     0,     0,    26,   104,     0,   110,     0,    32,   170,
       0,   117,     0,   108,    12,     0,    90,   115,     0,     0,
       0,     0,     0,     0,    57,     0,     0,   121,    38,    37,
       0,     0,     0,   129,   171,    73,    48,    96,     0,    43,
       0,    92,     0,    92,     0,     0,     0,    27,     0,    22,
     180,     0,    13,   113,    91,    83,     0,    54,    53,    55,
       0,    52,    51,    80,    49,     0,    61,     0,     0,   176,
      97,     0,   152,   153,   156,   155,   160,     0,   168,     0,
      99,    27,     0,     0,     0,     0,     0,     0,     0,   164,
       0,   163,     0,     0,     0,    92,     0,     0,     0,    18,
       0,    56,     0,    50,    39,     0,     0,     0,   158,   159,
     167,     0,    27,     0,     0,   166,   165,    44,    16,     0,
      19,     0,     0,     0,   109,    17,    14,     0,    15
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -306,  -306,  -149,    -3,   200,  -306,  -306,  -306,  -306,  -306,
    -306,  -306,  -306,    15,   -73,   -90,   220,  -305,  -306,    64,
     159,  -306,  -306,   -57,  -181,    -5,  -173,  -306,  -306,  -306,
    -306,  -306,   -20,  -101,  -306,  -219,  -161,   -79,   356,  -306,
    -306,  -306,   -37,  -306,  -306,   386,   118,  -306,   -41,  -306
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     4,   116,   117,   224,    92,   245,    93,   357,   352,
     343,    59,    94,    95,   158,   156,     5,   237,     6,    38,
      39,   301,    40,   143,   175,    96,    54,   176,   177,    97,
       7,   247,    42,    43,    55,   269,    98,   159,    99,   171,
     281,   184,   100,    44,    45,    46,    47,   226,    48,   101
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      41,   248,   160,   238,   189,   163,   155,    53,   166,   149,
     151,   168,   185,    72,   134,   135,   218,   133,   203,   181,
      37,   279,   341,   280,    51,    52,   252,   136,    14,   155,
     218,     8,   136,   136,   136,   264,    11,   130,   139,   223,
     310,   161,   256,   278,   353,   136,    14,   136,    56,   136,
     229,   144,   230,   103,   267,   146,   147,   148,   173,   102,
      11,   144,   144,   152,   153,    14,   186,   234,   287,    60,
     290,   291,   292,   196,   293,   294,   215,   178,    23,   162,
      25,   131,   132,   236,   155,   167,   131,   132,   241,   218,
      57,   131,   132,    58,   329,   121,    23,    24,    25,    41,
     188,   299,    61,   190,   191,   192,   193,   194,   195,    66,
     197,   199,   201,   316,   202,    23,   136,    25,   120,    37,
      36,   285,   348,   239,    14,   214,   271,   273,   144,   127,
     128,   129,   136,   219,   356,   221,   333,   306,   252,    65,
      67,   252,   252,   252,   252,   252,    68,    69,   288,    65,
      65,    65,    74,    75,    65,   119,   126,   133,   141,    65,
     240,   127,   128,   129,   242,   136,   238,   154,   145,    53,
     332,    65,   253,    49,    23,    24,    25,    50,   276,   265,
      41,    65,     2,   164,   252,   154,     3,   312,   238,   157,
     274,   124,   125,   154,   165,   169,   126,   311,    36,   170,
     220,   127,   128,   129,   155,   222,   325,   225,   233,   289,
     235,   244,   136,   284,    65,   180,   277,   300,   314,   266,
     303,   327,   270,   272,   305,   267,   330,   275,   315,   178,
     144,   178,   178,   178,   339,   178,   178,   313,   155,    65,
      65,    65,    65,    65,    65,    65,    65,    65,   121,   258,
     345,   346,   349,   138,   140,   296,   297,   298,   318,     0,
     104,     0,    65,   187,    65,    65,    65,     0,     0,   155,
      65,    65,   354,   295,   331,     0,     0,     0,     0,     0,
      65,    53,     0,     0,   253,    65,   337,   253,   253,   253,
     253,   253,     0,    65,     0,     0,     0,   178,     0,     0,
       0,     0,   324,     0,   326,     0,    65,   328,    65,    65,
      65,    65,    65,    65,     0,    65,   323,    65,     0,    65,
      65,     0,    65,     0,     0,   344,     0,     0,     0,     0,
     253,     0,    65,     0,     0,     0,     0,    65,     0,    65,
       0,     0,   122,   123,   124,   125,   227,   228,     0,   126,
       0,     0,   231,   232,   127,   128,   129,     0,    65,     0,
      65,     0,     0,     0,     0,     0,     0,   180,     0,   180,
     180,   180,     0,   180,   180,     0,   255,     0,     0,     0,
       0,     0,     0,     0,    65,     0,     0,     0,    65,     0,
      65,     0,     0,    65,     0,     0,     0,     0,     0,     0,
       0,     0,   261,   262,     0,    65,     0,    64,    65,    65,
      65,    65,    65,     0,    65,    65,    65,    70,    71,    73,
       0,     0,    76,     0,     0,     0,   268,   118,     0,     0,
       0,     0,     0,     0,     0,   180,     0,     0,   335,   118,
     336,     0,    65,     0,    65,     0,    65,     0,   140,    73,
     172,    65,     0,     0,     0,     0,   183,     0,     0,     0,
       0,     0,    65,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   307,   309,     0,     0,     0,     0,
       0,     0,   118,   179,     0,     0,     0,     0,     0,     0,
     183,   183,     0,     0,     0,     0,     0,   320,   322,     0,
       0,     0,     0,     0,     0,     0,     0,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   140,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   243,     0,   183,
     118,     0,   118,   118,   118,     0,     0,     0,   118,   118,
     257,     0,   183,     0,     0,     0,     0,     0,   118,     0,
       0,     0,     0,   118,     0,     0,     0,     0,     0,     0,
       0,   254,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   118,     0,   118,   118,   118,   118,
     118,   118,     0,   118,     0,   118,     0,   118,   118,     0,
     263,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     118,   283,     0,     0,     0,   118,     0,   118,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   118,     0,   118,     0,
       0,     0,     0,     0,     0,   179,     0,   179,   179,   179,
       0,   179,   179,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   118,     0,     0,     0,   118,     0,   118,     0,
       0,   118,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   254,   183,     0,   254,   254,   254,   254,
     254,     0,   118,   118,   118,     0,     0,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,   350,
       0,     0,     0,   179,     0,     0,     0,     0,   355,     0,
     118,     0,   118,   358,   118,   -29,     1,     0,     0,   254,
       0,     0,   -29,   -29,     2,     0,   -29,   -29,     3,   -29,
     118,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   -29,
     -29,     0,     0,     0,     0,     0,     0,   -29,   -29,   -29,
       0,   -29,     0,   -29,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   -29,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,     0,     0,     0,
      78,   -29,    11,    12,    79,    13,     0,     0,     0,   -29,
     -29,   -29,   -29,   -29,   -29,     0,     0,     0,     0,   -29,
     -29,     0,     0,   -29,   -29,     0,     0,   -29,     0,     0,
       0,   -29,   -29,   -29,     0,    14,    15,    80,    81,    82,
      83,    84,    85,     0,    17,    18,    86,    19,     0,    20,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,     0,     0,    88,    89,    22,    11,    12,
      79,    13,     0,   182,     0,    23,    24,    25,    26,    27,
      28,     0,     0,     0,    90,    29,    30,    91,     0,    31,
      32,     0,     0,    33,     0,     0,     0,    34,    35,    36,
       0,    14,    15,    80,    81,    82,    83,    84,    85,     0,
      17,    18,    86,    19,     0,    20,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,     0,
       0,    88,    89,    22,    11,    12,    79,    13,     0,   216,
       0,    23,    24,    25,    26,    27,    28,     0,     0,     0,
      90,    29,    30,    91,     0,    31,    32,     0,     0,    33,
       0,     0,     0,    34,    35,    36,     0,    14,    15,    80,
      81,    82,    83,    84,    85,     0,    17,    18,    86,    19,
       0,    20,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,     0,     0,    88,    89,    22,
      11,    12,    79,    13,     0,   217,     0,    23,    24,    25,
      26,    27,    28,     0,     0,     0,    90,    29,    30,    91,
       0,    31,    32,     0,     0,    33,     0,     0,     0,    34,
      35,    36,     0,    14,    15,    80,    81,    82,    83,    84,
      85,     0,    17,    18,    86,    19,     0,    20,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,     0,     0,    88,    89,    22,    11,    12,    79,    13,
       0,   246,     0,    23,    24,    25,    26,    27,    28,     0,
       0,     0,    90,    29,    30,    91,     0,    31,    32,     0,
       0,    33,     0,     0,     0,    34,    35,    36,     0,    14,
      15,    80,    81,    82,    83,    84,    85,     0,    17,    18,
      86,    19,     0,    20,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,     0,     0,    88,
      89,    22,    11,    12,    79,    13,     0,   259,     0,    23,
      24,    25,    26,    27,    28,     0,     0,     0,    90,    29,
      30,    91,     0,    31,    32,     0,     0,    33,     0,     0,
       0,    34,    35,    36,     0,    14,    15,    80,    81,    82,
      83,    84,    85,     0,    17,    18,    86,    19,     0,    20,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,    88,    89,    22,   282,     0,
      11,    12,    79,    13,     0,    23,    24,    25,    26,    27,
      28,     0,     0,     0,    90,    29,    30,    91,     0,    31,
      32,     0,     0,    33,     0,     0,     0,    34,    35,    36,
       0,     0,     0,    14,    15,    80,    81,    82,    83,    84,
      85,     0,    17,    18,    86,    19,     0,    20,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,     0,     0,    88,    89,    22,    11,    12,    79,    13,
       0,   334,     0,    23,    24,    25,    26,    27,    28,     0,
       0,     0,    90,    29,    30,    91,     0,    31,    32,     0,
       0,    33,     0,     0,     0,    34,    35,    36,     0,    14,
      15,    80,    81,    82,    83,    84,    85,     0,    17,    18,
      86,    19,     0,    20,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,     0,     0,    88,
      89,    22,    11,    12,    79,    13,     0,   347,     0,    23,
      24,    25,    26,    27,    28,     0,     0,     0,    90,    29,
      30,    91,     0,    31,    32,     0,     0,    33,     0,     0,
       0,    34,    35,    36,     0,    14,    15,    80,    81,    82,
      83,    84,    85,     0,    17,    18,    86,    19,     0,    20,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,     0,     0,    88,    89,    22,    11,    12,
      79,    13,     0,     0,     0,    23,    24,    25,    26,    27,
      28,     0,     0,     0,    90,    29,    30,    91,     0,    31,
      32,     0,     0,    33,     0,     0,     0,    34,    35,    36,
       0,    14,    15,    80,    81,    82,    83,    84,    85,     0,
      17,    18,    86,    19,     0,    20,    87,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,    21,     0,     0,
       0,     0,     0,     0,     0,    12,   -31,    13,     0,     0,
       0,    88,    89,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,     0,     0,     0,
      90,    29,    30,    91,     0,    31,    32,    14,    15,    33,
       0,    82,     0,    34,    35,    36,    17,    18,     0,    19,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
      77,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,    12,     0,    13,   -31,     0,     0,    88,    89,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,     0,     0,     0,     0,    29,    30,     0,
       0,    31,    32,    14,    15,    33,     0,    82,     0,    34,
      35,    36,    17,    18,     0,    19,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     9,    10,     0,     0,    11,
      12,     0,    13,    88,    89,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
       0,     0,     0,    29,    30,     0,     0,    31,    32,     0,
       0,    33,    14,    15,     0,    34,    35,    36,     0,     0,
      16,    17,    18,     0,    19,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,     0,     0,
       0,     0,    29,    30,     0,   154,    31,    32,    62,   157,
      33,     0,     0,     0,    34,    35,    36,     0,   105,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   106,   107,   108,   109,   110,   111,   112,   113,   114,
      14,    15,     0,     0,     0,     0,     0,     0,     0,    17,
      18,     0,    19,     0,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,     0,     0,     0,
       0,   154,     0,     0,    12,   157,    13,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,     0,   115,     0,     0,
      29,    30,     0,     0,    31,    32,    14,    15,    63,     0,
       0,     0,    34,    35,    36,    17,    18,     0,    19,     0,
      20,     0,     0,     0,     0,     0,     0,     0,   198,     0,
       0,    12,    21,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,    14,    15,     0,    29,    30,     0,     0,
      31,    32,    17,    18,    33,    19,     0,    20,    34,    35,
      36,     0,     0,     0,     0,   200,     0,     0,    12,    21,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
      14,    15,     0,    29,    30,     0,     0,    31,    32,    17,
      18,    33,    19,     0,    20,    34,    35,    36,     0,     0,
       0,     0,   218,     0,     0,    12,    21,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,     0,    14,    15,     0,
      29,    30,     0,     0,    31,    32,    17,    18,    33,    19,
       0,    20,    34,    35,    36,     0,     0,     0,     0,   198,
       0,     0,   286,    21,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,     0,    14,    15,     0,    29,    30,     0,
       0,    31,    32,    17,    18,    33,    19,     0,    20,    34,
      35,    36,     0,     0,     0,     0,   200,     0,     0,   286,
      21,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,    14,    15,     0,    29,    30,     0,     0,    31,    32,
      17,    18,    33,    19,     0,    20,    34,    35,    36,     0,
       0,     0,     0,   218,     0,     0,   286,    21,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,     0,    14,    15,
       0,    29,    30,     0,     0,    31,    32,    17,    18,    33,
      19,     0,    20,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,     0,   136,     0,    62,    29,    30,
     137,     0,    31,    32,     0,     0,    33,   105,     0,     0,
      34,    35,    36,     0,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,   115,     0,     0,    29,
      30,     0,     0,    31,    32,     0,     0,    63,     0,     0,
       0,    34,    35,    36,   136,     0,    62,     0,     0,   308,
       0,     0,     0,     0,     0,     0,   105,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    14,    15,
       0,     0,     0,     0,     0,     0,     0,    17,    18,     0,
      19,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,     0,   115,     0,     0,    29,    30,
       0,     0,    31,    32,     0,     0,    63,     0,     0,     0,
      34,    35,    36,   136,     0,    62,     0,     0,   319,     0,
       0,     0,     0,     0,     0,   105,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    17,    18,     0,    19,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,     0,   115,     0,     0,    29,    30,     0,
       0,    31,    32,     0,     0,    63,     0,     0,     0,    34,
      35,    36,   136,     0,    62,     0,     0,   321,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    14,    15,     0,     0,
       0,     0,     0,     0,     0,    17,    18,     0,    19,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,   115,     0,     0,    29,    30,     0,     0,
      31,    32,     0,   136,    63,    62,     0,     0,    34,    35,
      36,     0,     0,     0,     0,   105,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    17,    18,     0,    19,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,    62,   115,     0,   137,    29,    30,     0,
       0,    31,    32,   105,     0,    63,     0,     0,     0,    34,
      35,    36,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    14,    15,     0,     0,     0,
       0,     0,     0,     0,    17,    18,     0,    19,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,    62,   115,     0,     0,    29,    30,     0,     0,    31,
      32,   105,     0,    63,     0,     0,     0,    34,    35,    36,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    14,    15,     0,     0,     0,     0,     0,
       0,     0,    17,    18,     0,    19,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,    62,
     115,   260,   267,    29,    30,     0,     0,    31,    32,   105,
       0,    63,     0,     0,     0,    34,    35,    36,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    14,    15,     0,     0,     0,     0,     0,     0,     0,
      17,    18,     0,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    62,   115,     0,
     302,    29,    30,     0,     0,    31,    32,   105,     0,    63,
       0,     0,     0,    34,    35,    36,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,    62,   115,     0,   304,    29,
      30,     0,     0,    31,    32,   105,     0,    63,     0,     0,
       0,    34,    35,    36,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    17,    18,     0,    19,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,    62,   115,     0,   338,    29,    30,     0,
       0,    31,    32,   105,     0,    63,     0,     0,     0,    34,
      35,    36,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    14,    15,     0,     0,     0,
       0,     0,     0,     0,    17,    18,     0,    19,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,    62,   115,     0,   340,    29,    30,     0,     0,    31,
      32,   105,     0,    63,     0,     0,     0,    34,    35,    36,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    14,    15,     0,     0,     0,     0,     0,
       0,     0,    17,    18,     0,    19,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
     115,     0,     0,    29,    30,     0,     0,    31,    32,    62,
     342,    63,     0,     0,     0,    34,    35,    36,     0,   105,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    14,    15,     0,     0,     0,     0,     0,     0,     0,
      17,    18,     0,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    62,   115,     0,
     351,    29,    30,     0,     0,    31,    32,   105,     0,    63,
       0,     0,     0,    34,    35,    36,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,    62,   115,     0,     0,    29,
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
      35,    36,     0,     0,     0,     0,   106,     0,   108,   109,
     110,   111,   112,   113,   114,    14,    15,     0,     0,     0,
       0,     0,     0,     0,    17,    18,     0,    19,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,    62,     0,     0,     0,    29,    30,     0,     0,    31,
      32,   -32,     0,    63,     0,     0,     0,    34,    35,    36,
       0,     0,     0,     0,     0,     0,   -32,   -32,   -32,   -32,
     -32,   -32,   -32,    14,    15,     0,     0,     0,     0,     0,
       0,     0,    17,    18,     0,    19,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,    62,
       0,     0,     0,    29,    30,     0,     0,    31,    32,   105,
       0,    63,     0,     0,     0,    34,    35,    36,     0,     0,
       0,     0,     0,     0,   108,   109,   110,   111,   112,   113,
     114,    14,    15,     0,     0,     0,     0,     0,     0,     0,
      17,    18,     0,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    21,     0,     0,
       0,     0,     0,     0,     0,     0,   249,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,   106,
     107,    23,    24,    25,    26,    27,    28,   250,    14,    15,
       0,    29,    30,     0,     0,    31,    32,    17,    18,    63,
      19,     0,    20,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,    62,     0,     0,     0,    23,    24,
      25,    26,    27,    28,   249,   251,   317,     0,    29,    30,
       0,     0,    31,    32,     0,     0,    63,   106,   107,     0,
      34,    35,    36,     0,     0,   250,    14,    15,     0,     0,
       0,     0,     0,     0,     0,    17,    18,     0,    19,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,    62,     0,     0,     0,    23,    24,    25,    26,
      27,    28,   249,   251,     0,     0,    29,    30,     0,     0,
      31,    32,     0,     0,    63,   106,     0,     0,    34,    35,
      36,     0,     0,   250,    14,    15,     0,     0,     0,     0,
       0,     0,     0,    17,    18,     0,    19,     0,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      21,    13,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,    14,    15,     0,    29,    30,     0,     0,    31,    32,
      17,    18,    63,    19,     0,    20,    34,    35,    36,     0,
       0,     0,     0,     0,     0,     0,    12,    21,    13,   150,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,     0,    14,    15,
       0,    29,    30,     0,     0,    31,    32,    17,    18,    33,
      19,     0,    20,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,    62,    21,     0,     0,     0,     0,     0,
       0,     0,     0,   -32,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,   -32,    14,    15,     0,    29,    30,
       0,     0,    31,    32,    17,    18,    33,    19,     0,    20,
      34,    35,    36,     0,     0,     0,     0,     0,     0,     0,
      62,    21,     0,     0,     0,     0,     0,     0,     0,     0,
     249,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,   250,    14,    15,     0,    29,    30,     0,     0,    31,
      32,    17,    18,    63,    19,     0,    20,    34,    35,    36,
       0,     0,     0,     0,     0,     0,     0,    12,    21,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,     0,    14,
      15,     0,    29,    30,     0,     0,    31,    32,    17,    18,
      63,    19,     0,    20,    34,    35,    36,     0,     0,     0,
       0,     0,     0,     0,    62,    21,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,    14,    15,     0,    29,
      30,     0,     0,    31,    32,    17,    18,    33,    19,     0,
      20,    34,    35,    36,     0,     0,     0,     0,     0,     0,
       0,   174,    21,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,    14,    15,     0,    29,    30,     0,     0,
      31,    32,    17,    18,    33,    19,     0,    20,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,    62,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
      14,    15,     0,    29,    30,     0,     0,    31,    32,    17,
      18,    33,    19,     0,    20,    34,    35,    36,     0,     0,
       0,     0,     0,     0,     0,   286,    21,    13,   204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,     0,    14,    15,     0,
      29,    30,     0,     0,    31,    32,    17,    18,    63,    19,
       0,    20,    34,    35,    36,     0,     0,     0,     0,     0,
       0,     0,    62,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,     0,    14,    15,     0,    29,    30,     0,
       0,    31,    32,    17,    18,    33,    19,     0,    20,    34,
      35,    36,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    29,    30,     0,     0,    31,    32,
       0,     0,    63,     0,     0,     0,    34,    35,    36
};

static const yytype_int16 yycheck[] =
{
       5,   174,    81,   164,   105,    84,    79,    12,    87,    66,
      67,    90,   102,    33,    51,    52,     9,    17,   119,    98,
       5,   240,   327,   242,     9,    10,   175,    10,    44,   102,
       9,     0,    10,    10,    10,    18,    11,    72,    15,    15,
      18,    82,    72,    43,   349,    10,    44,    10,    12,    10,
      15,    56,    15,    38,    15,    60,    61,    62,    95,    10,
      11,    66,    67,    68,    69,    44,   103,   157,   249,    12,
     251,   252,   253,   114,   255,   256,   133,    97,    94,    84,
      96,   116,   117,   162,   157,    90,   116,   117,   167,     9,
      94,   116,   117,    97,   313,    14,    94,    95,    96,   104,
     105,    94,    12,   108,   109,   110,   111,   112,   113,    12,
     115,   116,   117,   286,   119,    94,    10,    96,    99,   104,
     118,    15,   341,   164,    44,   130,   227,   228,   133,   120,
     121,   122,    10,   138,   353,   140,   317,    15,   287,    21,
      12,   290,   291,   292,   293,   294,    12,    12,   249,    31,
      32,    33,    34,    35,    36,    12,   115,    17,   100,    41,
     165,   120,   121,   122,   169,    10,   327,     9,    12,   174,
      15,    53,   175,     9,    94,    95,    96,    13,   235,   220,
     185,    63,     9,    12,   333,     9,    13,   277,   349,    13,
     231,   110,   111,     9,    12,    12,   115,    13,   118,     9,
      43,   120,   121,   122,   277,    14,   307,    94,     9,   250,
      17,    87,    10,     9,    96,    97,    13,     9,   106,   224,
      15,   311,   227,   228,    15,    15,    12,   232,    43,   249,
     235,   251,   252,   253,    15,   255,   256,   278,   311,   121,
     122,   123,   124,   125,   126,   127,   128,   129,    14,   185,
      15,    15,   342,    53,    54,   260,   261,   262,   295,    -1,
      40,    -1,   144,   104,   146,   147,   148,    -1,    -1,   342,
     152,   153,   351,   258,   315,    -1,    -1,    -1,    -1,    -1,
     162,   286,    -1,    -1,   287,   167,   323,   290,   291,   292,
     293,   294,    -1,   175,    -1,    -1,    -1,   317,    -1,    -1,
      -1,    -1,   307,    -1,   309,    -1,   188,   312,   190,   191,
     192,   193,   194,   195,    -1,   197,   301,   199,    -1,   201,
     202,    -1,   204,    -1,    -1,   330,    -1,    -1,    -1,    -1,
     333,    -1,   214,    -1,    -1,    -1,    -1,   219,    -1,   221,
      -1,    -1,   108,   109,   110,   111,   146,   147,    -1,   115,
      -1,    -1,   152,   153,   120,   121,   122,    -1,   240,    -1,
     242,    -1,    -1,    -1,    -1,    -1,    -1,   249,    -1,   251,
     252,   253,    -1,   255,   256,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   266,    -1,    -1,    -1,   270,    -1,
     272,    -1,    -1,   275,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   202,   203,    -1,   287,    -1,    21,   290,   291,
     292,   293,   294,    -1,   296,   297,   298,    31,    32,    33,
      -1,    -1,    36,    -1,    -1,    -1,   226,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   317,    -1,    -1,   320,    53,
     322,    -1,   324,    -1,   326,    -1,   328,    -1,   248,    63,
      94,   333,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,   344,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   274,   275,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    97,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,    -1,    -1,    -1,    -1,   297,   298,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   316,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,   173,
     144,    -1,   146,   147,   148,    -1,    -1,    -1,   152,   153,
     184,    -1,   186,    -1,    -1,    -1,    -1,    -1,   162,    -1,
      -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   188,    -1,   190,   191,   192,   193,
     194,   195,    -1,   197,    -1,   199,    -1,   201,   202,    -1,
     204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     214,   245,    -1,    -1,    -1,   219,    -1,   221,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   240,    -1,   242,    -1,
      -1,    -1,    -1,    -1,    -1,   249,    -1,   251,   252,   253,
      -1,   255,   256,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   266,    -1,    -1,    -1,   270,    -1,   272,    -1,
      -1,   275,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   287,   318,    -1,   290,   291,   292,   293,
     294,    -1,   296,   297,   298,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   337,    -1,    -1,    -1,    -1,    -1,   343,
      -1,    -1,    -1,   317,    -1,    -1,    -1,    -1,   352,    -1,
     324,    -1,   326,   357,   328,     0,     1,    -1,    -1,   333,
      -1,    -1,     7,     8,     9,    -1,    11,    12,    13,    14,
     344,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       9,    86,    11,    12,    13,    14,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,   112,    -1,    -1,
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
      -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    84,    85,    86,     9,    -1,
      11,    12,    13,    14,    -1,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,    -1,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    -1,    -1,    44,    45,    46,    47,    48,    49,    50,
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
      13,    14,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,    -1,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    -1,    58,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    12,    13,    14,    -1,    -1,
      -1,    84,    85,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    44,    45,   112,
      -1,    48,    -1,   116,   117,   118,    53,    54,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    12,    -1,    14,    15,    -1,    -1,    84,    85,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,    -1,   104,   105,    -1,
      -1,   108,   109,    44,    45,   112,    -1,    48,    -1,   116,
     117,   118,    53,    54,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,     7,     8,    -1,    -1,    11,
      12,    -1,    14,    84,    85,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,   112,    44,    45,    -1,   116,   117,   118,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,    -1,
      -1,    -1,   104,   105,    -1,     9,   108,   109,    12,    13,
     112,    -1,    -1,    -1,   116,   117,   118,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,
      -1,     9,    -1,    -1,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,   101,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    44,    45,   112,    -1,
      -1,    -1,   116,   117,   118,    53,    54,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    -1,
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
      -1,    -1,     9,    -1,    -1,    12,    70,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    44,    45,    -1,
     104,   105,    -1,    -1,   108,   109,    53,    54,   112,    56,
      -1,    58,   116,   117,   118,    -1,    -1,    -1,    -1,     9,
      -1,    -1,    12,    70,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    -1,    44,    45,    -1,   104,   105,    -1,
      -1,   108,   109,    53,    54,   112,    56,    -1,    58,   116,
     117,   118,    -1,    -1,    -1,    -1,     9,    -1,    -1,    12,
      70,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
      -1,    44,    45,    -1,   104,   105,    -1,    -1,   108,   109,
      53,    54,   112,    56,    -1,    58,   116,   117,   118,    -1,
      -1,    -1,    -1,     9,    -1,    -1,    12,    70,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    -1,    44,    45,
      -1,   104,   105,    -1,    -1,   108,   109,    53,    54,   112,
      56,    -1,    58,   116,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    -1,    10,    -1,    12,   104,   105,
      15,    -1,   108,   109,    -1,    -1,   112,    22,    -1,    -1,
     116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   108,   109,    -1,    -1,   112,    -1,    -1,    -1,
     116,   117,   118,    10,    -1,    12,    -1,    -1,    15,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,   112,    -1,    -1,    -1,   116,
     117,   118,    10,    -1,    12,    -1,    -1,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
     108,   109,    -1,    10,   112,    12,    -1,    -1,   116,   117,
     118,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
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
      99,    12,   101,    -1,    -1,   104,   105,    -1,    -1,   108,
     109,    22,    -1,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    12,
     101,   102,    15,   104,   105,    -1,    -1,   108,   109,    22,
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
      99,    12,   101,    -1,    15,   104,   105,    -1,    -1,   108,
     109,    22,    -1,   112,    -1,    -1,    -1,   116,   117,   118,
      -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,   108,   109,    12,
      13,   112,    -1,    -1,    -1,   116,   117,   118,    -1,    22,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      95,    96,    97,    98,    99,    12,   101,    -1,    -1,   104,
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
     117,   118,    -1,    -1,    -1,    -1,    35,    -1,    37,    38,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    12,
      -1,    -1,    -1,   104,   105,    -1,    -1,   108,   109,    22,
      -1,   112,    -1,    -1,    -1,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    35,
      36,    94,    95,    96,    97,    98,    99,    43,    44,    45,
      -1,   104,   105,    -1,    -1,   108,   109,    53,    54,   112,
      56,    -1,    58,   116,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    12,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    22,   101,   102,    -1,   104,   105,
      -1,    -1,   108,   109,    -1,    -1,   112,    35,    36,    -1,
     116,   117,   118,    -1,    -1,    43,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,    22,   101,    -1,    -1,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,   112,    35,    -1,    -1,   116,   117,
     118,    -1,    -1,    43,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    -1,    56,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,
      70,    14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
      -1,    44,    45,    -1,   104,   105,    -1,    -1,   108,   109,
      53,    54,   112,    56,    -1,    58,   116,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    70,    14,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    -1,    44,    45,
      -1,   104,   105,    -1,    -1,   108,   109,    53,    54,   112,
      56,    -1,    58,   116,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    12,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    43,    44,    45,    -1,   104,   105,
      -1,    -1,   108,   109,    53,    54,   112,    56,    -1,    58,
     116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      12,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    43,    44,    45,    -1,   104,   105,    -1,    -1,   108,
     109,    53,    54,   112,    56,    -1,    58,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    70,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,    44,
      45,    -1,   104,   105,    -1,    -1,   108,   109,    53,    54,
     112,    56,    -1,    58,   116,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    12,    70,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    44,    45,    -1,   104,
     105,    -1,    -1,   108,   109,    53,    54,   112,    56,    -1,
      58,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    12,    70,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,    -1,    44,    45,    -1,   104,   105,    -1,    -1,
     108,   109,    53,    54,   112,    56,    -1,    58,   116,   117,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    -1,
      44,    45,    -1,   104,   105,    -1,    -1,   108,   109,    53,
      54,   112,    56,    -1,    58,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    12,    70,    14,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    44,    45,    -1,
     104,   105,    -1,    -1,   108,   109,    53,    54,   112,    56,
      -1,    58,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    12,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    -1,    44,    45,    -1,   104,   105,    -1,
      -1,   108,   109,    53,    54,   112,    56,    -1,    58,   116,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
      -1,    -1,    -1,    -1,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,   112,    -1,    -1,    -1,   116,   117,   118
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
      99,    14,   108,   109,   110,   111,   115,   120,   121,   122,
      72,   116,   117,    17,   165,   165,    10,    15,   127,    15,
     127,   100,    15,   146,   148,    12,   148,   148,   148,   146,
      15,   146,   148,   148,     9,   137,   138,    13,   137,   160,
     160,   171,   148,   160,    12,    12,   160,   148,   160,    12,
       9,   162,   161,   165,    12,   147,   150,   151,   155,   168,
     169,   160,    16,   161,   164,   138,   165,   143,   148,   156,
     148,   148,   148,   148,   148,   148,   171,   148,     9,   148,
       9,   148,   148,   156,    72,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   148,   146,    16,    16,     9,   148,
      43,   148,    14,    15,   127,    94,   170,   127,   127,    15,
      15,   127,   127,     9,   138,    17,   160,   140,   159,   171,
     148,   160,   148,   161,    87,   129,    16,   154,   149,    22,
      43,   101,   125,   126,   168,   127,    72,   161,   142,    16,
     102,   127,   127,   168,    18,   171,   148,    15,   127,   158,
     148,   156,   148,   156,   171,   148,   146,    13,    43,   158,
     158,   163,     9,   161,     9,    15,    12,   147,   156,   171,
     147,   147,   147,   147,   147,   136,   148,   148,   148,    94,
       9,   144,    15,    15,    15,    15,    15,   127,    15,   127,
      18,    13,   138,   171,   106,    43,   149,   102,   165,    15,
     127,    15,   127,   136,   148,   156,   148,   138,   148,   158,
      12,   171,    15,   147,    16,   169,   169,   165,    15,    15,
      15,   140,    13,   133,   148,    15,    15,    16,   158,   138,
     161,    15,   132,   140,   160,   161,   158,   131,   161
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
     169,   169,   169,   169,   170,   170,   170,   171,   171,   171,
     172
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
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     4,     1,     3,     4,     2,     2,
       2,     2,     6,     6,     3,     6,     6,     1,     8,     8,
       6,     4,     1,     6,     6,     8,     8,     8,     6,     1,
       1,     4,     1,     2,     0,     1,     3,     1,     1,     1,
       4
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
#line 2386 "awkgram.tab.c"
    break;

  case 3: /* program: error  */
#line 113 "awkgram.y"
                { yyclearin; bracecheck(); SYNTAX("bailing out"); }
#line 2392 "awkgram.tab.c"
    break;

  case 14: /* $@1: %empty  */
#line 137 "awkgram.y"
                                                                                       {inloop++;}
#line 2398 "awkgram.tab.c"
    break;

  case 15: /* for: FOR '(' opt_simple_stmt ';' opt_nl pattern ';' opt_nl opt_simple_stmt rparen $@1 stmt  */
#line 138 "awkgram.y"
                { --inloop; (yyval.p) = stat4(FOR, (yyvsp[-9].p), notnull((yyvsp[-6].p)), (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2404 "awkgram.tab.c"
    break;

  case 16: /* $@2: %empty  */
#line 139 "awkgram.y"
                                                                         {inloop++;}
#line 2410 "awkgram.tab.c"
    break;

  case 17: /* for: FOR '(' opt_simple_stmt ';' ';' opt_nl opt_simple_stmt rparen $@2 stmt  */
#line 140 "awkgram.y"
                { --inloop; (yyval.p) = stat4(FOR, (yyvsp[-7].p), NIL, (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2416 "awkgram.tab.c"
    break;

  case 18: /* $@3: %empty  */
#line 141 "awkgram.y"
                                            {inloop++;}
#line 2422 "awkgram.tab.c"
    break;

  case 19: /* for: FOR '(' varname IN varname rparen $@3 stmt  */
#line 142 "awkgram.y"
                { --inloop; (yyval.p) = stat3(IN, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (yyvsp[0].p)); }
#line 2428 "awkgram.tab.c"
    break;

  case 20: /* funcname: VAR  */
#line 146 "awkgram.y"
                { setfname((yyvsp[0].cp)); }
#line 2434 "awkgram.tab.c"
    break;

  case 21: /* funcname: CALL  */
#line 147 "awkgram.y"
                { setfname((yyvsp[0].cp)); }
#line 2440 "awkgram.tab.c"
    break;

  case 22: /* if: IF '(' pattern rparen  */
#line 151 "awkgram.y"
                                        { (yyval.p) = notnull((yyvsp[-1].p)); }
#line 2446 "awkgram.tab.c"
    break;

  case 27: /* opt_nl: %empty  */
#line 163 "awkgram.y"
                        { (yyval.i) = 0; }
#line 2452 "awkgram.tab.c"
    break;

  case 29: /* opt_pst: %empty  */
#line 168 "awkgram.y"
                        { (yyval.i) = 0; }
#line 2458 "awkgram.tab.c"
    break;

  case 31: /* opt_simple_stmt: %empty  */
#line 174 "awkgram.y"
                                        { (yyval.p) = 0; }
#line 2464 "awkgram.tab.c"
    break;

  case 33: /* pas: opt_pst  */
#line 179 "awkgram.y"
                                        { (yyval.p) = 0; }
#line 2470 "awkgram.tab.c"
    break;

  case 34: /* pas: opt_pst pa_stats opt_pst  */
#line 180 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2476 "awkgram.tab.c"
    break;

  case 35: /* pa_pat: pattern  */
#line 184 "awkgram.y"
                        { (yyval.p) = notnull((yyvsp[0].p)); }
#line 2482 "awkgram.tab.c"
    break;

  case 36: /* pa_stat: pa_pat  */
#line 188 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, (yyvsp[0].p), stat2(PRINT, rectonode(), NIL)); }
#line 2488 "awkgram.tab.c"
    break;

  case 37: /* pa_stat: pa_pat lbrace stmtlist '}'  */
#line 189 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2494 "awkgram.tab.c"
    break;

  case 38: /* pa_stat: pa_pat ',' opt_nl pa_pat  */
#line 190 "awkgram.y"
                                                { (yyval.p) = pa2stat((yyvsp[-3].p), (yyvsp[0].p), stat2(PRINT, rectonode(), NIL)); }
#line 2500 "awkgram.tab.c"
    break;

  case 39: /* pa_stat: pa_pat ',' opt_nl pa_pat lbrace stmtlist '}'  */
#line 191 "awkgram.y"
                                                        { (yyval.p) = pa2stat((yyvsp[-6].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2506 "awkgram.tab.c"
    break;

  case 40: /* pa_stat: lbrace stmtlist '}'  */
#line 192 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, NIL, (yyvsp[-1].p)); }
#line 2512 "awkgram.tab.c"
    break;

  case 41: /* pa_stat: XBEGIN lbrace stmtlist '}'  */
#line 194 "awkgram.y"
                { beginloc = linkum(beginloc, (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2518 "awkgram.tab.c"
    break;

  case 42: /* pa_stat: XEND lbrace stmtlist '}'  */
#line 196 "awkgram.y"
                { endloc = linkum(endloc, (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2524 "awkgram.tab.c"
    break;

  case 43: /* $@4: %empty  */
#line 197 "awkgram.y"
                                           {infunc = true;}
#line 2530 "awkgram.tab.c"
    break;

  case 44: /* pa_stat: FUNC funcname '(' varlist rparen $@4 lbrace stmtlist '}'  */
#line 198 "awkgram.y"
                { infunc = false; curfname=0; defn((Cell *)(yyvsp[-7].p), (yyvsp[-5].p), (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2536 "awkgram.tab.c"
    break;

  case 46: /* pa_stats: pa_stats opt_pst pa_stat  */
#line 203 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2542 "awkgram.tab.c"
    break;

  case 48: /* patlist: patlist comma pattern  */
#line 208 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2548 "awkgram.tab.c"
    break;

  case 49: /* ppattern: var ASGNOP ppattern  */
#line 212 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2554 "awkgram.tab.c"
    break;

  case 50: /* ppattern: ppattern '?' ppattern ':' ppattern  */
#line 214 "awkgram.y"
                { (yyval.p) = op3(CONDEXPR, notnull((yyvsp[-4].p)), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2560 "awkgram.tab.c"
    break;

  case 51: /* ppattern: ppattern lor ppattern  */
#line 216 "awkgram.y"
                { (yyval.p) = op2(LOR, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2566 "awkgram.tab.c"
    break;

  case 52: /* ppattern: ppattern land ppattern  */
#line 218 "awkgram.y"
                { (yyval.p) = op2(LAND, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2572 "awkgram.tab.c"
    break;

  case 53: /* ppattern: ppattern MATCHOP reg_expr  */
#line 219 "awkgram.y"
                                        { (yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2578 "awkgram.tab.c"
    break;

  case 54: /* ppattern: ppattern MATCHOP ppattern  */
#line 221 "awkgram.y"
                { if (constnode((yyvsp[0].p))) {
			(yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa(strnode((yyvsp[0].p)), 0));
			free((yyvsp[0].p));
		  } else
			(yyval.p) = op3((yyvsp[-1].i), (Node *)1, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2588 "awkgram.tab.c"
    break;

  case 55: /* ppattern: ppattern IN varname  */
#line 226 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-2].p), makearr((yyvsp[0].p))); }
#line 2594 "awkgram.tab.c"
    break;

  case 56: /* ppattern: '(' plist ')' IN varname  */
#line 227 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-3].p), makearr((yyvsp[0].p))); }
#line 2600 "awkgram.tab.c"
    break;

  case 57: /* ppattern: ppattern term  */
#line 228 "awkgram.y"
                                        { (yyval.p) = op2(CAT, (yyvsp[-1].p), (yyvsp[0].p)); }
#line 2606 "awkgram.tab.c"
    break;

  case 60: /* pattern: var ASGNOP pattern  */
#line 234 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2612 "awkgram.tab.c"
    break;

  case 61: /* pattern: pattern '?' pattern ':' pattern  */
#line 236 "awkgram.y"
                { (yyval.p) = op3(CONDEXPR, notnull((yyvsp[-4].p)), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2618 "awkgram.tab.c"
    break;

  case 62: /* pattern: pattern lor pattern  */
#line 238 "awkgram.y"
                { (yyval.p) = op2(LOR, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2624 "awkgram.tab.c"
    break;

  case 63: /* pattern: pattern land pattern  */
#line 240 "awkgram.y"
                { (yyval.p) = op2(LAND, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2630 "awkgram.tab.c"
    break;

  case 64: /* pattern: pattern EQ pattern  */
#line 241 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2636 "awkgram.tab.c"
    break;

  case 65: /* pattern: pattern GE pattern  */
#line 242 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2642 "awkgram.tab.c"
    break;

  case 66: /* pattern: pattern GT pattern  */
#line 243 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2648 "awkgram.tab.c"
    break;

  case 67: /* pattern: pattern LE pattern  */
#line 244 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2654 "awkgram.tab.c"
    break;

  case 68: /* pattern: pattern LT pattern  */
#line 245 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2660 "awkgram.tab.c"
    break;

  case 69: /* pattern: pattern NE pattern  */
#line 246 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2666 "awkgram.tab.c"
    break;

  case 70: /* pattern: pattern MATCHOP reg_expr  */
#line 247 "awkgram.y"
                                        { (yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2672 "awkgram.tab.c"
    break;

  case 71: /* pattern: pattern MATCHOP pattern  */
#line 249 "awkgram.y"
                { if (constnode((yyvsp[0].p))) {
			(yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa(strnode((yyvsp[0].p)), 0));
			free((yyvsp[0].p));
		  } else
			(yyval.p) = op3((yyvsp[-1].i), (Node *)1, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2682 "awkgram.tab.c"
    break;

  case 72: /* pattern: pattern IN varname  */
#line 254 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-2].p), makearr((yyvsp[0].p))); }
#line 2688 "awkgram.tab.c"
    break;

  case 73: /* pattern: '(' plist ')' IN varname  */
#line 255 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-3].p), makearr((yyvsp[0].p))); }
#line 2694 "awkgram.tab.c"
    break;

  case 74: /* pattern: pattern term  */
#line 256 "awkgram.y"
                                        { (yyval.p) = op2(CAT, (yyvsp[-1].p), (yyvsp[0].p)); }
#line 2700 "awkgram.tab.c"
    break;

  case 77: /* plist: pattern comma pattern  */
#line 262 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2706 "awkgram.tab.c"
    break;

  case 78: /* plist: plist comma pattern  */
#line 263 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2712 "awkgram.tab.c"
    break;

  case 80: /* pplist: pplist comma ppattern  */
#line 268 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2718 "awkgram.tab.c"
    break;

  case 81: /* prarg: %empty  */
#line 272 "awkgram.y"
                                        { (yyval.p) = rectonode(); }
#line 2724 "awkgram.tab.c"
    break;

  case 83: /* prarg: '(' plist ')'  */
#line 274 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2730 "awkgram.tab.c"
    break;

  case 92: /* re: reg_expr  */
#line 291 "awkgram.y"
                { (yyval.p) = op3(MATCH, NIL, rectonode(), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2736 "awkgram.tab.c"
    break;

  case 93: /* re: NOT re  */
#line 292 "awkgram.y"
                        { (yyval.p) = op1(NOT, notnull((yyvsp[0].p))); }
#line 2742 "awkgram.tab.c"
    break;

  case 94: /* $@5: %empty  */
#line 296 "awkgram.y"
              {startreg();}
#line 2748 "awkgram.tab.c"
    break;

  case 95: /* reg_expr: '/' $@5 REGEXPR '/'  */
#line 296 "awkgram.y"
                                                { (yyval.s) = (yyvsp[-1].s); }
#line 2754 "awkgram.tab.c"
    break;

  case 98: /* simple_stmt: print prarg  */
#line 304 "awkgram.y"
                                        { (yyval.p) = stat3((yyvsp[-1].i), (yyvsp[0].p), NIL, NIL); }
#line 2760 "awkgram.tab.c"
    break;

  case 99: /* simple_stmt: DELETE varname '[' patlist ']'  */
#line 305 "awkgram.y"
                                         { (yyval.p) = stat2(DELETE, makearr((yyvsp[-3].p)), (yyvsp[-1].p)); }
#line 2766 "awkgram.tab.c"
    break;

  case 100: /* simple_stmt: DELETE varname  */
#line 306 "awkgram.y"
                                         { (yyval.p) = stat2(DELETE, makearr((yyvsp[0].p)), 0); }
#line 2772 "awkgram.tab.c"
    break;

  case 101: /* simple_stmt: pattern  */
#line 307 "awkgram.y"
                                        { (yyval.p) = exptostat((yyvsp[0].p)); }
#line 2778 "awkgram.tab.c"
    break;

  case 102: /* simple_stmt: error  */
#line 308 "awkgram.y"
                                        { yyclearin; SYNTAX("illegal statement"); }
#line 2784 "awkgram.tab.c"
    break;

  case 105: /* stmt: BREAK st  */
#line 317 "awkgram.y"
                                { if (!inloop) SYNTAX("break illegal outside of loops");
				  (yyval.p) = stat1(BREAK, NIL); }
#line 2791 "awkgram.tab.c"
    break;

  case 106: /* stmt: CONTINUE st  */
#line 319 "awkgram.y"
                                {  if (!inloop) SYNTAX("continue illegal outside of loops");
				  (yyval.p) = stat1(CONTINUE, NIL); }
#line 2798 "awkgram.tab.c"
    break;

  case 107: /* $@6: %empty  */
#line 321 "awkgram.y"
             {inloop++;}
#line 2804 "awkgram.tab.c"
    break;

  case 108: /* $@7: %empty  */
#line 321 "awkgram.y"
                              {--inloop;}
#line 2810 "awkgram.tab.c"
    break;

  case 109: /* stmt: do $@6 stmt $@7 WHILE '(' pattern ')' st  */
#line 322 "awkgram.y"
                { (yyval.p) = stat2(DO, (yyvsp[-6].p), notnull((yyvsp[-2].p))); }
#line 2816 "awkgram.tab.c"
    break;

  case 110: /* stmt: EXIT pattern st  */
#line 323 "awkgram.y"
                                { (yyval.p) = stat1(EXIT, (yyvsp[-1].p)); }
#line 2822 "awkgram.tab.c"
    break;

  case 111: /* stmt: EXIT st  */
#line 324 "awkgram.y"
                                { (yyval.p) = stat1(EXIT, NIL); }
#line 2828 "awkgram.tab.c"
    break;

  case 113: /* stmt: if stmt else stmt  */
#line 326 "awkgram.y"
                                { (yyval.p) = stat3(IF, (yyvsp[-3].p), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2834 "awkgram.tab.c"
    break;

  case 114: /* stmt: if stmt  */
#line 327 "awkgram.y"
                                { (yyval.p) = stat3(IF, (yyvsp[-1].p), (yyvsp[0].p), NIL); }
#line 2840 "awkgram.tab.c"
    break;

  case 115: /* stmt: lbrace stmtlist rbrace  */
#line 328 "awkgram.y"
                                 { (yyval.p) = (yyvsp[-1].p); }
#line 2846 "awkgram.tab.c"
    break;

  case 116: /* stmt: NEXT st  */
#line 329 "awkgram.y"
                        { if (infunc)
				SYNTAX("next is illegal inside a function");
			  (yyval.p) = stat1(NEXT, NIL); }
#line 2854 "awkgram.tab.c"
    break;

  case 117: /* stmt: RETURN pattern st  */
#line 332 "awkgram.y"
                                { (yyval.p) = stat1(RETURN, (yyvsp[-1].p)); }
#line 2860 "awkgram.tab.c"
    break;

  case 118: /* stmt: RETURN st  */
#line 333 "awkgram.y"
                                { (yyval.p) = stat1(RETURN, NIL); }
#line 2866 "awkgram.tab.c"
    break;

  case 120: /* $@8: %empty  */
#line 335 "awkgram.y"
                {inloop++;}
#line 2872 "awkgram.tab.c"
    break;

  case 121: /* stmt: while $@8 stmt  */
#line 335 "awkgram.y"
                                        { --inloop; (yyval.p) = stat2(WHILE, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2878 "awkgram.tab.c"
    break;

  case 122: /* stmt: ';' opt_nl  */
#line 336 "awkgram.y"
                                { (yyval.p) = 0; }
#line 2884 "awkgram.tab.c"
    break;

  case 124: /* stmtlist: stmtlist stmt  */
#line 341 "awkgram.y"
                                { (yyval.p) = linkum((yyvsp[-1].p), (yyvsp[0].p)); }
#line 2890 "awkgram.tab.c"
    break;

  case 128: /* string: string STRING  */
#line 350 "awkgram.y"
                                { (yyval.cp) = catstr((yyvsp[-1].cp), (yyvsp[0].cp)); }
#line 2896 "awkgram.tab.c"
    break;

  case 129: /* term: term '/' ASGNOP term  */
#line 354 "awkgram.y"
                                        { (yyval.p) = op2(DIVEQ, (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2902 "awkgram.tab.c"
    break;

  case 130: /* term: term '+' term  */
#line 355 "awkgram.y"
                                        { (yyval.p) = op2(ADD, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2908 "awkgram.tab.c"
    break;

  case 131: /* term: term '-' term  */
#line 356 "awkgram.y"
                                        { (yyval.p) = op2(MINUS, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2914 "awkgram.tab.c"
    break;

  case 132: /* term: term '*' term  */
#line 357 "awkgram.y"
                                        { (yyval.p) = op2(MULT, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2920 "awkgram.tab.c"
    break;

  case 133: /* term: term '/' term  */
#line 358 "awkgram.y"
                                        { (yyval.p) = op2(DIVIDE, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2926 "awkgram.tab.c"
    break;

  case 134: /* term: term '%' term  */
#line 359 "awkgram.y"
                                        { (yyval.p) = op2(MOD, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2932 "awkgram.tab.c"
    break;

  case 135: /* term: term '^' term  */
#line 360 "awkgram.y"
                                        { (yyval.p) = op2(XOR, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2938 "awkgram.tab.c"
    break;

  case 136: /* term: term '|' term  */
#line 361 "awkgram.y"
                                        { (yyval.p) = op2(BOR, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2944 "awkgram.tab.c"
    break;

  case 137: /* term: term '&' term  */
#line 362 "awkgram.y"
                                        { (yyval.p) = op2(BAND, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2950 "awkgram.tab.c"
    break;

  case 138: /* term: term POWER term  */
#line 363 "awkgram.y"
                                        { (yyval.p) = op2(POWER, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2956 "awkgram.tab.c"
    break;

  case 139: /* term: CMPL term  */
#line 364 "awkgram.y"
                                        { (yyval.p) = op1(CMPL, (yyvsp[0].p)); }
#line 2962 "awkgram.tab.c"
    break;

  case 140: /* term: '-' term  */
#line 365 "awkgram.y"
                                        { (yyval.p) = op1(UMINUS, (yyvsp[0].p)); }
#line 2968 "awkgram.tab.c"
    break;

  case 141: /* term: '+' term  */
#line 366 "awkgram.y"
                                        { (yyval.p) = op1(UPLUS, (yyvsp[0].p)); }
#line 2974 "awkgram.tab.c"
    break;

  case 142: /* term: NOT term  */
#line 367 "awkgram.y"
                                        { (yyval.p) = op1(NOT, notnull((yyvsp[0].p))); }
#line 2980 "awkgram.tab.c"
    break;

  case 143: /* term: BLTIN '(' ')'  */
#line 368 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[-2].i)), rectonode()); }
#line 2986 "awkgram.tab.c"
    break;

  case 144: /* term: BLTIN '(' patlist ')'  */
#line 369 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[-3].i)), (yyvsp[-1].p)); }
#line 2992 "awkgram.tab.c"
    break;

  case 145: /* term: BLTIN  */
#line 370 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[0].i)), rectonode()); }
#line 2998 "awkgram.tab.c"
    break;

  case 146: /* term: CALL '(' ')'  */
#line 371 "awkgram.y"
                                        { (yyval.p) = op2(CALL, celltonode((yyvsp[-2].cp),CVAR), NIL); }
#line 3004 "awkgram.tab.c"
    break;

  case 147: /* term: CALL '(' patlist ')'  */
#line 372 "awkgram.y"
                                        { (yyval.p) = op2(CALL, celltonode((yyvsp[-3].cp),CVAR), (yyvsp[-1].p)); }
#line 3010 "awkgram.tab.c"
    break;

  case 148: /* term: DECR var  */
#line 373 "awkgram.y"
                                        { (yyval.p) = op1(PREDECR, (yyvsp[0].p)); }
#line 3016 "awkgram.tab.c"
    break;

  case 149: /* term: INCR var  */
#line 374 "awkgram.y"
                                        { (yyval.p) = op1(PREINCR, (yyvsp[0].p)); }
#line 3022 "awkgram.tab.c"
    break;

  case 150: /* term: var DECR  */
#line 375 "awkgram.y"
                                        { (yyval.p) = op1(POSTDECR, (yyvsp[-1].p)); }
#line 3028 "awkgram.tab.c"
    break;

  case 151: /* term: var INCR  */
#line 376 "awkgram.y"
                                        { (yyval.p) = op1(POSTINCR, (yyvsp[-1].p)); }
#line 3034 "awkgram.tab.c"
    break;

  case 152: /* term: INDEX '(' pattern comma pattern ')'  */
#line 378 "awkgram.y"
                { (yyval.p) = op2(INDEX, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3040 "awkgram.tab.c"
    break;

  case 153: /* term: INDEX '(' pattern comma reg_expr ')'  */
#line 380 "awkgram.y"
                { SYNTAX("index() doesn't permit regular expressions");
		  (yyval.p) = op2(INDEX, (yyvsp[-3].p), (Node*)(yyvsp[-1].s)); }
#line 3047 "awkgram.tab.c"
    break;

  case 154: /* term: '(' pattern ')'  */
#line 382 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 3053 "awkgram.tab.c"
    break;

  case 155: /* term: MATCHFCN '(' pattern comma reg_expr ')'  */
#line 384 "awkgram.y"
                { (yyval.p) = op3(MATCHFCN, NIL, (yyvsp[-3].p), (Node*)makedfa((yyvsp[-1].s), 1)); free((yyvsp[-1].s)); }
#line 3059 "awkgram.tab.c"
    break;

  case 156: /* term: MATCHFCN '(' pattern comma pattern ')'  */
#line 386 "awkgram.y"
                { if (constnode((yyvsp[-1].p))) {
			(yyval.p) = op3(MATCHFCN, NIL, (yyvsp[-3].p), (Node*)makedfa(strnode((yyvsp[-1].p)), 1));
			free((yyvsp[-1].p));
		  } else
			(yyval.p) = op3(MATCHFCN, (Node *)1, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3069 "awkgram.tab.c"
    break;

  case 157: /* term: NUMBER  */
#line 391 "awkgram.y"
                                        { (yyval.p) = celltonode((yyvsp[0].cp), CCON); }
#line 3075 "awkgram.tab.c"
    break;

  case 158: /* term: SPLIT '(' pattern comma varname comma pattern ')'  */
#line 393 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (yyvsp[-1].p), (Node*)STRING); }
#line 3081 "awkgram.tab.c"
    break;

  case 159: /* term: SPLIT '(' pattern comma varname comma reg_expr ')'  */
#line 395 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (Node*)makedfa((yyvsp[-1].s), 1), (Node *)REGEXPR); free((yyvsp[-1].s)); }
#line 3087 "awkgram.tab.c"
    break;

  case 160: /* term: SPLIT '(' pattern comma varname ')'  */
#line 397 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-3].p), makearr((yyvsp[-1].p)), NIL, (Node*)STRING); }
#line 3093 "awkgram.tab.c"
    break;

  case 161: /* term: SPRINTF '(' patlist ')'  */
#line 398 "awkgram.y"
                                        { (yyval.p) = op1((yyvsp[-3].i), (yyvsp[-1].p)); }
#line 3099 "awkgram.tab.c"
    break;

  case 162: /* term: string  */
#line 399 "awkgram.y"
                                        { (yyval.p) = celltonode((yyvsp[0].cp), CCON); }
#line 3105 "awkgram.tab.c"
    break;

  case 163: /* term: subop '(' reg_expr comma pattern ')'  */
#line 401 "awkgram.y"
                { (yyval.p) = op4((yyvsp[-5].i), NIL, (Node*)makedfa((yyvsp[-3].s), 1), (yyvsp[-1].p), rectonode()); free((yyvsp[-3].s)); }
#line 3111 "awkgram.tab.c"
    break;

  case 164: /* term: subop '(' pattern comma pattern ')'  */
#line 403 "awkgram.y"
                { if (constnode((yyvsp[-3].p))) {
			(yyval.p) = op4((yyvsp[-5].i), NIL, (Node*)makedfa(strnode((yyvsp[-3].p)), 1), (yyvsp[-1].p), rectonode());
			free((yyvsp[-3].p));
		  } else
			(yyval.p) = op4((yyvsp[-5].i), (Node *)1, (yyvsp[-3].p), (yyvsp[-1].p), rectonode()); }
#line 3121 "awkgram.tab.c"
    break;

  case 165: /* term: subop '(' reg_expr comma pattern comma var ')'  */
#line 409 "awkgram.y"
                { (yyval.p) = op4((yyvsp[-7].i), NIL, (Node*)makedfa((yyvsp[-5].s), 1), (yyvsp[-3].p), (yyvsp[-1].p)); free((yyvsp[-5].s)); }
#line 3127 "awkgram.tab.c"
    break;

  case 166: /* term: subop '(' pattern comma pattern comma var ')'  */
#line 411 "awkgram.y"
                { if (constnode((yyvsp[-5].p))) {
			(yyval.p) = op4((yyvsp[-7].i), NIL, (Node*)makedfa(strnode((yyvsp[-5].p)), 1), (yyvsp[-3].p), (yyvsp[-1].p));
			free((yyvsp[-5].p));
		  } else
			(yyval.p) = op4((yyvsp[-7].i), (Node *)1, (yyvsp[-5].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3137 "awkgram.tab.c"
    break;

  case 167: /* term: SUBSTR '(' pattern comma pattern comma pattern ')'  */
#line 417 "awkgram.y"
                { (yyval.p) = op3(SUBSTR, (yyvsp[-5].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3143 "awkgram.tab.c"
    break;

  case 168: /* term: SUBSTR '(' pattern comma pattern ')'  */
#line 419 "awkgram.y"
                { (yyval.p) = op3(SUBSTR, (yyvsp[-3].p), (yyvsp[-1].p), NIL); }
#line 3149 "awkgram.tab.c"
    break;

  case 171: /* var: varname '[' patlist ']'  */
#line 425 "awkgram.y"
                                        { (yyval.p) = op2(ARRAY, makearr((yyvsp[-3].p)), (yyvsp[-1].p)); }
#line 3155 "awkgram.tab.c"
    break;

  case 172: /* var: IVAR  */
#line 426 "awkgram.y"
                                        { (yyval.p) = op1(INDIRECT, celltonode((yyvsp[0].cp), CVAR)); }
#line 3161 "awkgram.tab.c"
    break;

  case 173: /* var: INDIRECT term  */
#line 427 "awkgram.y"
                                        { (yyval.p) = op1(INDIRECT, (yyvsp[0].p)); }
#line 3167 "awkgram.tab.c"
    break;

  case 174: /* varlist: %empty  */
#line 431 "awkgram.y"
                                { arglist = (yyval.p) = 0; }
#line 3173 "awkgram.tab.c"
    break;

  case 175: /* varlist: VAR  */
#line 432 "awkgram.y"
                                { arglist = (yyval.p) = celltonode((yyvsp[0].cp),CVAR); }
#line 3179 "awkgram.tab.c"
    break;

  case 176: /* varlist: varlist comma VAR  */
#line 433 "awkgram.y"
                                {
			checkdup((yyvsp[-2].p), (yyvsp[0].cp));
			arglist = (yyval.p) = linkum((yyvsp[-2].p),celltonode((yyvsp[0].cp),CVAR)); }
#line 3187 "awkgram.tab.c"
    break;

  case 177: /* varname: VAR  */
#line 439 "awkgram.y"
                                { (yyval.p) = celltonode((yyvsp[0].cp), CVAR); }
#line 3193 "awkgram.tab.c"
    break;

  case 178: /* varname: ARG  */
#line 440 "awkgram.y"
                                { (yyval.p) = op1(ARG, itonp((yyvsp[0].i))); }
#line 3199 "awkgram.tab.c"
    break;

  case 179: /* varname: VARNF  */
#line 441 "awkgram.y"
                                { (yyval.p) = op1(VARNF, (Node *) (yyvsp[0].cp)); }
#line 3205 "awkgram.tab.c"
    break;

  case 180: /* while: WHILE '(' pattern rparen  */
#line 446 "awkgram.y"
                                        { (yyval.p) = notnull((yyvsp[-1].p)); }
#line 3211 "awkgram.tab.c"
    break;


#line 3215 "awkgram.tab.c"

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

#line 449 "awkgram.y"


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
