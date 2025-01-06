/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_AWKGRAM_TAB_H_INCLUDED
# define YY_YY_AWKGRAM_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    FIRSTTOKEN = 258,              /* FIRSTTOKEN  */
    PROGRAM = 259,                 /* PROGRAM  */
    PASTAT = 260,                  /* PASTAT  */
    PASTAT2 = 261,                 /* PASTAT2  */
    XBEGIN = 262,                  /* XBEGIN  */
    XEND = 263,                    /* XEND  */
    NL = 264,                      /* NL  */
    ARRAY = 265,                   /* ARRAY  */
    MATCH = 266,                   /* MATCH  */
    NOTMATCH = 267,                /* NOTMATCH  */
    MATCHOP = 268,                 /* MATCHOP  */
    FINAL = 269,                   /* FINAL  */
    DOT = 270,                     /* DOT  */
    ALL = 271,                     /* ALL  */
    CCL = 272,                     /* CCL  */
    NCCL = 273,                    /* NCCL  */
    CHAR = 274,                    /* CHAR  */
    OR = 275,                      /* OR  */
    STAR = 276,                    /* STAR  */
    QUEST = 277,                   /* QUEST  */
    PLUS = 278,                    /* PLUS  */
    EMPTYRE = 279,                 /* EMPTYRE  */
    ZERO = 280,                    /* ZERO  */
    LAND = 281,                    /* LAND  */
    LOR = 282,                     /* LOR  */
    EQ = 283,                      /* EQ  */
    GE = 284,                      /* GE  */
    GT = 285,                      /* GT  */
    LE = 286,                      /* LE  */
    LT = 287,                      /* LT  */
    NE = 288,                      /* NE  */
    IN = 289,                      /* IN  */
    ADDON = 290,                   /* ADDON  */
    ARG = 291,                     /* ARG  */
    BLTIN = 292,                   /* BLTIN  */
    BREAK = 293,                   /* BREAK  */
    CONTINUE = 294,                /* CONTINUE  */
    DELETE = 295,                  /* DELETE  */
    DO = 296,                      /* DO  */
    EXIT = 297,                    /* EXIT  */
    FOR = 298,                     /* FOR  */
    FUNC = 299,                    /* FUNC  */
    SUB = 300,                     /* SUB  */
    GSUB = 301,                    /* GSUB  */
    IF = 302,                      /* IF  */
    INDEX = 303,                   /* INDEX  */
    LSUBSTR = 304,                 /* LSUBSTR  */
    MATCHFCN = 305,                /* MATCHFCN  */
    NEXT = 306,                    /* NEXT  */
    ADD = 307,                     /* ADD  */
    MINUS = 308,                   /* MINUS  */
    MULT = 309,                    /* MULT  */
    DIVIDE = 310,                  /* DIVIDE  */
    MOD = 311,                     /* MOD  */
    LSHIFT = 312,                  /* LSHIFT  */
    RSHIFT = 313,                  /* RSHIFT  */
    XOR = 314,                     /* XOR  */
    BAND = 315,                    /* BAND  */
    BOR = 316,                     /* BOR  */
    CMPL = 317,                    /* CMPL  */
    ASSIGN = 318,                  /* ASSIGN  */
    ASGNOP = 319,                  /* ASGNOP  */
    ADDEQ = 320,                   /* ADDEQ  */
    SUBEQ = 321,                   /* SUBEQ  */
    MULTEQ = 322,                  /* MULTEQ  */
    DIVEQ = 323,                   /* DIVEQ  */
    MODEQ = 324,                   /* MODEQ  */
    POWEQ = 325,                   /* POWEQ  */
    BANDEQ = 326,                  /* BANDEQ  */
    BOREQ = 327,                   /* BOREQ  */
    XOREQ = 328,                   /* XOREQ  */
    SHLEQ = 329,                   /* SHLEQ  */
    SHREQ = 330,                   /* SHREQ  */
    PRINT = 331,                   /* PRINT  */
    PRINTF = 332,                  /* PRINTF  */
    SPRINTF = 333,                 /* SPRINTF  */
    ELSE = 334,                    /* ELSE  */
    INTEST = 335,                  /* INTEST  */
    CONDEXPR = 336,                /* CONDEXPR  */
    POSTINCR = 337,                /* POSTINCR  */
    PREINCR = 338,                 /* PREINCR  */
    POSTDECR = 339,                /* POSTDECR  */
    PREDECR = 340,                 /* PREDECR  */
    VAR = 341,                     /* VAR  */
    IVAR = 342,                    /* IVAR  */
    VARNF = 343,                   /* VARNF  */
    CALL = 344,                    /* CALL  */
    NUMBER = 345,                  /* NUMBER  */
    STRING = 346,                  /* STRING  */
    REGEXPR = 347,                 /* REGEXPR  */
    RETURN = 348,                  /* RETURN  */
    SPLIT = 349,                   /* SPLIT  */
    SUBSTR = 350,                  /* SUBSTR  */
    WHILE = 351,                   /* WHILE  */
    CAT = 352,                     /* CAT  */
    NOT = 353,                     /* NOT  */
    UMINUS = 354,                  /* UMINUS  */
    UPLUS = 355,                   /* UPLUS  */
    POWER = 356,                   /* POWER  */
    DECR = 357,                    /* DECR  */
    INCR = 358,                    /* INCR  */
    INDIRECT = 359,                /* INDIRECT  */
    LASTTOKEN = 360                /* LASTTOKEN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 42 "awkgram.y"

	TNode	*p;
	Cell	*cp;
	int	i;
	char	*s;

#line 176 "awkgram.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_AWKGRAM_TAB_H_INCLUDED  */
