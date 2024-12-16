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
#include <string.h>
#ifndef _PLAN9_SOURCE
#include <strings.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include "awk.h"

extern int u8_nextlen(const char *s);
extern int bracecnt, brackcnt, parencnt;
extern bool infunc;

char	EMPTY[] = { '\0' };
FILE	*infile	= NULL;
bool	innew;		/* true = infile has not been read by readrec */
char	*file	= EMPTY;
char	*record;
int	recsize	= RECSIZE;
char	*fields;
int	fieldssize = RECSIZE;

Cell	**fldtab;	/* pointers to Cells */
static size_t	len_inputFS = 0;
static char	*inputFS = NULL; /* FS at time of input, for field splitting */

#define	MAXFLD	2
int	nfields	= MAXFLD;	/* last allocated slot for $i */

bool	donefld;	/* true = implies rec broken into fields */
bool	donerec;	/* true = record is valid (no flds have changed) */

int	lastfld	= 0;	/* last used field */
int	argno	= 1;	/* current input argument number */
extern	Awknum *ARGC;

extern jmp_buf evalenv;

static Cell dollar0 = { OCELL, CFLD, NULL, EMPTY, {0}, REC|STR|DONTFREE, NULL, NULL };
static Cell dollar1 = { OCELL, CFLD, NULL, EMPTY, {0}, FLD|STR|DONTFREE, NULL, NULL };

static size_t nalloc, renalloc;

/* FIXME: use strangepg's emalloc functions when linked? */
void *dmalloc(size_t size, const char *fn)
{
	void *p;

	nalloc += size;
	p = malloc(size);
	DPRINTF("ALLOC malloc %s %p %zd / %zd\n", fn, p, size, nalloc);
	return p;
}
char *dstrdup(const char *s, const char *fn)
{
	char *p;
	size_t size;
	
	size = strlen(s);
	nalloc += size;
	p = strdup(s);
	DPRINTF("ALLOC strdup %s %p %zd / %zd [%s]\n", fn, (void*)p, size, nalloc, s);
	return p;
}
void *dcalloc(size_t nmemb, size_t size, const char *fn)
{
	void *p;

	nalloc += nmemb * size;
	p = calloc(nmemb, size);
	DPRINTF("ALLOC calloc %s %p %zd / %zd\n", fn, p, nmemb*size, nalloc);
	return p;
}
void *drealloc(void *ptr, size_t size, const char *fn)
{
	void *p;

	renalloc += size;
	p = realloc(ptr, size);
	DPRINTF("ALLOC realloc %s %p→%p %zd / %zd\n", fn, ptr, p, size, renalloc);
	return p;
}
void dfree(void *ptr, const char *fn)
{
	if(ptr == NULL)
		return;
	DPRINTF("ALLOC free %s %p\n", fn, ptr);
	free(ptr);
}

void recinit(unsigned int n)
{
	record = (char *) MALLOC(n);
	fields = (char *) MALLOC(n+1);
	fldtab = (Cell **) CALLOC(nfields+2, sizeof(*fldtab));
	fldtab[0] = (Cell *) MALLOC(sizeof(**fldtab));
	*record = '\0';
	*fldtab[0] = dollar0;
	fldtab[0]->sval = record;
	fldtab[0]->nval = tostring("0");
	makefields(1, nfields);
}

void makefields(int n1, int n2)		/* create $n1..$n2 inclusive */
{
	char temp[50];
	int i;

	for (i = n1; i <= n2; i++) {
		fldtab[i] = (Cell *) MALLOC(sizeof(**fldtab));
		*fldtab[i] = dollar1;
		snprintf(temp, sizeof(temp), "%d", i);
		fldtab[i]->nval = tostring(temp);
	}
}

void initgetrec(void)
{
	int i;
	char *p;

	for (i = 1; i < *ARGC; i++) {
		p = getargv(i); /* find 1st real filename */
		if (p == NULL || *p == '\0') {  /* deleted or zapped */
			argno++;
			continue;
		}
		if (!isclvar(p))
			return;
		setclvar(p);	/* a commandline assignment before filename */
		argno++;
	}
	infile = awkstdin;		/* no filenames, so use stdin */
	innew = true;
}

/*
 * POSIX specifies that fields are supposed to be evaluated as if they were
 * split using the value of FS at the time that the record's value ($0) was
 * read.
 *
 * Since field-splitting is done lazily, we save the current value of FS
 * whenever a new record is read in (implicitly or via getline), or when
 * a new value is assigned to $0.
 */
void savefs(void)
{
	size_t len;
	if ((len = strlen(getsval(fsloc))) < len_inputFS) {
		strcpy(inputFS, *FS);	/* for subsequent field splitting */
		return;
	}

	len_inputFS = len + 1;
	inputFS = (char *) REALLOC(inputFS, len_inputFS);
	memcpy(inputFS, *FS, len_inputFS);
}

static bool firsttime = true;

int getrec(char **pbuf, int *pbufsize, bool isrecord)	/* get next input record */
{			/* note: cares whether buf == record */
	int c, r;
	char *buf = *pbuf;
	uschar saveb0;
	int bufsize = *pbufsize, savebufsize = bufsize;
	Value v;

	if (firsttime) {
		firsttime = false;
		initgetrec();
	}
	DPRINTF("RS=<%s>, FS=<%s>, ARGC=%lld, FILENAME=%s\n",
		*RS, *FS, *ARGC, *FILENAME);
	saveb0 = buf[0];
	buf[0] = 0;
	while (argno < *ARGC || infile == awkstdin) {
		DPRINTF("argno=%d, file=|%s|\n", argno, file);
		if (infile == NULL) {	/* have to open a new file */
			file = getargv(argno);
			if (file == NULL || *file == '\0') {	/* deleted or zapped */
				argno++;
				continue;
			}
			if (isclvar(file)) {	/* a var=value arg */
				setclvar(file);
				argno++;
				continue;
			}
			*FILENAME = file;
			DPRINTF("opening file %s\n", file);
			if (*file == '-' && *(file+1) == '\0')
				infile = awkstdin;
			else if ((infile = fopen(file, "r")) == NULL)
				FATAL("can't open file %s", file);
			innew = true;
			setival(fnrloc, 0);
		}
		c = readrec(&buf, &bufsize, infile, innew);
		if (innew)
			innew = false;
		if (c != 0 || buf[0] != '\0') {	/* normal record */
			if (isrecord) {
				if (freeable(fldtab[0]))
					xfree(fldtab[0]->sval);
				fldtab[0]->sval = buf;	/* buf == record */
				fldtab[0]->tval = REC | STR | DONTFREE;
				if(r = is_number(buf, &v)) {
					fldtab[0]->val = v;
					fldtab[0]->tval |= r;
				}
				donefld = false;
				donerec = true;
				savefs();
			}
			setival(nrloc, nrloc->val.i+1);
			setival(fnrloc, fnrloc->val.i+1);
			*pbuf = buf;
			*pbufsize = bufsize;
			return 1;
		}
		/* EOF arrived on this file; set up next */
		if (infile != awkstdin)
			fclose(infile);
		infile = NULL;
		argno++;
	}
	buf[0] = saveb0;
	*pbuf = buf;
	*pbufsize = savebufsize;
	return 0;	/* true end of file */
}

int readrec(char **pbuf, int *pbufsize, FILE *inf, bool newflag)	/* read one record into buf */
{
	int sep, c, isrec; // POTENTIAL BUG? isrec is a macro in awk.h
	char *rr = *pbuf, *buf = *pbuf;
	int bufsize = *pbufsize;
	char *rs = getsval(rsloc);

	if (*rs && rs[1]) {
		bool found;

		memset(buf, 0, bufsize);
		fa *pfa = makedfa(rs, 1);
		if (newflag)
			found = fnematch(pfa, inf, &buf, &bufsize, recsize);
		else {
			int tempstat = pfa->initstat;
			pfa->initstat = 2;
			found = fnematch(pfa, inf, &buf, &bufsize, recsize);
			pfa->initstat = tempstat;
		}
		if (found)
			setptr(patbeg, '\0');
		isrec = (found == 0 && *buf == '\0') ? false : true;

	} else {
		if ((sep = *rs) == 0) {
			sep = '\n';
			while ((c=getc(inf)) == '\n' && c != EOF)	/* skip leading \n's */
				;
			if (c != EOF)
				ungetc(c, inf);
		}
		for (rr = buf; ; ) {
			for (; (c=getc(inf)) != sep && c != EOF; ) {
				if (rr-buf+1 > bufsize)
					if (!adjbuf(&buf, &bufsize, 1+rr-buf,
					    recsize, &rr, "readrec 1"))
						FATAL("input record `%.30s...' too long", buf);
				*rr++ = c;
			}
			if (*rs == sep || c == EOF)
				break;
			if ((c = getc(inf)) == '\n' || c == EOF)	/* 2 in a row */
				break;
			if (!adjbuf(&buf, &bufsize, 2+rr-buf, recsize, &rr,
			    "readrec 2"))
				FATAL("input record `%.30s...' too long", buf);
			*rr++ = '\n';
			*rr++ = c;
		}
		if (!adjbuf(&buf, &bufsize, 1+rr-buf, recsize, &rr, "readrec 3"))
			FATAL("input record `%.30s...' too long", buf);
		*rr = 0;
		isrec = (c == EOF && rr == buf) ? false : true;
	}
	*pbuf = buf;
	*pbufsize = bufsize;
	DPRINTF("readrec saw <%s>, returns %d\n", buf, isrec);
	return isrec;
}

char *getargv(int n)	/* get ARGV[n] */
{
	Array *ap;
	Cell *x;
	char *s, temp[50];
	Value v;
	extern Cell *ARGVcell;

	ap = (Array *)ARGVcell->sval;
	snprintf(temp, sizeof(temp), "%d", n);
	if (lookup(temp, ap) == NULL)
		return NULL;
	v.i = 0;
	x = setsymtab(temp, NULL, v, STR, ap);
	s = getsval(x);
	DPRINTF("getargv(%d) returns |%s|\n", n, s);
	return s;
}

void setclvar(char *s)	/* set var=value from s */
{
	int r;
	char *e, *p;
	Cell *q;
	Value v;

/* commit f3d9187d4e0f02294fb1b0e31152070506314e67 broke T.argv test */
/* I don't understand why it was changed. */

	v.i = 0;
	for (p=s; *p != '='; p++)
		;
	e = p;
	*p++ = 0;
	p = qstring(p, '\0');
	if((q = lookup(s, symtab)) != NULL){
		if(strcmp(q->sval, p) != 0){
			if(freeable(q))
				xfree(q->sval);
			q->sval = p;
			q->tval |= STR;
			if(r = is_number(p, &v))
				q->tval |= r;
		}else
			free(p);
	}else{
		q = setsym(s, p, symtab);
		if(q->sval == EMPTY){
			q->sval = p;
			q->tval |= STR;
		}else
			free(p);
	}
	DPRINTF("command line set %s to |%s|\n", s, q->sval);
	*e = '=';
}


void fldbld(void)	/* create fields from current record */
{
	/* this relies on having fields[] the same length as $0 */
	/* the fields are all stored in this one array with \0's */
	/* possibly with a final trailing \0 not associated with any field */
	char *r, *fr, sep;
	Cell *p;
	int i, j, n, t;
	Value v;

	if (donefld)
		return;
	if (!isstr(fldtab[0]))
		getsval(fldtab[0]);
	r = fldtab[0]->sval;
	n = strlen(r);
	if (n > fieldssize) {
		fields = (char *) REALLOC(fields,n+2); /* possibly 2 final \0s */
		fieldssize = n;
	}
	fr = fields;
	i = 0;	/* number of fields accumulated here */
	if (inputFS == NULL)	/* make sure we have a copy of FS */
		savefs();
	if (strlen(inputFS) > 1) {	/* it's a regular expression */
		i = refldbld(r, inputFS);
	} else if ((sep = *inputFS) == ' ') {	/* default whitespace */
		for (i = 0; ; ) {
			while (*r == ' ' || *r == '\t' || *r == '\n')
				r++;
			if (*r == 0)
				break;
			i++;
			if (i > nfields)
				growfldtab(i);
			if (freeable(fldtab[i]))
				xfree(fldtab[i]->sval);
			fldtab[i]->sval = fr;
			fldtab[i]->tval = FLD | STR | DONTFREE;
			do
				*fr++ = *r++;
			while (*r != ' ' && *r != '\t' && *r != '\n' && *r != '\0');
			*fr++ = 0;
		}
		*fr = 0;
	} else if ((sep = *inputFS) == 0) {	/* new: FS="" => 1 char/field */
		for (i = 0; *r != '\0'; ) {
			char buf[10];
			i++;
			if (i > nfields)
				growfldtab(i);
			if (freeable(fldtab[i]))
				xfree(fldtab[i]->sval);
			n = u8_nextlen(r);
			for (j = 0; j < n; j++)
				buf[j] = *r++;
			buf[j] = '\0';
			fldtab[i]->sval = tostring(buf);
			fldtab[i]->tval = FLD | STR;
		}
		*fr = 0;
	} else if (*r != 0) {	/* if 0, it's a null field */
		/* subtle case: if length(FS) == 1 && length(RS > 0)
		 * \n is NOT a field separator (cf awk book 61,84).
		 * this variable is tested in the inner while loop.
		 */
		int rtest = '\n';  /* normal case */
		if (strlen(*RS) > 0)
			rtest = '\0';
		for (;;) {
			i++;
			if (i > nfields)
				growfldtab(i);
			if (freeable(fldtab[i]))
				xfree(fldtab[i]->sval);
			fldtab[i]->sval = fr;
			fldtab[i]->tval = FLD | STR | DONTFREE;
			while (*r != sep && *r != rtest && *r != '\0')	/* \n is always a separator */
				*fr++ = *r++;
			*fr++ = 0;
			if (*r++ == 0)
				break;
		}
		*fr = 0;
	}
	if (i > nfields)
		FATAL("record `%.30s...' has too many fields; can't happen", r);
	cleanfld(i+1, lastfld);	/* clean out junk from previous record */
	lastfld = i;
	donefld = true;
	for (j = 1; j <= lastfld; j++) {
		p = fldtab[j];
		if(t = is_number(p->sval, &v)) {
			p->val = v;
			p->tval |= t;
		}
	}
	setival(nfloc, (Awknum) lastfld);
	donerec = true; /* restore */
	if (dbg) {
		for (j = 0; j <= lastfld; j++) {
			p = fldtab[j];
			printf("field %d (%s): |%s|\n", j, p->nval, p->sval);
		}
	}
}

void cleanfld(int n1, int n2)	/* clean out fields n1 .. n2 inclusive */
{				/* nvals remain intact */
	Cell *p;
	int i;

	for (i = n1; i <= n2; i++) {
		p = fldtab[i];
		if (freeable(p))
			xfree(p->sval);
		p->sval = EMPTY,
		p->tval = FLD | STR | DONTFREE;
	}
}

void newfld(int n)	/* add field n after end of existing lastfld */
{
	if (n > nfields)
		growfldtab(n);
	cleanfld(lastfld+1, n);
	lastfld = n;
	setival(nfloc, (Awknum) n);
}

void setlastfld(int n)	/* set lastfld cleaning fldtab cells if necessary */
{
	if (n < 0)
		FATAL("cannot set NF to a negative value");
	if (n > nfields)
		growfldtab(n);

	if (lastfld < n)
	    cleanfld(lastfld+1, n);
	else
	    cleanfld(n+1, lastfld);

	lastfld = n;
}

Cell *fieldadr(int n)	/* get nth field */
{
	if (n < 0)
		FATAL("trying to access out of range field %d", n);
	if (n > nfields)	/* fields after NF are empty */
		growfldtab(n);	/* but does not increase NF */
	return(fldtab[n]);
}

void growfldtab(int n)	/* make new fields up to at least $n */
{
	int nf = 2 * nfields;
	size_t s;

	if (n > nf)
		nf = n;
	s = (nf+1) * (sizeof (struct Cell *));  /* freebsd: how much do we need? */
	if (s / sizeof(struct Cell *) - 1 == (size_t)nf) /* didn't overflow */
		fldtab = (Cell **) REALLOC(fldtab, s);
	else					/* overflow sizeof int */
		xfree(fldtab);	/* make it null */
	makefields(nfields+1, nf);
	nfields = nf;
}

int refldbld(const char *rec, const char *fs)	/* build fields from reg expr in FS */
{
	/* this relies on having fields[] the same length as $0 */
	/* the fields are all stored in this one array with \0's */
	char *fr;
	int i, tempstat, n;
	fa *pfa;

	n = strlen(rec);
	if (n > fieldssize) {
		fields = (char *) REALLOC(fields,n+1);
		fieldssize = n;
	}
	fr = fields;
	*fr = '\0';
	if (*rec == '\0')
		return 0;
	pfa = makedfa(fs, 1);
	DPRINTF("into refldbld, rec = <%s>, pat = <%s>\n", rec, fs);
	tempstat = pfa->initstat;
	for (i = 1; ; i++) {
		if (i > nfields)
			growfldtab(i);
		if (freeable(fldtab[i]))
			xfree(fldtab[i]->sval);
		fldtab[i]->tval = FLD | STR | DONTFREE;
		fldtab[i]->sval = fr;
		DPRINTF("refldbld: i=%d\n", i);
		if (nematch(pfa, rec)) {
			pfa->initstat = 2;	/* horrible coupling to b.c */
			DPRINTF("match %s (%d chars)\n", patbeg, patlen);
			strncpy(fr, rec, patbeg-rec);
			fr += patbeg - rec + 1;
			*(fr-1) = '\0';
			rec = patbeg + patlen;
		} else {
			DPRINTF("no match %s\n", rec);
			strcpy(fr, rec);
			pfa->initstat = tempstat;
			break;
		}
	}
	return i;
}

void recbld(void)	/* create $0 from $1..$NF if necessary */
{
	int i;
	char *r, *p;
	char *sep = getsval(ofsloc);

	if (donerec)
		return;
	r = record;
	for (i = 1; i <= *NF; i++) {
		p = getsval(fldtab[i]);
		if (!adjbuf(&record, &recsize, 1+strlen(p)+r-record, recsize, &r, "recbld 1"))
			FATAL("created $0 `%.30s...' too long", record);
		while ((*r = *p++) != 0)
			r++;
		if (i < *NF) {
			if (!adjbuf(&record, &recsize, 2+strlen(sep)+r-record, recsize, &r, "recbld 2"))
				FATAL("created $0 `%.30s...' too long", record);
			for (p = sep; (*r = *p++) != 0; )
				r++;
		}
	}
	if (!adjbuf(&record, &recsize, 2+r-record, recsize, &r, "recbld 3"))
		FATAL("built giant record `%.30s...'", record);
	*r = '\0';
	DPRINTF("in recbld inputFS=%s, fldtab[0]=%p\n", inputFS, (void*)fldtab[0]);

	if (freeable(fldtab[0]))
		xfree(fldtab[0]->sval);
	fldtab[0]->tval = REC | STR | DONTFREE;
	fldtab[0]->sval = record;

	DPRINTF("in recbld inputFS=%s, fldtab[0]=%p\n", inputFS, (void*)fldtab[0]);
	DPRINTF("recbld = |%s|\n", record);
	donerec = true;
}

int	errorflag	= 0;

void yyerror(const char *s)
{
	SYNTAX("%s", s);
}

void SYNTAX(const char *fmt, ...)
{
	extern char *cmdname, *curfname;
	va_list varg;

	fprintf(awkstderr, "%s: ", cmdname);
	va_start(varg, fmt);
	vfprintf(awkstderr, fmt, varg);
	va_end(varg);
	fprintf(awkstderr, " at source line %d", lineno);
	if (curfname != NULL)
		fprintf(awkstderr, " in function %s", curfname);
	if (compile_time == COMPILING && cursource() != NULL)
		fprintf(awkstderr, " source file %s", cursource());
	fprintf(awkstderr, "\n");
	errorflag = 2;
	eprint();
	bracecnt = brackcnt = parencnt = 0;
	infunc = false;
}

void bracecheck(void)
{
	int c;
	static int beenhere = 0;

	if (beenhere++)
		return;
	while ((c = input()) != EOF && c != '\0')
		bclass(c);
	bcheck2(bracecnt, '{', '}');
	bcheck2(brackcnt, '[', ']');
	bcheck2(parencnt, '(', ')');
}

void bcheck2(int n, int c1, int c2)
{
	if (n == 1)
		fprintf(awkstderr, "\tmissing %c\n", c2);
	else if (n > 1)
		fprintf(awkstderr, "\t%d missing %c's\n", n, c2);
	else if (n == -1)
		fprintf(awkstderr, "\textra %c\n", c2);
	else if (n < -1)
		fprintf(awkstderr, "\t%d extra %c's\n", -n, c2);
}

void FATAL(const char *fmt, ...)
{
	extern char *cmdname;
	va_list varg;

	fflush(awkstdout);
	fprintf(awkstderr, "%s: ", cmdname);
	va_start(varg, fmt);
	vfprintf(awkstderr, fmt, varg);
	va_end(varg);
	ERROR();
	if(runnerup != NULL){
		fflush(awkstderr);
		longjmp(evalenv, -1);
	}
	if (dbg > 1)		/* core dump if serious debugging on */
		abort();
	exit(2);
}

void WARNING(const char *fmt, ...)
{
	extern char *cmdname;
	va_list varg;

	fflush(awkstdout);
	fprintf(awkstderr, "%s: ", cmdname);
	va_start(varg, fmt);
	vfprintf(awkstderr, fmt, varg);
	va_end(varg);
	ERROR();
}

void ERROR(void)
{
	extern TNode *curnode;

	if (runnerup != NULL) {
		fprintf(awkstderr, " eval(\"%s\")\n", evalstr);
		eprint();
	} else if (compile_time != ERROR_PRINTING) {
		fprintf(awkstderr, "\n");
		if (NR && *NR > 0) {
			fprintf(awkstderr, " input record number %d", (int) (*FNR));
			if (strcmp(*FILENAME, "-") != 0)
				fprintf(awkstderr, ", file %s", *FILENAME);
			fprintf(awkstderr, "\n");
		}
		if (curnode)
			fprintf(awkstderr, " source line number %d", curnode->lineno);
		else if (lineno)
			fprintf(awkstderr, " source line number %d", lineno);
		if (compile_time == COMPILING && cursource() != NULL)
			fprintf(awkstderr, " source file %s", cursource());
		fprintf(awkstderr, "\n");
		eprint();
	} else
		fprintf(awkstderr, "\n");
}

void eprint(void)	/* try to print context around error */
{
	char *p, *q;
	int c;
	extern char ebuf[], *ep;

	if (compile_time != COMPILING || ebuf == ep)
		return;
	if (ebuf == ep)
		return;
	p = ep - 1;
	if (p > ebuf && *p == '\n')
		p--;
	for ( ; p > ebuf && *p != '\n' && *p != '\0'; p--)
		;
	while (*p == '\n')
		p++;
	fprintf(awkstderr, " context is\n\t");
	for (q=ep-1; q>=p && *q!=' ' && *q!='\t' && *q!='\n'; q--)
		;
	for ( ; p < q; p++)
		if (*p)
			putc(*p, awkstderr);
	fprintf(awkstderr, " >>> ");
	for ( ; p < ep; p++)
		if (*p)
			putc(*p, awkstderr);
	fprintf(awkstderr, " <<< ");
	if (*ep)
		while ((c = input()) != '\n' && c != '\0' && c != EOF) {
			putc(c, awkstderr);
			bclass(c);
		}
	putc('\n', awkstderr);
	ep = ebuf;
}

void bclass(int c)
{
	switch (c) {
	case '{': bracecnt++; break;
	case '}': bracecnt--; break;
	case '[': brackcnt++; break;
	case ']': brackcnt--; break;
	case '(': parencnt++; break;
	case ')': parencnt--; break;
	}
}

Awkfloat errcheck(Awkfloat x, const char *s)
{

	if (errno == EDOM) {
		errno = 0;
		WARNING("%s argument out of domain", s);
		x = 1;
	} else if (errno == ERANGE) {
		errno = 0;
		WARNING("%s result out of range", s);
		x = 1;
	}
	return x;
}

int isclvar(const char *s)	/* is s of form var=something ? */
{
	const char *os = s;

	if (!isalpha((int) *s) && *s != '_')
		return 0;
	for ( ; *s; s++)
		if (!(isalnum((int) *s) || *s == '_'))
			break;
	return *s == '=' && s > os;
}

/* strtod is supposed to be a proper test of what's a valid number */
/* appears to be broken in gcc on linux: thinks 0x123 is a valid FP number */
/* wrong: violates 4.10.1.4 of ansi C standard */

/* well, not quite. As of C99, hex floating point is allowed. so this is
 * a bit of a mess. We work around the mess by checking for a hexadecimal
 * value and disallowing it. Similarly, we now follow gawk and allow only
 * +nan, -nan, +inf, and -inf for NaN and infinity values.
 */

/*
 * This routine now has a more complicated interface, the main point
 * being to avoid the double conversion of a string to double, and
 * also to convey out, if requested, the information that the numeric
 * value was a leading string or is all of the string. The latter bit
 * is used in getfval().
 */

int is_valid_number(const char *s, bool trailing_stuff_ok,
			bool *no_trailing, char **trail, Value *result)
{
	int ret;
	Awkfloat r;
	Awkword v;
	char c, *ep;
	bool is_nan = false;
	bool is_inf = false;

	if (no_trailing)
		*no_trailing = false;
	ret = 0;
	if(isspace((int)*s)){
		ret |= STR;
		do
			s++;
		while (isspace((int) *s));
	}
	if(*s == '.')
		goto fp;

	v = strtoull(s, &ep, 0);
	if(trail != NULL)
		*trail = ep;
	if(ep == s)
		return 0;
	if(result != NULL)
		result->u = v;
	if((c = *ep) != 0 && (c == '.' || c == 'e' || c == 'E'))	/* floating point */
		goto fp;
	if(*ep != 0){
		ret |= STR;
		while(isspace((int)*ep))
			ep++;
	}
	if(*ep != 0){
		if(!trailing_stuff_ok)
			return 0;
	}else if(no_trailing != NULL)
		*no_trailing = true;
	return ret | NUM;

fp:
	/* no hex floating point, sorry */
	if (s[0] == '0' && tolower(s[1]) == 'x')
		return 0;

	/* allow +nan, -nan, +inf, -inf, any other letter, no */
	if (s[0] == '+' || s[0] == '-') {
		is_nan = (strncasecmp(s+1, "nan", 3) == 0);
		is_inf = (strncasecmp(s+1, "inf", 3) == 0);
		if ((is_nan || is_inf)
		    && (isspace((int) s[4]) || s[4] == '\0'))
			goto convert;
		else if (! isdigit(s[1]) && s[1] != '.')
			return 0;
	}else if (! isdigit(s[0]) && s[0] != '.')
		return 0;

convert:
	errno = 0;
	r = strtod(s, &ep);
	if(trail != NULL)
		*trail = ep;
	if (ep == s || errno == ERANGE)
		return 0;

	if (isnan(r) && s[0] == '-' && signbit(r) == 0)
		r = -r;

	if (result != NULL)
		result->f = r;

	/*
	 * check for trailing stuff
	 */
	if(*ep != 0){
		ret |= STR;
		while(isspace((int)*ep))
			ep++;
	}
	if(*ep != 0){
		if(!trailing_stuff_ok)
			return 0;
	}else if(no_trailing != NULL)
		*no_trailing = true;
	return ret | NUM | FLT;
}
