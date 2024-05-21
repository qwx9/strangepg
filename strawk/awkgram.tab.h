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
    ARG = 290,                     /* ARG  */
    BLTIN = 291,                   /* BLTIN  */
    BREAK = 292,                   /* BREAK  */
    CONTINUE = 293,                /* CONTINUE  */
    DELETE = 294,                  /* DELETE  */
    DO = 295,                      /* DO  */
    EXIT = 296,                    /* EXIT  */
    FOR = 297,                     /* FOR  */
    FUNC = 298,                    /* FUNC  */
    SUB = 299,                     /* SUB  */
    GSUB = 300,                    /* GSUB  */
    IF = 301,                      /* IF  */
    INDEX = 302,                   /* INDEX  */
    LSUBSTR = 303,                 /* LSUBSTR  */
    MATCHFCN = 304,                /* MATCHFCN  */
    NEXT = 305,                    /* NEXT  */
    ADD = 306,                     /* ADD  */
    MINUS = 307,                   /* MINUS  */
    MULT = 308,                    /* MULT  */
    DIVIDE = 309,                  /* DIVIDE  */
    MOD = 310,                     /* MOD  */
    LSHIFT = 311,                  /* LSHIFT  */
    RSHIFT = 312,                  /* RSHIFT  */
    XOR = 313,                     /* XOR  */
    BAND = 314,                    /* BAND  */
    BOR = 315,                     /* BOR  */
    CMPL = 316,                    /* CMPL  */
    ASSIGN = 317,                  /* ASSIGN  */
    ASGNOP = 318,                  /* ASGNOP  */
    ADDEQ = 319,                   /* ADDEQ  */
    SUBEQ = 320,                   /* SUBEQ  */
    MULTEQ = 321,                  /* MULTEQ  */
    DIVEQ = 322,                   /* DIVEQ  */
    MODEQ = 323,                   /* MODEQ  */
    POWEQ = 324,                   /* POWEQ  */
    BANDEQ = 325,                  /* BANDEQ  */
    BOREQ = 326,                   /* BOREQ  */
    XOREQ = 327,                   /* XOREQ  */
    SHLEQ = 328,                   /* SHLEQ  */
    SHREQ = 329,                   /* SHREQ  */
    PRINT = 330,                   /* PRINT  */
    PRINTF = 331,                  /* PRINTF  */
    SPRINTF = 332,                 /* SPRINTF  */
    ELSE = 333,                    /* ELSE  */
    INTEST = 334,                  /* INTEST  */
    CONDEXPR = 335,                /* CONDEXPR  */
    POSTINCR = 336,                /* POSTINCR  */
    PREINCR = 337,                 /* PREINCR  */
    POSTDECR = 338,                /* POSTDECR  */
    PREDECR = 339,                 /* PREDECR  */
    VAR = 340,                     /* VAR  */
    IVAR = 341,                    /* IVAR  */
    VARNF = 342,                   /* VARNF  */
    CALL = 343,                    /* CALL  */
    NUMBER = 344,                  /* NUMBER  */
    STRING = 345,                  /* STRING  */
    REGEXPR = 346,                 /* REGEXPR  */
    RETURN = 347,                  /* RETURN  */
    SPLIT = 348,                   /* SPLIT  */
    SUBSTR = 349,                  /* SUBSTR  */
    WHILE = 350,                   /* WHILE  */
    CAT = 351,                     /* CAT  */
    NOT = 352,                     /* NOT  */
    UMINUS = 353,                  /* UMINUS  */
    UPLUS = 354,                   /* UPLUS  */
    POWER = 355,                   /* POWER  */
    DECR = 356,                    /* DECR  */
    INCR = 357,                    /* INCR  */
    INDIRECT = 358,                /* INDIRECT  */
    LASTTOKEN = 359                /* LASTTOKEN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 42 "awkgram.y"

	Node	*p;
	Cell	*cp;
	int	i;
	char	*s;

#line 175 "awkgram.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_AWKGRAM_TAB_H_INCLUDED  */
