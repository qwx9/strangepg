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
  YYSYMBOL_COMPL = 70,                     /* COMPL  */
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
  YYSYMBOL_CMPL = 112,                     /* CMPL  */
  YYSYMBOL_NOT = 113,                      /* NOT  */
  YYSYMBOL_UMINUS = 114,                   /* UMINUS  */
  YYSYMBOL_UPLUS = 115,                    /* UPLUS  */
  YYSYMBOL_POWER = 116,                    /* POWER  */
  YYSYMBOL_DECR = 117,                     /* DECR  */
  YYSYMBOL_INCR = 118,                     /* INCR  */
  YYSYMBOL_INDIRECT = 119,                 /* INDIRECT  */
  YYSYMBOL_LASTTOKEN = 120,                /* LASTTOKEN  */
  YYSYMBOL_121_ = 121,                     /* '^'  */
  YYSYMBOL_122_ = 122,                     /* '|'  */
  YYSYMBOL_123_ = 123,                     /* '&'  */
  YYSYMBOL_YYACCEPT = 124,                 /* $accept  */
  YYSYMBOL_program = 125,                  /* program  */
  YYSYMBOL_land = 126,                     /* land  */
  YYSYMBOL_lor = 127,                      /* lor  */
  YYSYMBOL_comma = 128,                    /* comma  */
  YYSYMBOL_do = 129,                       /* do  */
  YYSYMBOL_else = 130,                     /* else  */
  YYSYMBOL_for = 131,                      /* for  */
  YYSYMBOL_132_1 = 132,                    /* $@1  */
  YYSYMBOL_133_2 = 133,                    /* $@2  */
  YYSYMBOL_134_3 = 134,                    /* $@3  */
  YYSYMBOL_funcname = 135,                 /* funcname  */
  YYSYMBOL_if = 136,                       /* if  */
  YYSYMBOL_lbrace = 137,                   /* lbrace  */
  YYSYMBOL_nl = 138,                       /* nl  */
  YYSYMBOL_opt_nl = 139,                   /* opt_nl  */
  YYSYMBOL_opt_pst = 140,                  /* opt_pst  */
  YYSYMBOL_opt_simple_stmt = 141,          /* opt_simple_stmt  */
  YYSYMBOL_pas = 142,                      /* pas  */
  YYSYMBOL_pa_pat = 143,                   /* pa_pat  */
  YYSYMBOL_pa_stat = 144,                  /* pa_stat  */
  YYSYMBOL_145_4 = 145,                    /* $@4  */
  YYSYMBOL_pa_stats = 146,                 /* pa_stats  */
  YYSYMBOL_patlist = 147,                  /* patlist  */
  YYSYMBOL_ppattern = 148,                 /* ppattern  */
  YYSYMBOL_pattern = 149,                  /* pattern  */
  YYSYMBOL_plist = 150,                    /* plist  */
  YYSYMBOL_pplist = 151,                   /* pplist  */
  YYSYMBOL_prarg = 152,                    /* prarg  */
  YYSYMBOL_print = 153,                    /* print  */
  YYSYMBOL_pst = 154,                      /* pst  */
  YYSYMBOL_rbrace = 155,                   /* rbrace  */
  YYSYMBOL_re = 156,                       /* re  */
  YYSYMBOL_reg_expr = 157,                 /* reg_expr  */
  YYSYMBOL_158_5 = 158,                    /* $@5  */
  YYSYMBOL_rparen = 159,                   /* rparen  */
  YYSYMBOL_simple_stmt = 160,              /* simple_stmt  */
  YYSYMBOL_st = 161,                       /* st  */
  YYSYMBOL_stmt = 162,                     /* stmt  */
  YYSYMBOL_163_6 = 163,                    /* $@6  */
  YYSYMBOL_164_7 = 164,                    /* $@7  */
  YYSYMBOL_165_8 = 165,                    /* $@8  */
  YYSYMBOL_stmtlist = 166,                 /* stmtlist  */
  YYSYMBOL_subop = 167,                    /* subop  */
  YYSYMBOL_string = 168,                   /* string  */
  YYSYMBOL_term = 169,                     /* term  */
  YYSYMBOL_var = 170,                      /* var  */
  YYSYMBOL_varlist = 171,                  /* varlist  */
  YYSYMBOL_varname = 172,                  /* varname  */
  YYSYMBOL_while = 173                     /* while  */
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
#define YYLAST   4434

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  124
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  180
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  359

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
       2,     2,     2,     2,     2,     2,     2,   111,   123,     2,
      12,    15,   110,   108,    10,   109,     2,    14,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   102,    13,
       2,     2,     2,   101,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    17,     2,    18,   121,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,   122,    16,     2,     2,     2,     2,
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
     106,   107,   112,   113,   114,   115,   116,   117,   118,   119,
     120
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
  "LSHIFT", "RSHIFT", "XOR", "BAND", "BOR", "COMPL", "ASSIGN", "ASGNOP",
  "ADDEQ", "SUBEQ", "MULTEQ", "DIVEQ", "MODEQ", "POWEQ", "BANDEQ", "BOREQ",
  "XOREQ", "SHLEQ", "SHREQ", "PRINT", "PRINTF", "SPRINTF", "ELSE",
  "INTEST", "CONDEXPR", "POSTINCR", "PREINCR", "POSTDECR", "PREDECR",
  "VAR", "IVAR", "VARNF", "CALL", "NUMBER", "STRING", "REGEXPR", "'?'",
  "':'", "RETURN", "SPLIT", "SUBSTR", "WHILE", "CAT", "'+'", "'-'", "'*'",
  "'%'", "CMPL", "NOT", "UMINUS", "UPLUS", "POWER", "DECR", "INCR",
  "INDIRECT", "LASTTOKEN", "'^'", "'|'", "'&'", "$accept", "program",
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

#define YYPACT_NINF (-281)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-32)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     717,  -281,  -281,  -281,    27,  1744,  -281,    20,  -281,    35,
      35,  -281,  4036,  -281,  -281,    32,   -52,  -281,  -281,    63,
      97,   111,  -281,  -281,  -281,   119,  -281,  -281,   134,   143,
    4315,  4315,  4315,  4112,   -30,   -30,  4315,   793,   133,  -281,
      69,  3433,  -281,  -281,   145,   -11,   365,   -50,    80,  -281,
    -281,   793,   793,  2378,    33,    73,  3813,  -281,  -281,   167,
    4036,  4036,  4036,  3889,  4036,  4036,  4036,  4315,    45,    64,
      45,    45,  -281,    45,  -281,  -281,    -6,  -281,  -281,   175,
      85,    85,   -23,  -281,  1875,   174,   177,    85,  -281,  -281,
    1875,   178,   182,  -281,  1479,   793,  3433,  4146,    85,  -281,
     869,  -281,   175,   793,  1744,  4036,  -281,  -281,  4036,  4036,
    4036,  4036,  4036,  4036,   -23,  4036,  1904,  2006,   365,  4036,
    -281,  4216,  4315,  4315,  4315,  4315,  4315,  4315,  4315,  4315,
    4036,  -281,  -281,  4036,   945,  1021,  -281,  -281,  2035,   149,
    2035,   179,  -281,    66,  3433,   100,  2812,  2812,    76,  -281,
     114,  2812,  2812,  2847,  -281,   186,  -281,   175,   186,  -281,
    -281,   180,  1773,  -281,  1555,  4036,  -281,  1773,  -281,  4036,
    -281,  1479,   109,  1097,  4036,  3699,   188,  -281,  -281,   365,
     -22,  -281,  -281,  -281,  1479,  4036,  1173,  -281,  3556,  -281,
    3556,  3556,  3556,  3556,  3556,  3556,  -281,  2935,  -281,  3591,
    -281,  3468,  2812,   188,  4315,    45,    16,    16,    45,    45,
      45,   365,   365,   365,  3433,    21,  -281,  -281,  -281,  3433,
     -23,  3433,  -281,  -281,  2035,  -281,   132,  2035,  2035,  -281,
    -281,    40,  2035,  -281,  -281,  4036,  -281,   187,  -281,     9,
    2970,  -281,  2970,  -281,  -281,  1251,  -281,   190,   148,  4286,
     -23,  4286,  2137,  2166,   365,  2268,  4286,  -281,    35,  -281,
    4036,  2035,  2035,   365,  -281,  -281,  3433,  -281,     7,   192,
    3058,   189,  3093,   191,   160,  2488,    62,   165,   -23,   192,
     192,    96,  -281,  -281,  -281,   164,  4036,  3924,  -281,  -281,
    3670,  4002,  3779,  3699,  3699,   793,  3433,  2598,  2708,  -281,
    -281,    35,  -281,  -281,  -281,  -281,  -281,  2035,  -281,  2035,
    -281,   175,  4036,   198,   204,   -23,   162,  4286,  1327,  -281,
      26,  -281,    26,   793,  3181,   203,  3216,  1631,  3310,   192,
    4036,  -281,   164,  3699,  -281,   206,   209,  1403,  -281,  -281,
    -281,   198,   175,  1479,  3345,  -281,  -281,  -281,   192,  1631,
    -281,    85,  1479,   198,  -281,  -281,   192,  1479,  -281
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     3,    86,    87,     0,    33,     2,    30,     1,     0,
       0,    23,     0,    94,   178,   145,     0,   125,   126,     0,
       0,     0,   177,   172,   179,     0,   157,   127,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    45,
      29,    35,    75,    92,     0,   162,    76,   169,   170,    88,
      89,     0,     0,     0,     0,     0,     0,    20,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   141,   169,
     140,   139,    93,   142,   148,   149,   173,   102,    24,    27,
       0,     0,     0,    10,     0,     0,     0,     0,    84,    85,
       0,     0,   107,   112,     0,     0,   101,    81,     0,   123,
       0,   120,    27,     0,    34,     0,     4,     6,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,     0,
     128,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   150,   151,     0,     0,     0,     8,   154,     0,     0,
       0,     0,   143,     0,    47,   174,     0,     0,     0,   146,
       0,     0,     0,     0,    25,    28,   122,    27,   103,   105,
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
    -281,  -281,   -82,    53,   230,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,    15,   -74,   -94,   185,  -280,  -281,    41,
     130,  -281,  -281,   -59,  -215,    -5,  -173,  -281,  -281,  -281,
    -281,  -281,   -20,   -87,  -281,  -223,  -162,   -75,   358,  -281,
    -281,  -281,   -41,  -281,  -281,   388,   118,  -281,    74,  -281
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
      41,   248,   238,   148,   150,   155,   160,    53,   185,   163,
     134,   135,   166,    72,    14,   168,   218,   279,   189,   280,
      37,    14,   130,   181,    51,    52,   133,     8,   155,    49,
     121,   136,   203,    50,   287,   218,   290,   291,   292,   264,
     293,   294,    57,   136,    56,    58,    11,   341,   139,   218,
     256,   144,   278,   103,   173,   146,   147,   144,   144,   151,
     152,   153,   186,   234,    22,    23,    24,   131,   132,   353,
      14,    22,   136,    24,   215,    60,   136,   178,     2,   162,
     310,   223,     3,   155,    14,   167,   136,   236,   120,    36,
     329,   229,   241,   252,   154,   131,   132,   133,   157,    41,
     188,   299,   333,   190,   191,   192,   193,   194,   195,    61,
     197,   199,   201,   316,   202,   127,   128,   129,   348,    37,
      22,    23,    24,    62,   136,   214,   124,   125,   144,   230,
     356,    63,   126,   219,    22,   221,    24,   127,   128,   129,
     271,   273,   136,   102,    11,    36,    64,   267,    69,    69,
      69,    69,    74,    75,    69,    65,   161,   119,   136,    69,
     240,   126,   288,   285,   242,   238,   127,   128,   129,    53,
     136,    69,   136,   141,   154,   306,   276,   332,   311,   145,
      41,   131,   132,   312,   154,    69,   164,   238,   196,   165,
     169,   170,   220,   222,   225,   233,   244,   235,   136,   284,
     277,   300,   314,   155,   303,   252,   305,   315,   252,   252,
     252,   252,   252,   267,    69,   180,   330,   327,   339,   266,
     325,   345,   270,   272,   346,   104,   258,   275,   253,   178,
     144,   178,   178,   178,   187,   178,   178,   155,   239,    69,
      69,    69,    69,    69,    69,    69,    69,    69,   349,     0,
       0,   252,     0,     0,   318,   296,   297,   298,     0,     0,
       0,     0,    69,     0,    69,    69,     0,     0,   155,    69,
      69,    69,     0,   295,     0,     0,   354,     0,     0,     0,
      69,    53,   337,   138,   140,    69,     0,     0,     0,     0,
       0,     0,     0,    69,   265,     0,     0,   178,     0,     0,
       0,     0,   324,     0,   326,   274,    69,   328,    69,    69,
      69,    69,    69,    69,     0,    69,   323,    69,     0,    69,
      69,     0,    69,     0,   289,   344,     0,     0,     0,     0,
       0,     0,    69,     0,     0,     0,     0,    69,     0,    69,
     253,     0,     0,   253,   253,   253,   253,   253,     0,     0,
       0,     0,   313,     0,     0,     0,     0,     0,    69,     0,
      69,     0,     0,     0,     0,     0,     0,   180,     0,   180,
     180,   180,     0,   180,   180,     0,   227,   228,     0,   121,
       0,   231,   232,     0,    69,     0,   253,     0,    69,   331,
      69,     0,     0,    69,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,   255,     0,    69,    69,
      69,    69,    69,     0,    69,    69,    69,     0,    68,    70,
      71,    73,     0,     0,    76,     0,     0,     0,     0,   118,
       0,     0,   261,   262,     0,   180,     0,     0,   335,     0,
     336,   118,    69,     0,    69,     0,    69,     0,     0,     0,
       0,    69,   172,     0,     0,    73,   268,     0,   183,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   122,   123,   124,   125,     0,   140,     0,
       0,   126,     0,     0,   118,   179,   127,   128,   129,     0,
       0,     0,   183,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   307,   309,     0,     0,     0,   205,
     206,   207,   208,   209,   210,   211,   212,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   320,   322,   243,
       0,   183,   118,     0,   118,   118,     0,     0,     0,   118,
     118,   118,   257,     0,   183,     0,   140,     0,     0,     0,
     118,     0,     0,     0,     0,   118,     0,     0,     0,     0,
       0,     0,     0,   254,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   118,     0,   118,   118,
     118,   118,   118,   118,     0,   118,     0,   118,     0,   118,
     118,     0,   263,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   118,   283,     0,     0,     0,   118,     0,   118,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   118,     0,
     118,     0,     0,     0,     0,     0,     0,   179,     0,   179,
     179,   179,     0,   179,   179,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   118,     0,     0,     0,   118,     0,
     118,     0,     0,   118,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   254,   183,     0,   254,   254,
     254,   254,   254,     0,   118,   118,   118,     0,     0,     0,
       0,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,   350,     0,     0,     0,   179,     0,     0,     0,     0,
     355,     0,   118,     0,   118,   358,   118,   -29,     1,     0,
       0,   254,     0,     0,   -29,   -29,     2,     0,   -29,   -29,
       3,   -29,   118,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   -29,   -29,     0,     0,     0,     0,     0,     0,   -29,
     -29,   -29,     0,   -29,     0,   -29,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,     0,
       0,     0,    78,   -29,    11,    12,    79,    13,     0,     0,
       0,   -29,   -29,   -29,   -29,   -29,   -29,     0,     0,     0,
       0,   -29,   -29,     0,     0,   -29,   -29,     0,     0,   -29,
     -29,     0,     0,     0,   -29,   -29,   -29,    14,    15,    80,
      81,    82,    83,    84,    85,     0,    17,    18,    86,    19,
       0,    20,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,     0,     0,    88,    89,    21,
      11,    12,    79,    13,     0,   182,     0,    22,    23,    24,
      25,    26,    27,     0,     0,     0,    90,    28,    29,    91,
       0,    30,    31,     0,     0,    32,    33,     0,     0,     0,
      34,    35,    36,    14,    15,    80,    81,    82,    83,    84,
      85,     0,    17,    18,    86,    19,     0,    20,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,     0,     0,    88,    89,    21,    11,    12,    79,    13,
       0,   216,     0,    22,    23,    24,    25,    26,    27,     0,
       0,     0,    90,    28,    29,    91,     0,    30,    31,     0,
       0,    32,    33,     0,     0,     0,    34,    35,    36,    14,
      15,    80,    81,    82,    83,    84,    85,     0,    17,    18,
      86,    19,     0,    20,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,     0,     0,    88,
      89,    21,    11,    12,    79,    13,     0,   217,     0,    22,
      23,    24,    25,    26,    27,     0,     0,     0,    90,    28,
      29,    91,     0,    30,    31,     0,     0,    32,    33,     0,
       0,     0,    34,    35,    36,    14,    15,    80,    81,    82,
      83,    84,    85,     0,    17,    18,    86,    19,     0,    20,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,     0,     0,    88,    89,    21,    11,    12,
      79,    13,     0,   246,     0,    22,    23,    24,    25,    26,
      27,     0,     0,     0,    90,    28,    29,    91,     0,    30,
      31,     0,     0,    32,    33,     0,     0,     0,    34,    35,
      36,    14,    15,    80,    81,    82,    83,    84,    85,     0,
      17,    18,    86,    19,     0,    20,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,     0,
       0,    88,    89,    21,    11,    12,    79,    13,     0,   259,
       0,    22,    23,    24,    25,    26,    27,     0,     0,     0,
      90,    28,    29,    91,     0,    30,    31,     0,     0,    32,
      33,     0,     0,     0,    34,    35,    36,    14,    15,    80,
      81,    82,    83,    84,    85,     0,    17,    18,    86,    19,
       0,    20,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,    88,    89,    21,
     282,     0,    11,    12,    79,    13,     0,    22,    23,    24,
      25,    26,    27,     0,     0,     0,    90,    28,    29,    91,
       0,    30,    31,     0,     0,    32,    33,     0,     0,     0,
      34,    35,    36,     0,     0,    14,    15,    80,    81,    82,
      83,    84,    85,     0,    17,    18,    86,    19,     0,    20,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,     0,     0,    88,    89,    21,    11,    12,
      79,    13,     0,   334,     0,    22,    23,    24,    25,    26,
      27,     0,     0,     0,    90,    28,    29,    91,     0,    30,
      31,     0,     0,    32,    33,     0,     0,     0,    34,    35,
      36,    14,    15,    80,    81,    82,    83,    84,    85,     0,
      17,    18,    86,    19,     0,    20,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,     0,
       0,    88,    89,    21,    11,    12,    79,    13,     0,   347,
       0,    22,    23,    24,    25,    26,    27,     0,     0,     0,
      90,    28,    29,    91,     0,    30,    31,     0,     0,    32,
      33,     0,     0,     0,    34,    35,    36,    14,    15,    80,
      81,    82,    83,    84,    85,     0,    17,    18,    86,    19,
       0,    20,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,     0,     0,    88,    89,    21,
      11,    12,    79,    13,     0,     0,     0,    22,    23,    24,
      25,    26,    27,     0,     0,     0,    90,    28,    29,    91,
       0,    30,    31,     0,     0,    32,    33,     0,     0,     0,
      34,    35,    36,    14,    15,    80,    81,    82,    83,    84,
      85,     0,    17,    18,    86,    19,     0,    20,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,     0,     0,    88,    89,    21,     0,    12,   -31,    13,
       0,     0,     0,    22,    23,    24,    25,    26,    27,     0,
       0,     0,    90,    28,    29,    91,     0,    30,    31,     0,
       0,    32,    33,     0,     0,     0,    34,    35,    36,    14,
      15,     0,     0,    82,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,     0,     0,    88,
      89,    21,     0,    12,     0,    13,   -31,     0,     0,    22,
      23,    24,    25,    26,    27,     0,     0,     0,     0,    28,
      29,     0,     0,    30,    31,     0,     0,    32,    33,     0,
       0,     0,    34,    35,    36,    14,    15,     0,     0,    82,
       0,     0,     0,     0,    17,    18,     0,    19,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    88,    89,    21,     0,     0,
       0,     0,     0,     0,     0,    22,    23,    24,    25,    26,
      27,     0,     0,     0,     0,    28,    29,     0,     0,    30,
      31,     0,     0,    32,    33,     0,     0,     0,    34,    35,
      36,     9,    10,     0,     0,    11,    12,     0,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   154,     0,     0,    66,   157,     0,    14,    15,
       0,     0,     0,     0,     0,   105,    16,    17,    18,     0,
      19,     0,    20,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    17,    18,     0,    19,
      21,    20,     0,     0,     0,     0,     0,     0,    22,    23,
      24,    25,    26,    27,     0,     0,     0,     0,    28,    29,
       0,     0,    30,    31,     0,     0,    32,    33,     0,    21,
       0,    34,    35,    36,     0,     0,     0,    22,    23,    24,
      25,    26,    27,     0,   115,     0,     0,    28,    29,     0,
       0,    30,    31,     0,   154,    32,    67,    12,   157,    13,
      34,    35,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   198,     0,     0,    12,     0,    13,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,     0,     0,     0,    17,    18,     0,
      19,    21,    20,     0,     0,     0,     0,     0,     0,    22,
      23,    24,    25,    26,    27,     0,     0,     0,     0,    28,
      29,     0,     0,    30,    31,     0,     0,    32,    33,     0,
      21,     0,    34,    35,    36,     0,     0,     0,    22,    23,
      24,    25,    26,    27,     0,     0,     0,     0,    28,    29,
       0,     0,    30,    31,     0,   200,    32,    33,    12,     0,
      13,    34,    35,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   218,     0,     0,    12,     0,    13,
      14,    15,     0,     0,     0,     0,     0,     0,     0,    17,
      18,     0,    19,     0,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,    21,    20,     0,     0,     0,     0,     0,     0,
      22,    23,    24,    25,    26,    27,     0,     0,     0,     0,
      28,    29,     0,     0,    30,    31,     0,     0,    32,    33,
       0,    21,     0,    34,    35,    36,     0,     0,     0,    22,
      23,    24,    25,    26,    27,     0,     0,     0,     0,    28,
      29,     0,     0,    30,    31,     0,   198,    32,    33,   286,
       0,    13,    34,    35,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   200,     0,     0,   286,     0,
      13,    14,    15,     0,     0,     0,     0,     0,     0,     0,
      17,    18,     0,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,    17,
      18,     0,    19,    21,    20,     0,     0,     0,     0,     0,
       0,    22,    23,    24,    25,    26,    27,     0,     0,     0,
       0,    28,    29,     0,     0,    30,    31,     0,     0,    32,
      33,     0,    21,     0,    34,    35,    36,     0,     0,     0,
      22,    23,    24,    25,    26,    27,     0,     0,     0,     0,
      28,    29,     0,     0,    30,    31,     0,   218,    32,    33,
     286,     0,    13,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,     0,     0,
       0,    17,    18,     0,    19,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,    25,    26,    27,     0,     0,
       0,     0,    28,    29,     0,     0,    30,    31,     0,     0,
      32,    33,     0,     0,     0,    34,    35,    36,   136,     0,
      66,     0,     0,   137,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    14,    15,     0,     0,     0,     0,     0,     0,
       0,    17,    18,     0,    19,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,    25,    26,    27,     0,   115,
       0,     0,    28,    29,     0,     0,    30,    31,     0,     0,
      32,    67,     0,     0,     0,    34,    35,    36,   136,     0,
      66,     0,     0,   308,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    14,    15,     0,     0,     0,     0,     0,     0,
       0,    17,    18,     0,    19,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,    25,    26,    27,     0,   115,
       0,     0,    28,    29,     0,     0,    30,    31,     0,     0,
      32,    67,     0,     0,     0,    34,    35,    36,   136,     0,
      66,     0,     0,   319,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    14,    15,     0,     0,     0,     0,     0,     0,
       0,    17,    18,     0,    19,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,    25,    26,    27,     0,   115,
       0,     0,    28,    29,     0,     0,    30,    31,     0,     0,
      32,    67,     0,     0,     0,    34,    35,    36,   136,     0,
      66,     0,     0,   321,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    14,    15,     0,     0,     0,     0,     0,     0,
       0,    17,    18,     0,    19,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,    25,    26,    27,     0,   115,
       0,     0,    28,    29,     0,     0,    30,    31,     0,     0,
      32,    67,   136,     0,    66,    34,    35,    36,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    14,    15,     0,    66,
       0,     0,   137,     0,     0,    17,    18,     0,    19,   105,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    14,    15,     0,     0,     0,     0,     0,    21,     0,
      17,    18,     0,    19,     0,    20,    22,    23,    24,    25,
      26,    27,     0,   115,     0,     0,    28,    29,     0,     0,
      30,    31,     0,     0,    32,    67,     0,     0,     0,    34,
      35,    36,     0,    21,     0,     0,     0,     0,     0,     0,
       0,    22,    23,    24,    25,    26,    27,    66,   115,     0,
       0,    28,    29,     0,     0,    30,    31,   105,     0,    32,
      67,     0,     0,     0,    34,    35,    36,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,    66,     0,     0,   267,     0,     0,    17,    18,
       0,    19,   105,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    14,    15,     0,     0,     0,     0,
       0,    21,     0,    17,    18,     0,    19,     0,    20,    22,
      23,    24,    25,    26,    27,     0,   115,   260,     0,    28,
      29,     0,     0,    30,    31,     0,     0,    32,    67,     0,
       0,     0,    34,    35,    36,     0,    21,     0,     0,     0,
       0,     0,     0,     0,    22,    23,    24,    25,    26,    27,
      66,   115,     0,   302,    28,    29,     0,     0,    30,    31,
     105,     0,    32,    67,     0,     0,     0,    34,    35,    36,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    14,    15,     0,    66,     0,     0,   304,     0,
       0,    17,    18,     0,    19,   105,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
       0,     0,     0,     0,    21,     0,    17,    18,     0,    19,
       0,    20,    22,    23,    24,    25,    26,    27,     0,   115,
       0,     0,    28,    29,     0,     0,    30,    31,     0,     0,
      32,    67,     0,     0,     0,    34,    35,    36,     0,    21,
       0,     0,     0,     0,     0,     0,     0,    22,    23,    24,
      25,    26,    27,    66,   115,     0,   338,    28,    29,     0,
       0,    30,    31,   105,     0,    32,    67,     0,     0,     0,
      34,    35,    36,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    14,    15,     0,    66,     0,
       0,   340,     0,     0,    17,    18,     0,    19,   105,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   106,   107,   108,   109,   110,   111,   112,   113,   114,
      14,    15,     0,     0,     0,     0,     0,    21,     0,    17,
      18,     0,    19,     0,    20,    22,    23,    24,    25,    26,
      27,     0,   115,     0,     0,    28,    29,     0,     0,    30,
      31,     0,     0,    32,    67,     0,     0,     0,    34,    35,
      36,     0,    21,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,    25,    26,    27,     0,   115,     0,     0,
      28,    29,    66,   342,    30,    31,     0,     0,    32,    67,
       0,     0,   105,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    14,    15,     0,    66,     0,     0,
     351,     0,     0,    17,    18,     0,    19,   105,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    14,
      15,     0,     0,     0,     0,     0,    21,     0,    17,    18,
       0,    19,     0,    20,    22,    23,    24,    25,    26,    27,
       0,   115,     0,     0,    28,    29,     0,     0,    30,    31,
       0,     0,    32,    67,     0,     0,     0,    34,    35,    36,
       0,    21,     0,     0,     0,     0,     0,     0,     0,    22,
      23,    24,    25,    26,    27,    66,   115,     0,     0,    28,
      29,     0,     0,    30,    31,   105,     0,    32,    67,     0,
       0,     0,    34,    35,    36,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    15,     0,
      66,     0,     0,     0,     0,     0,    17,    18,     0,    19,
     105,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   106,     0,   108,   109,   110,   111,   112,
     113,   114,    14,    15,     0,     0,     0,     0,     0,    21,
       0,    17,    18,     0,    19,     0,    20,    22,    23,    24,
      25,    26,    27,     0,   115,     0,     0,    28,    29,     0,
       0,    30,    31,     0,     0,    32,    67,     0,     0,     0,
      34,    35,    36,     0,    21,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,    25,    26,    27,    66,     0,
       0,     0,    28,    29,     0,     0,    30,    31,   -32,     0,
      32,    67,     0,     0,     0,    34,    35,    36,     0,     0,
       0,     0,     0,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
      14,    15,     0,    66,     0,     0,     0,     0,     0,    17,
      18,     0,    19,   105,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   108,   109,
     110,   111,   112,   113,   114,    14,    15,     0,     0,     0,
       0,     0,    21,     0,    17,    18,     0,    19,     0,    20,
      22,    23,    24,    25,    26,    27,     0,     0,     0,     0,
      28,    29,     0,     0,    30,    31,     0,     0,    32,    67,
       0,     0,     0,    34,    35,    36,     0,    21,     0,     0,
       0,     0,    66,     0,     0,    22,    23,    24,    25,    26,
      27,     0,   249,     0,     0,    28,    29,     0,     0,    30,
      31,     0,     0,    32,    67,   106,   107,     0,    34,    35,
      36,    66,     0,   250,    14,    15,     0,     0,     0,     0,
       0,   249,     0,    17,    18,     0,    19,     0,    20,     0,
       0,     0,     0,     0,   106,   107,     0,     0,     0,     0,
       0,     0,   250,    14,    15,     0,     0,     0,     0,     0,
       0,     0,    17,    18,     0,    19,    21,    20,     0,     0,
       0,     0,     0,     0,    22,    23,    24,    25,    26,    27,
       0,   251,   317,     0,    28,    29,     0,     0,    30,    31,
       0,     0,    32,    67,     0,    21,     0,    34,    35,    36,
       0,    66,     0,    22,    23,    24,    25,    26,    27,     0,
     251,   249,     0,    28,    29,     0,     0,    30,    31,     0,
       0,    32,    67,     0,   106,     0,    34,    35,    36,     0,
       0,     0,   250,    14,    15,    12,     0,    13,   142,     0,
       0,     0,    17,    18,     0,    19,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,     0,     0,    21,    17,    18,     0,    19,
       0,    20,     0,    22,    23,    24,    25,    26,    27,     0,
       0,     0,     0,    28,    29,     0,     0,    30,    31,     0,
       0,    32,    67,     0,     0,     0,    34,    35,    36,    21,
       0,    12,     0,    13,   149,     0,     0,    22,    23,    24,
      25,    26,    27,     0,     0,     0,     0,    28,    29,     0,
       0,    30,    31,     0,     0,    32,    33,     0,     0,     0,
      34,    35,    36,    14,    15,     0,    66,     0,     0,     0,
       0,     0,    17,    18,     0,    19,   -32,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   -32,    14,    15,
       0,     0,     0,     0,     0,    21,     0,    17,    18,     0,
      19,     0,    20,    22,    23,    24,    25,    26,    27,     0,
       0,     0,     0,    28,    29,     0,     0,    30,    31,     0,
       0,    32,    33,     0,     0,     0,    34,    35,    36,     0,
      21,     0,     0,     0,    66,     0,     0,     0,    22,    23,
      24,    25,    26,    27,   249,     0,     0,     0,    28,    29,
       0,     0,    30,    31,     0,     0,    32,    67,     0,     0,
       0,    34,    35,    36,     0,   250,    14,    15,    12,     0,
      13,     0,     0,     0,     0,    17,    18,     0,    19,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,    21,    17,
      18,     0,    19,     0,    20,     0,    22,    23,    24,    25,
      26,    27,     0,     0,     0,     0,    28,    29,     0,     0,
      30,    31,     0,     0,    32,    67,     0,     0,     0,    34,
      35,    36,    21,     0,    66,     0,    13,     0,     0,     0,
      22,    23,    24,    25,    26,    27,     0,     0,     0,     0,
      28,    29,     0,     0,    30,    31,     0,     0,    32,    33,
       0,     0,     0,    34,    35,    36,    14,    15,   174,     0,
      13,     0,     0,     0,     0,    17,    18,     0,    19,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,     0,     0,    21,    17,
      18,     0,    19,     0,    20,     0,    22,    23,    24,    25,
      26,    27,     0,     0,     0,     0,    28,    29,     0,     0,
      30,    31,     0,     0,    32,    33,     0,     0,    66,    34,
      35,    36,    21,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,    25,    26,    27,     0,     0,     0,     0,
      28,    29,     0,     0,    30,    31,     0,     0,    32,    33,
      14,    15,     0,    34,    35,    36,     0,     0,     0,    17,
      18,     0,    19,     0,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   286,     0,
      13,     0,    21,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,    25,    26,    27,     0,     0,     0,     0,
      28,    29,     0,     0,    30,    31,     0,    66,    32,    67,
      14,    15,     0,    34,    35,    36,     0,     0,     0,    17,
      18,     0,    19,     0,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,    19,    21,    20,     0,     0,     0,     0,     0,     0,
      22,    23,    24,    25,    26,    27,     0,     0,     0,     0,
      28,    29,     0,     0,    30,    31,     0,     0,    32,    33,
       0,    21,     0,    34,    35,    36,     0,     0,     0,    22,
      23,    24,    25,    26,    27,     0,     0,     0,     0,    28,
      29,     0,     0,    30,    31,     0,     0,    32,    67,     0,
       0,     0,    34,    35,    36
};

static const yytype_int16 yycheck[] =
{
       5,   174,   164,    62,    63,    79,    81,    12,   102,    84,
      51,    52,    87,    33,    44,    90,     9,   240,   105,   242,
       5,    44,    72,    98,     9,    10,    17,     0,   102,     9,
      14,    10,   119,    13,   249,     9,   251,   252,   253,    18,
     255,   256,    94,    10,    12,    97,    11,   327,    15,     9,
      72,    56,    43,    38,    95,    60,    61,    62,    63,    64,
      65,    66,   103,   157,    94,    95,    96,   117,   118,   349,
      44,    94,    10,    96,   133,    12,    10,    97,     9,    84,
      18,    15,    13,   157,    44,    90,    10,   162,    99,   119,
     313,    15,   167,   175,     9,   117,   118,    17,    13,   104,
     105,    94,   317,   108,   109,   110,   111,   112,   113,    12,
     115,   116,   117,   286,   119,   121,   122,   123,   341,   104,
      94,    95,    96,    12,    10,   130,   110,   111,   133,    15,
     353,    12,   116,   138,    94,   140,    96,   121,   122,   123,
     227,   228,    10,    10,    11,   119,    12,    15,    30,    31,
      32,    33,    34,    35,    36,    12,    82,    12,    10,    41,
     165,   116,   249,    15,   169,   327,   121,   122,   123,   174,
      10,    53,    10,   100,     9,    15,   235,    15,    13,    12,
     185,   117,   118,   277,     9,    67,    12,   349,   114,    12,
      12,     9,    43,    14,    94,     9,    87,    17,    10,     9,
      13,     9,   106,   277,    15,   287,    15,    43,   290,   291,
     292,   293,   294,    15,    96,    97,    12,   311,    15,   224,
     307,    15,   227,   228,    15,    40,   185,   232,   175,   249,
     235,   251,   252,   253,   104,   255,   256,   311,   164,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   342,    -1,
      -1,   333,    -1,    -1,   295,   260,   261,   262,    -1,    -1,
      -1,    -1,   144,    -1,   146,   147,    -1,    -1,   342,   151,
     152,   153,    -1,   258,    -1,    -1,   351,    -1,    -1,    -1,
     162,   286,   323,    53,    54,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   175,   220,    -1,    -1,   317,    -1,    -1,
      -1,    -1,   307,    -1,   309,   231,   188,   312,   190,   191,
     192,   193,   194,   195,    -1,   197,   301,   199,    -1,   201,
     202,    -1,   204,    -1,   250,   330,    -1,    -1,    -1,    -1,
      -1,    -1,   214,    -1,    -1,    -1,    -1,   219,    -1,   221,
     287,    -1,    -1,   290,   291,   292,   293,   294,    -1,    -1,
      -1,    -1,   278,    -1,    -1,    -1,    -1,    -1,   240,    -1,
     242,    -1,    -1,    -1,    -1,    -1,    -1,   249,    -1,   251,
     252,   253,    -1,   255,   256,    -1,   146,   147,    -1,    14,
      -1,   151,   152,    -1,   266,    -1,   333,    -1,   270,   315,
     272,    -1,    -1,   275,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   287,   176,    -1,   290,   291,
     292,   293,   294,    -1,   296,   297,   298,    -1,    30,    31,
      32,    33,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,
      -1,    -1,   202,   203,    -1,   317,    -1,    -1,   320,    -1,
     322,    53,   324,    -1,   326,    -1,   328,    -1,    -1,    -1,
      -1,   333,    94,    -1,    -1,    67,   226,    -1,   100,    -1,
      -1,    -1,   344,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   248,    -1,
      -1,   116,    -1,    -1,    96,    97,   121,   122,   123,    -1,
      -1,    -1,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   274,   275,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   297,   298,   171,
      -1,   173,   144,    -1,   146,   147,    -1,    -1,    -1,   151,
     152,   153,   184,    -1,   186,    -1,   316,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,   190,   191,
     192,   193,   194,   195,    -1,   197,    -1,   199,    -1,   201,
     202,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   214,   245,    -1,    -1,    -1,   219,    -1,   221,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   240,    -1,
     242,    -1,    -1,    -1,    -1,    -1,    -1,   249,    -1,   251,
     252,   253,    -1,   255,   256,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   266,    -1,    -1,    -1,   270,    -1,
     272,    -1,    -1,   275,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   287,   318,    -1,   290,   291,
     292,   293,   294,    -1,   296,   297,   298,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   337,    -1,    -1,    -1,    -1,
      -1,   343,    -1,    -1,    -1,   317,    -1,    -1,    -1,    -1,
     352,    -1,   324,    -1,   326,   357,   328,     0,     1,    -1,
      -1,   333,    -1,    -1,     7,     8,     9,    -1,    11,    12,
      13,    14,   344,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    86,    11,    12,    13,    14,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    -1,    -1,    -1,
      -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,
     113,    -1,    -1,    -1,   117,   118,   119,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,    86,
      11,    12,    13,    14,    -1,    16,    -1,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,    -1,   112,   113,    -1,    -1,    -1,
     117,   118,   119,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    -1,    58,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    84,    85,    86,    11,    12,    13,    14,
      -1,    16,    -1,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
      -1,   112,   113,    -1,    -1,    -1,   117,   118,   119,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    -1,    58,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    84,
      85,    86,    11,    12,    13,    14,    -1,    16,    -1,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,    -1,    -1,   112,   113,    -1,
      -1,    -1,   117,   118,   119,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    -1,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    -1,    -1,    -1,    -1,    84,    85,    86,    11,    12,
      13,    14,    -1,    16,    -1,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,    -1,   112,   113,    -1,    -1,    -1,   117,   118,
     119,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    -1,    58,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    84,    85,    86,    11,    12,    13,    14,    -1,    16,
      -1,    94,    95,    96,    97,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,    -1,   112,
     113,    -1,    -1,    -1,   117,   118,   119,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    84,    85,    86,
       9,    -1,    11,    12,    13,    14,    -1,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,    -1,   112,   113,    -1,    -1,    -1,
     117,   118,   119,    -1,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    -1,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    -1,    -1,    -1,    -1,    84,    85,    86,    11,    12,
      13,    14,    -1,    16,    -1,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,    -1,   112,   113,    -1,    -1,    -1,   117,   118,
     119,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    -1,    58,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    84,    85,    86,    11,    12,    13,    14,    -1,    16,
      -1,    94,    95,    96,    97,    98,    99,    -1,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,    -1,   112,
     113,    -1,    -1,    -1,   117,   118,   119,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,    86,
      11,    12,    13,    14,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,    -1,   112,   113,    -1,    -1,    -1,
     117,   118,   119,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    -1,    58,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    84,    85,    86,    -1,    12,    13,    14,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
      -1,   112,   113,    -1,    -1,    -1,   117,   118,   119,    44,
      45,    -1,    -1,    48,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    84,
      85,    86,    -1,    12,    -1,    14,    15,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,    -1,
      -1,    -1,   117,   118,   119,    44,    45,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    84,    85,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,    -1,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,   112,   113,    -1,    -1,    -1,   117,   118,
     119,     7,     8,    -1,    -1,    11,    12,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    -1,    -1,    12,    13,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    22,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,
      86,    58,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    -1,    -1,    -1,    -1,   104,   105,
      -1,    -1,   108,   109,    -1,    -1,   112,   113,    -1,    86,
      -1,   117,   118,   119,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,   108,   109,    -1,     9,   112,   113,    12,    13,    14,
     117,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     9,    -1,    -1,    12,    -1,    14,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      56,    86,    58,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,    -1,
      86,    -1,   117,   118,   119,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    -1,    -1,    -1,    -1,   104,   105,
      -1,    -1,   108,   109,    -1,     9,   112,   113,    12,    -1,
      14,   117,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     9,    -1,    -1,    12,    -1,    14,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    86,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,
      -1,    86,    -1,   117,   118,   119,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    -1,     9,   112,   113,    12,
      -1,    14,   117,   118,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     9,    -1,    -1,    12,    -1,
      14,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    56,    86,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    -1,    -1,    -1,
      -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,
     113,    -1,    86,    -1,   117,   118,   119,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,     9,   112,   113,
      12,    -1,    14,   117,   118,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,    -1,
      -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
     112,   113,    -1,    -1,    -1,   117,   118,   119,    10,    -1,
      12,    -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
     112,   113,    -1,    -1,    -1,   117,   118,   119,    10,    -1,
      12,    -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
     112,   113,    -1,    -1,    -1,   117,   118,   119,    10,    -1,
      12,    -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
     112,   113,    -1,    -1,    -1,   117,   118,   119,    10,    -1,
      12,    -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
     112,   113,    10,    -1,    12,   117,   118,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    -1,    12,
      -1,    -1,    15,    -1,    -1,    53,    54,    -1,    56,    22,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      53,    54,    -1,    56,    -1,    58,    94,    95,    96,    97,
      98,    99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,   112,   113,    -1,    -1,    -1,   117,
     118,   119,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,    12,   101,    -1,
      -1,   104,   105,    -1,    -1,   108,   109,    22,    -1,   112,
     113,    -1,    -1,    -1,   117,   118,   119,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    -1,    12,    -1,    -1,    15,    -1,    -1,    53,    54,
      -1,    56,    22,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    53,    54,    -1,    56,    -1,    58,    94,
      95,    96,    97,    98,    99,    -1,   101,   102,    -1,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,    -1,
      -1,    -1,   117,   118,   119,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
      12,   101,    -1,    15,   104,   105,    -1,    -1,   108,   109,
      22,    -1,   112,   113,    -1,    -1,    -1,   117,   118,   119,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    12,    -1,    -1,    15,    -1,
      -1,    53,    54,    -1,    56,    22,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    53,    54,    -1,    56,
      -1,    58,    94,    95,    96,    97,    98,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
     112,   113,    -1,    -1,    -1,   117,   118,   119,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,    12,   101,    -1,    15,   104,   105,    -1,
      -1,   108,   109,    22,    -1,   112,   113,    -1,    -1,    -1,
     117,   118,   119,    -1,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    12,    -1,
      -1,    15,    -1,    -1,    53,    54,    -1,    56,    22,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    -1,    -1,    -1,    -1,    -1,    86,    -1,    53,
      54,    -1,    56,    -1,    58,    94,    95,    96,    97,    98,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,   112,   113,    -1,    -1,    -1,   117,   118,
     119,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,   101,    -1,    -1,
     104,   105,    12,    13,   108,   109,    -1,    -1,   112,   113,
      -1,    -1,    22,   117,   118,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    -1,    12,    -1,    -1,
      15,    -1,    -1,    53,    54,    -1,    56,    22,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    -1,    -1,    86,    -1,    53,    54,
      -1,    56,    -1,    58,    94,    95,    96,    97,    98,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,   112,   113,    -1,    -1,    -1,   117,   118,   119,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    12,   101,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    22,    -1,   112,   113,    -1,
      -1,    -1,   117,   118,   119,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      12,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,
      22,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    53,    54,    -1,    56,    -1,    58,    94,    95,    96,
      97,    98,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,   112,   113,    -1,    -1,    -1,
     117,   118,   119,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,    12,    -1,
      -1,    -1,   104,   105,    -1,    -1,   108,   109,    22,    -1,
     112,   113,    -1,    -1,    -1,   117,   118,   119,    -1,    -1,
      -1,    -1,    -1,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    -1,    12,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    56,    22,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    53,    54,    -1,    56,    -1,    58,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,
      -1,    -1,    -1,   117,   118,   119,    -1,    86,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    94,    95,    96,    97,    98,
      99,    -1,    22,    -1,    -1,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,   112,   113,    35,    36,    -1,   117,   118,
     119,    12,    -1,    43,    44,    45,    -1,    -1,    -1,    -1,
      -1,    22,    -1,    53,    54,    -1,    56,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    86,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,   112,   113,    -1,    86,    -1,   117,   118,   119,
      -1,    12,    -1,    94,    95,    96,    97,    98,    99,    -1,
     101,    22,    -1,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,   112,   113,    -1,    35,    -1,   117,   118,   119,    -1,
      -1,    -1,    43,    44,    45,    12,    -1,    14,    15,    -1,
      -1,    -1,    53,    54,    -1,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    53,    54,    -1,    56,
      -1,    58,    -1,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,   112,   113,    -1,    -1,    -1,   117,   118,   119,    86,
      -1,    12,    -1,    14,    15,    -1,    -1,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,    -1,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,   112,   113,    -1,    -1,    -1,
     117,   118,   119,    44,    45,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    22,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    53,    54,    -1,
      56,    -1,    58,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,    -1,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,   112,   113,    -1,    -1,    -1,   117,   118,   119,    -1,
      86,    -1,    -1,    -1,    12,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,    22,    -1,    -1,    -1,   104,   105,
      -1,    -1,   108,   109,    -1,    -1,   112,   113,    -1,    -1,
      -1,   117,   118,   119,    -1,    43,    44,    45,    12,    -1,
      14,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    86,    53,
      54,    -1,    56,    -1,    58,    -1,    94,    95,    96,    97,
      98,    99,    -1,    -1,    -1,    -1,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,   112,   113,    -1,    -1,    -1,   117,
     118,   119,    86,    -1,    12,    -1,    14,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,
      -1,    -1,    -1,   117,   118,   119,    44,    45,    12,    -1,
      14,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    86,    53,
      54,    -1,    56,    -1,    58,    -1,    94,    95,    96,    97,
      98,    99,    -1,    -1,    -1,    -1,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,   112,   113,    -1,    -1,    12,   117,
     118,   119,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,
      44,    45,    -1,   117,   118,   119,    -1,    -1,    -1,    53,
      54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    -1,
      14,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,    12,   112,   113,
      44,    45,    -1,   117,   118,   119,    -1,    -1,    -1,    53,
      54,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    86,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,
      -1,    86,    -1,   117,   118,   119,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,   112,   113,    -1,
      -1,    -1,   117,   118,   119
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     9,    13,   125,   140,   142,   154,     0,     7,
       8,    11,    12,    14,    44,    45,    52,    53,    54,    56,
      58,    86,    94,    95,    96,    97,    98,    99,   104,   105,
     108,   109,   112,   113,   117,   118,   119,   137,   143,   144,
     146,   149,   156,   157,   167,   168,   169,   170,   172,     9,
      13,   137,   137,   149,   150,   158,    12,    94,    97,   135,
      12,    12,    12,    12,    12,    12,    12,   113,   169,   170,
     169,   169,   156,   169,   170,   170,   169,     1,     9,    13,
      46,    47,    48,    49,    50,    51,    55,    59,    84,    85,
     103,   106,   129,   131,   136,   137,   149,   153,   160,   162,
     166,   173,    10,   137,   140,    22,    35,    36,    37,    38,
      39,    40,    41,    42,    43,   101,   126,   127,   169,    12,
      99,    14,   108,   109,   110,   111,   116,   121,   122,   123,
      72,   117,   118,    17,   166,   166,    10,    15,   128,    15,
     128,   100,    15,   147,   149,    12,   149,   149,   147,    15,
     147,   149,   149,   149,     9,   138,   139,    13,   138,   161,
     161,   172,   149,   161,    12,    12,   161,   149,   161,    12,
       9,   163,   162,   166,    12,   148,   151,   152,   156,   169,
     170,   161,    16,   162,   165,   139,   166,   144,   149,   157,
     149,   149,   149,   149,   149,   149,   172,   149,     9,   149,
       9,   149,   149,   157,    72,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   149,   147,    16,    16,     9,   149,
      43,   149,    14,    15,   128,    94,   171,   128,   128,    15,
      15,   128,   128,     9,   139,    17,   161,   141,   160,   172,
     149,   161,   149,   162,    87,   130,    16,   155,   150,    22,
      43,   101,   126,   127,   169,   128,    72,   162,   143,    16,
     102,   128,   128,   169,    18,   172,   149,    15,   128,   159,
     149,   157,   149,   157,   172,   149,   147,    13,    43,   159,
     159,   164,     9,   162,     9,    15,    12,   148,   157,   172,
     148,   148,   148,   148,   148,   137,   149,   149,   149,    94,
       9,   145,    15,    15,    15,    15,    15,   128,    15,   128,
      18,    13,   139,   172,   106,    43,   150,   102,   166,    15,
     128,    15,   128,   137,   149,   157,   149,   139,   149,   159,
      12,   172,    15,   148,    16,   170,   170,   166,    15,    15,
      15,   141,    13,   134,   149,    15,    15,    16,   159,   139,
     162,    15,   133,   141,   161,   162,   159,   132,   162
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   124,   125,   125,   126,   126,   127,   127,   128,   128,
     129,   129,   130,   130,   132,   131,   133,   131,   134,   131,
     135,   135,   136,   137,   137,   138,   138,   139,   139,   140,
     140,   141,   141,   142,   142,   143,   144,   144,   144,   144,
     144,   144,   144,   145,   144,   146,   146,   147,   147,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   150,   150,   151,
     151,   152,   152,   152,   153,   153,   154,   154,   154,   154,
     155,   155,   156,   156,   158,   157,   159,   159,   160,   160,
     160,   160,   160,   161,   161,   162,   162,   163,   164,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
     165,   162,   162,   166,   166,   167,   167,   168,   168,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     170,   170,   170,   170,   171,   171,   171,   172,   172,   172,
     173
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
#line 2325 "awkgram.tab.c"
    break;

  case 3: /* program: error  */
#line 113 "awkgram.y"
                { yyclearin; bracecheck(); SYNTAX("bailing out"); }
#line 2331 "awkgram.tab.c"
    break;

  case 14: /* $@1: %empty  */
#line 137 "awkgram.y"
                                                                                       {inloop++;}
#line 2337 "awkgram.tab.c"
    break;

  case 15: /* for: FOR '(' opt_simple_stmt ';' opt_nl pattern ';' opt_nl opt_simple_stmt rparen $@1 stmt  */
#line 138 "awkgram.y"
                { --inloop; (yyval.p) = stat4(FOR, (yyvsp[-9].p), notnull((yyvsp[-6].p)), (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2343 "awkgram.tab.c"
    break;

  case 16: /* $@2: %empty  */
#line 139 "awkgram.y"
                                                                         {inloop++;}
#line 2349 "awkgram.tab.c"
    break;

  case 17: /* for: FOR '(' opt_simple_stmt ';' ';' opt_nl opt_simple_stmt rparen $@2 stmt  */
#line 140 "awkgram.y"
                { --inloop; (yyval.p) = stat4(FOR, (yyvsp[-7].p), NIL, (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2355 "awkgram.tab.c"
    break;

  case 18: /* $@3: %empty  */
#line 141 "awkgram.y"
                                            {inloop++;}
#line 2361 "awkgram.tab.c"
    break;

  case 19: /* for: FOR '(' varname IN varname rparen $@3 stmt  */
#line 142 "awkgram.y"
                { --inloop; (yyval.p) = stat3(IN, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (yyvsp[0].p)); }
#line 2367 "awkgram.tab.c"
    break;

  case 20: /* funcname: VAR  */
#line 146 "awkgram.y"
                { setfname((yyvsp[0].cp)); }
#line 2373 "awkgram.tab.c"
    break;

  case 21: /* funcname: CALL  */
#line 147 "awkgram.y"
                { setfname((yyvsp[0].cp)); }
#line 2379 "awkgram.tab.c"
    break;

  case 22: /* if: IF '(' pattern rparen  */
#line 151 "awkgram.y"
                                        { (yyval.p) = notnull((yyvsp[-1].p)); }
#line 2385 "awkgram.tab.c"
    break;

  case 27: /* opt_nl: %empty  */
#line 163 "awkgram.y"
                        { (yyval.i) = 0; }
#line 2391 "awkgram.tab.c"
    break;

  case 29: /* opt_pst: %empty  */
#line 168 "awkgram.y"
                        { (yyval.i) = 0; }
#line 2397 "awkgram.tab.c"
    break;

  case 31: /* opt_simple_stmt: %empty  */
#line 174 "awkgram.y"
                                        { (yyval.p) = 0; }
#line 2403 "awkgram.tab.c"
    break;

  case 33: /* pas: opt_pst  */
#line 179 "awkgram.y"
                                        { (yyval.p) = 0; }
#line 2409 "awkgram.tab.c"
    break;

  case 34: /* pas: opt_pst pa_stats opt_pst  */
#line 180 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2415 "awkgram.tab.c"
    break;

  case 35: /* pa_pat: pattern  */
#line 184 "awkgram.y"
                        { (yyval.p) = notnull((yyvsp[0].p)); }
#line 2421 "awkgram.tab.c"
    break;

  case 36: /* pa_stat: pa_pat  */
#line 188 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, (yyvsp[0].p), stat2(PRINT, rectonode(), NIL)); }
#line 2427 "awkgram.tab.c"
    break;

  case 37: /* pa_stat: pa_pat lbrace stmtlist '}'  */
#line 189 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2433 "awkgram.tab.c"
    break;

  case 38: /* pa_stat: pa_pat ',' opt_nl pa_pat  */
#line 190 "awkgram.y"
                                                { (yyval.p) = pa2stat((yyvsp[-3].p), (yyvsp[0].p), stat2(PRINT, rectonode(), NIL)); }
#line 2439 "awkgram.tab.c"
    break;

  case 39: /* pa_stat: pa_pat ',' opt_nl pa_pat lbrace stmtlist '}'  */
#line 191 "awkgram.y"
                                                        { (yyval.p) = pa2stat((yyvsp[-6].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2445 "awkgram.tab.c"
    break;

  case 40: /* pa_stat: lbrace stmtlist '}'  */
#line 192 "awkgram.y"
                                        { (yyval.p) = stat2(PASTAT, NIL, (yyvsp[-1].p)); }
#line 2451 "awkgram.tab.c"
    break;

  case 41: /* pa_stat: XBEGIN lbrace stmtlist '}'  */
#line 194 "awkgram.y"
                { beginloc = linkum(beginloc, (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2457 "awkgram.tab.c"
    break;

  case 42: /* pa_stat: XEND lbrace stmtlist '}'  */
#line 196 "awkgram.y"
                { endloc = linkum(endloc, (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2463 "awkgram.tab.c"
    break;

  case 43: /* $@4: %empty  */
#line 197 "awkgram.y"
                                           {infunc = true;}
#line 2469 "awkgram.tab.c"
    break;

  case 44: /* pa_stat: FUNC funcname '(' varlist rparen $@4 lbrace stmtlist '}'  */
#line 198 "awkgram.y"
                { infunc = false; curfname=0; defn((Cell *)(yyvsp[-7].p), (yyvsp[-5].p), (yyvsp[-1].p)); (yyval.p) = 0; }
#line 2475 "awkgram.tab.c"
    break;

  case 46: /* pa_stats: pa_stats opt_pst pa_stat  */
#line 203 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2481 "awkgram.tab.c"
    break;

  case 48: /* patlist: patlist comma pattern  */
#line 208 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2487 "awkgram.tab.c"
    break;

  case 49: /* ppattern: var ASGNOP ppattern  */
#line 212 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2493 "awkgram.tab.c"
    break;

  case 50: /* ppattern: ppattern '?' ppattern ':' ppattern  */
#line 214 "awkgram.y"
                { (yyval.p) = op3(CONDEXPR, notnull((yyvsp[-4].p)), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2499 "awkgram.tab.c"
    break;

  case 51: /* ppattern: ppattern lor ppattern  */
#line 216 "awkgram.y"
                { (yyval.p) = op2(LOR, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2505 "awkgram.tab.c"
    break;

  case 52: /* ppattern: ppattern land ppattern  */
#line 218 "awkgram.y"
                { (yyval.p) = op2(LAND, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2511 "awkgram.tab.c"
    break;

  case 53: /* ppattern: ppattern MATCHOP reg_expr  */
#line 219 "awkgram.y"
                                        { (yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2517 "awkgram.tab.c"
    break;

  case 54: /* ppattern: ppattern MATCHOP ppattern  */
#line 221 "awkgram.y"
                { if (constnode((yyvsp[0].p))) {
			(yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa(strnode((yyvsp[0].p)), 0));
			free((yyvsp[0].p));
		  } else
			(yyval.p) = op3((yyvsp[-1].i), (Node *)1, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2527 "awkgram.tab.c"
    break;

  case 55: /* ppattern: ppattern IN varname  */
#line 226 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-2].p), makearr((yyvsp[0].p))); }
#line 2533 "awkgram.tab.c"
    break;

  case 56: /* ppattern: '(' plist ')' IN varname  */
#line 227 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-3].p), makearr((yyvsp[0].p))); }
#line 2539 "awkgram.tab.c"
    break;

  case 57: /* ppattern: ppattern term  */
#line 228 "awkgram.y"
                                        { (yyval.p) = op2(CAT, (yyvsp[-1].p), (yyvsp[0].p)); }
#line 2545 "awkgram.tab.c"
    break;

  case 60: /* pattern: var ASGNOP pattern  */
#line 234 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2551 "awkgram.tab.c"
    break;

  case 61: /* pattern: pattern '?' pattern ':' pattern  */
#line 236 "awkgram.y"
                { (yyval.p) = op3(CONDEXPR, notnull((yyvsp[-4].p)), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2557 "awkgram.tab.c"
    break;

  case 62: /* pattern: pattern lor pattern  */
#line 238 "awkgram.y"
                { (yyval.p) = op2(LOR, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2563 "awkgram.tab.c"
    break;

  case 63: /* pattern: pattern land pattern  */
#line 240 "awkgram.y"
                { (yyval.p) = op2(LAND, notnull((yyvsp[-2].p)), notnull((yyvsp[0].p))); }
#line 2569 "awkgram.tab.c"
    break;

  case 64: /* pattern: pattern EQ pattern  */
#line 241 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2575 "awkgram.tab.c"
    break;

  case 65: /* pattern: pattern GE pattern  */
#line 242 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2581 "awkgram.tab.c"
    break;

  case 66: /* pattern: pattern GT pattern  */
#line 243 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2587 "awkgram.tab.c"
    break;

  case 67: /* pattern: pattern LE pattern  */
#line 244 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2593 "awkgram.tab.c"
    break;

  case 68: /* pattern: pattern LT pattern  */
#line 245 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2599 "awkgram.tab.c"
    break;

  case 69: /* pattern: pattern NE pattern  */
#line 246 "awkgram.y"
                                        { (yyval.p) = op2((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2605 "awkgram.tab.c"
    break;

  case 70: /* pattern: pattern MATCHOP reg_expr  */
#line 247 "awkgram.y"
                                        { (yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2611 "awkgram.tab.c"
    break;

  case 71: /* pattern: pattern MATCHOP pattern  */
#line 249 "awkgram.y"
                { if (constnode((yyvsp[0].p))) {
			(yyval.p) = op3((yyvsp[-1].i), NIL, (yyvsp[-2].p), (Node*)makedfa(strnode((yyvsp[0].p)), 0));
			free((yyvsp[0].p));
		  } else
			(yyval.p) = op3((yyvsp[-1].i), (Node *)1, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2621 "awkgram.tab.c"
    break;

  case 72: /* pattern: pattern IN varname  */
#line 254 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-2].p), makearr((yyvsp[0].p))); }
#line 2627 "awkgram.tab.c"
    break;

  case 73: /* pattern: '(' plist ')' IN varname  */
#line 255 "awkgram.y"
                                        { (yyval.p) = op2(INTEST, (yyvsp[-3].p), makearr((yyvsp[0].p))); }
#line 2633 "awkgram.tab.c"
    break;

  case 74: /* pattern: pattern term  */
#line 256 "awkgram.y"
                                        { (yyval.p) = op2(CAT, (yyvsp[-1].p), (yyvsp[0].p)); }
#line 2639 "awkgram.tab.c"
    break;

  case 77: /* plist: pattern comma pattern  */
#line 262 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2645 "awkgram.tab.c"
    break;

  case 78: /* plist: plist comma pattern  */
#line 263 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2651 "awkgram.tab.c"
    break;

  case 80: /* pplist: pplist comma ppattern  */
#line 268 "awkgram.y"
                                        { (yyval.p) = linkum((yyvsp[-2].p), (yyvsp[0].p)); }
#line 2657 "awkgram.tab.c"
    break;

  case 81: /* prarg: %empty  */
#line 272 "awkgram.y"
                                        { (yyval.p) = rectonode(); }
#line 2663 "awkgram.tab.c"
    break;

  case 83: /* prarg: '(' plist ')'  */
#line 274 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2669 "awkgram.tab.c"
    break;

  case 92: /* re: reg_expr  */
#line 291 "awkgram.y"
                { (yyval.p) = op3(MATCH, NIL, rectonode(), (Node*)makedfa((yyvsp[0].s), 0)); free((yyvsp[0].s)); }
#line 2675 "awkgram.tab.c"
    break;

  case 93: /* re: NOT re  */
#line 292 "awkgram.y"
                        { (yyval.p) = op1(NOT, notnull((yyvsp[0].p))); }
#line 2681 "awkgram.tab.c"
    break;

  case 94: /* $@5: %empty  */
#line 296 "awkgram.y"
              {startreg();}
#line 2687 "awkgram.tab.c"
    break;

  case 95: /* reg_expr: '/' $@5 REGEXPR '/'  */
#line 296 "awkgram.y"
                                                { (yyval.s) = (yyvsp[-1].s); }
#line 2693 "awkgram.tab.c"
    break;

  case 98: /* simple_stmt: print prarg  */
#line 304 "awkgram.y"
                                        { (yyval.p) = stat3((yyvsp[-1].i), (yyvsp[0].p), NIL, NIL); }
#line 2699 "awkgram.tab.c"
    break;

  case 99: /* simple_stmt: DELETE varname '[' patlist ']'  */
#line 305 "awkgram.y"
                                         { (yyval.p) = stat2(DELETE, makearr((yyvsp[-3].p)), (yyvsp[-1].p)); }
#line 2705 "awkgram.tab.c"
    break;

  case 100: /* simple_stmt: DELETE varname  */
#line 306 "awkgram.y"
                                         { (yyval.p) = stat2(DELETE, makearr((yyvsp[0].p)), 0); }
#line 2711 "awkgram.tab.c"
    break;

  case 101: /* simple_stmt: pattern  */
#line 307 "awkgram.y"
                                        { (yyval.p) = exptostat((yyvsp[0].p)); }
#line 2717 "awkgram.tab.c"
    break;

  case 102: /* simple_stmt: error  */
#line 308 "awkgram.y"
                                        { yyclearin; SYNTAX("illegal statement"); }
#line 2723 "awkgram.tab.c"
    break;

  case 105: /* stmt: BREAK st  */
#line 317 "awkgram.y"
                                { if (!inloop) SYNTAX("break illegal outside of loops");
				  (yyval.p) = stat1(BREAK, NIL); }
#line 2730 "awkgram.tab.c"
    break;

  case 106: /* stmt: CONTINUE st  */
#line 319 "awkgram.y"
                                {  if (!inloop) SYNTAX("continue illegal outside of loops");
				  (yyval.p) = stat1(CONTINUE, NIL); }
#line 2737 "awkgram.tab.c"
    break;

  case 107: /* $@6: %empty  */
#line 321 "awkgram.y"
             {inloop++;}
#line 2743 "awkgram.tab.c"
    break;

  case 108: /* $@7: %empty  */
#line 321 "awkgram.y"
                              {--inloop;}
#line 2749 "awkgram.tab.c"
    break;

  case 109: /* stmt: do $@6 stmt $@7 WHILE '(' pattern ')' st  */
#line 322 "awkgram.y"
                { (yyval.p) = stat2(DO, (yyvsp[-6].p), notnull((yyvsp[-2].p))); }
#line 2755 "awkgram.tab.c"
    break;

  case 110: /* stmt: EXIT pattern st  */
#line 323 "awkgram.y"
                                { (yyval.p) = stat1(EXIT, (yyvsp[-1].p)); }
#line 2761 "awkgram.tab.c"
    break;

  case 111: /* stmt: EXIT st  */
#line 324 "awkgram.y"
                                { (yyval.p) = stat1(EXIT, NIL); }
#line 2767 "awkgram.tab.c"
    break;

  case 113: /* stmt: if stmt else stmt  */
#line 326 "awkgram.y"
                                { (yyval.p) = stat3(IF, (yyvsp[-3].p), (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2773 "awkgram.tab.c"
    break;

  case 114: /* stmt: if stmt  */
#line 327 "awkgram.y"
                                { (yyval.p) = stat3(IF, (yyvsp[-1].p), (yyvsp[0].p), NIL); }
#line 2779 "awkgram.tab.c"
    break;

  case 115: /* stmt: lbrace stmtlist rbrace  */
#line 328 "awkgram.y"
                                 { (yyval.p) = (yyvsp[-1].p); }
#line 2785 "awkgram.tab.c"
    break;

  case 116: /* stmt: NEXT st  */
#line 329 "awkgram.y"
                        { if (infunc)
				SYNTAX("next is illegal inside a function");
			  (yyval.p) = stat1(NEXT, NIL); }
#line 2793 "awkgram.tab.c"
    break;

  case 117: /* stmt: RETURN pattern st  */
#line 332 "awkgram.y"
                                { (yyval.p) = stat1(RETURN, (yyvsp[-1].p)); }
#line 2799 "awkgram.tab.c"
    break;

  case 118: /* stmt: RETURN st  */
#line 333 "awkgram.y"
                                { (yyval.p) = stat1(RETURN, NIL); }
#line 2805 "awkgram.tab.c"
    break;

  case 120: /* $@8: %empty  */
#line 335 "awkgram.y"
                {inloop++;}
#line 2811 "awkgram.tab.c"
    break;

  case 121: /* stmt: while $@8 stmt  */
#line 335 "awkgram.y"
                                        { --inloop; (yyval.p) = stat2(WHILE, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2817 "awkgram.tab.c"
    break;

  case 122: /* stmt: ';' opt_nl  */
#line 336 "awkgram.y"
                                { (yyval.p) = 0; }
#line 2823 "awkgram.tab.c"
    break;

  case 124: /* stmtlist: stmtlist stmt  */
#line 341 "awkgram.y"
                                { (yyval.p) = linkum((yyvsp[-1].p), (yyvsp[0].p)); }
#line 2829 "awkgram.tab.c"
    break;

  case 128: /* string: string STRING  */
#line 350 "awkgram.y"
                                { (yyval.cp) = catstr((yyvsp[-1].cp), (yyvsp[0].cp)); }
#line 2835 "awkgram.tab.c"
    break;

  case 129: /* term: term '/' ASGNOP term  */
#line 354 "awkgram.y"
                                        { (yyval.p) = op2(DIVEQ, (yyvsp[-3].p), (yyvsp[0].p)); }
#line 2841 "awkgram.tab.c"
    break;

  case 130: /* term: term '+' term  */
#line 355 "awkgram.y"
                                        { (yyval.p) = op2(ADD, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2847 "awkgram.tab.c"
    break;

  case 131: /* term: term '-' term  */
#line 356 "awkgram.y"
                                        { (yyval.p) = op2(MINUS, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2853 "awkgram.tab.c"
    break;

  case 132: /* term: term '*' term  */
#line 357 "awkgram.y"
                                        { (yyval.p) = op2(MULT, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2859 "awkgram.tab.c"
    break;

  case 133: /* term: term '/' term  */
#line 358 "awkgram.y"
                                        { (yyval.p) = op2(DIVIDE, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2865 "awkgram.tab.c"
    break;

  case 134: /* term: term '%' term  */
#line 359 "awkgram.y"
                                        { (yyval.p) = op2(MOD, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2871 "awkgram.tab.c"
    break;

  case 135: /* term: term '^' term  */
#line 360 "awkgram.y"
                                        { (yyval.p) = op2(XOR, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2877 "awkgram.tab.c"
    break;

  case 136: /* term: term '|' term  */
#line 361 "awkgram.y"
                                        { (yyval.p) = op2(BOR, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2883 "awkgram.tab.c"
    break;

  case 137: /* term: term '&' term  */
#line 362 "awkgram.y"
                                        { (yyval.p) = op2(BAND, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2889 "awkgram.tab.c"
    break;

  case 138: /* term: term POWER term  */
#line 363 "awkgram.y"
                                        { (yyval.p) = op2(POWER, (yyvsp[-2].p), (yyvsp[0].p)); }
#line 2895 "awkgram.tab.c"
    break;

  case 139: /* term: CMPL term  */
#line 364 "awkgram.y"
                                        { (yyval.p) = op1(CMPL, (yyvsp[0].p)); }
#line 2901 "awkgram.tab.c"
    break;

  case 140: /* term: '-' term  */
#line 365 "awkgram.y"
                                        { (yyval.p) = op1(UMINUS, (yyvsp[0].p)); }
#line 2907 "awkgram.tab.c"
    break;

  case 141: /* term: '+' term  */
#line 366 "awkgram.y"
                                        { (yyval.p) = op1(UPLUS, (yyvsp[0].p)); }
#line 2913 "awkgram.tab.c"
    break;

  case 142: /* term: NOT term  */
#line 367 "awkgram.y"
                                        { (yyval.p) = op1(NOT, notnull((yyvsp[0].p))); }
#line 2919 "awkgram.tab.c"
    break;

  case 143: /* term: BLTIN '(' ')'  */
#line 368 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[-2].i)), rectonode()); }
#line 2925 "awkgram.tab.c"
    break;

  case 144: /* term: BLTIN '(' patlist ')'  */
#line 369 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[-3].i)), (yyvsp[-1].p)); }
#line 2931 "awkgram.tab.c"
    break;

  case 145: /* term: BLTIN  */
#line 370 "awkgram.y"
                                        { (yyval.p) = op2(BLTIN, itonp((yyvsp[0].i)), rectonode()); }
#line 2937 "awkgram.tab.c"
    break;

  case 146: /* term: CALL '(' ')'  */
#line 371 "awkgram.y"
                                        { (yyval.p) = op2(CALL, celltonode((yyvsp[-2].cp),CVAR), NIL); }
#line 2943 "awkgram.tab.c"
    break;

  case 147: /* term: CALL '(' patlist ')'  */
#line 372 "awkgram.y"
                                        { (yyval.p) = op2(CALL, celltonode((yyvsp[-3].cp),CVAR), (yyvsp[-1].p)); }
#line 2949 "awkgram.tab.c"
    break;

  case 148: /* term: DECR var  */
#line 373 "awkgram.y"
                                        { (yyval.p) = op1(PREDECR, (yyvsp[0].p)); }
#line 2955 "awkgram.tab.c"
    break;

  case 149: /* term: INCR var  */
#line 374 "awkgram.y"
                                        { (yyval.p) = op1(PREINCR, (yyvsp[0].p)); }
#line 2961 "awkgram.tab.c"
    break;

  case 150: /* term: var DECR  */
#line 375 "awkgram.y"
                                        { (yyval.p) = op1(POSTDECR, (yyvsp[-1].p)); }
#line 2967 "awkgram.tab.c"
    break;

  case 151: /* term: var INCR  */
#line 376 "awkgram.y"
                                        { (yyval.p) = op1(POSTINCR, (yyvsp[-1].p)); }
#line 2973 "awkgram.tab.c"
    break;

  case 152: /* term: INDEX '(' pattern comma pattern ')'  */
#line 378 "awkgram.y"
                { (yyval.p) = op2(INDEX, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 2979 "awkgram.tab.c"
    break;

  case 153: /* term: INDEX '(' pattern comma reg_expr ')'  */
#line 380 "awkgram.y"
                { SYNTAX("index() doesn't permit regular expressions");
		  (yyval.p) = op2(INDEX, (yyvsp[-3].p), (Node*)(yyvsp[-1].s)); }
#line 2986 "awkgram.tab.c"
    break;

  case 154: /* term: '(' pattern ')'  */
#line 382 "awkgram.y"
                                        { (yyval.p) = (yyvsp[-1].p); }
#line 2992 "awkgram.tab.c"
    break;

  case 155: /* term: MATCHFCN '(' pattern comma reg_expr ')'  */
#line 384 "awkgram.y"
                { (yyval.p) = op3(MATCHFCN, NIL, (yyvsp[-3].p), (Node*)makedfa((yyvsp[-1].s), 1)); free((yyvsp[-1].s)); }
#line 2998 "awkgram.tab.c"
    break;

  case 156: /* term: MATCHFCN '(' pattern comma pattern ')'  */
#line 386 "awkgram.y"
                { if (constnode((yyvsp[-1].p))) {
			(yyval.p) = op3(MATCHFCN, NIL, (yyvsp[-3].p), (Node*)makedfa(strnode((yyvsp[-1].p)), 1));
			free((yyvsp[-1].p));
		  } else
			(yyval.p) = op3(MATCHFCN, (Node *)1, (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3008 "awkgram.tab.c"
    break;

  case 157: /* term: NUMBER  */
#line 391 "awkgram.y"
                                        { (yyval.p) = celltonode((yyvsp[0].cp), CCON); }
#line 3014 "awkgram.tab.c"
    break;

  case 158: /* term: SPLIT '(' pattern comma varname comma pattern ')'  */
#line 393 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (yyvsp[-1].p), (Node*)STRING); }
#line 3020 "awkgram.tab.c"
    break;

  case 159: /* term: SPLIT '(' pattern comma varname comma reg_expr ')'  */
#line 395 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-5].p), makearr((yyvsp[-3].p)), (Node*)makedfa((yyvsp[-1].s), 1), (Node *)REGEXPR); free((yyvsp[-1].s)); }
#line 3026 "awkgram.tab.c"
    break;

  case 160: /* term: SPLIT '(' pattern comma varname ')'  */
#line 397 "awkgram.y"
                { (yyval.p) = op4(SPLIT, (yyvsp[-3].p), makearr((yyvsp[-1].p)), NIL, (Node*)STRING); }
#line 3032 "awkgram.tab.c"
    break;

  case 161: /* term: SPRINTF '(' patlist ')'  */
#line 398 "awkgram.y"
                                        { (yyval.p) = op1((yyvsp[-3].i), (yyvsp[-1].p)); }
#line 3038 "awkgram.tab.c"
    break;

  case 162: /* term: string  */
#line 399 "awkgram.y"
                                        { (yyval.p) = celltonode((yyvsp[0].cp), CCON); }
#line 3044 "awkgram.tab.c"
    break;

  case 163: /* term: subop '(' reg_expr comma pattern ')'  */
#line 401 "awkgram.y"
                { (yyval.p) = op4((yyvsp[-5].i), NIL, (Node*)makedfa((yyvsp[-3].s), 1), (yyvsp[-1].p), rectonode()); free((yyvsp[-3].s)); }
#line 3050 "awkgram.tab.c"
    break;

  case 164: /* term: subop '(' pattern comma pattern ')'  */
#line 403 "awkgram.y"
                { if (constnode((yyvsp[-3].p))) {
			(yyval.p) = op4((yyvsp[-5].i), NIL, (Node*)makedfa(strnode((yyvsp[-3].p)), 1), (yyvsp[-1].p), rectonode());
			free((yyvsp[-3].p));
		  } else
			(yyval.p) = op4((yyvsp[-5].i), (Node *)1, (yyvsp[-3].p), (yyvsp[-1].p), rectonode()); }
#line 3060 "awkgram.tab.c"
    break;

  case 165: /* term: subop '(' reg_expr comma pattern comma var ')'  */
#line 409 "awkgram.y"
                { (yyval.p) = op4((yyvsp[-7].i), NIL, (Node*)makedfa((yyvsp[-5].s), 1), (yyvsp[-3].p), (yyvsp[-1].p)); free((yyvsp[-5].s)); }
#line 3066 "awkgram.tab.c"
    break;

  case 166: /* term: subop '(' pattern comma pattern comma var ')'  */
#line 411 "awkgram.y"
                { if (constnode((yyvsp[-5].p))) {
			(yyval.p) = op4((yyvsp[-7].i), NIL, (Node*)makedfa(strnode((yyvsp[-5].p)), 1), (yyvsp[-3].p), (yyvsp[-1].p));
			free((yyvsp[-5].p));
		  } else
			(yyval.p) = op4((yyvsp[-7].i), (Node *)1, (yyvsp[-5].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3076 "awkgram.tab.c"
    break;

  case 167: /* term: SUBSTR '(' pattern comma pattern comma pattern ')'  */
#line 417 "awkgram.y"
                { (yyval.p) = op3(SUBSTR, (yyvsp[-5].p), (yyvsp[-3].p), (yyvsp[-1].p)); }
#line 3082 "awkgram.tab.c"
    break;

  case 168: /* term: SUBSTR '(' pattern comma pattern ')'  */
#line 419 "awkgram.y"
                { (yyval.p) = op3(SUBSTR, (yyvsp[-3].p), (yyvsp[-1].p), NIL); }
#line 3088 "awkgram.tab.c"
    break;

  case 171: /* var: varname '[' patlist ']'  */
#line 425 "awkgram.y"
                                        { (yyval.p) = op2(ARRAY, makearr((yyvsp[-3].p)), (yyvsp[-1].p)); }
#line 3094 "awkgram.tab.c"
    break;

  case 172: /* var: IVAR  */
#line 426 "awkgram.y"
                                        { (yyval.p) = op1(INDIRECT, celltonode((yyvsp[0].cp), CVAR)); }
#line 3100 "awkgram.tab.c"
    break;

  case 173: /* var: INDIRECT term  */
#line 427 "awkgram.y"
                                        { (yyval.p) = op1(INDIRECT, (yyvsp[0].p)); }
#line 3106 "awkgram.tab.c"
    break;

  case 174: /* varlist: %empty  */
#line 431 "awkgram.y"
                                { arglist = (yyval.p) = 0; }
#line 3112 "awkgram.tab.c"
    break;

  case 175: /* varlist: VAR  */
#line 432 "awkgram.y"
                                { arglist = (yyval.p) = celltonode((yyvsp[0].cp),CVAR); }
#line 3118 "awkgram.tab.c"
    break;

  case 176: /* varlist: varlist comma VAR  */
#line 433 "awkgram.y"
                                {
			checkdup((yyvsp[-2].p), (yyvsp[0].cp));
			arglist = (yyval.p) = linkum((yyvsp[-2].p),celltonode((yyvsp[0].cp),CVAR)); }
#line 3126 "awkgram.tab.c"
    break;

  case 177: /* varname: VAR  */
#line 439 "awkgram.y"
                                { (yyval.p) = celltonode((yyvsp[0].cp), CVAR); }
#line 3132 "awkgram.tab.c"
    break;

  case 178: /* varname: ARG  */
#line 440 "awkgram.y"
                                { (yyval.p) = op1(ARG, itonp((yyvsp[0].i))); }
#line 3138 "awkgram.tab.c"
    break;

  case 179: /* varname: VARNF  */
#line 441 "awkgram.y"
                                { (yyval.p) = op1(VARNF, (Node *) (yyvsp[0].cp)); }
#line 3144 "awkgram.tab.c"
    break;

  case 180: /* while: WHILE '(' pattern rparen  */
#line 446 "awkgram.y"
                                        { (yyval.p) = notnull((yyvsp[-1].p)); }
#line 3150 "awkgram.tab.c"
    break;


#line 3154 "awkgram.tab.c"

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
