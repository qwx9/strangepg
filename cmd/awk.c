#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include <locale.h>
#include "strawk/awk.h"
#include "strawk/awkgram.tab.h"

/* [0] is read, [1] is write to cater to windows' _pipe */
int infd[2] = {-1, -1}, outfd[2] = {-1, -1};

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
	{ "loadall",	ALOAD,	ADDON },
	{ "loadbatch",	ALOADBATCH,	ADDON },
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
const size_t nkeywords = nelem(keywords);

noreturn void
awk(void *)
{
	if((awkstdin = fdopen(infd[0], "rb")) == NULL
	|| (awkstdout = fdopen(outfd[1], "wb")) == NULL
	|| (awkstderr = fdopen(outfd[1], "wb")) == NULL)
		sysfatal("awk: %s", error());
	setlocale(LC_CTYPE, "");
	setlocale(LC_NUMERIC, "C"); /* for parsing cmdline & prog */
	initpool();
	cmdname = "strawk";
	lexprog = awkprog;
	dbg = (debug & Debugawk) != 0;
	catchfpe();
	init_genrand64(srand_seed);
	symtab = makesymtab(1);
	recinit(recsize);
	syminit();
	compile_time = COMPILING;
	arginit(0, NULL);
	yyparse();
	*FS = qstring("\t", '\0');
	*OFS = qstring("\t", '\0');
	if (errorflag == 0) {
		compile_time = RUNNING;
		run(winner);
	} else
		bracecheck();
}

/* use [1] on our side, [0] on awk side */

int
initrepl(void)
{
	if(pipe(infd) < 0 || pipe(outfd) < 0)
		return -1;
	initext();
	newthread(awk, nil, nil, nil, "strawk", mainstacksize);
	return 0;
}
