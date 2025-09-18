#include <stdio.h>
#ifdef VERSION
#include "strpg.h"
#include "threads.h"
#endif
#include "awk.h"
#include "awkgram.tab.h"

static const char * const printname[103] = {
	"FIRSTTOKEN",	/* 258 */
	"PROGRAM",	/* 259 */
	"PASTAT",	/* 260 */
	"PASTAT2",	/* 261 */
	"XBEGIN",	/* 262 */
	"XEND",	/* 263 */
	"NL",	/* 264 */
	"ARRAY",	/* 265 */
	"MATCH",	/* 266 */
	"NOTMATCH",	/* 267 */
	"MATCHOP",	/* 268 */
	"FINAL",	/* 269 */
	"DOT",	/* 270 */
	"ALL",	/* 271 */
	"CCL",	/* 272 */
	"NCCL",	/* 273 */
	"CHAR",	/* 274 */
	"OR",	/* 275 */
	"STAR",	/* 276 */
	"QUEST",	/* 277 */
	"PLUS",	/* 278 */
	"EMPTYRE",	/* 279 */
	"ZERO",	/* 280 */
	"LAND",	/* 281 */
	"LOR",	/* 282 */
	"EQ",	/* 283 */
	"GE",	/* 284 */
	"GT",	/* 285 */
	"LE",	/* 286 */
	"LT",	/* 287 */
	"NE",	/* 288 */
	"IN",	/* 289 */
	"ADDON",	/* 290 */
	"ARG",	/* 291 */
	"BLTIN",	/* 292 */
	"BREAK",	/* 293 */
	"CONTINUE",	/* 294 */
	"DELETE",	/* 295 */
	"DO",	/* 296 */
	"EXIT",	/* 297 */
	"FOR",	/* 298 */
	"FUNC",	/* 299 */
	"SUB",	/* 300 */
	"GSUB",	/* 301 */
	"IF",	/* 302 */
	"INDEX",	/* 303 */
	"LSUBSTR",	/* 304 */
	"MATCHFCN",	/* 305 */
	"NEXT",	/* 306 */
	"ADD",	/* 307 */
	"MINUS",	/* 308 */
	"MULT",	/* 309 */
	"DIVIDE",	/* 310 */
	"MOD",	/* 311 */
	"LSHIFT",	/* 312 */
	"RSHIFT",	/* 313 */
	"XOR",	/* 314 */
	"BAND",	/* 315 */
	"BOR",	/* 316 */
	"CMPL",	/* 317 */
	"ASSIGN",	/* 318 */
	"ASGNOP",	/* 319 */
	"ADDEQ",	/* 320 */
	"SUBEQ",	/* 321 */
	"MULTEQ",	/* 322 */
	"DIVEQ",	/* 323 */
	"MODEQ",	/* 324 */
	"POWEQ",	/* 325 */
	"BANDEQ",	/* 326 */
	"BOREQ",	/* 327 */
	"XOREQ",	/* 328 */
	"SHLEQ",	/* 329 */
	"SHREQ",	/* 330 */
	"PRINT",	/* 331 */
	"PRINTF",	/* 332 */
	"SPRINTF",	/* 333 */
	"ELSE",	/* 334 */
	"INTEST",	/* 335 */
	"CONDEXPR",	/* 336 */
	"POSTINCR",	/* 337 */
	"PREINCR",	/* 338 */
	"POSTDECR",	/* 339 */
	"PREDECR",	/* 340 */
	"VAR",	/* 341 */
	"IVAR",	/* 342 */
	"VARNF",	/* 343 */
	"CALL",	/* 344 */
	"NUMBER",	/* 345 */
	"STRING",	/* 346 */
	"REGEXPR",	/* 347 */
	"RETURN",	/* 348 */
	"SPLIT",	/* 349 */
	"SUBSTR",	/* 350 */
	"WHILE",	/* 351 */
	"CAT",	/* 352 */
	"NOT",	/* 353 */
	"UMINUS",	/* 354 */
	"UPLUS",	/* 355 */
	"POWER",	/* 356 */
	"DECR",	/* 357 */
	"INCR",	/* 358 */
	"INDIRECT",	/* 359 */
	"LASTTOKEN",	/* 360 */
};


Cell *(*proctab[103])(TNode **, int) = {
	nullproc,	/* FIRSTTOKEN */
	program,	/* PROGRAM */
	pastat,	/* PASTAT */
	dopa2,	/* PASTAT2 */
	nullproc,	/* XBEGIN */
	nullproc,	/* XEND */
	nullproc,	/* NL */
	array,	/* ARRAY */
	matchop,	/* MATCH */
	matchop,	/* NOTMATCH */
	nullproc,	/* MATCHOP */
	nullproc,	/* FINAL */
	nullproc,	/* DOT */
	nullproc,	/* ALL */
	nullproc,	/* CCL */
	nullproc,	/* NCCL */
	nullproc,	/* CHAR */
	nullproc,	/* OR */
	nullproc,	/* STAR */
	nullproc,	/* QUEST */
	nullproc,	/* PLUS */
	nullproc,	/* EMPTYRE */
	nullproc,	/* ZERO */
	boolop,	/* LAND */
	boolop,	/* LOR */
	relop,	/* EQ */
	relop,	/* GE */
	relop,	/* GT */
	relop,	/* LE */
	relop,	/* LT */
	relop,	/* NE */
	instat,	/* IN */
	addon,	/* ADDON */
	arg,	/* ARG */
	bltin,	/* BLTIN */
	jump,	/* BREAK */
	jump,	/* CONTINUE */
	awkdelete,	/* DELETE */
	dostat,	/* DO */
	jump,	/* EXIT */
	forstat,	/* FOR */
	nullproc,	/* FUNC */
	dosub,	/* SUB */
	dosub,	/* GSUB */
	ifstat,	/* IF */
	sindex,	/* INDEX */
	nullproc,	/* LSUBSTR */
	matchop,	/* MATCHFCN */
	jump,	/* NEXT */
	arith,	/* ADD */
	arith,	/* MINUS */
	arith,	/* MULT */
	arith,	/* DIVIDE */
	arith,	/* MOD */
	arith,	/* LSHIFT */
	arith,	/* RSHIFT */
	arith,	/* XOR */
	arith,	/* BAND */
	arith,	/* BOR */
	arith,	/* CMPL */
	assign,	/* ASSIGN */
	nullproc,	/* ASGNOP */
	assign,	/* ADDEQ */
	assign,	/* SUBEQ */
	assign,	/* MULTEQ */
	assign,	/* DIVEQ */
	assign,	/* MODEQ */
	assign,	/* POWEQ */
	assign,	/* BANDEQ */
	assign,	/* BOREQ */
	assign,	/* XOREQ */
	assign,	/* SHLEQ */
	assign,	/* SHREQ */
	printstat,	/* PRINT */
	awkprintf,	/* PRINTF */
	awksprintf,	/* SPRINTF */
	nullproc,	/* ELSE */
	intest,	/* INTEST */
	condexpr,	/* CONDEXPR */
	incrdecr,	/* POSTINCR */
	incrdecr,	/* PREINCR */
	incrdecr,	/* POSTDECR */
	incrdecr,	/* PREDECR */
	nullproc,	/* VAR */
	nullproc,	/* IVAR */
	getnf,	/* VARNF */
	call,	/* CALL */
	nullproc,	/* NUMBER */
	nullproc,	/* STRING */
	nullproc,	/* REGEXPR */
	jump,	/* RETURN */
	split,	/* SPLIT */
	substr,	/* SUBSTR */
	whilestat,	/* WHILE */
	cat,	/* CAT */
	boolop,	/* NOT */
	arith,	/* UMINUS */
	arith,	/* UPLUS */
	arith,	/* POWER */
	nullproc,	/* DECR */
	nullproc,	/* INCR */
	indirect,	/* INDIRECT */
	nullproc,	/* LASTTOKEN */
};

const char *tokname(int n)
{
	static char buf[100];

	if (n < FIRSTTOKEN || n > LASTTOKEN) {
		snprintf(buf, sizeof(buf), "token %d", n);
		return buf;
	}
	return printname[n-FIRSTTOKEN];
}
