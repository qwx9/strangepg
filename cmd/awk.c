#include "strpg.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include "strawk/awk.h"
#include "strawk/awkgram.tab.h"

/* [0] is read, [1] is write to cater to windows' _pipe */
int infd[2] = {-1, -1}, outfd[2] = {-1, -1}, eoutfd[2] = {-1, -1};

const Keyword keywords[] = {	/* keep sorted: binary searched */
	{ "BEGIN",	XBEGIN,		XBEGIN },
	{ "END",	XEND,		XEND },
	{ "NF",		VARNF,		VARNF },
	{ "atan2",	FATAN,		BLTIN },
	{ "break",	BREAK,		BREAK },
	{ "bytes",	FBYTES,		BLTIN },
	{ "collapse",	ACOLLAPSE,	ADDON },
	{ "commit",	ACOMMIT,	ADDON },	/* FIXME: remove */
	{ "continue",	CONTINUE,	CONTINUE },
	{ "cos",	FCOS,		BLTIN },
	{ "delete",	DELETE,		DELETE },
	{ "do",		DO,		DO },
	{ "else",	ELSE,		ELSE },
	{ "eval",	FEVAL,		BLTIN },
	{ "exit",	EXIT,		EXIT },
	{ "exp",	FEXP,		BLTIN },
	{ "expand1",	AEXPAND1,		ADDON },
	{ "expandall",	AEXPANDALL,		ADDON },
	{ "explode1",	AEXPLODE1,		ADDON },
	{ "exportcoarse",	AEXPORTCOARSE,		ADDON },
	{ "exportgfa",	AEXPORTGFA,		ADDON },
	{ "exportsvg",	AEXPORTSVG,		ADDON },
	{ "float",	FFLOAT,		BLTIN },
	{ "for",	FOR,		FOR },
	{ "func",	FUNC,		FUNC },
	{ "function",	FUNC,		FUNC },
	{ "gsub",	GSUB,		GSUB },
	{ "if",		IF,		IF },
	{ "in",		IN,		IN },
	{ "index",	INDEX,		INDEX },
	{ "info",	AINFO,		ADDON },
	{ "int",	FINT,		BLTIN },
	{ "length",	FLENGTH,	BLTIN },
	{ "loadbatch",	ALOADBATCH,	ADDON },
	{ "log",	FLOG,		BLTIN },
	{ "match",	MATCHFCN,	MATCHFCN },
	{ "next",	NEXT,		NEXT },
	{ "nrand",	FNRAND,		BLTIN },
	{ "print",	PRINT,		PRINT },
	{ "printf",	PRINTF,		PRINTF },
	{ "rand",	FFRAND,		BLTIN },
	{ "refresh",	AREFRESH,		ADDON },
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
	{ "typeof",	FTYPEOF,	BLTIN },
	{ "while",	WHILE,		WHILE },
};
const size_t nkeywords = nelem(keywords);

void
awkprint(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vfprintf(awkstdout, fmt, arg);
	va_end(arg);
}

static void
awk(void *)
{
	char *args[] = {"strawk", awkprog};

	if((awkstdin = fdopen(infd[0], "rb")) == NULL
	|| (awkstdout = fdopen(outfd[1], "wb")) == NULL
	|| (awkstderr = fdopen(eoutfd[1], "wb")) == NULL)
		sysfatal("awk: %s", error());
	dbg = (debug & Debugstrawk) != 0;
	quiet = status & FSquiet || (debug & Debuginfo) == 0;
	/* note: srand has already been called, so awk's srand_seed
	 * has a wrong value of 1; we don't care. */
	compileawk(nelem(args), args);
	initvars();
	runawk();
}

/* use [1] on our side, [0] on awk side */
int
initrepl(void)
{
	if(pipe(infd) < 0 || pipe(outfd) < 0 || pipe(eoutfd))
		return -1;
	initext();
	newthread(awk, nil, nil, nil, "strawk", mainstacksize);
	return 0;
}
