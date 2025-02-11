/****************************************************************
Copyright (C) Lucent Technologies 1997
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name Lucent Technologies or any of
its entities not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#if __STDC_VERSION__ <= 199901L
#define noreturn
#else
#include <stdnoreturn.h>
#endif

typedef double	Awkfloat;
typedef long long int	Awknum;
typedef unsigned long long int Awkword;
typedef char *Awkstr;
union Value{
	Awknum i;
	Awkfloat f;
	Awkword u;
	Awkstr s;
	unsigned char buf[8];
};
typedef union Value Value;

/* unsigned char is more trouble than it's worth */

typedef	unsigned char uschar;

/*
 * We sometimes cheat writing read-only pointers to NUL-terminate them
 * and then put back the original value
 */
#define setptr(ptr, a)	(*(char *)(intptr_t)(ptr)) = (a)

#define	NN(p)	((p) ? (p) : "(null)")	/* guaranteed non-null for DPRINTF
*/
#define	DPRINTF(...)	if (dbg) printf(__VA_ARGS__)

extern enum compile_states {
	RUNNING,
	COMPILING,
	ERROR_PRINTING
} compile_time;

//#define	RECSIZE	(8 * 1024)	/* sets limit on records, fields, etc., etc. */
#define	RECSIZE	128	/* sets limit on records, fields, etc., etc. */
extern int	recsize;	/* size of current record, orig RECSIZE */

extern char	EMPTY[];	/* this avoid -Wwritable-strings issues */
extern char	**FS;
extern char	**RS;
extern char	**ORS;
extern char	**OFS;
extern char	**OFMT;
extern Awknum *NR;
extern Awknum *FNR;
extern Awknum *NF;
extern char	**FILENAME;
extern char	**SUBSEP;
extern Awknum *RSTART;
extern Awknum *RLENGTH;

extern FILE *awkstdin, *awkstdout, *awkstderr;

extern char	*record;	/* points to $0 */
extern int	lineno;		/* line number in awk program */
extern int	errorflag;	/* 1 if error has occurred */
extern bool	donefld;	/* true if record broken into fields */
extern bool	donerec;	/* true if record is valid (no fld has changed */
extern int	dbg;
extern char	*lexprog, *cmdname, *evalstr;
extern Awknum	srand_seed;

extern const char *patbeg;	/* beginning of pattern matched */
extern	int	patlen;		/* length of pattern matched.  set in b.c */

/* Cell:  all information about a variable or constant */

typedef struct Cell {
	uschar	ctype;		/* OCELL, OBOOL, OJUMP, etc. */
	uschar	csub;		/* CCON, CTEMP, CFLD, etc. */
	char	*nval;		/* name, for variables only */
	char	*sval;		/* string value */
	Value val;
	int	 tval;		/* type info: STR|NUM|ARR|FCN|FLD|CON|DONTFREE|CONVC|CONVO|FLT */
	char	*fmt;		/* CONVFMT/OFMT value used to convert from number */
	struct Cell *cnext;	/* ptr to next if chained */
} Cell;

typedef struct Array {		/* symbol table array */
	int	nelem;		/* elements in table right now */
	int	size;		/* size of tab */
	Cell	**tab;		/* hash table pointers */
} Array;

/* note: affects test results */
#define	NSYMTAB	50	/* initial size of a symbol table */
extern Array	*symtab;

extern Cell	*nrloc;		/* NR */
extern Cell	*fnrloc;	/* FNR */
extern Cell	*fsloc;		/* FS */
extern Cell	*nfloc;		/* NF */
extern Cell	*ofsloc;	/* OFS */
extern Cell	*orsloc;	/* ORS */
extern Cell	*rsloc;		/* RS */
extern Cell	*rstartloc;	/* RSTART */
extern Cell	*rlengthloc;	/* RLENGTH */
extern Cell	*subseploc;	/* SUBSEP */
extern Cell	*symtabloc;	/* SYMTAB */

/* Cell.tval values: */
#define	NUM	01	/* number value is valid */
#define	STR	02	/* string value is valid */
#define DONTFREE 04	/* string space is not freeable */
#define	CON	010	/* this is a constant */
#define	ARR	020	/* this is an array */
#define	FCN	040	/* this is a function name */
#define FLD	0100	/* this is a field $1, $2, ... */
#define	REC	0200	/* this is $0 */
#define CONVC	0400	/* string was converted from number via CONVFMT */
#define CONVO	01000	/* string was converted from number via OFMT */
#define	FLT		02000	/* if valid, number is floating point */


/* function types */
enum{
	FLENGTH = 1,
	FSQRT = 2,
	FEXP = 3,
	FLOG = 4,
	FINT = 5,
	FFRAND = 6,
	FSRAND = 7,
	FSIN = 8,
	FCOS = 9,
	FATAN = 10,
	FTOUPPER = 11,
	FTOLOWER = 12,
	FUTF = 13,
	FEVAL = 14,
	FFLOAT = 15,
	FNRAND = 16,
	FBYTES = 17,
};
typedef struct Keyword {
	const char *word;
	int	sub;
	int	type;
} Keyword;
extern	const Keyword keywords[];
extern	const size_t nkeywords;

/* TNode:  parse tree is made of nodes, with Cell's at bottom */

typedef struct TNode {
	int	ntype;
	struct	TNode *nnext;
	int	lineno;
	int	nobj;
	struct	TNode *narg[1];	/* variable: actual size set by calling malloc */
} TNode;

#define	NIL	((TNode *) 0)

extern TNode	*winner;
extern TNode	*nullnode;
extern TNode *runnerup;

/* ctypes */
#define OCELL	1
#define OBOOL	2
#define OJUMP	3

/* Cell subtypes: csub */
#define	CFREE	7
#define CCOPY	6
#define CCON	5
#define CTEMP	4
#define CNAME	3
#define CVAR	2
#define CFLD	1
#define	CUNK	0

/* bool subtypes */
#define BTRUE	11
#define BFALSE	12

/* jump subtypes */
#define JEXIT	21
#define JNEXT	22
#define	JBREAK	23
#define	JCONT	24
#define	JRET	25

/* node types */
#define NVALUE	1
#define NSTAT	2
#define NEXPR	3


extern	int	pairstack[], paircnt;

#define notlegal(n)	(n <= FIRSTTOKEN || n >= LASTTOKEN || proctab[n-FIRSTTOKEN] == nullproc)
#define isvalue(n)	((n)->ntype == NVALUE)
#define isexpr(n)	((n)->ntype == NEXPR)
#define isjump(n)	((n)->ctype == OJUMP)
#define isexit(n)	((n)->csub == JEXIT)
#define	isbreak(n)	((n)->csub == JBREAK)
#define	iscont(n)	((n)->csub == JCONT)
#define	isnext(n)	((n)->csub == JNEXT)
#define	isret(n)	((n)->csub == JRET)
#define isrec(n)	((n)->tval & REC)
#define isfld(n)	((n)->tval & FLD)
#define isstr(n)	((n)->tval & STR)
#define isnum(n)	((n)->tval & NUM)
#define isarr(n)	((n)->tval & ARR)
#define isfcn(n)	((n)->tval & FCN)
#define istrue(n)	((n)->csub == BTRUE)
#define istemp(n)	((n)->csub == CTEMP)
#define	isargument(n)	((n)->nobj == ARG)
/* #define freeable(p)	(!((p)->tval & DONTFREE)) */
#define freeable(p)	( ((p)->tval & (STR|DONTFREE)) == STR )

/* structures used by regular expression matching machinery, mostly b.c: */

//#define NCHARS	(1256+3)		/* 256 handles 8-bit chars; 128 does 7-bit */
#define NCHARS	(128+3)		/* 256 handles 8-bit chars; 128 does 7-bit */
				/* BUG: some overflows (caught) if we use 256 */
				/* watch out in match(), etc. */
#define	HAT	(NCHARS+2)	/* matches ^ in regular expr */
#define NSTATES	32

typedef struct rrow {
	long	ltype;	/* long avoids pointer warnings on 64-bit */
	union {
		int i;
		TNode *np;
		uschar *up;
		int *rp; /* rune representation of char class */
	} lval;		/* because Al stores a pointer in it! */
	int	*lfollow;
} rrow;

typedef struct gtte { /* gototab entry */
	unsigned int ch;
	unsigned int state;
} gtte;

typedef struct gtt {	/* gototab */
	size_t	allocated;
	size_t	inuse;
	gtte	*entries;
} gtt;

typedef struct fa {
	gtt	*gototab;
	uschar	*out;
	uschar	*restr;
	int	**posns;
	int	state_count;
	bool	anchor;
	int	use;
	int	initstat;
	int	curstat;
	int	accept;
	int cst;
	struct	rrow re[1];	/* variable: actual size set by calling malloc */
} fa;

#include "mt19937-64.h"
#define	AWKTAB	"awkgram.tab.h"
#ifdef _PLAN9_SOURCE
#undef	AWKTAB
#define	AWKTAB	"awkgram.plan9.h"
#include "plan9.h"
#endif
#include "proto.h"
