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

void checkdup(TNode *list, Cell *item);
int yywrap(void) { return(1); }

TNode	*beginloc = 0;
TNode	*endloc = 0;
bool	infunc	= false;	/* = true if in arglist or body of func */
int	inloop	= 0;	/* >= 1 if in while, for, do; can't be bool, since loops can next */
char	*curfname = 0;	/* current function name */
TNode	*arglist = 0;	/* list of args for current function */

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
  YYSYMBOL_ADDON = 44,                     /* ADDON  */
  YYSYMBOL_ARG = 45,                       /* ARG  */
  YYSYMBOL_BLTIN = 46,                     /* BLTIN  */
  YYSYMBOL_BREAK = 47,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 48,                  /* CONTINUE  */
  YYSYMBOL_DELETE = 49,                    /* DELETE  */
  YYSYMBOL_DO = 50,                        /* DO  */
  YYSYMBOL_EXIT = 51,                      /* EXIT  */
  YYSYMBOL_FOR = 52,                       /* FOR  */
  YYSYMBOL_FUNC = 53,                      /* FUNC  */
  YYSYMBOL_SUB = 54,                       /* SUB  */
  YYSYMBOL_GSUB = 55,                      /* GSUB  */
  YYSYMBOL_IF = 56,                        /* IF  */
  YYSYMBOL_INDEX = 57,                     /* INDEX  */
  YYSYMBOL_LSUBSTR = 58,                   /* LSUBSTR  */
  YYSYMBOL_MATCHFCN = 59,                  /* MATCHFCN  */
  YYSYMBOL_NEXT = 60,                      /* NEXT  */
  YYSYMBOL_ADD = 61,                       /* ADD  */
  YYSYMBOL_MINUS = 62,                     /* MINUS  */
  YYSYMBOL_MULT = 63,                      /* MULT  */
  YYSYMBOL_DIVIDE = 64,                    /* DIVIDE  */
  YYSYMBOL_MOD = 65,                       /* MOD  */
  YYSYMBOL_LSHIFT = 66,                    /* LSHIFT  */
  YYSYMBOL_RSHIFT = 67,                    /* RSHIFT  */
  YYSYMBOL_XOR = 68,                       /* XOR  */
  YYSYMBOL_BAND = 69,                      /* BAND  */
  YYSYMBOL_BOR = 70,                       /* BOR  */
  YYSYMBOL_CMPL = 71,                      /* CMPL  */
  YYSYMBOL_ASSIGN = 72,                    /* ASSIGN  */
  YYSYMBOL_ASGNOP = 73,                    /* ASGNOP  */
  YYSYMBOL_ADDEQ = 74,                     /* ADDEQ  */
  YYSYMBOL_SUBEQ = 75,                     /* SUBEQ  */
  YYSYMBOL_MULTEQ = 76,                    /* MULTEQ  */
  YYSYMBOL_DIVEQ = 77,                     /* DIVEQ  */
  YYSYMBOL_MODEQ = 78,                     /* MODEQ  */
  YYSYMBOL_POWEQ = 79,                     /* POWEQ  */
  YYSYMBOL_BANDEQ = 80,                    /* BANDEQ  */
  YYSYMBOL_BOREQ = 81,                     /* BOREQ  */
  YYSYMBOL_XOREQ = 82,                     /* XOREQ  */
  YYSYMBOL_SHLEQ = 83,                     /* SHLEQ  */
  YYSYMBOL_SHREQ = 84,                     /* SHREQ  */
  YYSYMBOL_PRINT = 85,                     /* PRINT  */
  YYSYMBOL_PRINTF = 86,                    /* PRINTF  */
  YYSYMBOL_SPRINTF = 87,                   /* SPRINTF  */
  YYSYMBOL_ELSE = 88,                      /* ELSE  */
  YYSYMBOL_INTEST = 89,                    /* INTEST  */
  YYSYMBOL_CONDEXPR = 90,                  /* CONDEXPR  */
  YYSYMBOL_POSTINCR = 91,                  /* POSTINCR  */
  YYSYMBOL_PREINCR = 92,                   /* PREINCR  */
  YYSYMBOL_POSTDECR = 93,                  /* POSTDECR  */
  YYSYMBOL_PREDECR = 94,                   /* PREDECR  */
  YYSYMBOL_VAR = 95,                       /* VAR  */
  YYSYMBOL_IVAR = 96,                      /* IVAR  */
  YYSYMBOL_VARNF = 97,                     /* VARNF  */
  YYSYMBOL_CALL = 98,                      /* CALL  */
  YYSYMBOL_NUMBER = 99,                    /* NUMBER  */
  YYSYMBOL_STRING = 100,                   /* STRING  */
  YYSYMBOL_REGEXPR = 101,                  /* REGEXPR  */
  YYSYMBOL_102_ = 102,                     /* '?'  */
  YYSYMBOL_103_ = 103,                     /* ':'  */
  YYSYMBOL_RETURN = 104,                   /* RETURN  */
  YYSYMBOL_SPLIT = 105,                    /* SPLIT  */
  YYSYMBOL_SUBSTR = 106,                   /* SUBSTR  */
  YYSYMBOL_WHILE = 107,                    /* WHILE  */
  YYSYMBOL_CAT = 108,                      /* CAT  */
  YYSYMBOL_109_ = 109,                     /* '^'  */
  YYSYMBOL_110_ = 110,                     /* '+'  */
  YYSYMBOL_111_ = 111,                     /* '-'  */
  YYSYMBOL_112_ = 112,                     /* '*'  */
  YYSYMBOL_113_ = 113,                     /* '%'  */
  YYSYMBOL_NOT = 114,                      /* NOT  */
  YYSYMBOL_UMINUS = 115,                   /* UMINUS  */
  YYSYMBOL_UPLUS = 116,                    /* UPLUS  */
  YYSYMBOL_POWER = 117,                    /* POWER  */
  YYSYMBOL_DECR = 118,                     /* DECR  */
  YYSYMBOL_INCR = 119,                     /* INCR  */
  YYSYMBOL_INDIRECT = 120,                 /* INDIRECT  */
  YYSYMBOL_LASTTOKEN = 121,                /* LASTTOKEN  */
  YYSYMBOL_YYACCEPT = 122,                 /* $accept  */
  YYSYMBOL_program = 123,                  /* program  */
  YYSYMBOL_land = 124,                     /* land  */
  YYSYMBOL_lor = 125,                      /* lor  */
  YYSYMBOL_comma = 126,                    /* comma  */
  YYSYMBOL_do = 127,                       /* do  */
  YYSYMBOL_else = 128,                     /* else  */
  YYSYMBOL_for = 129,                      /* for  */
  YYSYMBOL_130_1 = 130,                    /* $@1  */
  YYSYMBOL_131_2 = 131,                    /* $@2  */
  YYSYMBOL_132_3 = 132,                    /* $@3  */
  YYSYMBOL_funcname = 133,                 /* funcname  */
  YYSYMBOL_if = 134,                       /* if  */
  YYSYMBOL_lbrace = 135,                   /* lbrace  */
  YYSYMBOL_nl = 136,                       /* nl  */
  YYSYMBOL_opt_nl = 137,                   /* opt_nl  */
  YYSYMBOL_opt_pst = 138,                  /* opt_pst  */
  YYSYMBOL_opt_simple_stmt = 139,          /* opt_simple_stmt  */
  YYSYMBOL_pas = 140,                      /* pas  */
  YYSYMBOL_pa_pat = 141,                   /* pa_pat  */
  YYSYMBOL_pa_stat = 142,                  /* pa_stat  */
  YYSYMBOL_143_4 = 143,                    /* $@4  */
  YYSYMBOL_pa_stats = 144,                 /* pa_stats  */
  YYSYMBOL_patlist = 145,                  /* patlist  */
  YYSYMBOL_ppattern = 146,                 /* ppattern  */
  YYSYMBOL_pattern = 147,                  /* pattern  */
  YYSYMBOL_plist = 148,                    /* plist  */
  YYSYMBOL_pplist = 149,                   /* pplist  */
  YYSYMBOL_prarg = 150,                    /* prarg  */
  YYSYMBOL_print = 151,                    /* print  */
  YYSYMBOL_pst = 152,                      /* pst  */
  YYSYMBOL_rbrace = 153,                   /* rbrace  */
  YYSYMBOL_re = 154,                       /* re  */
  YYSYMBOL_reg_expr = 155,                 /* reg_expr  */
  YYSYMBOL_156_5 = 156,                    /* $@5  */
  YYSYMBOL_rparen = 157,                   /* rparen  */
  YYSYMBOL_simple_stmt = 158,              /* simple_stmt  */
  YYSYMBOL_st = 159,                       /* st  */
  YYSYMBOL_stmt = 160,                     /* stmt  */
  YYSYMBOL_161_6 = 161,                    /* $@6  */
  YYSYMBOL_162_7 = 162,                    /* $@7  */
  YYSYMBOL_163_8 = 163,                    /* $@8  */
  YYSYMBOL_stmtlist = 164,                 /* stmtlist  */
  YYSYMBOL_subop = 165,                    /* subop  */
  YYSYMBOL_string = 166,                   /* string  */
  YYSYMBOL_term = 167,                     /* term  */
  YYSYMBOL_var = 168,                      /* var  */
  YYSYMBOL_varlist = 169,                  /* varlist  */
  YYSYMBOL_varname = 170,                  /* varname  */
  YYSYMBOL_while = 171                     /* while  */
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
#define YYLAST   4867

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  122
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  185
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  368

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   360


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
       2,     2,     2,     2,     2,     2,     2,   113,     2,     2,
      12,    15,   112,   110,    10,   111,     2,    14,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   103,    13,
       2,     2,     2,   102,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    17,     2,    18,   109,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,     2,    16,     2,     2,     2,     2,
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
      94,    95,    96,    97,    98,    99,   100,   101,   104,   105,
     106,   107,   108,   114,   115,   116,   117,   118,   119,   120,
     121
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
     373,   374,   375,   376,   377,   378,   379,   380,   382,   385,
     386,   388,   393,   394,   396,   398,   400,   401,   402,   404,
     409,   411,   416,   418,   420,   424,   425,   426,   427,   431,
     432,   433,   439,   440,   441,   446
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
  "GE", "GT", "LE", "LT", "NE", "IN", "ADDON", "ARG", "BLTIN", "BREAK",
  "CONTINUE", "DELETE", "DO", "EXIT", "FOR", "FUNC", "SUB", "GSUB", "IF",
  "INDEX", "LSUBSTR", "MATCHFCN", "NEXT", "ADD", "MINUS", "MULT", "DIVIDE",
  "MOD", "LSHIFT", "RSHIFT", "XOR", "BAND", "BOR", "CMPL", "ASSIGN",
  "ASGNOP", "ADDEQ", "SUBEQ", "MULTEQ", "DIVEQ", "MODEQ", "POWEQ",
  "BANDEQ", "BOREQ", "XOREQ", "SHLEQ", "SHREQ", "PRINT", "PRINTF",
  "SPRINTF", "ELSE", "INTEST", "CONDEXPR", "POSTINCR", "PREINCR",
  "POSTDECR", "PREDECR", "VAR", "IVAR", "VARNF", "CALL", "NUMBER",
  "STRING", "REGEXPR", "'?'", "':'", "RETURN", "SPLIT", "SUBSTR", "WHILE",
  "CAT", "'^'", "'+'", "'-'", "'*'", "'%'", "NOT", "UMINUS", "UPLUS",
  "POWER", "DECR", "INCR", "INDIRECT", "LASTTOKEN", "$accept", "program",
  "land", "lor", "comma", "do", "else", "for", "$@1", "$@2", "$@3",
  "funcname", "if", "lbrace", "nl", "opt_nl", "opt_pst", "opt_simple_stmt",
  "pas", "pa_pat", "pa_stat", "$@4", "pa_stats", "patlist", "ppattern",
  "pattern", "plist", "pplist", "prarg", "print", "pst", "rbrace", "re",
  "reg_expr", "$@5", "rparen", "simple_stmt", "st", "stmt", "$@6", "$@7",
  "$@8", "stmtlist", "subop", "string", "term", "var", "varlist",
  "varname", "while", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-319)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-32)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     710,  -319,  -319,  -319,    49,  1750,  -319,    87,  -319,    24,
      24,  -319,  4464,  -319,    55,  -319,   107,   -29,  -319,  -319,
     114,   116,   131,  -319,  -319,  -319,   138,  -319,  -319,   139,
     140,  4747,  4747,  4747,  4541,   108,   108,  4747,   787,    72,
    -319,   160,  3670,  -319,  -319,   150,    63,   397,   -64,   154,
    -319,  -319,   787,   787,  2273,    31,    71,  4209,  4235,  -319,
    -319,   169,  4464,  4464,  4464,  4324,  4464,  4464,  4464,  4747,
      65,    15,    65,    65,  -319,    65,  -319,  -319,  -319,  -319,
    -319,   175,   165,   165,   -21,  -319,  1853,   173,   178,   165,
    -319,  -319,  1853,   179,   184,  -319,  1482,   787,  3670,  4567,
     165,  -319,   864,  -319,   175,   787,  1750,  4464,  -319,  -319,
    4464,  4464,  4464,  4464,  4464,  4464,   -21,  4464,  1879,  1956,
     397,  4464,  -319,  4644,  4747,  4747,  4747,  4747,  4747,  4747,
    4747,  4747,  4747,  4747,  4464,  -319,  -319,  4464,   941,  1018,
    -319,  -319,  1982,   144,  1982,   182,  -319,    33,  3670,  -319,
      35,   112,  2697,  2697,    41,  -319,    58,  2697,  2697,  2797,
    -319,   201,  -319,   175,   201,  -319,  -319,   194,  1776,  -319,
    1559,  4464,  -319,  1776,  -319,  4464,  -319,  1482,   124,  1095,
    4464,  4094,   203,  -319,  -319,   397,   -54,  -319,  -319,  -319,
    1482,  4464,  1172,  -319,  3851,  -319,  3851,  3851,  3851,  3851,
    3851,  3851,  -319,  2888,  -319,  3941,  -319,  3761,  2697,   203,
    4747,    65,   254,   254,   227,   471,   243,    18,    18,    65,
      65,    65,  3670,    10,  -319,  -319,  -319,  3670,   -21,  3670,
    -319,  -319,  1982,  -319,  -319,    70,  1982,  1982,  -319,  -319,
      -7,  1982,  -319,  -319,  4464,  -319,   202,  -319,    -9,  2988,
    -319,  2988,  -319,  -319,  1251,  -319,   205,    74,  4670,   -21,
    4670,  2059,  2085,   397,  2162,  4670,  -319,    24,  -319,  4464,
    1982,  1982,   397,  -319,  -319,  3670,  -319,     2,   207,  3088,
     204,  3188,   208,    88,  2384,    11,   170,   -21,   207,   207,
     110,  -319,  -319,  -319,   177,  4464,  4350,  -319,  -319,  3999,
    4429,  4120,  4094,  4094,   787,  3670,  2495,  2606,  -319,  -319,
      24,  -319,  -319,  -319,  -319,  -319,  1982,  -319,  1982,  -319,
     175,  4464,   209,   206,   -21,    89,  4670,  1328,  -319,    -3,
    -319,    -3,   787,  3288,   210,  3388,  1636,  3479,   207,  4464,
    -319,   177,  4094,  -319,   215,   218,  1405,  -319,  -319,  -319,
     209,   175,  1482,  3579,  -319,  -319,  -319,   207,  1636,  -319,
     165,  1482,   209,  -319,  -319,   207,  1482,  -319
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     3,    86,    87,     0,    33,     2,    30,     1,     0,
       0,    23,     0,    94,   147,   183,   150,     0,   125,   126,
       0,     0,     0,   182,   177,   184,     0,   162,   127,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
      45,    29,    35,    75,    92,     0,   167,    76,   174,   175,
      88,    89,     0,     0,     0,     0,     0,     0,     0,    20,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     141,   174,   143,   142,    93,   144,   153,   154,   178,   102,
      24,    27,     0,     0,     0,    10,     0,     0,     0,     0,
      84,    85,     0,     0,   107,   112,     0,     0,   101,    81,
       0,   123,     0,   120,    27,     0,    34,     0,     4,     6,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,     0,   128,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   155,   156,     0,     0,     0,
       8,   159,     0,     0,     0,     0,   145,     0,    47,   148,
       0,   179,     0,     0,     0,   151,     0,     0,     0,     0,
      25,    28,   122,    27,   103,   105,   106,   100,     0,   111,
       0,     0,   116,     0,   118,     0,    11,     0,   114,     0,
       0,    79,    82,    98,    58,    59,   174,   119,    40,   124,
       0,     0,     0,    46,    71,    70,    64,    65,    66,    67,
      68,    69,    72,     0,     5,    63,     7,    62,     0,    92,
       0,   133,   139,   140,   137,   136,   135,   130,   131,   132,
     134,   138,    60,     0,    41,    42,     9,    77,     0,    78,
      95,   146,     0,   149,   180,     0,     0,     0,   166,   152,
       0,     0,    26,   104,     0,   110,     0,    32,   175,     0,
     117,     0,   108,    12,     0,    90,   115,     0,     0,     0,
       0,     0,     0,    57,     0,     0,   121,    38,    37,     0,
       0,     0,   129,   176,    73,    48,    96,     0,    43,     0,
      92,     0,    92,     0,     0,     0,    27,     0,    22,   185,
       0,    13,   113,    91,    83,     0,    54,    53,    55,     0,
      52,    51,    80,    49,     0,    61,     0,     0,   181,    97,
       0,   157,   158,   161,   160,   165,     0,   173,     0,    99,
      27,     0,     0,     0,     0,     0,     0,     0,   169,     0,
     168,     0,     0,     0,    92,     0,     0,     0,    18,     0,
      56,     0,    50,    39,     0,     0,     0,   163,   164,   172,
       0,    27,     0,     0,   171,   170,    44,    16,     0,    19,
       0,     0,     0,   109,    17,    14,     0,    15
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -319,  -319,  -178,  -102,   293,  -319,  -319,  -319,  -319,  -319,
    -319,  -319,  -319,     5,   -77,   -91,   193,  -318,  -319,    44,
     132,  -319,  -319,   -42,  -120,    -5,  -168,  -319,  -319,  -319,
    -319,  -319,    37,   -90,  -319,  -174,  -169,   -53,   340,  -319,
    -319,  -319,   -27,  -319,  -319,   363,   123,  -319,   -79,  -319
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     4,   118,   119,   232,    94,   254,    95,   366,   361,
     352,    61,    96,    97,   164,   162,     5,   246,     6,    39,
      40,   310,    41,   147,   181,    98,    55,   182,   183,    99,
       7,   256,    43,    44,    56,   278,   100,   165,   101,   177,
     290,   190,   102,    45,    46,    47,    48,   235,    49,   103
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      42,   247,   226,   261,   161,   167,   226,    54,   137,   134,
      38,   226,   257,   191,    52,    53,   150,   195,   350,   265,
     140,   140,   154,   156,    15,   138,   139,   161,   273,   319,
     166,   209,   123,   169,   287,    11,   172,   202,    15,   174,
     362,   140,    15,   140,   105,   140,   143,   187,   231,     8,
     233,   140,   148,   148,   135,   136,   238,   152,   153,   148,
     148,   157,   158,   159,   135,   136,    59,    57,   140,    60,
     179,    74,   243,   239,    23,   288,    25,   289,   192,   262,
     140,   168,   104,    11,   140,   276,   161,   173,    23,   294,
      25,   248,    23,    24,    25,   223,    50,   308,   140,   140,
      51,    42,   194,   315,   341,   196,   197,   198,   199,   200,
     201,    38,   203,   205,   207,   245,   208,    37,   261,    58,
     250,   261,   261,   261,   261,   261,    62,   325,    63,   222,
     131,   132,   148,   135,   136,   133,   184,   227,   296,   229,
     299,   300,   301,    64,   302,   303,   280,   282,   338,   274,
      65,    66,    67,    15,    71,    71,    71,    71,    76,    77,
      71,   283,   121,   122,   261,    71,   249,   247,   297,     2,
     251,   137,   145,     3,   160,    54,   357,    71,   163,   160,
     298,   151,   133,   320,   160,   170,    42,   228,   365,   247,
     171,   175,    71,   176,   262,   321,   230,   262,   262,   262,
     262,   262,   285,    23,    24,    25,   342,   234,   322,   161,
     242,   244,   253,   140,   293,   286,   309,   323,   339,   312,
     324,    71,   186,   314,   276,   348,   334,   275,    37,   336,
     354,   279,   281,   355,   106,   267,   284,     0,   193,   148,
     262,   123,     0,   161,     0,   340,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,   123,     0,     0,
     358,     0,     0,     0,   305,   306,   307,     0,   123,     0,
       0,    71,   304,     0,   161,    71,    71,   327,     0,     0,
      71,    71,    71,     0,     0,     0,     0,     0,     0,     0,
      54,    71,     0,   124,   125,   184,    71,   184,   184,   184,
       0,   184,   184,     0,    71,   346,     0,   363,     0,   124,
     125,   333,   126,   335,     0,   332,   337,    71,     0,    71,
      71,    71,    71,    71,    71,     0,    71,     0,    71,     0,
      71,    71,     0,    71,   353,     0,     0,   129,   130,   131,
     132,     0,     0,     0,   133,    71,     0,   142,   144,     0,
      71,     0,    71,   129,   130,   131,   132,     0,     0,     0,
     133,     0,     0,   184,   129,   130,   131,   132,     0,     0,
       0,   133,    71,     0,    71,     0,     0,     0,     0,     0,
       0,   186,     0,   186,   186,   186,     0,   186,   186,     0,
       0,     0,     0,     0,    70,    72,    73,    75,    71,     0,
      78,     0,    71,     0,    71,   120,     0,    71,     0,     0,
       0,   123,     0,     0,     0,     0,     0,   120,     0,    71,
       0,     0,    71,    71,    71,    71,    71,     0,    71,    71,
      71,     0,    75,     0,     0,     0,   178,     0,     0,     0,
       0,     0,   189,     0,     0,   236,   237,     0,     0,   186,
     240,   241,   344,     0,   345,     0,    71,     0,    71,     0,
      71,   120,   185,   124,   125,    71,   126,   127,     0,     0,
       0,     0,     0,     0,     0,   264,    71,     0,   189,   189,
       0,     0,     0,     0,     0,   123,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,     0,     0,     0,
       0,   270,   271,     0,     0,     0,   128,   129,   130,   131,
     132,   120,     0,     0,   133,   120,   120,   252,     0,   189,
     120,   120,   120,     0,     0,     0,     0,     0,   277,     0,
     266,   120,   189,     0,     0,     0,   120,   124,   125,     0,
     126,     0,     0,     0,   263,     0,     0,     0,     0,     0,
     144,     0,     0,     0,     0,     0,     0,   120,     0,   120,
     120,   120,   120,   120,   120,     0,   120,     0,   120,     0,
     120,   120,     0,   272,     0,     0,   316,   318,     0,     0,
     128,   129,   130,   131,   132,   120,     0,     0,   133,     0,
     120,     0,   120,     0,   292,     0,     0,     0,     0,   329,
     331,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   120,     0,   120,     0,     0,     0,   144,     0,
       0,   185,     0,   185,   185,   185,     0,   185,   185,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   120,     0,
       0,     0,   120,     0,   120,     0,     0,   120,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   263,
       0,     0,   263,   263,   263,   263,   263,   189,   120,   120,
     120,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   189,     0,     0,   185,
       0,     0,   359,     0,     0,     0,   120,     0,   120,     0,
     120,   364,     0,     0,     0,   263,   367,     0,     0,     0,
     -29,     1,     0,     0,     0,     0,   120,   -29,   -29,     2,
       0,   -29,   -29,     3,   -29,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   -29,   -29,   -29,     0,     0,     0,
       0,     0,     0,   -29,   -29,   -29,     0,   -29,     0,   -29,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,     0,     0,     0,    80,   -29,    11,    12,
      81,    13,     0,     0,     0,   -29,   -29,   -29,   -29,   -29,
     -29,     0,     0,     0,     0,   -29,   -29,     0,     0,   -29,
     -29,   -29,     0,     0,   -29,     0,     0,     0,   -29,   -29,
     -29,    14,    15,    16,    82,    83,    84,    85,    86,    87,
       0,    18,    19,    88,    20,     0,    21,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    79,     0,     0,     0,     0,
       0,     0,    90,    91,    22,    11,    12,    81,    13,     0,
     188,     0,    23,    24,    25,    26,    27,    28,     0,     0,
       0,    92,    29,    30,    93,     0,    31,    32,    33,     0,
       0,    34,     0,     0,     0,    35,    36,    37,    14,    15,
      16,    82,    83,    84,    85,    86,    87,     0,    18,    19,
      88,    20,     0,    21,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,     0,     0,    90,
      91,    22,    11,    12,    81,    13,     0,   224,     0,    23,
      24,    25,    26,    27,    28,     0,     0,     0,    92,    29,
      30,    93,     0,    31,    32,    33,     0,     0,    34,     0,
       0,     0,    35,    36,    37,    14,    15,    16,    82,    83,
      84,    85,    86,    87,     0,    18,    19,    88,    20,     0,
      21,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    79,
       0,     0,     0,     0,     0,     0,    90,    91,    22,    11,
      12,    81,    13,     0,   225,     0,    23,    24,    25,    26,
      27,    28,     0,     0,     0,    92,    29,    30,    93,     0,
      31,    32,    33,     0,     0,    34,     0,     0,     0,    35,
      36,    37,    14,    15,    16,    82,    83,    84,    85,    86,
      87,     0,    18,    19,    88,    20,     0,    21,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,     0,     0,    90,    91,    22,    11,    12,    81,    13,
       0,   255,     0,    23,    24,    25,    26,    27,    28,     0,
       0,     0,    92,    29,    30,    93,     0,    31,    32,    33,
       0,     0,    34,     0,     0,     0,    35,    36,    37,    14,
      15,    16,    82,    83,    84,    85,    86,    87,     0,    18,
      19,    88,    20,     0,    21,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    79,     0,     0,     0,     0,     0,     0,
      90,    91,    22,    11,    12,    81,    13,     0,   268,     0,
      23,    24,    25,    26,    27,    28,     0,     0,     0,    92,
      29,    30,    93,     0,    31,    32,    33,     0,     0,    34,
       0,     0,     0,    35,    36,    37,    14,    15,    16,    82,
      83,    84,    85,    86,    87,     0,    18,    19,    88,    20,
       0,    21,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,    90,    91,    22,
     291,     0,    11,    12,    81,    13,     0,    23,    24,    25,
      26,    27,    28,     0,     0,     0,    92,    29,    30,    93,
       0,    31,    32,    33,     0,     0,    34,     0,     0,     0,
      35,    36,    37,     0,     0,    14,    15,    16,    82,    83,
      84,    85,    86,    87,     0,    18,    19,    88,    20,     0,
      21,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    79,
       0,     0,     0,     0,     0,     0,    90,    91,    22,    11,
      12,    81,    13,     0,   343,     0,    23,    24,    25,    26,
      27,    28,     0,     0,     0,    92,    29,    30,    93,     0,
      31,    32,    33,     0,     0,    34,     0,     0,     0,    35,
      36,    37,    14,    15,    16,    82,    83,    84,    85,    86,
      87,     0,    18,    19,    88,    20,     0,    21,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,     0,     0,    90,    91,    22,    11,    12,    81,    13,
       0,   356,     0,    23,    24,    25,    26,    27,    28,     0,
       0,     0,    92,    29,    30,    93,     0,    31,    32,    33,
       0,     0,    34,     0,     0,     0,    35,    36,    37,    14,
      15,    16,    82,    83,    84,    85,    86,    87,     0,    18,
      19,    88,    20,     0,    21,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    79,     0,     0,     0,     0,     0,     0,
      90,    91,    22,    11,    12,    81,    13,     0,     0,     0,
      23,    24,    25,    26,    27,    28,     0,     0,     0,    92,
      29,    30,    93,     0,    31,    32,    33,     0,     0,    34,
       0,     0,     0,    35,    36,    37,    14,    15,    16,    82,
      83,    84,    85,    86,    87,     0,    18,    19,    88,    20,
       0,    21,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,     0,     0,    90,    91,    22,
       0,    12,   -31,    13,     0,     0,     0,    23,    24,    25,
      26,    27,    28,     0,     0,     0,    92,    29,    30,    93,
       0,    31,    32,    33,     0,     0,    34,     0,     0,     0,
      35,    36,    37,    14,    15,    16,     0,     0,    84,     0,
       0,     0,     0,    18,    19,     0,    20,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    79,     0,     0,
       0,     0,     0,     0,    90,    91,    22,     0,    12,     0,
      13,   -31,     0,     0,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    29,    30,     0,     0,    31,    32,
      33,     0,     0,    34,     0,     0,     0,    35,    36,    37,
      14,    15,    16,     0,     0,    84,     0,     0,     0,     0,
      18,    19,     0,    20,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    90,    91,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,     0,     0,     0,
       0,    29,    30,     0,     0,    31,    32,    33,     0,     0,
      34,     0,     0,     0,    35,    36,    37,     9,    10,     0,
       0,    11,    12,     0,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   160,     0,     0,    68,   163,
       0,     0,     0,     0,    14,    15,    16,     0,   107,     0,
       0,     0,     0,    17,    18,    19,     0,    20,     0,    21,
       0,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
      18,    19,     0,    20,     0,    21,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,     0,     0,     0,     0,    29,    30,     0,     0,    31,
      32,    33,   160,    22,    34,    12,   163,    13,    35,    36,
      37,    23,    24,    25,    26,    27,    28,     0,   117,     0,
       0,    29,    30,     0,     0,    31,    32,    33,   204,     0,
      69,    12,     0,    13,    35,    36,    37,    14,    15,    16,
       0,     0,     0,     0,     0,     0,     0,    18,    19,     0,
      20,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,    16,     0,     0,     0,     0,
       0,     0,     0,    18,    19,     0,    20,     0,    21,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,     0,     0,     0,     0,    29,    30,
       0,     0,    31,    32,    33,   206,    22,    34,    12,     0,
      13,    35,    36,    37,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    29,    30,     0,     0,    31,    32,
      33,   226,     0,    34,    12,     0,    13,    35,    36,    37,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
      18,    19,     0,    20,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,    16,     0,
       0,     0,     0,     0,     0,     0,    18,    19,     0,    20,
       0,    21,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,     0,     0,     0,
       0,    29,    30,     0,     0,    31,    32,    33,   204,    22,
      34,   295,     0,    13,    35,    36,    37,    23,    24,    25,
      26,    27,    28,     0,     0,     0,     0,    29,    30,     0,
       0,    31,    32,    33,   206,     0,    34,   295,     0,    13,
      35,    36,    37,    14,    15,    16,     0,     0,     0,     0,
       0,     0,     0,    18,    19,     0,    20,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,     0,     0,     0,    18,
      19,     0,    20,     0,    21,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    29,    30,     0,     0,    31,    32,
      33,   226,    22,    34,   295,     0,    13,    35,    36,    37,
      23,    24,    25,    26,    27,    28,     0,     0,     0,     0,
      29,    30,     0,     0,    31,    32,    33,     0,     0,    34,
       0,     0,     0,    35,    36,    37,    14,    15,    16,     0,
       0,     0,     0,     0,     0,     0,    18,    19,     0,    20,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    23,    24,    25,
      26,    27,    28,     0,     0,     0,     0,    29,    30,     0,
       0,    31,    32,    33,     0,     0,    34,     0,     0,     0,
      35,    36,    37,   140,     0,    68,     0,     0,   141,     0,
       0,     0,     0,     0,     0,   107,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    14,    15,    16,
       0,     0,     0,     0,     0,     0,     0,    18,    19,     0,
      20,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,     0,   117,     0,     0,    29,    30,
       0,     0,    31,    32,    33,     0,     0,    69,     0,     0,
       0,    35,    36,    37,   140,     0,    68,     0,     0,   317,
       0,     0,     0,     0,     0,     0,   107,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    14,    15,
      16,     0,     0,     0,     0,     0,     0,     0,    18,    19,
       0,    20,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,   117,     0,     0,    29,
      30,     0,     0,    31,    32,    33,     0,     0,    69,     0,
       0,     0,    35,    36,    37,   140,     0,    68,     0,     0,
     328,     0,     0,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    14,
      15,    16,     0,     0,     0,     0,     0,     0,     0,    18,
      19,     0,    20,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,     0,   117,     0,     0,
      29,    30,     0,     0,    31,    32,    33,     0,     0,    69,
       0,     0,     0,    35,    36,    37,   140,     0,    68,     0,
       0,   330,     0,     0,     0,     0,     0,     0,   107,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
      18,    19,     0,    20,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,   140,   117,    68,
       0,    29,    30,     0,     0,    31,    32,    33,     0,   107,
      69,     0,     0,     0,    35,    36,    37,     0,     0,     0,
       0,     0,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    14,    15,    16,     0,     0,     0,     0,     0,     0,
       0,    18,    19,     0,    20,     0,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,     0,   117,
       0,     0,    29,    30,     0,     0,    31,    32,    33,    68,
       0,    69,   141,     0,     0,    35,    36,    37,     0,   107,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    14,    15,    16,     0,     0,     0,     0,     0,     0,
       0,    18,    19,     0,    20,     0,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,     0,   117,
      68,     0,    29,    30,     0,     0,    31,    32,    33,     0,
     107,    69,     0,     0,     0,    35,    36,    37,     0,     0,
       0,     0,     0,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    14,    15,    16,     0,     0,     0,     0,     0,
       0,     0,    18,    19,     0,    20,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
     117,   269,     0,    29,    30,     0,     0,    31,    32,    33,
      68,     0,    69,   276,     0,     0,    35,    36,    37,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    14,    15,    16,     0,     0,     0,     0,     0,
       0,     0,    18,    19,     0,    20,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
     117,     0,     0,    29,    30,     0,     0,    31,    32,    33,
      68,     0,    69,   311,     0,     0,    35,    36,    37,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    14,    15,    16,     0,     0,     0,     0,     0,
       0,     0,    18,    19,     0,    20,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
     117,     0,     0,    29,    30,     0,     0,    31,    32,    33,
      68,     0,    69,   313,     0,     0,    35,    36,    37,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    14,    15,    16,     0,     0,     0,     0,     0,
       0,     0,    18,    19,     0,    20,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
     117,     0,     0,    29,    30,     0,     0,    31,    32,    33,
      68,     0,    69,   347,     0,     0,    35,    36,    37,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    14,    15,    16,     0,     0,     0,     0,     0,
       0,     0,    18,    19,     0,    20,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
     117,     0,     0,    29,    30,     0,     0,    31,    32,    33,
      68,     0,    69,   349,     0,     0,    35,    36,    37,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    14,    15,    16,     0,     0,     0,     0,     0,
       0,     0,    18,    19,     0,    20,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,    26,    27,    28,     0,
     117,    68,   351,    29,    30,     0,     0,    31,    32,    33,
       0,   107,    69,     0,     0,     0,    35,    36,    37,     0,
       0,     0,     0,     0,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    14,    15,    16,     0,     0,     0,     0,
       0,     0,     0,    18,    19,     0,    20,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,   117,     0,     0,    29,    30,     0,     0,    31,    32,
      33,    68,     0,    69,   360,     0,     0,    35,    36,    37,
       0,   107,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    14,    15,    16,     0,     0,     0,     0,
       0,     0,     0,    18,    19,     0,    20,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,   117,    68,     0,    29,    30,     0,     0,    31,    32,
      33,     0,   107,    69,     0,     0,     0,    35,    36,    37,
       0,     0,     0,     0,     0,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    14,    15,    16,     0,     0,     0,
       0,     0,     0,     0,    18,    19,     0,    20,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,    26,    27,
      28,     0,   117,    68,     0,    29,    30,     0,     0,    31,
      32,    33,     0,   107,    69,     0,     0,     0,    35,    36,
      37,     0,     0,     0,     0,     0,   108,     0,   110,   111,
     112,   113,   114,   115,   116,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,    20,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,    68,     0,     0,    29,    30,     0,     0,
      31,    32,    33,   -32,     0,    69,     0,     0,     0,    35,
      36,    37,     0,     0,     0,     0,     0,     0,   -32,   -32,
     -32,   -32,   -32,   -32,   -32,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,    20,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,    68,     0,     0,    29,    30,     0,     0,
      31,    32,    33,   107,     0,    69,     0,     0,     0,    35,
      36,    37,     0,     0,     0,     0,     0,     0,   110,   111,
     112,   113,   114,   115,   116,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,    20,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   258,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,   108,   109,    23,    24,    25,    26,
      27,    28,   259,    14,    15,    16,    29,    30,     0,     0,
      31,    32,    33,    18,    19,    69,    20,     0,    21,    35,
      36,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,   260,   326,     0,    29,    30,    68,     0,    31,    32,
      33,     0,     0,    69,     0,     0,   258,    35,    36,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   108,
     109,     0,    68,     0,     0,     0,     0,   259,    14,    15,
      16,     0,   258,     0,     0,     0,     0,     0,    18,    19,
       0,    20,     0,    21,     0,   108,     0,     0,     0,     0,
       0,     0,     0,   259,    14,    15,    16,     0,     0,     0,
       0,     0,     0,     0,    18,    19,     0,    20,     0,    21,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,   260,     0,     0,    29,
      30,     0,     0,    31,    32,    33,     0,    22,    69,     0,
       0,     0,    35,    36,    37,    23,    24,    25,    26,    27,
      28,    12,     0,    13,   146,    29,    30,     0,     0,    31,
      32,    33,     0,     0,    69,     0,     0,     0,    35,    36,
      37,     0,     0,     0,     0,     0,     0,    12,     0,    13,
     149,     0,     0,    14,    15,    16,     0,     0,     0,     0,
       0,     0,     0,    18,    19,     0,    20,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,    16,     0,     0,     0,     0,     0,     0,     0,    18,
      19,     0,    20,     0,    21,     0,    22,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    29,    30,     0,     0,    31,    32,
      33,     0,    22,    34,     0,     0,     0,    35,    36,    37,
      23,    24,    25,    26,    27,    28,    12,     0,    13,   155,
      29,    30,     0,     0,    31,    32,    33,     0,     0,    34,
       0,     0,     0,    35,    36,    37,     0,     0,     0,     0,
       0,     0,    68,     0,     0,     0,     0,     0,    14,    15,
      16,     0,   -32,     0,     0,     0,     0,     0,    18,    19,
       0,    20,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   -32,    14,    15,    16,     0,     0,     0,
       0,     0,     0,     0,    18,    19,     0,    20,     0,    21,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,     0,     0,     0,    29,
      30,     0,     0,    31,    32,    33,     0,    22,    34,     0,
       0,    68,    35,    36,    37,    23,    24,    25,    26,    27,
      28,   258,     0,     0,     0,    29,    30,     0,     0,    31,
      32,    33,     0,     0,    69,     0,     0,     0,    35,    36,
      37,     0,   259,    14,    15,    16,    12,     0,    13,     0,
       0,     0,     0,    18,    19,     0,    20,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
      16,     0,     0,     0,     0,     0,    22,     0,    18,    19,
       0,    20,     0,    21,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    29,    30,     0,     0,    31,    32,
      33,     0,     0,    69,     0,     0,     0,    35,    36,    37,
       0,    22,     0,    68,     0,    13,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,     0,     0,     0,    29,
      30,     0,     0,    31,    32,    33,     0,     0,    34,   180,
       0,    13,    35,    36,    37,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,    18,    19,     0,    20,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,     0,     0,
       0,    18,    19,     0,    20,     0,    21,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    23,    24,    25,    26,
      27,    28,     0,     0,     0,     0,    29,    30,     0,     0,
      31,    32,    33,     0,    22,    34,    68,     0,     0,    35,
      36,    37,    23,    24,    25,    26,    27,    28,     0,     0,
       0,     0,    29,    30,     0,     0,    31,    32,    33,     0,
       0,    34,   295,     0,    13,    35,    36,    37,    14,    15,
      16,     0,     0,     0,     0,     0,     0,     0,    18,    19,
       0,    20,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,    16,   210,     0,     0,
       0,     0,     0,     0,    18,    19,     0,    20,     0,    21,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    27,    28,     0,     0,     0,     0,    29,
      30,     0,     0,    31,    32,    33,     0,    22,    69,    68,
       0,     0,    35,    36,    37,    23,    24,    25,    26,    27,
      28,     0,     0,     0,     0,    29,    30,     0,     0,    31,
      32,    33,     0,     0,    34,     0,     0,     0,    35,    36,
      37,    14,    15,    16,     0,     0,     0,     0,     0,     0,
       0,    18,    19,     0,    20,     0,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,     0,     0,
       0,     0,    29,    30,     0,     0,    31,    32,    33,     0,
       0,    69,     0,     0,     0,    35,    36,    37
};

static const yytype_int16 yycheck[] =
{
       5,   170,     9,   181,    81,    84,     9,    12,    17,    73,
       5,     9,   180,   104,     9,    10,    58,   107,   336,    73,
      10,    10,    64,    65,    45,    52,    53,   104,    18,    18,
      83,   121,    14,    86,    43,    11,    89,   116,    45,    92,
     358,    10,    45,    10,    39,    10,    15,   100,    15,     0,
      15,    10,    57,    58,   118,   119,    15,    62,    63,    64,
      65,    66,    67,    68,   118,   119,    95,    12,    10,    98,
      97,    34,   163,    15,    95,   249,    97,   251,   105,   181,
      10,    86,    10,    11,    10,    15,   163,    92,    95,    15,
      97,   170,    95,    96,    97,   137,     9,    95,    10,    10,
      13,   106,   107,    15,    15,   110,   111,   112,   113,   114,
     115,   106,   117,   118,   119,   168,   121,   120,   296,    12,
     173,   299,   300,   301,   302,   303,    12,   295,    12,   134,
     112,   113,   137,   118,   119,   117,    99,   142,   258,   144,
     260,   261,   262,    12,   264,   265,   236,   237,   322,   228,
      12,    12,    12,    45,    31,    32,    33,    34,    35,    36,
      37,   240,    12,   100,   342,    42,   171,   336,   258,     9,
     175,    17,   101,    13,     9,   180,   350,    54,    13,     9,
     259,    12,   117,    13,     9,    12,   191,    43,   362,   358,
      12,    12,    69,     9,   296,   286,    14,   299,   300,   301,
     302,   303,   244,    95,    96,    97,   326,    95,   287,   286,
       9,    17,    88,    10,     9,    13,     9,   107,    12,    15,
      43,    98,    99,    15,    15,    15,   316,   232,   120,   320,
      15,   236,   237,    15,    41,   191,   241,    -1,   106,   244,
     342,    14,    -1,   320,    -1,   324,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,    14,    -1,    -1,
     351,    -1,    -1,    -1,   269,   270,   271,    -1,    14,    -1,
      -1,   148,   267,    -1,   351,   152,   153,   304,    -1,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     295,   168,    -1,    66,    67,   258,   173,   260,   261,   262,
      -1,   264,   265,    -1,   181,   332,    -1,   360,    -1,    66,
      67,   316,    69,   318,    -1,   310,   321,   194,    -1,   196,
     197,   198,   199,   200,   201,    -1,   203,    -1,   205,    -1,
     207,   208,    -1,   210,   339,    -1,    -1,   110,   111,   112,
     113,    -1,    -1,    -1,   117,   222,    -1,    54,    55,    -1,
     227,    -1,   229,   110,   111,   112,   113,    -1,    -1,    -1,
     117,    -1,    -1,   326,   110,   111,   112,   113,    -1,    -1,
      -1,   117,   249,    -1,   251,    -1,    -1,    -1,    -1,    -1,
      -1,   258,    -1,   260,   261,   262,    -1,   264,   265,    -1,
      -1,    -1,    -1,    -1,    31,    32,    33,    34,   275,    -1,
      37,    -1,   279,    -1,   281,    42,    -1,   284,    -1,    -1,
      -1,    14,    -1,    -1,    -1,    -1,    -1,    54,    -1,   296,
      -1,    -1,   299,   300,   301,   302,   303,    -1,   305,   306,
     307,    -1,    69,    -1,    -1,    -1,    96,    -1,    -1,    -1,
      -1,    -1,   102,    -1,    -1,   152,   153,    -1,    -1,   326,
     157,   158,   329,    -1,   331,    -1,   333,    -1,   335,    -1,
     337,    98,    99,    66,    67,   342,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   353,    -1,   138,   139,
      -1,    -1,    -1,    -1,    -1,    14,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,   208,   209,    -1,    -1,    -1,   109,   110,   111,   112,
     113,   148,    -1,    -1,   117,   152,   153,   177,    -1,   179,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,   235,    -1,
     190,   168,   192,    -1,    -1,    -1,   173,    66,    67,    -1,
      69,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     257,    -1,    -1,    -1,    -1,    -1,    -1,   194,    -1,   196,
     197,   198,   199,   200,   201,    -1,   203,    -1,   205,    -1,
     207,   208,    -1,   210,    -1,    -1,   283,   284,    -1,    -1,
     109,   110,   111,   112,   113,   222,    -1,    -1,   117,    -1,
     227,    -1,   229,    -1,   254,    -1,    -1,    -1,    -1,   306,
     307,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   249,    -1,   251,    -1,    -1,    -1,   325,    -1,
      -1,   258,    -1,   260,   261,   262,    -1,   264,   265,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   275,    -1,
      -1,    -1,   279,    -1,   281,    -1,    -1,   284,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   296,
      -1,    -1,   299,   300,   301,   302,   303,   327,   305,   306,
     307,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   346,    -1,    -1,   326,
      -1,    -1,   352,    -1,    -1,    -1,   333,    -1,   335,    -1,
     337,   361,    -1,    -1,    -1,   342,   366,    -1,    -1,    -1,
       0,     1,    -1,    -1,    -1,    -1,   353,     7,     8,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    55,    -1,    57,    -1,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    87,    11,    12,
      13,    14,    -1,    -1,    -1,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,
     110,   111,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,
     120,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    55,    56,    57,    -1,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    86,    87,    11,    12,    13,    14,    -1,
      16,    -1,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,   111,    -1,
      -1,   114,    -1,    -1,    -1,   118,   119,   120,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    55,
      56,    57,    -1,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    85,
      86,    87,    11,    12,    13,    14,    -1,    16,    -1,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,   104,   105,
     106,   107,    -1,   109,   110,   111,    -1,    -1,   114,    -1,
      -1,    -1,   118,   119,   120,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    55,    56,    57,    -1,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,
      -1,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,    11,
      12,    13,    14,    -1,    16,    -1,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,   111,    -1,    -1,   114,    -1,    -1,    -1,   118,
     119,   120,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    55,    56,    57,    -1,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    85,    86,    87,    11,    12,    13,    14,
      -1,    16,    -1,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,   111,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      55,    56,    57,    -1,    59,    60,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      85,    86,    87,    11,    12,    13,    14,    -1,    16,    -1,
      95,    96,    97,    98,    99,   100,    -1,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,   111,    -1,    -1,   114,
      -1,    -1,    -1,   118,   119,   120,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    55,    56,    57,
      -1,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    85,    86,    87,
       9,    -1,    11,    12,    13,    14,    -1,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,   111,    -1,    -1,   114,    -1,    -1,    -1,
     118,   119,   120,    -1,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    55,    56,    57,    -1,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,
      -1,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,    11,
      12,    13,    14,    -1,    16,    -1,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,   111,    -1,    -1,   114,    -1,    -1,    -1,   118,
     119,   120,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    55,    56,    57,    -1,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    85,    86,    87,    11,    12,    13,    14,
      -1,    16,    -1,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,   111,
      -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      55,    56,    57,    -1,    59,    60,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      85,    86,    87,    11,    12,    13,    14,    -1,    -1,    -1,
      95,    96,    97,    98,    99,   100,    -1,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,   111,    -1,    -1,   114,
      -1,    -1,    -1,   118,   119,   120,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    55,    56,    57,
      -1,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,
      -1,    12,    13,    14,    -1,    -1,    -1,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,   111,    -1,    -1,   114,    -1,    -1,    -1,
     118,   119,   120,    44,    45,    46,    -1,    -1,    49,    -1,
      -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    85,    86,    87,    -1,    12,    -1,
      14,    15,    -1,    -1,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,
      44,    45,    46,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      54,    55,    -1,    57,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    85,    86,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    95,    96,    97,    98,    99,   100,    -1,    -1,    -1,
      -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,    -1,
     114,    -1,    -1,    -1,   118,   119,   120,     7,     8,    -1,
      -1,    11,    12,    -1,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     9,    -1,    -1,    12,    13,
      -1,    -1,    -1,    -1,    44,    45,    46,    -1,    22,    -1,
      -1,    -1,    -1,    53,    54,    55,    -1,    57,    -1,    59,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    55,    -1,    57,    -1,    59,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,
     110,   111,     9,    87,   114,    12,    13,    14,   118,   119,
     120,    95,    96,    97,    98,    99,   100,    -1,   102,    -1,
      -1,   105,   106,    -1,    -1,   109,   110,   111,     9,    -1,
     114,    12,    -1,    14,   118,   119,   120,    44,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    55,    -1,
      57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,    -1,   105,   106,
      -1,    -1,   109,   110,   111,     9,    87,   114,    12,    -1,
      14,   118,   119,   120,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,   110,
     111,     9,    -1,   114,    12,    -1,    14,   118,   119,   120,
      44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    55,    -1,    57,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    55,    -1,    57,
      -1,    59,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    95,    96,    97,    98,    99,   100,    -1,    -1,    -1,
      -1,   105,   106,    -1,    -1,   109,   110,   111,     9,    87,
     114,    12,    -1,    14,   118,   119,   120,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,    -1,   105,   106,    -1,
      -1,   109,   110,   111,     9,    -1,   114,    12,    -1,    14,
     118,   119,   120,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      55,    -1,    57,    -1,    59,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,   110,
     111,     9,    87,   114,    12,    -1,    14,   118,   119,   120,
      95,    96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,
     105,   106,    -1,    -1,   109,   110,   111,    -1,    -1,   114,
      -1,    -1,    -1,   118,   119,   120,    44,    45,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    55,    -1,    57,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,    -1,   105,   106,    -1,
      -1,   109,   110,   111,    -1,    -1,   114,    -1,    -1,    -1,
     118,   119,   120,    10,    -1,    12,    -1,    -1,    15,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    55,    -1,
      57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,
      97,    98,    99,   100,    -1,   102,    -1,    -1,   105,   106,
      -1,    -1,   109,   110,   111,    -1,    -1,   114,    -1,    -1,
      -1,   118,   119,   120,    10,    -1,    12,    -1,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    55,
      -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      96,    97,    98,    99,   100,    -1,   102,    -1,    -1,   105,
     106,    -1,    -1,   109,   110,   111,    -1,    -1,   114,    -1,
      -1,    -1,   118,   119,   120,    10,    -1,    12,    -1,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      55,    -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      95,    96,    97,    98,    99,   100,    -1,   102,    -1,    -1,
     105,   106,    -1,    -1,   109,   110,   111,    -1,    -1,   114,
      -1,    -1,    -1,   118,   119,   120,    10,    -1,    12,    -1,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    55,    -1,    57,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    95,    96,    97,    98,    99,   100,    10,   102,    12,
      -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,    22,
     114,    -1,    -1,    -1,   118,   119,   120,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,    -1,   102,
      -1,    -1,   105,   106,    -1,    -1,   109,   110,   111,    12,
      -1,   114,    15,    -1,    -1,   118,   119,   120,    -1,    22,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,    -1,   102,
      12,    -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,
      22,   114,    -1,    -1,    -1,   118,   119,   120,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,   106,    -1,    -1,   109,   110,   111,
      12,    -1,   114,    15,    -1,    -1,   118,   119,   120,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,    -1,    -1,   105,   106,    -1,    -1,   109,   110,   111,
      12,    -1,   114,    15,    -1,    -1,   118,   119,   120,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,    -1,    -1,   105,   106,    -1,    -1,   109,   110,   111,
      12,    -1,   114,    15,    -1,    -1,   118,   119,   120,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,    -1,    -1,   105,   106,    -1,    -1,   109,   110,   111,
      12,    -1,   114,    15,    -1,    -1,   118,   119,   120,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,    -1,    -1,   105,   106,    -1,    -1,   109,   110,   111,
      12,    -1,   114,    15,    -1,    -1,   118,   119,   120,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,    12,    13,   105,   106,    -1,    -1,   109,   110,   111,
      -1,    22,   114,    -1,    -1,    -1,   118,   119,   120,    -1,
      -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,   100,
      -1,   102,    -1,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    12,    -1,   114,    15,    -1,    -1,   118,   119,   120,
      -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,   100,
      -1,   102,    12,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    -1,    22,   114,    -1,    -1,    -1,   118,   119,   120,
      -1,    -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,    12,    -1,   105,   106,    -1,    -1,   109,
     110,   111,    -1,    22,   114,    -1,    -1,    -1,   118,   119,
     120,    -1,    -1,    -1,    -1,    -1,    35,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    55,    -1,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,    -1,    12,    -1,    -1,   105,   106,    -1,    -1,
     109,   110,   111,    22,    -1,   114,    -1,    -1,    -1,   118,
     119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    55,    -1,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,    -1,    12,    -1,    -1,   105,   106,    -1,    -1,
     109,   110,   111,    22,    -1,   114,    -1,    -1,    -1,   118,
     119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    55,    -1,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    35,    36,    95,    96,    97,    98,
      99,   100,    43,    44,    45,    46,   105,   106,    -1,    -1,
     109,   110,   111,    54,    55,   114,    57,    -1,    59,   118,
     119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,   106,    12,    -1,   109,   110,
     111,    -1,    -1,   114,    -1,    -1,    22,   118,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,
      36,    -1,    12,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    -1,    22,    -1,    -1,    -1,    -1,    -1,    54,    55,
      -1,    57,    -1,    59,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      96,    97,    98,    99,   100,    -1,   102,    -1,    -1,   105,
     106,    -1,    -1,   109,   110,   111,    -1,    87,   114,    -1,
      -1,    -1,   118,   119,   120,    95,    96,    97,    98,    99,
     100,    12,    -1,    14,    15,   105,   106,    -1,    -1,   109,
     110,   111,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    12,    -1,    14,
      15,    -1,    -1,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      55,    -1,    57,    -1,    59,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    -1,    87,   114,    -1,    -1,    -1,   118,   119,   120,
      95,    96,    97,    98,    99,   100,    12,    -1,    14,    15,
     105,   106,    -1,    -1,   109,   110,   111,    -1,    -1,   114,
      -1,    -1,    -1,   118,   119,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    44,    45,
      46,    -1,    22,    -1,    -1,    -1,    -1,    -1,    54,    55,
      -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,   105,
     106,    -1,    -1,   109,   110,   111,    -1,    87,   114,    -1,
      -1,    12,   118,   119,   120,    95,    96,    97,    98,    99,
     100,    22,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,
     110,   111,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,
     120,    -1,    43,    44,    45,    46,    12,    -1,    14,    -1,
      -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      46,    -1,    -1,    -1,    -1,    -1,    87,    -1,    54,    55,
      -1,    57,    -1,    59,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,   120,
      -1,    87,    -1,    12,    -1,    14,    -1,    -1,    -1,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,   105,
     106,    -1,    -1,   109,   110,   111,    -1,    -1,   114,    12,
      -1,    14,   118,   119,   120,    44,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    55,    -1,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    55,    -1,    57,    -1,    59,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,    -1,   105,   106,    -1,    -1,
     109,   110,   111,    -1,    87,   114,    12,    -1,    -1,   118,
     119,   120,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,    -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,
      -1,   114,    12,    -1,    14,   118,   119,   120,    44,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    55,
      -1,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    46,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    -1,    57,    -1,    59,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,   105,
     106,    -1,    -1,   109,   110,   111,    -1,    87,   114,    12,
      -1,    -1,   118,   119,   120,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,    -1,   105,   106,    -1,    -1,   109,
     110,   111,    -1,    -1,   114,    -1,    -1,    -1,   118,   119,
     120,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    55,    -1,    57,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,    -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,
      -1,   114,    -1,    -1,    -1,   118,   119,   120
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     9,    13,   123,   138,   140,   152,     0,     7,
       8,    11,    12,    14,    44,    45,    46,    53,    54,    55,
      57,    59,    87,    95,    96,    97,    98,    99,   100,   105,
     106,   109,   110,   111,   114,   118,   119,   120,   135,   141,
     142,   144,   147,   154,   155,   165,   166,   167,   168,   170,
       9,    13,   135,   135,   147,   148,   156,    12,    12,    95,
      98,   133,    12,    12,    12,    12,    12,    12,    12,   114,
     167,   168,   167,   167,   154,   167,   168,   168,   167,     1,
       9,    13,    47,    48,    49,    50,    51,    52,    56,    60,
      85,    86,   104,   107,   127,   129,   134,   135,   147,   151,
     158,   160,   164,   171,    10,   135,   138,    22,    35,    36,
      37,    38,    39,    40,    41,    42,    43,   102,   124,   125,
     167,    12,   100,    14,    66,    67,    69,    70,   109,   110,
     111,   112,   113,   117,    73,   118,   119,    17,   164,   164,
      10,    15,   126,    15,   126,   101,    15,   145,   147,    15,
     145,    12,   147,   147,   145,    15,   145,   147,   147,   147,
       9,   136,   137,    13,   136,   159,   159,   170,   147,   159,
      12,    12,   159,   147,   159,    12,     9,   161,   160,   164,
      12,   146,   149,   150,   154,   167,   168,   159,    16,   160,
     163,   137,   164,   142,   147,   155,   147,   147,   147,   147,
     147,   147,   170,   147,     9,   147,     9,   147,   147,   155,
      73,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   147,   145,    16,    16,     9,   147,    43,   147,
      14,    15,   126,    15,    95,   169,   126,   126,    15,    15,
     126,   126,     9,   137,    17,   159,   139,   158,   170,   147,
     159,   147,   160,    88,   128,    16,   153,   148,    22,    43,
     102,   124,   125,   167,   126,    73,   160,   141,    16,   103,
     126,   126,   167,    18,   170,   147,    15,   126,   157,   147,
     155,   147,   155,   170,   147,   145,    13,    43,   157,   157,
     162,     9,   160,     9,    15,    12,   146,   155,   170,   146,
     146,   146,   146,   146,   135,   147,   147,   147,    95,     9,
     143,    15,    15,    15,    15,    15,   126,    15,   126,    18,
      13,   137,   170,   107,    43,   148,   103,   164,    15,   126,
      15,   126,   135,   147,   155,   147,   137,   147,   157,    12,
     170,    15,   146,    16,   168,   168,   164,    15,    15,    15,
     139,    13,   132,   147,    15,    15,    16,   157,   137,   160,
      15,   131,   139,   159,   160,   157,   130,   160
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   122,   123,   123,   124,   124,   125,   125,   126,   126,
     127,   127,   128,   128,   130,   129,   131,   129,   132,   129,
     133,   133,   134,   135,   135,   136,   136,   137,   137,   138,
     138,   139,   139,   140,   140,   141,   142,   142,   142,   142,
     142,   142,   142,   143,   142,   144,   144,   145,   145,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   148,   148,   149,
     149,   150,   150,   150,   151,   151,   152,   152,   152,   152,
     153,   153,   154,   154,   156,   155,   157,   157,   158,   158,
     158,   158,   158,   159,   159,   160,   160,   161,   162,   160,
     160,   160,   160,   160,   160,   160,   160,   160,   160,   160,
     163,   160,   160,   164,   164,   165,   165,   166,   166,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   168,   168,   168,   168,   169,
     169,   169,   170,   170,   170,   171
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
       1,     3,     4,     2,     2,     2,     2,     6,     6,     3,
       6,     6,     1,     8,     8,     6,     4,     1,     6,     6,
       8,     8,     8,     6,     1,     1,     4,     1,     2,     0,
       1,     3,     1,     1,     1,     4
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
				winner = (TNode *)stat3(PROGRAM, beginloc, (yyvsp[0].p), endloc);
			else
				runnerup = (TNode *)stat3(PROGRAM, (yyvsp[0].p), NIL, NIL);
		}
	  }
#line 2412 "awkgram.tab.c"
    break;

  case 3: /* program: error  */
#line 113 "awkgram.y"
                { yyclearin; bracecheck(); SYNTAX("bailing out"); }
#line 2418 "awkgram.tab.c"
    break;

  case 14: /* $@1: %empty  */
#line 137 "awkgram.y"
                                                                                       {inloop++;}
#line 2424 "awkgram.tab.c"
    break;

  case 15: /* for: FOR '(' opt_simple_stmt ';' opt_nl pattern ';' opt_nl opt_simple_stmt rparen $@1 stmt  */
#line 138 "awkgram.y"
                { --inloop; (yyval.p) = stat4(FOR, (yyvsp[-9].p), notnull((yyvsp[-6].p)), (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2430 "awkgram.tab.c"
    break;

  case 16: /* $@2: %empty  */
#line 139 "awkgram.y"
                                                                         {inloop++;}
#line 2436 "awkgram.tab.c"
    break;

  case 17: /* for: FOR '(' opt_simple_stmt ';' ';' opt_nl opt_simple_stmt rparen $@2 stmt  */
#line 140 "awkgram.y"
                { --inloop; (yyval.p) = stat4(FOR, (yyvsp[-7].p), NIL, (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2442 "awkgram.tab.c"
    break;

  case 18: /* $@3: %empty  */
#line 141 "awkgram.y"
                                            {inloop++;}
#line 2448 "awkgram.tab.c"
    break;

  case 19: /* for: FOR '(' varname IN varname rparen $@3 stmt  */
#line 142 "awkgram.y"
                { --inloop; (yyval.p) = stat3(IN, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (yyvsp[0].p)); }
#line 2454 "awkgram.tab.c"
    break;

  case 20: /* funcname: VAR  */
#line 146 "awkgram.y"
                { setfname((yyvsp[0].cp)); }
#line 2460 "awkgram.tab.c"
    break;

  case 21: /* funcname: CALL  */
#line 147 "awkgram.y"
                { setfname((yyvsp[0].cp)); }
#line 2466 "awkgram.tab.c"
    break;

  case 22: /* if: IF '(' pattern rparen  */
#line 151 "awkgram.y"
                                        { (yyval.p) = notnull((yyvsp[-1].p)); }
#line 2472 "awkgram.tab.c"
    break;

  case 27: /* opt_nl: %empty  */
#line 163 "awkgram.y"
                        { (yyval.i) = 0; }
#line 2478 "awkgram.tab.c"
    break;

  case 29: /* opt_pst: %empty  */
#line 168 "awkgram.y"
                        { (yyval.i) = 0; }
#line 2484 "awkgram.tab.c"
    break;

  case 31: /* opt_simple_stmt: %empty  */
#line 174 "awkgram.y"
                                        { (yyval.p) = 0; }
#line 2490 "awkgram.tab.c"
    break;

  case 33: /* pas: opt_pst  */
#line 179 "awkgram.y"
                                        { (yyval.p) = 0; }
#line 2496 "awkgram.tab.c"
    break;

  case 34: /* pas: opt_pst pa_stats opt_pst  */
#line 180 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2502 "awkgram.tab.c"
    break;

  case 35: /* pa_pat: pattern  */
#line 184 "awkgram.y"
                        { (yyval.p) = notnull((yyvsp[0].p)); }
#line 2508 "awkgram.tab.c"
    break;

  case 36: /* pa_stat: pa_pat  */
#line 188 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, (yyvsp[0].p), stat2(PRINT, rectonode(), NIL)); }
#line 2514 "awkgram.tab.c"
    break;

  case 37: /* pa_stat: pa_pat lbrace stmtlist '}'  */
#line 189 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2520 "awkgram.tab.c"
    break;

  case 38: /* pa_stat: pa_pat ',' opt_nl pa_pat  */
#line 190 "awkgram.y"
                                                { (yyval.p) = pa2stat((yyvsp[-3].p), (yyvsp[0].p), stat2(PRINT, rectonode(), NIL)); }
#line 2526 "awkgram.tab.c"
    break;

  case 39: /* pa_stat: pa_pat ',' opt_nl pa_pat lbrace stmtlist '}'  */
#line 191 "awkgram.y"
                                                        { (yyval.p) = pa2stat((yyvsp[-6].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2532 "awkgram.tab.c"
    break;

  case 40: /* pa_stat: lbrace stmtlist '}'  */
#line 192 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, NIL, (yyvsp[-1].p)); }
#line 2538 "awkgram.tab.c"
    break;

  case 41: /* pa_stat: XBEGIN lbrace stmtlist '}'  */
#line 194 "awkgram.y"
                { beginloc = linkum(beginloc, (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2544 "awkgram.tab.c"
    break;

  case 42: /* pa_stat: XEND lbrace stmtlist '}'  */
#line 196 "awkgram.y"
                { endloc = linkum(endloc, (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2550 "awkgram.tab.c"
    break;

  case 43: /* $@4: %empty  */
#line 197 "awkgram.y"
                                           {infunc = true;}
#line 2556 "awkgram.tab.c"
    break;

  case 44: /* pa_stat: FUNC funcname '(' varlist rparen $@4 lbrace stmtlist '}'  */
#line 198 "awkgram.y"
                { infunc = false; curfname=0; defn((Cell *)(yyvsp[-7].p), (yyvsp[-5].p), (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2562 "awkgram.tab.c"
    break;

  case 46: /* pa_stats: pa_stats opt_pst pa_stat  */
#line 203 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2568 "awkgram.tab.c"
    break;

  case 48: /* patlist: patlist comma pattern  */
#line 208 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2574 "awkgram.tab.c"
    break;

  case 49: /* ppattern: var ASGNOP ppattern  */
#line 212 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2580 "awkgram.tab.c"
    break;

  case 50: /* ppattern: ppattern '?' ppattern ':' ppattern  */
#line 214 "awkgram.y"
                { (yyval.p) = op3(CONDEXPR, notnull((yyvsp[-4].p)), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2586 "awkgram.tab.c"
    break;

  case 51: /* ppattern: ppattern lor ppattern  */
#line 216 "awkgram.y"
                { (yyval.p) = op2(LOR, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2592 "awkgram.tab.c"
    break;

  case 52: /* ppattern: ppattern land ppattern  */
#line 218 "awkgram.y"
                { (yyval.p) = op2(LAND, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2598 "awkgram.tab.c"
    break;

  case 53: /* ppattern: ppattern MATCHOP reg_expr  */
#line 219 "awkgram.y"
                                        { (yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (TNode*)makedfa((yyvsp[0].s), 0)); FREE((yyvsp[0].s)); }
#line 2604 "awkgram.tab.c"
    break;

  case 54: /* ppattern: ppattern MATCHOP ppattern  */
#line 221 "awkgram.y"
                { if (constnode((yyvsp[0].p)))
			(yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (TNode*)makedfa(strnode((yyvsp[0].p)), 0));
		  else
			(yyval.p) = op3((yyvsp[-1].i), (TNode *)1, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2613 "awkgram.tab.c"
    break;

  case 55: /* ppattern: ppattern IN varname  */
#line 225 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-2].p), makearr((yyvsp[0].p))); }
#line 2619 "awkgram.tab.c"
    break;

  case 56: /* ppattern: '(' plist ')' IN varname  */
#line 226 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-3].p), makearr((yyvsp[0].p))); }
#line 2625 "awkgram.tab.c"
    break;

  case 57: /* ppattern: ppattern term  */
#line 227 "awkgram.y"
                                        { (yyval.p) = op2(CAT, (yyvsp[-1].p), (yyvsp[0].p)); }
#line 2631 "awkgram.tab.c"
    break;

  case 60: /* pattern: var ASGNOP pattern  */
#line 233 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2637 "awkgram.tab.c"
    break;

  case 61: /* pattern: pattern '?' pattern ':' pattern  */
#line 235 "awkgram.y"
                { (yyval.p) = op3(CONDEXPR, notnull((yyvsp[-4].p)), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2643 "awkgram.tab.c"
    break;

  case 62: /* pattern: pattern lor pattern  */
#line 237 "awkgram.y"
                { (yyval.p) = op2(LOR, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2649 "awkgram.tab.c"
    break;

  case 63: /* pattern: pattern land pattern  */
#line 239 "awkgram.y"
                { (yyval.p) = op2(LAND, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2655 "awkgram.tab.c"
    break;

  case 64: /* pattern: pattern EQ pattern  */
#line 240 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2661 "awkgram.tab.c"
    break;

  case 65: /* pattern: pattern GE pattern  */
#line 241 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2667 "awkgram.tab.c"
    break;

  case 66: /* pattern: pattern GT pattern  */
#line 242 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2673 "awkgram.tab.c"
    break;

  case 67: /* pattern: pattern LE pattern  */
#line 243 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2679 "awkgram.tab.c"
    break;

  case 68: /* pattern: pattern LT pattern  */
#line 244 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2685 "awkgram.tab.c"
    break;

  case 69: /* pattern: pattern NE pattern  */
#line 245 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2691 "awkgram.tab.c"
    break;

  case 70: /* pattern: pattern MATCHOP reg_expr  */
#line 246 "awkgram.y"
                                        { (yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (TNode*)makedfa((yyvsp[0].s), 0)); FREE((yyvsp[0].s)); }
#line 2697 "awkgram.tab.c"
    break;

  case 71: /* pattern: pattern MATCHOP pattern  */
#line 248 "awkgram.y"
                { if (constnode((yyvsp[0].p)))
			(yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (TNode*)makedfa(strnode((yyvsp[0].p)), 0));
		  else
			(yyval.p) = op3((yyvsp[-1].i), (TNode *)1, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2706 "awkgram.tab.c"
    break;

  case 72: /* pattern: pattern IN varname  */
#line 252 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-2].p), makearr((yyvsp[0].p))); }
#line 2712 "awkgram.tab.c"
    break;

  case 73: /* pattern: '(' plist ')' IN varname  */
#line 253 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-3].p), makearr((yyvsp[0].p))); }
#line 2718 "awkgram.tab.c"
    break;

  case 74: /* pattern: pattern term  */
#line 254 "awkgram.y"
                                        { (yyval.p) = op2(CAT, (yyvsp[-1].p), (yyvsp[0].p)); }
#line 2724 "awkgram.tab.c"
    break;

  case 77: /* plist: pattern comma pattern  */
#line 260 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2730 "awkgram.tab.c"
    break;

  case 78: /* plist: plist comma pattern  */
#line 261 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2736 "awkgram.tab.c"
    break;

  case 80: /* pplist: pplist comma ppattern  */
#line 266 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2742 "awkgram.tab.c"
    break;

  case 81: /* prarg: %empty  */
#line 270 "awkgram.y"
                                        { (yyval.p) = rectonode(); }
#line 2748 "awkgram.tab.c"
    break;

  case 83: /* prarg: '(' plist ')'  */
#line 272 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2754 "awkgram.tab.c"
    break;

  case 92: /* re: reg_expr  */
#line 289 "awkgram.y"
                { (yyval.p) = op3(MATCH, NIL, rectonode(), (TNode*)makedfa((yyvsp[0].s), 0)); FREE((yyvsp[0].s)); }
#line 2760 "awkgram.tab.c"
    break;

  case 93: /* re: NOT re  */
#line 290 "awkgram.y"
                        { (yyval.p) = op1(NOT, notnull((yyvsp[0].p))); }
#line 2766 "awkgram.tab.c"
    break;

  case 94: /* $@5: %empty  */
#line 294 "awkgram.y"
              {startreg();}
#line 2772 "awkgram.tab.c"
    break;

  case 95: /* reg_expr: '/' $@5 REGEXPR '/'  */
#line 294 "awkgram.y"
                                                { (yyval.s) = (yyvsp[-1].s); }
#line 2778 "awkgram.tab.c"
    break;

  case 98: /* simple_stmt: print prarg  */
#line 302 "awkgram.y"
                                        { (yyval.p) = stat3((yyvsp[-1].i), (yyvsp[0].p), NIL, NIL); }
#line 2784 "awkgram.tab.c"
    break;

  case 99: /* simple_stmt: DELETE varname '[' patlist ']'  */
#line 303 "awkgram.y"
                                         { (yyval.p) = stat2(DELETE, makearr((yyvsp[-3].p)), (yyvsp[-1].p)); }
#line 2790 "awkgram.tab.c"
    break;

  case 100: /* simple_stmt: DELETE varname  */
#line 304 "awkgram.y"
                                         { (yyval.p) = stat2(DELETE, makearr((yyvsp[0].p)), 0); }
#line 2796 "awkgram.tab.c"
    break;

  case 101: /* simple_stmt: pattern  */
#line 305 "awkgram.y"
                                        { (yyval.p) = exptostat((yyvsp[0].p)); }
#line 2802 "awkgram.tab.c"
    break;

  case 102: /* simple_stmt: error  */
#line 306 "awkgram.y"
                                        { yyclearin; SYNTAX("illegal statement"); }
#line 2808 "awkgram.tab.c"
    break;

  case 105: /* stmt: BREAK st  */
#line 315 "awkgram.y"
                                { if (!inloop) SYNTAX("break illegal outside of loops");
				  (yyval.p) = stat1(BREAK, NIL); }
#line 2815 "awkgram.tab.c"
    break;

  case 106: /* stmt: CONTINUE st  */
#line 317 "awkgram.y"
                                {  if (!inloop) SYNTAX("continue illegal outside of loops");
				  (yyval.p) = stat1(CONTINUE, NIL); }
#line 2822 "awkgram.tab.c"
    break;

  case 107: /* $@6: %empty  */
#line 319 "awkgram.y"
             {inloop++;}
#line 2828 "awkgram.tab.c"
    break;

  case 108: /* $@7: %empty  */
#line 319 "awkgram.y"
                              {--inloop;}
#line 2834 "awkgram.tab.c"
    break;

  case 109: /* stmt: do $@6 stmt $@7 WHILE '(' pattern ')' st  */
#line 320 "awkgram.y"
                { (yyval.p) = stat2(DO, (yyvsp[-6].p), notnull((yyvsp[-2].p))); }
#line 2840 "awkgram.tab.c"
    break;

  case 110: /* stmt: EXIT pattern st  */
#line 321 "awkgram.y"
                                { (yyval.p) = stat1(EXIT, (yyvsp[-1].p)); }
#line 2846 "awkgram.tab.c"
    break;

  case 111: /* stmt: EXIT st  */
#line 322 "awkgram.y"
                                { (yyval.p) = stat1(EXIT, NIL); }
#line 2852 "awkgram.tab.c"
    break;

  case 113: /* stmt: if stmt else stmt  */
#line 324 "awkgram.y"
                                { (yyval.p) = stat3(IF, (yyvsp[-3].p), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2858 "awkgram.tab.c"
    break;

  case 114: /* stmt: if stmt  */
#line 325 "awkgram.y"
                                { (yyval.p) = stat3(IF, (yyvsp[-1].p), (yyvsp[0].p), NIL); }
#line 2864 "awkgram.tab.c"
    break;

  case 115: /* stmt: lbrace stmtlist rbrace  */
#line 326 "awkgram.y"
                                 { (yyval.p) = (yyvsp[-1].p); }
#line 2870 "awkgram.tab.c"
    break;

  case 116: /* stmt: NEXT st  */
#line 327 "awkgram.y"
                        { if (infunc)
				SYNTAX("next is illegal inside a function");
			  (yyval.p) = stat1(NEXT, NIL); }
#line 2878 "awkgram.tab.c"
    break;

  case 117: /* stmt: RETURN pattern st  */
#line 330 "awkgram.y"
                                { (yyval.p) = stat1(RETURN, (yyvsp[-1].p)); }
#line 2884 "awkgram.tab.c"
    break;

  case 118: /* stmt: RETURN st  */
#line 331 "awkgram.y"
                                { (yyval.p) = stat1(RETURN, NIL); }
#line 2890 "awkgram.tab.c"
    break;

  case 120: /* $@8: %empty  */
#line 333 "awkgram.y"
                {inloop++;}
#line 2896 "awkgram.tab.c"
    break;

  case 121: /* stmt: while $@8 stmt  */
#line 333 "awkgram.y"
                                        { --inloop; (yyval.p) = stat2(WHILE, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2902 "awkgram.tab.c"
    break;

  case 122: /* stmt: ';' opt_nl  */
#line 334 "awkgram.y"
                                { (yyval.p) = 0; }
#line 2908 "awkgram.tab.c"
    break;

  case 124: /* stmtlist: stmtlist stmt  */
#line 339 "awkgram.y"
                                { (yyval.p) = linkum((yyvsp[-1].p), (yyvsp[0].p)); }
#line 2914 "awkgram.tab.c"
    break;

  case 128: /* string: string STRING  */
#line 348 "awkgram.y"
                                { (yyval.cp) = catstr((yyvsp[-1].cp), (yyvsp[0].cp)); }
#line 2920 "awkgram.tab.c"
    break;

  case 129: /* term: term '/' ASGNOP term  */
#line 352 "awkgram.y"
                                        { (yyval.p) = op2(DIVEQ, (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2926 "awkgram.tab.c"
    break;

  case 130: /* term: term '+' term  */
#line 353 "awkgram.y"
                                        { (yyval.p) = op2(ADD, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2932 "awkgram.tab.c"
    break;

  case 131: /* term: term '-' term  */
#line 354 "awkgram.y"
                                        { (yyval.p) = op2(MINUS, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2938 "awkgram.tab.c"
    break;

  case 132: /* term: term '*' term  */
#line 355 "awkgram.y"
                                        { (yyval.p) = op2(MULT, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2944 "awkgram.tab.c"
    break;

  case 133: /* term: term '/' term  */
#line 356 "awkgram.y"
                                        { (yyval.p) = op2(DIVIDE, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2950 "awkgram.tab.c"
    break;

  case 134: /* term: term '%' term  */
#line 357 "awkgram.y"
                                        { (yyval.p) = op2(MOD, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2956 "awkgram.tab.c"
    break;

  case 135: /* term: term '^' term  */
#line 358 "awkgram.y"
                                        { (yyval.p) = op2(XOR, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2962 "awkgram.tab.c"
    break;

  case 136: /* term: term BOR term  */
#line 359 "awkgram.y"
                                        { (yyval.p) = op2(BOR, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2968 "awkgram.tab.c"
    break;

  case 137: /* term: term BAND term  */
#line 360 "awkgram.y"
                                        { (yyval.p) = op2(BAND, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2974 "awkgram.tab.c"
    break;

  case 138: /* term: term POWER term  */
#line 361 "awkgram.y"
                                        { (yyval.p) = op2(POWER, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2980 "awkgram.tab.c"
    break;

  case 139: /* term: term LSHIFT term  */
#line 362 "awkgram.y"
                                        { (yyval.p) = op2(LSHIFT, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2986 "awkgram.tab.c"
    break;

  case 140: /* term: term RSHIFT term  */
#line 363 "awkgram.y"
                                        { (yyval.p) = op2(RSHIFT, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2992 "awkgram.tab.c"
    break;

  case 141: /* term: '^' term  */
#line 364 "awkgram.y"
                                { (yyval.p) = op1(CMPL, (yyvsp[0].p)); }
#line 2998 "awkgram.tab.c"
    break;

  case 142: /* term: '-' term  */
#line 365 "awkgram.y"
                                        { (yyval.p) = op1(UMINUS, (yyvsp[0].p)); }
#line 3004 "awkgram.tab.c"
    break;

  case 143: /* term: '+' term  */
#line 366 "awkgram.y"
                                        { (yyval.p) = op1(UPLUS, (yyvsp[0].p)); }
#line 3010 "awkgram.tab.c"
    break;

  case 144: /* term: NOT term  */
#line 367 "awkgram.y"
                                        { (yyval.p) = op1(NOT, notnull((yyvsp[0].p))); }
#line 3016 "awkgram.tab.c"
    break;

  case 145: /* term: ADDON '(' ')'  */
#line 368 "awkgram.y"
                                        { (yyval.p) = op2(ADDON, itonp((yyvsp[-2].i)), rectonode()); }
#line 3022 "awkgram.tab.c"
    break;

  case 146: /* term: ADDON '(' patlist ')'  */
#line 369 "awkgram.y"
                                        { (yyval.p) = op2(ADDON, itonp((yyvsp[-3].i)), (yyvsp[-1].p)); }
#line 3028 "awkgram.tab.c"
    break;

  case 147: /* term: ADDON  */
#line 370 "awkgram.y"
                                        { (yyval.p) = op2(ADDON, itonp((yyvsp[0].i)), rectonode()); }
#line 3034 "awkgram.tab.c"
    break;

  case 148: /* term: BLTIN '(' ')'  */
#line 371 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[-2].i)), rectonode()); }
#line 3040 "awkgram.tab.c"
    break;

  case 149: /* term: BLTIN '(' patlist ')'  */
#line 372 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[-3].i)), (yyvsp[-1].p)); }
#line 3046 "awkgram.tab.c"
    break;

  case 150: /* term: BLTIN  */
#line 373 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[0].i)), rectonode()); }
#line 3052 "awkgram.tab.c"
    break;

  case 151: /* term: CALL '(' ')'  */
#line 374 "awkgram.y"
                                        { (yyval.p) = op2(CALL, celltonode((yyvsp[-2].cp),CVAR), NIL); }
#line 3058 "awkgram.tab.c"
    break;

  case 152: /* term: CALL '(' patlist ')'  */
#line 375 "awkgram.y"
                                        { (yyval.p) = op2(CALL, celltonode((yyvsp[-3].cp),CVAR), (yyvsp[-1].p)); }
#line 3064 "awkgram.tab.c"
    break;

  case 153: /* term: DECR var  */
#line 376 "awkgram.y"
                                        { (yyval.p) = op1(PREDECR, (yyvsp[0].p)); }
#line 3070 "awkgram.tab.c"
    break;

  case 154: /* term: INCR var  */
#line 377 "awkgram.y"
                                        { (yyval.p) = op1(PREINCR, (yyvsp[0].p)); }
#line 3076 "awkgram.tab.c"
    break;

  case 155: /* term: var DECR  */
#line 378 "awkgram.y"
                                        { (yyval.p) = op1(POSTDECR, (yyvsp[-1].p)); }
#line 3082 "awkgram.tab.c"
    break;

  case 156: /* term: var INCR  */
#line 379 "awkgram.y"
                                        { (yyval.p) = op1(POSTINCR, (yyvsp[-1].p)); }
#line 3088 "awkgram.tab.c"
    break;

  case 157: /* term: INDEX '(' pattern comma pattern ')'  */
#line 381 "awkgram.y"
                { (yyval.p) = op2(INDEX, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3094 "awkgram.tab.c"
    break;

  case 158: /* term: INDEX '(' pattern comma reg_expr ')'  */
#line 383 "awkgram.y"
                { SYNTAX("index() doesn't permit regular expressions");
		  (yyval.p) = op2(INDEX, (yyvsp[-3].p), (TNode*)(yyvsp[-1].s)); }
#line 3101 "awkgram.tab.c"
    break;

  case 159: /* term: '(' pattern ')'  */
#line 385 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 3107 "awkgram.tab.c"
    break;

  case 160: /* term: MATCHFCN '(' pattern comma reg_expr ')'  */
#line 387 "awkgram.y"
                { (yyval.p) = op3(MATCHFCN, NIL, (yyvsp[-3].p), (TNode*)makedfa((yyvsp[-1].s), 1)); FREE((yyvsp[-1].s)); }
#line 3113 "awkgram.tab.c"
    break;

  case 161: /* term: MATCHFCN '(' pattern comma pattern ')'  */
#line 389 "awkgram.y"
                { if (constnode((yyvsp[-1].p)))
			(yyval.p) = op3(MATCHFCN, NIL, (yyvsp[-3].p), (TNode*)makedfa(strnode((yyvsp[-1].p)), 1));
		  else
			(yyval.p) = op3(MATCHFCN, (TNode *)1, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3122 "awkgram.tab.c"
    break;

  case 162: /* term: NUMBER  */
#line 393 "awkgram.y"
                                        { (yyval.p) = celltonode((yyvsp[0].cp), CCON); }
#line 3128 "awkgram.tab.c"
    break;

  case 163: /* term: SPLIT '(' pattern comma varname comma pattern ')'  */
#line 395 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (yyvsp[-1].p), (TNode*)STRING); }
#line 3134 "awkgram.tab.c"
    break;

  case 164: /* term: SPLIT '(' pattern comma varname comma reg_expr ')'  */
#line 397 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (TNode*)makedfa((yyvsp[-1].s), 1), (TNode *)REGEXPR); FREE((yyvsp[-1].s)); }
#line 3140 "awkgram.tab.c"
    break;

  case 165: /* term: SPLIT '(' pattern comma varname ')'  */
#line 399 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-3].p), makearr((yyvsp[-1].p)), NIL, (TNode*)STRING); }
#line 3146 "awkgram.tab.c"
    break;

  case 166: /* term: SPRINTF '(' patlist ')'  */
#line 400 "awkgram.y"
                                        { (yyval.p) = op1((yyvsp[-3].i), (yyvsp[-1].p)); }
#line 3152 "awkgram.tab.c"
    break;

  case 167: /* term: string  */
#line 401 "awkgram.y"
                                        { (yyval.p) = celltonode((yyvsp[0].cp), CCON); }
#line 3158 "awkgram.tab.c"
    break;

  case 168: /* term: subop '(' reg_expr comma pattern ')'  */
#line 403 "awkgram.y"
                { (yyval.p) = op4((yyvsp[-5].i), NIL, (TNode*)makedfa((yyvsp[-3].s), 1), (yyvsp[-1].p), rectonode()); FREE((yyvsp[-3].s)); }
#line 3164 "awkgram.tab.c"
    break;

  case 169: /* term: subop '(' pattern comma pattern ')'  */
#line 405 "awkgram.y"
                { if (constnode((yyvsp[-3].p)))
			(yyval.p) = op4((yyvsp[-5].i), NIL, (TNode*)makedfa(strnode((yyvsp[-3].p)), 1), (yyvsp[-1].p), rectonode());
		  else
			(yyval.p) = op4((yyvsp[-5].i), (TNode *)1, (yyvsp[-3].p), (yyvsp[-1].p), rectonode()); }
#line 3173 "awkgram.tab.c"
    break;

  case 170: /* term: subop '(' reg_expr comma pattern comma var ')'  */
#line 410 "awkgram.y"
                { (yyval.p) = op4((yyvsp[-7].i), NIL, (TNode*)makedfa((yyvsp[-5].s), 1), (yyvsp[-3].p), (yyvsp[-1].p)); FREE((yyvsp[-5].s)); }
#line 3179 "awkgram.tab.c"
    break;

  case 171: /* term: subop '(' pattern comma pattern comma var ')'  */
#line 412 "awkgram.y"
                { if (constnode((yyvsp[-5].p)))
			(yyval.p) = op4((yyvsp[-7].i), NIL, (TNode*)makedfa(strnode((yyvsp[-5].p)), 1), (yyvsp[-3].p), (yyvsp[-1].p));
		  else
			(yyval.p) = op4((yyvsp[-7].i), (TNode *)1, (yyvsp[-5].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3188 "awkgram.tab.c"
    break;

  case 172: /* term: SUBSTR '(' pattern comma pattern comma pattern ')'  */
#line 417 "awkgram.y"
                { (yyval.p) = op3(SUBSTR, (yyvsp[-5].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3194 "awkgram.tab.c"
    break;

  case 173: /* term: SUBSTR '(' pattern comma pattern ')'  */
#line 419 "awkgram.y"
                { (yyval.p) = op3(SUBSTR, (yyvsp[-3].p), (yyvsp[-1].p), NIL); }
#line 3200 "awkgram.tab.c"
    break;

  case 176: /* var: varname '[' patlist ']'  */
#line 425 "awkgram.y"
                                        { (yyval.p) = op2(ARRAY, makearr((yyvsp[-3].p)), (yyvsp[-1].p)); }
#line 3206 "awkgram.tab.c"
    break;

  case 177: /* var: IVAR  */
#line 426 "awkgram.y"
                                        { (yyval.p) = op1(INDIRECT, celltonode((yyvsp[0].cp), CVAR)); }
#line 3212 "awkgram.tab.c"
    break;

  case 178: /* var: INDIRECT term  */
#line 427 "awkgram.y"
                                        { (yyval.p) = op1(INDIRECT, (yyvsp[0].p)); }
#line 3218 "awkgram.tab.c"
    break;

  case 179: /* varlist: %empty  */
#line 431 "awkgram.y"
                                { arglist = (yyval.p) = 0; }
#line 3224 "awkgram.tab.c"
    break;

  case 180: /* varlist: VAR  */
#line 432 "awkgram.y"
                                { arglist = (yyval.p) = celltonode((yyvsp[0].cp),CVAR); }
#line 3230 "awkgram.tab.c"
    break;

  case 181: /* varlist: varlist comma VAR  */
#line 433 "awkgram.y"
                                {
			checkdup((yyvsp[-2].p), (yyvsp[0].cp));
			arglist = (yyval.p) = linkum((yyvsp[-2].p),celltonode((yyvsp[0].cp),CVAR)); }
#line 3238 "awkgram.tab.c"
    break;

  case 182: /* varname: VAR  */
#line 439 "awkgram.y"
                                { (yyval.p) = celltonode((yyvsp[0].cp), CVAR); }
#line 3244 "awkgram.tab.c"
    break;

  case 183: /* varname: ARG  */
#line 440 "awkgram.y"
                                { (yyval.p) = op1(ARG, itonp((yyvsp[0].i))); }
#line 3250 "awkgram.tab.c"
    break;

  case 184: /* varname: VARNF  */
#line 441 "awkgram.y"
                                { (yyval.p) = op1(VARNF, (TNode *) (yyvsp[0].cp)); }
#line 3256 "awkgram.tab.c"
    break;

  case 185: /* while: WHILE '(' pattern rparen  */
#line 446 "awkgram.y"
                                        { (yyval.p) = notnull((yyvsp[-1].p)); }
#line 3262 "awkgram.tab.c"
    break;


#line 3266 "awkgram.tab.c"

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

int constnode(TNode *p)
{
	return isvalue(p) && ((Cell *) (p->narg[0]))->csub == CCON;
}

char *strnode(TNode *p)
{
	return ((Cell *)(p->narg[0]))->sval;
}

TNode *notnull(TNode *n)
{
	switch (n->nobj) {
	case LE: case LT: case EQ: case NE: case GT: case GE:
	case LOR: case LAND: case NOT:
		return n;
	default:
		return op2(NE, n, nullnode);
	}
}

void checkdup(TNode *vl, Cell *cp)	/* check if name already in list */
{
	if(isptr(cp) && !isarr(cp))
		FATAL("checkdup: ptr argument, shouldn\'t happen");
	char *s = cp->nval;
	for ( ; vl; vl = vl->nnext) {
		if (strcmp(s, ((Cell *)(vl->narg[0]))->nval) == 0) {
			SYNTAX("duplicate argument %s", s);
			break;
		}
	}
}
