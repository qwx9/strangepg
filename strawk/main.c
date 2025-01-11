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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "awk.h"
#include AWKTAB

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

Cell *addon(TNode **a, int n)	/* external addons */
{
	FATAL("what are you doing here?");
	return NULL;
}

int main(int argc, char *argv[])
{
	awkstdin = stdin;
	awkstdout = stdout;
	awkstderr = stderr;
	return awkmain(argc, argv);
}
