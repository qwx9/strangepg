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
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "awk.h"
#include AWKTAB

#define	FULLTAB	2	/* rehash when table gets this x full */
#define	GROWTAB 4	/* grow table by this factor */

Array	*symtab;	/* main symbol table */

char	**FS;		/* initial field sep */
char	**RS;		/* initial record sep */
char	**OFS;		/* output field sep */
char	**ORS;		/* output record sep */
char	**OFMT;		/* output format for numbers */
char	**CONVFMT;	/* format for conversions in getsval */
Awknum *NF;		/* number of fields in current record */
Awknum *NR;		/* number of current record */
Awknum *FNR;		/* number of current record in current file */
char	**FILENAME;	/* current filename argument */
Awknum *ARGC;		/* number of arguments from command line */
char	**SUBSEP;	/* subscript separator for a[i,j,k]; default \034 */
Awknum *RSTART;	/* start of re matched with ~; origin 1 (!) */
Awknum *RLENGTH;	/* length of same */

Cell	*fsloc;		/* FS */
Cell	*nrloc;		/* NR */
Cell	*nfloc;		/* NF */
Cell	*fnrloc;	/* FNR */
Cell	*ofsloc;	/* OFS */
Cell	*orsloc;	/* ORS */
Cell	*rsloc;		/* RS */
Cell	*ARGVcell;	/* cell with symbol table containing ARGV[...] */
Cell	*rstartloc;	/* RSTART */
Cell	*rlengthloc;	/* RLENGTH */
Cell	*subseploc;	/* SUBSEP */
Cell	*symtabloc;	/* SYMTAB */

Cell	*nullloc;	/* a guaranteed empty cell */
TNode	*nullnode;	/* zero&null, converted into a node for comparisons */
Cell	*literal0;

extern Cell **fldtab;

void syminit(void)	/* initialize symbol table with builtin vars */
{
	Value v;

	v.i = 0;
	literal0 = setsymtab("0", "0", v, NUM|STR|CON|DONTFREE, symtab);
	/* this is used for if(x)... tests: */
	nullloc = setsymtab("$zero&null", NULL, v, NUM|STR|CON|DONTFREE, symtab);
	nullnode = celltonode(nullloc, CCON);

	/* default FS and OFS assumed 0x20 in tests */
	fsloc = setsymtab("FS", " ", v, STR|DONTFREE, symtab);
	FS = &fsloc->sval;
	rsloc = setsymtab("RS", "\n", v, STR|DONTFREE, symtab);
	RS = &rsloc->sval;
	ofsloc = setsymtab("OFS", " ", v, STR|DONTFREE, symtab);
	OFS = &ofsloc->sval;
	orsloc = setsymtab("ORS", "\n", v, STR|DONTFREE, symtab);
	ORS = &orsloc->sval;
	OFMT = &setsymtab("OFMT", "%.6g", v, STR|DONTFREE, symtab)->sval;
	CONVFMT = &setsymtab("CONVFMT", "%.6g", v, STR|DONTFREE, symtab)->sval;
	FILENAME = &setsymtab("FILENAME", NULL, v, STR|DONTFREE, symtab)->sval;
	nfloc = setsymtab("NF", NULL, v, NUM, symtab);
	NF = &nfloc->val.i;
	nrloc = setsymtab("NR", NULL, v, NUM, symtab);
	NR = &nrloc->val.i;
	fnrloc = setsymtab("FNR", NULL, v, NUM, symtab);
	FNR = &fnrloc->val.i;
	subseploc = setsymtab("SUBSEP", "\034", v, STR|DONTFREE, symtab);
	SUBSEP = &subseploc->sval;
	rstartloc = setsymtab("RSTART", NULL, v, NUM, symtab);
	RSTART = &rstartloc->val.i;
	rlengthloc = setsymtab("RLENGTH", NULL, v, NUM, symtab);
	RLENGTH = &rlengthloc->val.i;
	symtabloc = setsymtab("SYMTAB", NULL, v, ARR, symtab);
	symtabloc->sval = (char *) symtab;
}

void arginit(int ac, char **av)	/* set up ARGV and ARGC */
{
	Array *ap;
	Cell *cp;
	int i;
	char temp[50];
	Value v;

	v.i = (Awknum)ac;
	ARGC = &setsymtab("ARGC", NULL, v, NUM, symtab)->val.i;
	v.i = 0;
	cp = setsymtab("ARGV", NULL, v, ARR, symtab);
	ap = makesymtab(NSYMTAB);	/* could be (int) ARGC as well */
	cp->sval = (char *) ap;
	for (i = 0; i < ac; i++) {
		sprintf(temp, "%d", i);
		setsym(temp, *av, ap);
		av++;
	}
	ARGVcell = cp;
}

Array *makesymtab(int n)	/* make a new symbol table */
{
	Array *ap;
	Cell **tp;

	ap = (Array *) MALLOC(sizeof(*ap));
	tp = (Cell **) CALLOC(n, sizeof(*tp));
	ap->nelem = 0;
	ap->size = n;
	ap->tab = tp;
	return(ap);
}

void freesymtab(Cell *ap)	/* free a symbol table */
{
	Cell *cp, *temp;
	Array *tp;
	int i;

	if (!isarr(ap))
		return;
	tp = (Array *) ap->sval;
	if (tp == NULL)
		return;
	for (i = 0; i < tp->size; i++) {
		for (cp = tp->tab[i]; cp != NULL; cp = temp) {
			xfree(cp->nval);
			if (freeable(cp))
				xfree(cp->sval);
			temp = cp->cnext;	/* avoids freeing then using */
			free(cp);
			tp->nelem--;
		}
		tp->tab[i] = NULL;
	}
	if (tp->nelem != 0)
		WARNING("can't happen: inconsistent element count freeing %s", ap->nval);
	free(tp->tab);
	free(tp);
}

void freeelem(Cell *ap, const char *s)	/* free elem s from ap (i.e., ap["s"] */
{
	Array *tp;
	Cell *p, *prev = NULL;
	int h;

	tp = (Array *) ap->sval;
	h = hash(s, tp->size);
	for (p = tp->tab[h]; p != NULL; prev = p, p = p->cnext)
		if (strcmp(s, p->nval) == 0) {
			if (prev == NULL)	/* 1st one */
				tp->tab[h] = p->cnext;
			else			/* middle somewhere */
				prev->cnext = p->cnext;
			if (freeable(p))
				xfree(p->sval);
			free(p->nval);
			free(p);
			tp->nelem--;
			return;
		}
}

Cell *setsym(const char *n, const char *s, Array *tp)
{
	int r;
	Value v;

	v.i = 0;
	if(r = is_number(s, &v))
		return setsymtab(n, r & STR ? s : NULL, v, r, tp);
	else
		return setsymtab(n, s, v, STR, tp);
}

Cell *setsymtab(const char *n, const char *s, Value v, unsigned t, Array *tp)
{
	int h;
	Cell *p;

	if (n != NULL && (p = lookup(n, tp)) != NULL) {
		DPRINTF("setsymtab found %p: n=%s s=\"%s\" i=%lld f=%g t=%o\n",
			(void*)p, NN(p->nval), NN(p->sval), p->val.i, p->val.f, p->tval);
		return(p);
	}
	p = (Cell *) MALLOC(sizeof(*p));
	p->nval = tostring(n);
	p->sval = s ? tostring(s) : EMPTY;
	p->val = v;
	p->tval = t;
	p->csub = CUNK;
	p->ctype = OCELL;
	tp->nelem++;
	if (tp->nelem > FULLTAB * tp->size)
		rehash(tp);
	h = hash(n, tp->size);
	p->cnext = tp->tab[h];
	tp->tab[h] = p;
	DPRINTF("setsymtab set %p: n=%s s=\"%s\" i=%lld f=%g t=%o\n",
		(void*)p, p->nval, p->sval, p->val.i, p->val.f, p->tval);
	return(p);
}

int hash(const char *s, int n)	/* form hash value for string s */
{
	unsigned hashval;

	for (hashval = 0; *s != '\0'; s++)
		hashval = (*s + 31 * hashval);
	return hashval % n;
}

void rehash(Array *tp)	/* rehash items in small table into big one */
{
	int i, nh, nsz;
	Cell *cp, *op, **np;

	nsz = GROWTAB * tp->size;
	np = (Cell **) CALLOC(nsz, sizeof(*np));
	for (i = 0; i < tp->size; i++) {
		for (cp = tp->tab[i]; cp; cp = op) {
			op = cp->cnext;
			nh = hash(cp->nval, nsz);
			cp->cnext = np[nh];
			np[nh] = cp;
		}
	}
	free(tp->tab);
	tp->tab = np;
	tp->size = nsz;
}

Cell *lookup(const char *s, Array *tp)	/* look for s in tp */
{
	Cell *p;
	int h;

	h = hash(s, tp->size);
	for (p = tp->tab[h]; p != NULL; p = p->cnext)
		if (strcmp(s, p->nval) == 0)
			return(p);	/* found it */
	return(NULL);			/* not found */
}

Awkfloat setfval(Cell *vp, Awkfloat f)	/* set float val of a Cell */
{
	int fldno;

	f += 0.0;		/* normalise negative zero to positive zero */
	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "assign to");
	if (isfld(vp)) {
		donerec = false;	/* mark $0 invalid */
		fldno = atoi(vp->nval);
		if (fldno > *NF)
			newfld(fldno);
		DPRINTF("setting field %d to %g\n", fldno, f);
	} else if (&vp->val.i == NF) {
		donerec = false;	/* mark $0 invalid */
		setlastfld(f);
		DPRINTF("setfval: setting NF to %g\n", f);
	} else if (isrec(vp)) {
		donefld = false;	/* mark $1... invalid */
		donerec = true;
		savefs();
	} else if (vp == ofsloc) {
		if (!donerec)
			recbld();
	}
	if (freeable(vp))
		xfree(vp->sval); /* free any previous string */
	vp->tval &= ~(STR|CONVC|CONVO); /* mark string invalid */
	vp->fmt = NULL;
	vp->tval |= NUM | FLT;	/* mark number ok */
	if (f == -0)  /* who would have thought this possible? */
		f = 0;
	DPRINTF("setfval %p: %s = %g, t=%o\n", (void*)vp, NN(vp->nval), f, vp->tval);
	return vp->val.f = f;
}

Awknum setival(Cell *vp, Awknum f)	/* set int val of a Cell */
{
	int fldno;

	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "assign to");
	if (isfld(vp)) {
		donerec = false;	/* mark $0 invalid */
		fldno = atoi(vp->nval);
		if (fldno > *NF)
			newfld(fldno);
		DPRINTF("setting field %d to %lld\n", fldno, f);
	} else if (&vp->val.i == NF) {
		donerec = false;	/* mark $0 invalid */
		setlastfld(f);
		DPRINTF("setival: setting NF to %lld\n", f);
	} else if (isrec(vp)) {
		donefld = false;	/* mark $1... invalid */
		donerec = true;
		savefs();
	} else if (vp == ofsloc) {
		if (!donerec)
			recbld();
	}
	if (freeable(vp))
		xfree(vp->sval); /* free any previous string */
	vp->tval &= ~(STR|CONVC|CONVO|FLT); /* mark string invalid; force int */
	vp->fmt = NULL;
	vp->tval |= NUM;	/* mark number ok */
	DPRINTF("setival %p: %s = %lld, t=%o\n", (void*)vp, NN(vp->nval), f, vp->tval);
	return vp->val.i = f;
}

void setval(Cell *vp, Cell *v)
{
	getval(v);
	if(v->tval & FLT)
		setfval(vp, v->val.f);
	else
		setival(vp, v->val.i);
}

void funnyvar(Cell *vp, const char *rw)
{
	if (isarr(vp))
		FATAL("can't %s %s; it's an array name.", rw, vp->nval);
	if (vp->tval & FCN)
		FATAL("can't %s %s; it's a function.", rw, vp->nval);
	WARNING("funny variable %p: n=%s s=\"%s\" i=%lld f=%g t=%o",
		(void *)vp, vp->nval, vp->sval, vp->val.i, vp->val.f, vp->tval);
}

char *setsval(Cell *vp, const char *s)	/* set string val of a Cell */
{
	char *t;
	int fldno;
	Awknum f;

	DPRINTF("starting setsval %p: %s = \"%s\", t=%o, done=%d,%d\n",
		(void*)vp, NN(vp->nval), s, vp->tval, donerec, donefld);
	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "assign to");
	if (isfld(vp)) {
		donerec = false;	/* mark $0 invalid */
		fldno = atoi(vp->nval);
		if (fldno > *NF)
			newfld(fldno);
		DPRINTF("setting field %d to %s (%p)\n", fldno, s, (const void*)s);
	} else if (isrec(vp)) {
		donefld = false;	/* mark $1... invalid */
		donerec = true;
		savefs();
	} else if (vp == ofsloc) {
		if (!donerec)
			recbld();
	}
	t = vp->sval;
	if(s != t || s == NULL){
		t = s ? tostring(s) : EMPTY;	/* in case it's self-assign */
		if (freeable(vp))
			xfree(vp->sval);
	}
	vp->tval &= ~(NUM|DONTFREE|CONVC|CONVO);
	vp->tval |= STR;
	vp->fmt = NULL;
	DPRINTF("setsval %p: %s = \"%s (%p) \", t=%o r,f=%d,%d\n",
		(void*)vp, NN(vp->nval), t, (void*)t, vp->tval, donerec, donefld);
	vp->sval = t;
	if (&vp->val.i == NF) {
		donerec = false;	/* mark $0 invalid */
		f = getival(vp);
		setlastfld(f);
		DPRINTF("setsval: setting NF to %lld\n", f);
	}

	return(vp->sval);
}

Value getval(Cell *vp)
{
	int r;
	Value v;
	bool no_trailing;

	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "read value of");
	if (isfld(vp) && !donefld)
		fldbld();
	else if (isrec(vp) && !donerec)
		recbld();
	r = 0;
	if(!isnum(vp)){
		if(r = is_valid_number(vp->sval, true, &no_trailing, NULL, &v)){
			if (no_trailing && (vp->tval & CON) == 0)
				vp->tval |= r;	/* make NUM only sparingly */
			if(r & FLT)
				vp->tval |= FLT;
			else
				vp->tval &= ~FLT;
		} else {
			v.i = 0;
			vp->tval &= ~FLT;
		}
		vp->val = v;
	}
	return vp->val;
}

Awknum getival(Cell *vp)	/* get int val of a Cell */
{
	getval(vp);
	DPRINTF("getival %p: %s = %lld,%g, t=%o\n",
		(void*)vp, NN(vp->nval), vp->val.i, vp->val.f, vp->tval);
	return (vp->tval & FLT) == 0 ? vp->val.i : vp->val.f;
}

Awkfloat getfval(Cell *vp)	/* get float val of a Cell */
{
	getval(vp);
	DPRINTF("getfval %p: %s = %lld,%g, t=%o\n",
		(void*)vp, NN(vp->nval), vp->val.i, vp->val.f, vp->tval);
	return (vp->tval & FLT) == 0 ? vp->val.i : vp->val.f;
}

static const char *get_inf_nan(Awkfloat d)
{
	if (isinf(d)) {
		return (d < 0 ? "-inf" : "+inf");
	} else if (isnan(d)) {
		return (signbit(d) != 0 ? "-nan" : "+nan");
	} else
		return NULL;
}

static inline void update_str_val(Cell *vp, char **fmt)
{
	const char *p;
	char s[256];

	if (freeable(vp))
		xfree(vp->sval);
	if ((p = get_inf_nan(vp->val.i)) != NULL)
		strcpy(s, p);
	else if((vp->tval & FLT) == 0)
		snprintf(s, sizeof(s), "%lld", vp->val.i);
	else
		snprintf(s, sizeof(s), *fmt, vp->val.f);
	vp->sval = tostring(s);
	vp->tval &= ~DONTFREE;
	vp->tval |= STR;
}

static char *get_str_val(Cell *vp, char **fmt)        /* get string val of a Cell */
{
	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "read value of");
	if (isfld(vp) && ! donefld)
		fldbld();
	else if (isrec(vp) && ! donerec)
		recbld();

	/*
	 * ADR: This is complicated and more fragile than is desirable.
	 * Retrieving a string value for a number associates the string
	 * value with the scalar.  Previously, the string value was
	 * sticky, meaning if converted via OFMT that became the value
	 * (even though POSIX wants it to be via CONVFMT). Or if CONVFMT
	 * changed after a string value was retrieved, the original value
	 * was maintained and used.  Also not per POSIX.
	 *
	 * We work around this design by adding two additional flags,
	 * CONVC and CONVO, indicating how the string value was
	 * obtained (via CONVFMT or OFMT) and _also_ maintaining a copy
	 * of the pointer to the xFMT format string used for the
	 * conversion.  This pointer is only read, **never** dereferenced.
	 * The next time we do a conversion, if it's coming from the same
	 * xFMT as last time, and the pointer value is different, we
	 * know that the xFMT format string changed, and we need to
	 * redo the conversion. If it's the same, we don't have to.
	 *
	 * There are also several cases where we don't do a conversion,
	 * such as for a field (see the checks below).
	 */

	if (isstr(vp) == 0) {
		update_str_val(vp, fmt);
		if (fmt == OFMT) {
			vp->tval &= ~CONVC;
			vp->tval |= CONVO;
		} else {
			/* CONVFMT */
			vp->tval &= ~CONVO;
			vp->tval |= CONVC;
		}
		vp->fmt = *fmt;
	} else if ((vp->tval & DONTFREE) != 0 || ! isnum(vp) || isfld(vp)) {
		goto done;
	} else if (isstr(vp)) {
		if (fmt == OFMT) {
			if ((vp->tval & CONVC) != 0
			    || ((vp->tval & CONVO) != 0 && vp->fmt != *fmt)) {
				update_str_val(vp, fmt);
				vp->tval &= ~CONVC;
				vp->tval |= CONVO;
				vp->fmt = *fmt;
			}
		} else {
			/* CONVFMT */
			if ((vp->tval & CONVO) != 0
			    || ((vp->tval & CONVC) != 0 && vp->fmt != *fmt)) {
				update_str_val(vp, fmt);
				vp->tval &= ~CONVO;
				vp->tval |= CONVC;
				vp->fmt = *fmt;
			}
		}
	}
done:
	DPRINTF("getsval %p: %s = \"%s (%p)\", t=%o\n",
		(void*)vp, NN(vp->nval), vp->sval, (void*)vp->sval, vp->tval);
	return(vp->sval);
}

char *getsval(Cell *vp)       /* get string val of a Cell */
{
      return get_str_val(vp, CONVFMT);
}

char *getpssval(Cell *vp)     /* get string val of a Cell for print */
{
      return get_str_val(vp, OFMT);
}


char *tostring(const char *s)	/* make a copy of string s */
{
	char *p = STRDUP(s);
	return(p);
}

/* FIXME: what the fuck */
char *tostringN(const char *s, size_t n)	/* make a copy of string s */
{
	char *p;

	p = (char *) MALLOC(n);
	strcpy(p, s);
	return(p);
}

Cell *catstr(Cell *a, Cell *b) /* concatenate a and b */
{
	Cell *c;
	Value v;
	char *sa = getsval(a);
	char *sb = getsval(b);
	size_t l = strlen(sa) + strlen(sb) + 1;
	static char *buf;
	static int bufsz;

	l++;	// add room for ' '
	if(buf == NULL){
		bufsz = 2 * recsize;
		if(l > bufsz)
			bufsz = l;
		buf = MALLOC(bufsz);
	}
	if(!adjbuf(&buf, &bufsz, l, recsize, NULL, "catstr"))
		FATAL("catstr: out of memory");
	snprintf(buf, l, "%s%s ", sa, sb);

	// See string() in lex.c; a string "xx" is stored in the symbol
	// table as "xx ".
	//snprintf(buf, l, "%s ", p);
	v.i = 0;
	c = setsymtab(buf, NULL, v, CON|STR|DONTFREE, symtab);
	buf[l-2] = '\0';
	c->sval = STRDUP(buf);
	return c;
}

char *qstring(const char *is, int delim)	/* collect string up to next delim */
{
	int c, n;
	const uschar *s = (const uschar *) is;
	uschar *buf, *bp;

	buf = (uschar *) defalloc(strlen(is)+3);
	for (bp = buf; (c = *s) != delim; s++) {
		if (c == '\n')
			SYNTAX( "newline in string %.20s...", is );
		else if (c != '\\')
			*bp++ = c;
		else {	/* \something */
			c = *++s;
			if (c == 0) {	/* \ at end */
				*bp++ = '\\';
				break;	/* for loop */
			}
			switch (c) {
			case '\\':	*bp++ = '\\'; break;
			case 'n':	*bp++ = '\n'; break;
			case 't':	*bp++ = '\t'; break;
			case 'b':	*bp++ = '\b'; break;
			case 'f':	*bp++ = '\f'; break;
			case 'r':	*bp++ = '\r'; break;
			case 'v':	*bp++ = '\v'; break;
			case 'a':	*bp++ = '\a'; break;
			default:
				if (!isdigit(c)) {
					*bp++ = c;
					break;
				}
				n = c - '0';
				if (isdigit(s[1])) {
					n = 8 * n + *++s - '0';
					if (isdigit(s[1]))
						n = 8 * n + *++s - '0';
				}
				*bp++ = n;
				break;
			}
		}
	}
	*bp++ = 0;
	return (char *) buf;
}

const char *flags2str(int flags)
{
	static const struct ftab {
		const char *name;
		int value;
	} flagtab[] = {
		{ "NUM", NUM },
		{ "STR", STR },
		{ "DONTFREE", DONTFREE },
		{ "CON", CON },
		{ "ARR", ARR },
		{ "FCN", FCN },
		{ "FLD", FLD },
		{ "REC", REC },
		{ "CONVC", CONVC },
		{ "CONVO", CONVO },
		{ NULL, 0 }
	};
	static char buf[100];
	int i;
	char *cp = buf;

	for (i = 0; flagtab[i].name != NULL; i++) {
		if ((flags & flagtab[i].value) != 0) {
			if (cp > buf)
				*cp++ = '|';
			strcpy(cp, flagtab[i].name);
			cp += strlen(cp);
		}
	}

	return buf;
}
