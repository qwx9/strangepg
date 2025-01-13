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
	{ "deselect",	ADESELECT,	ADDON },
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
	{ "nodecolor",	ANODECOLOR,		ADDON },
	{ "nrand",	FNRAND,		BLTIN },
	{ "print",	PRINT,		PRINT },
	{ "printf",	PRINTF,		PRINTF },
	{ "rand",	FFRAND,		BLTIN },
	{ "return",	RETURN,		RETURN },
//	{ "select",	ASELECT,	ADDON },
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

static noreturn void
awk(void *)
{
	char *args[] = {"strawk", awkprog};

	if((awkstdin = fdopen(infd[0], "rb")) == NULL
	|| (awkstdout = fdopen(outfd[1], "wb")) == NULL
	|| (awkstderr = fdopen(outfd[1], "wb")) == NULL)
		sysfatal("awk: %s", error());
	dbg = (debug & Debugstrawk) != 0;
	awkmain(nelem(args), args);
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
