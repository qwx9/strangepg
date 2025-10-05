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

extern	int	yywrap(void);
extern	void	setfname(Cell *);
extern	int	constnode(TNode *);
extern	char	*strnode(TNode *);
extern	TNode	*notnull(TNode *);
extern	int	yyparse(void);

extern	int	yylex(void);
extern	void	startreg(void);
extern	int	input(void);
extern	void	unput(int);
extern	void	unputstr(const char *);

extern	fa	*makedfa(const char *, bool);
extern	fa	*mkdfa(const char *, bool);
extern	int	makeinit(fa *, bool);
extern	void	penter(TNode *);
extern	void	freetr(TNode *);
extern	int	quoted(const uschar **);
extern	int	*cclenter(const char *);
extern	noreturn void	overflo(const char *);
extern	void	cfoll(fa *, TNode *);
extern	int	first(TNode *);
extern	void	follow(TNode *);
extern	int	member(int, int *);
extern	int	match(fa *, const char *);
extern	int	pmatch(fa *, const char *);
extern	int	nematch(fa *, const char *);
extern	bool	fnematch(fa *, FILE *, char **, int *, int);
extern	TNode	*reparse(const char *);
extern	TNode	*regexp(void);
extern	TNode	*primary(void);
extern	TNode	*concat(TNode *);
extern	TNode	*unary(TNode *);
extern	int	relex(void);
extern	int	cgoto(fa *, int, int);
extern	void	freefa(fa *);

extern	void	addfile(char *);
extern	int	pgetc(void);
extern	char	*cursource(void);

extern	void	cleanpool(void);
extern	void	initpool(void);
extern	char	*tempstrdup(const char *);
extern	void	*temprealloc(const void *, size_t, size_t);
extern	void	*tempalloc(size_t);
extern	char	*pstrdup(const char *);
extern	void	*prealloc(const void *, size_t, size_t);
extern	void	*palloc(size_t);
extern	char	*defstrdup(const char *);
extern	void	*defrealloc(const void *, size_t, size_t);
extern	void	*defalloc(size_t);
extern	void	*drealloc(void *, size_t, size_t, const char *);
extern	void	dgrow(void **, int *, void **, int , int, const char *);
extern	char	*dstrdup(const char *, const char *);
extern	void	*dmalloc(size_t, const char *);
extern	void	dfree(void *, const char *);

/* FIXME */
#define	MALLOC(a)	dmalloc((a), __func__)
#define	CALLOC(a, b)	dmalloc((a)*(b), __func__)
#define	REALLOC(a, b, c)	drealloc((a), (b), (c), __func__)
#define	STRDUP(a)	dstrdup((a), __func__)
#define	FREE(a)	dfree((a), __func__)
#define	xfree(a)	do{ \
	if((const char*)(a) != EMPTY){ \
		FREE((void *)(intptr_t)(a)); \
		(a) = NULL; \
	} \
}while(0)
#define tempfree(x)	do { if (istemp(x)) tfree(x); } while (0)

extern	Cell	*dummyptrsym(Cell *);
extern	Array	*attach(char *, Array *, void *, size_t, int, void (*)(size_t, Value));
extern	void	reattach(Array *, void *, size_t);
extern	void	hookdel(Array *, void (*)(Cell *));

extern	TNode	*exptostat(TNode *);
extern	TNode	*node1(int, TNode *);
extern	TNode	*node2(int, TNode *, TNode *);
extern	TNode	*node3(int, TNode *, TNode *, TNode *);
extern	TNode	*node4(int, TNode *, TNode *, TNode *, TNode *);
extern	TNode	*stat3(int, TNode *, TNode *, TNode *);
extern	TNode	*op2(int, TNode *, TNode *);
extern	TNode	*op1(int, TNode *);
extern	TNode	*stat1(int, TNode *);
extern	TNode	*op3(int, TNode *, TNode *, TNode *);
extern	TNode	*op4(int, TNode *, TNode *, TNode *, TNode *);
extern	TNode	*stat2(int, TNode *, TNode *);
extern	TNode	*stat4(int, TNode *, TNode *, TNode *, TNode *);
extern	TNode	*celltonode(Cell *, int);
extern	TNode	*rectonode(void);
extern	TNode	*makearr(TNode *);
extern	TNode	*pa2stat(TNode *, TNode *, TNode *);
extern	TNode	*linkum(TNode *, TNode *);
extern	void	defn(Cell *, TNode *, TNode *);
extern	int	isarg(const char *);
extern	const char *tokname(int);
extern	Cell	*(*proctab[])(TNode **, int);
extern	int	ptoi(void *);
extern	TNode	*itonp(int);

extern	void	inittran(void);
extern	void	syminit(void);
extern	void	arginit(int, char **);
extern	Array	*makesymtab(int);
extern	void	freesymtab(Cell *, int);
extern	void	freeelem(Cell *, const char *);
extern	Cell	*setsym(const char *, const char *, Array *);
extern	Cell	*setptrtab(Awknum, Array *, int);
extern	Cell	*setsymtab(const char *, const char *, Value, unsigned int, Array *);
extern	int	hash(const char *, int);
extern	void	rehash(Array *);
extern	Cell	*lookup(const char *, Array *);
extern	Awkfloat	setfval(Cell *, Awkfloat);
extern	Awknum	setival(Cell *, Awknum);
extern	void	setval(Cell *, Cell *);
extern	void	funnyvar(Cell *, const char *);
extern	char	*setsval(Cell *, const char *, int);
extern	Value	getval(Cell *, short *);
extern	Awknum	getival(Cell *);
extern	Awkfloat	getfval(Cell *);
extern	char	*getsval(Cell *);
extern	char	*getpssval(Cell *);     /* for print */
extern	char	*qstring(const char *, int);
extern	Cell	*catstr(Cell *, Cell *);

extern	void	recinit(unsigned int);
extern	void	initgetrec(void);
extern	void	makefields(int, int);
extern	void	growfldtab(int n);
extern	void	savefs(void);
extern	int	getrec(char **, int *, bool);
extern	int	readrec(char **buf, int *bufsize, FILE *inf, bool isnew);

extern	int	awkmain(int, char **);
extern	int	compileawk(int, char**);
extern	void	runawk(void);
extern	void	catchfpe(void);
extern	char	*getargv(int);
extern	void	setclvar(char *);
extern	void	fldbld(void);
extern	void	cleanfld(int, int);
extern	void	newfld(int);
extern	void	setlastfld(int);
extern	int	refldbld(const char *, const char *);
extern	void	recbld(void);
extern	Cell	*fieldadr(int);
extern	void	yyerror(const char *);
extern	void	bracecheck(void);
extern	void	bcheck2(int, int, int);
extern	void	SYNTAX(const char *, ...)
    __attribute__((__format__(__printf__, 1, 2)));
extern	noreturn void	FATAL(const char *, ...)
    __attribute__((__format__(__printf__, 1, 2)));
extern	void	WARNING(const char *, ...)
    __attribute__((__format__(__printf__, 1, 2)));
extern	void	ERROR(FILE *);
extern	int	eprint(FILE *);
extern	void	bclass(int);
extern	Awkfloat	errcheck(Awkfloat, const char *);
extern	int	isclvar(const char *);
extern	int	is_valid_number(const char *s, bool trailing_stuff_ok,
				bool *no_trailing, char **trail, Value *result);
#define is_number(s, val)	is_valid_number(s, false, NULL, NULL, val)

extern	int	adjbuf(char **pb, int *sz, int min, int q, char **pbp, const char *what);
extern	void	run(TNode *);
extern	Cell	*execute(TNode *);
extern	Cell	*program(TNode **, int);
extern	Cell	*call(TNode **, int);
extern	Cell	*copycell(Cell *);
extern	Cell	*arg(TNode **, int);
extern	Cell	*jump(TNode **, int);
extern	Cell	*getnf(TNode **, int);
extern	Cell	*array(TNode **, int);
extern	Cell	*awkdelete(TNode **, int);
extern	Cell	*intest(TNode **, int);
extern	Cell	*matchop(TNode **, int);
extern	Cell	*boolop(TNode **, int);
extern	Cell	*relop(TNode **, int);
extern	void	tfree(Cell *);
extern	Cell	*gettemp(int);
extern	Cell	*indirect(TNode **, int);
extern	Cell	*substr(TNode **, int);
extern	Cell	*sindex(TNode **, int);
extern	int	format(char **, int *, const char *, TNode *);
extern	Cell	*awksprintf(TNode **, int);
extern	Cell	*awkprintf(TNode **, int);
extern	Cell	*arith(TNode **, int);
extern	Awknum	ipow(Awknum, int);
extern	Cell	*incrdecr(TNode **, int);
extern	Cell	*assign(TNode **, int);
extern	Cell	*cat(TNode **, int);
extern	Cell	*pastat(TNode **, int);
extern	Cell	*dopa2(TNode **, int);
extern	Cell	*split(TNode **, int);
extern	Cell	*condexpr(TNode **, int);
extern	Cell	*ifstat(TNode **, int);
extern	Cell	*whilestat(TNode **, int);
extern	Cell	*dostat(TNode **, int);
extern	Cell	*forstat(TNode **, int);
extern	Cell	*instat(TNode **, int);
extern	Cell	*bltin(TNode **, int);
extern	Cell	*addon(TNode **, int);
extern	Cell	*printstat(TNode **, int);
extern	Cell	*nullproc(TNode **, int);
extern	Cell	*dosub(TNode **, int);
