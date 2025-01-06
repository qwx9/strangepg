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

const char	*version = "version 20240731";

#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "awk.h"
#include AWKTAB

extern	char	**environ;
extern	int	nfields;

const Keyword keywords[] = {	/* keep sorted: binary searched */
	{ "BEGIN",	XBEGIN,		XBEGIN },
	{ "END",	XEND,		XEND },
	{ "NF",		VARNF,		VARNF },
	{ "atan2",	FATAN,		BLTIN },
	{ "break",	BREAK,		BREAK },
	{ "bytes",	FBYTES,		BLTIN },
	{ "continue",	CONTINUE,	CONTINUE },
	{ "cos",	FCOS,		BLTIN },
	{ "delete",	DELETE,		DELETE },
	{ "do",		DO,		DO },
	{ "else",	ELSE,		ELSE },
	{ "eval",	FEVAL,		BLTIN },
	{ "exit",	EXIT,		EXIT },
	{ "exp",	FEXP,		BLTIN },
	{ "float",	FFLOAT,		BLTIN },
	{ "for",	FOR,		FOR },
	{ "func",	FUNC,		FUNC },
	{ "function",	FUNC,		FUNC },
	{ "gsub",	GSUB,		GSUB },
	{ "if",		IF,		IF },
	{ "in",		IN,		IN },
	{ "index",	INDEX,		INDEX },
	{ "int",	FINT,		BLTIN },
	{ "length",	FLENGTH,	BLTIN },
	{ "log",	FLOG,		BLTIN },
	{ "match",	MATCHFCN,	MATCHFCN },
	{ "next",	NEXT,		NEXT },
	{ "nrand",	FNRAND,		BLTIN },
	{ "print",	PRINT,		PRINT },
	{ "printf",	PRINTF,		PRINTF },
	{ "rand",	FFRAND,		BLTIN },
	{ "return",	RETURN,		RETURN },
	{ "sin",	FSIN,		BLTIN },
	{ "split",	SPLIT,		SPLIT },
	{ "sprintf",	SPRINTF,	SPRINTF },
	{ "sqrt",	FSQRT,		BLTIN },
	{ "srand",	FSRAND,		BLTIN },
	{ "sub",	SUB,		SUB },
	{ "substr",	SUBSTR,		SUBSTR },
	{ "tolower",	FTOLOWER,	BLTIN },
	{ "toupper",	FTOUPPER,	BLTIN },
	{ "while",	WHILE,		WHILE },
};
const size_t nkeywords = sizeof(keywords) / sizeof(keywords[0]);

extern	FILE	*yyin;	/* lex input file */
extern	int errorflag;	/* non-zero if any syntax errors; set by yyerror */

extern	size_t	npfile;

Cell *addon(TNode **a, int n)	/* external addons */
{
	FATAL("what are you doing here?");
	return NULL;
}

static const char *
setfs(char *p)
{
	/* wart: t=>\t */
	if (p[0] == 't' && p[1] == '\0')
		return "\t";
	return p;
}

static char *
getarg(int *argc, char ***argv, const char *msg)
{
	if ((*argv)[1][2] != '\0') {	/* arg is -fsomething */
		return &(*argv)[1][2];
	} else {			/* arg is -f something */
		(*argc)--; (*argv)++;
		if (*argc <= 1)
			FATAL("%s", msg);
		return (*argv)[1];
	}
}

int main(int argc, char *argv[])
{
	const char *fs = NULL;
	char *fn, *vn;

	awkstdin = stdin;
	awkstdout = stdout;
	awkstderr = stderr;
	setlocale(LC_CTYPE, "");
	setlocale(LC_NUMERIC, "C"); /* for parsing cmdline & prog */
	initpool();
	cmdname = argv[0];
	if (argc == 1) {
		fprintf(awkstderr,
		  "usage: %s [-F fs] [-v var=value] [-f progfile | 'prog'] [file ...]\n",
		  cmdname);
		exit(1);
	}
	catchfpe();

	/* Set and keep track of the random seed */
	srand_seed = 1;
	init_genrand64(srand_seed);

	yyin = NULL;
	symtab = makesymtab(NSYMTAB/NSYMTAB);
	while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
		if (strcmp(argv[1], "-version") == 0 || strcmp(argv[1], "--version") == 0) {
			printf("awk %s\n", version);
			return 0;
		}
		if (strcmp(argv[1], "--") == 0) {	/* explicit end of args */
			argc--;
			argv++;
			break;
		}
		switch (argv[1][1]) {
		case 'f':	/* next argument is program filename */
			fn = getarg(&argc, &argv, "no program filename");
			addfile(fn);
 			break;
		case 'F':	/* set field separator */
			fs = setfs(getarg(&argc, &argv, "no field separator"));
			break;
		case 'v':	/* -v a=1 to be done NOW.  one -v for each */
			vn = getarg(&argc, &argv, "no variable name");
			if (isclvar(vn))
				setclvar(vn);
			else
				FATAL("invalid -v option argument: %s", vn);
			break;
		case 'd':
			dbg = atoi(&argv[1][2]);
			if (dbg == 0)
				dbg = 1;
			printf("awk %s\n", version);
			break;
		default:
			WARNING("unknown option %s ignored", argv[1]);
			break;
		}
		argc--;
		argv++;
	}

	/* argv[1] is now the first argument */
	if (npfile == 0) {	/* no -f; first argument is program */
		if (argc <= 1) {
			if (dbg)
				exit(0);
			FATAL("no program given");
		}
		DPRINTF("program = |%s|\n", argv[1]);
		lexprog = argv[1];
		argc--;
		argv++;
	}
	recinit(recsize);
	syminit();
	compile_time = COMPILING;
	argv[0] = cmdname;	/* put prog name at front of arglist */
	DPRINTF("argc=%d, argv[0]=%s\n", argc, argv[0]);
	arginit(argc, argv);
	yyparse();
#if 0
	// Doing this would comply with POSIX, but is not compatible with
	// other awks and with what most users expect. So comment it out.
	setlocale(LC_NUMERIC, ""); /* back to whatever it is locally */
#endif
	if (fs)
		*FS = qstring(fs, '\0');
	DPRINTF("errorflag=%d\n", errorflag);
	if (errorflag == 0) {
		compile_time = RUNNING;
		run(winner);
	} else
		bracecheck();
	return(errorflag);
}
