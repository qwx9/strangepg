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
#include <locale.h>
#include "awk.h"
#include AWKTAB

const char	*version = "version 20250804";

#define	FULLTAB	2	/* rehash when table gets this x full */
#define	GROWTAB 4	/* grow table by this factor */

Value	ZV = {.i = 0};	/* convenience */

Array	*symtab;	/* main symbol table */

char	**FS;		/* initial field sep */
char	**RS;		/* initial record sep */
char	**OFS;		/* output field sep */
char	**ORS;		/* output record sep */
char	**OFMT;		/* output format for numbers */
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
extern	FILE	*yyin;	/* lex input file */
extern	size_t	npfile;

void syminit(void)	/* initialize symbol table with builtin vars */
{
	literal0 = setsymtab("0", EMPTY, ZV, NUM|CON, symtab);
	/* this is used for if(x)... tests: */
	nullloc = setsymtab("$zero&null", EMPTY, ZV, STR|NUM|CON, symtab);
	nullnode = celltonode(nullloc, CCON);

	/* default FS and OFS assumed 0x20 in tests */
	fsloc = setsymtab("FS", " ", ZV, STR, symtab);
	FS = &fsloc->sval;
	rsloc = setsymtab("RS", "\n", ZV, STR, symtab);
	RS = &rsloc->sval;
	ofsloc = setsymtab("OFS", " ", ZV, STR, symtab);
	OFS = &ofsloc->sval;
	orsloc = setsymtab("ORS", "\n", ZV, STR, symtab);
	ORS = &orsloc->sval;
	OFMT = &setsymtab("OFMT", "%.6g", ZV, STR, symtab)->sval;
	FILENAME = &setsymtab("FILENAME", NULL, ZV, STR|DONTFREE, symtab)->sval;
	nfloc = setsymtab("NF", NULL, ZV, NUM, symtab);
	NF = &nfloc->val.i;
	nrloc = setsymtab("NR", NULL, ZV, NUM, symtab);
	NR = &nrloc->val.i;
	fnrloc = setsymtab("FNR", NULL, ZV, NUM, symtab);
	FNR = &fnrloc->val.i;
	subseploc = setsymtab("SUBSEP", "\034", ZV, STR, symtab);
	SUBSEP = &subseploc->sval;
	rstartloc = setsymtab("RSTART", NULL, ZV, NUM, symtab);
	RSTART = &rstartloc->val.i;
	rlengthloc = setsymtab("RLENGTH", NULL, ZV, NUM, symtab);
	RLENGTH = &rlengthloc->val.i;
	symtabloc = setsymtab("SYMTAB", NULL, ZV, ARR, symtab);
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
	cp = setsymtab("ARGV", NULL, ZV, ARR, symtab);
	ap = makesymtab(*ARGC);
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
	ap->type = 0;
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
	else if (isptr(ap)){	/* FIXME: shouldn't happen */
		FREE(ap->sval);
		return;
	}
	tp = (Array *) ap->sval;
	if (tp == NULL)
		return;
	for (i = 0; i < tp->size; i++) {
		for (cp = tp->tab[i]; cp != NULL; cp = temp) {
			if ((cp->tval & CON) == 0){
				xfree(cp->nval);
				if (freeable(cp))
					xfree(cp->sval);
			}
			temp = cp->cnext;	/* avoids freeing then using */
			free(cp);
			tp->nelem--;
		}
		tp->tab[i] = NULL;
	}
	if (tp->nelem != 0)
		WARNING("can't happen: inconsistent element count freeing %s", ap->nval);
	FREE(tp->tab);
	FREE(tp);
}

void freeelem(Cell *ap, const char *s)	/* free elem s from ap (i.e., ap["s"] */
{
	Array *tp;
	Cell *p, *prev = NULL;
	int h;

	if (isptr(ap))	/* FIXME: shouldn't happen */
		return;
	tp = (Array *) ap->sval;
	h = hash(s, tp->size);
	for (p = tp->tab[h]; p != NULL; prev = p, p = p->cnext)
		if (strcmp(s, p->nval) == 0) {
			if (prev == NULL)	/* 1st one */
				tp->tab[h] = p->cnext;
			else			/* middle somewhere */
				prev->cnext = p->cnext;
			if ((p->tval & CON) == 0){
				if (freeable(p))
					xfree(p->sval);
				free(p->nval);
			}
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

static inline int updateptr(Cell *vp)
{
	int n;
	Value v;
	Awknum i;
	Array *ap;
	union{
		int i;
		float f;
		unsigned int u;
		short ih[2];
		unsigned short uh[2];
	} m;

	if(vp->tval & CON)
		return 0;
	n = 0;
	i = (Awknum)vp->nval;
	ap = (Array *)vp->cnext;
	switch(vp->tval & (STR|NUM|FLT)){
	case STR|FLT|NUM:
	case FLT|NUM:
		m.f = *((float *)ap->tab + i);
		if(m.u == UNSET){
			vp->tval |= UNS;
			v.f = 0.0;
			if(vp->tval & STR){
				if(freeable(vp))
					xfree(vp->sval);
				vp->sval = EMPTY;
				vp->tval |= DONTFREE;
			}
		}else{
			vp->tval &= ~UNS;
			if(m.f == -0.0)
				v.f = 0.0;
			else
				v.f = m.f;
			if((n = (vp->val.f != v.f)) && vp->tval & STR){
				if(freeable(vp))		/* invalidate string */
					xfree(vp->sval);
				vp->tval &= ~STR;
			}
		}
		vp->val = v;
		DPRINTF("updateptr %p: %f\n", (void *)vp, v.f);
		break;
	case STR|NUM:
	case NUM:
		/* FIXME: yuck */
		if((vp->tval & P16) == 0)
			m.i = *((unsigned int *)ap->tab + i);
		else{
			m.uh[0] = *((unsigned short *)ap->tab + i);
			m.uh[1] = m.uh[0];
		}
		if(m.u == UNSET){
			vp->tval |= UNS;
			v.i = 0;
			if(vp->tval & STR){
				if(freeable(vp))
					xfree(vp->sval);
				vp->sval = EMPTY;
				vp->tval |= DONTFREE;
			}
		}else{
			vp->tval &= ~UNS;
			switch(vp->tval & (P16|USG)){
			case 0: v.i = m.i; break;
			case USG: v.u = m.u; break;
			case P16: v.i = m.ih[0]; break;
			case P16|USG: v.u = m.uh[0]; break;
			default: v.i = 0;	/* because clang is stupid */
			}
			if((n = (vp->val.i != v.i)) && vp->tval & STR){
				if(freeable(vp))		/* invalidate string */
					xfree(vp->sval);
				vp->tval &= ~STR;
			}
		}
		vp->val = v;
		DPRINTF("updateptr %p: %lld\n", (void *)vp, v.i);
		break;
	case STR:
		if(freeable(vp))
			xfree(vp->sval);
		vp->sval = *((char **)ap->tab + i);
		if(vp->sval == NULL){
			vp->sval = EMPTY;
			vp->tval |= UNS;
		}
		vp->tval |= DONTFREE;
		DPRINTF("updateptr %p: %s\n", (void *)vp, vp->sval);
		break;
	default: FATAL("invalid pointer type %o", vp->tval);
	}
	return n;
}

/* FIXME: use CON to flag constant values (avoiding updates) */
Cell *setptrtab(Awknum i, Array *a, int readit)
{
	Cell *p;

	DPRINTF("setptrtab %p[%lld]\n", (void*)a, i);
	if(i < 0 || i >= a->nelem)
		FATAL("index out of bounds %lld", i);
	p = gettemp(a->type & (PTR|P16|USG|STR|FLT|NUM));
	p->nval = (char *)i;
	p->sval = EMPTY;
	p->cnext = (Cell *)a;
	if(readit)
		updateptr(p);
	return p;
}

Cell *setsymtab(const char *n, const char *s, Value v, unsigned t, Array *tp)
{
	int h;
	size_t i;
	Cell *p, *w;

	if(tp->type != 0){
		i = -1;
		if((t & (STR|NUM)) == STR){
			if(tp->ids != NULL && (w = lookup(s, tp->ids)) != NULL)
				i = getival(w);
			else
				FATAL("no such id %s in flat array", s);
		}else
			i = v.i;
		return setptrtab(i, tp, 1);
	}
	if (n != NULL && (p = lookup(n, tp)) != NULL) {
		DPRINTF("setsymtab found %p: n=%s s=\"%s\" i=%lld f=%g t=%o\n",
			(void*)p, NN(p->nval), NN(p->sval), p->val.i, p->val.f, p->tval);
		return(p);
	}
	p = (Cell *) MALLOC(sizeof(*p));
	p->nval = t & CON ? n : STRDUP(n);
	p->sval = s && s != EMPTY && *s != 0 ? t & CON ? s : STRDUP(s) : EMPTY;
	p->val = v;
	p->tval = t;
	if(p->sval == EMPTY || t & CON)
		p->tval |= DONTFREE;
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

	if(tp->type != 0)
		FATAL("BUG: rehashing ptr array");
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

	if(tp->type != 0)
		FATAL("BUG: lookup in ptr array");
	h = hash(s, tp->size);
	for (p = tp->tab[h]; p != NULL; p = p->cnext)
		if (strcmp(s, p->nval) == 0)
			return(p);	/* found it */
	return(NULL);			/* not found */
}

Awkfloat setfval(Cell *vp, Awkfloat f)	/* set float val of a Cell */
{
	int fldno;
	Array *ap;
	Awknum i;
	Value v;

	f += 0.0;		/* normalise negative zero to positive zero */
	if (f == -0)  /* who would have thought this possible? */
		f = 0;
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
	if (isptr(vp)) {
		if((vp->tval & NUM) == 0)
			FATAL("can\'t assign number to non-numeric pointer type");
		ap = (Array *)vp->cnext;
		i = (Awknum)vp->nval;
		if(vp->tval & FLT)
			*((float *)ap->tab + i) = f;
		else
			switch(vp->tval & (P16|USG)){
			case 0: *((int *)ap->tab + i) = f; break;
			case USG: *((unsigned int *)ap->tab + i) = f; break;
			case P16: *((short *)ap->tab + i) = f; break;
			case P16|USG: *((unsigned short *)ap->tab + i) = f; break;
			}
		if(ap->upfn != NULL){
			v.f = f;
			ap->upfn(i, v);
		}
		vp->tval &= ~UNS;
	} else if (freeable(vp))
		xfree(vp->sval); /* free any previous string */
	vp->tval &= ~STR; /* mark string invalid */
	vp->tval |= NUM | FLT;	/* mark number ok */
	if(dbg){
		if(!isptr(vp))
			DPRINTF("setfval %p: %s = %g, t=%o\n", (void*)vp, NN(vp->nval), f, vp->tval);
		else
			DPRINTF("setfval %p: [%lld] = %g, t=%o\n", (void*)vp, (Awknum)vp->nval, f, vp->tval);
	}
	return vp->val.f = f;
}

Awknum setival(Cell *vp, Awknum f)	/* set int val of a Cell */
{
	int fldno;
	Awknum i;
	Array *ap;
	Value v;

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
	if (isptr(vp)) {
		if((vp->tval & NUM) == 0)
			FATAL("can\'t assign number to non-numeric pointer type");
		ap = (Array *)vp->cnext;
		i = (Awknum)vp->nval;
		if(vp->tval & FLT)
			*((float *)ap->tab + i) = f;
		else
			switch(vp->tval & (P16|USG)){
			case 0: *((int *)ap->tab + i) = f; break;
			case USG: *((unsigned int *)ap->tab + i) = f; break;
			case P16: *((short *)ap->tab + i) = f; break;
			case P16|USG: *((unsigned short *)ap->tab + i) = f; break;
			}
		if(ap->upfn != NULL){
			v.i = f;
			ap->upfn(i, v);
		}
		vp->tval &= ~UNS;
	} else if (freeable(vp))
		xfree(vp->sval); /* free any previous string */
	vp->tval &= ~(STR|FLT); /* mark string invalid; force int */
	vp->tval |= NUM;	/* mark number ok */
	if(dbg){
		if(!isptr(vp))
			DPRINTF("setival %p: %s = %lld, t=%o\n", (void*)vp, NN(vp->nval), f, vp->tval);
		else
			DPRINTF("setival %p: [%lld] = %lld, t=%o\n", (void*)vp, (Awknum)vp->nval, f, vp->tval);
	}
	
	return vp->val.i = f;
}

void setval(Cell *vp, Cell *x)
{
	short t;
	Value v;

	v = getval(x, &t);
	if(t & FLT)
		setfval(vp, v.f);
	else
		setival(vp, v.i);
}

void funnyvar(Cell *vp, const char *rw)
{
	if (isarr(vp))
		FATAL("can't %s %s; it's an array name.", rw, vp->nval);
	if (vp->tval & FCN)
		FATAL("can't %s %s; it's a function.", rw, vp->nval);
	WARNING("funny variable %p: n=%s s=\"%s\" i=%lld f=%g t=%o",
		(void *)vp, isptr(vp) ? EMPTY : vp->nval, vp->sval, vp->val.i, vp->val.f, vp->tval);
}

char *setsval(Cell *vp, const char *s, int new)	/* set string val of a Cell */
{
	char *t, **sp;
	int fldno;
	Awknum i, f;
	Array *ap;
	Value v;

	DPRINTF("starting setsval %p: %s = \"%s\", t=%o, done=%d,%d\n",
		(void*)vp, isptr(vp) ? EMPTY : NN(vp->nval), s, vp->tval, donerec, donefld);
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
		t = s && s != EMPTY && *s != 0 ? new ? tostring(s) : s : EMPTY;
		if(freeable(vp))
			xfree(vp->sval);
		if(t == EMPTY || !new)
			vp->tval |= DONTFREE;
		else if(!isptr(vp))
			vp->tval &= ~DONTFREE;
	}
	if(isptr(vp)){
		if((vp->tval & STR) == 0)
			FATAL("can\'t assign string to numeric pointer type");
		ap = (Array *)vp->cnext;
		i = (Awknum)vp->nval;
		sp = (char **)ap->tab + i;
		if(*sp != NULL)
			free(*sp);
		*sp = t;
		if(ap->upfn != NULL){
			v.s = t;
			ap->upfn(i, v);
		}
		vp->tval &= ~UNS;
	}else{
		vp->tval |= STR;
		if(!isptr(vp))
			vp->tval &= ~(FLT|NUM);	/* no longer a number */
	}
	vp->sval = t;
	if(dbg){
		if(!isptr(vp))
			DPRINTF("setsval %p: %s = \"%s (%p) \", t=%o r,f=%d,%d\n",
				(void*)vp, NN(vp->nval), t, (void*)t, vp->tval, donerec, donefld);
		else
			DPRINTF("setsval %p: [%lld] = \"%s (%p) \", t=%o r,f=%d,%d\n",
				(void*)vp, (Awknum)vp->nval, t, (void*)t, vp->tval, donerec, donefld);
	}
	if (&vp->val.i == NF) {
		donerec = false;	/* mark $0 invalid */
		f = getival(vp);
		setlastfld(f);
		DPRINTF("setsval: setting NF to %lld\n", f);
	}
	return t;
}

Value getval(Cell *vp, short *type)
{
	int r, t;
	char *s;
	Value v;
	bool no_trailing;

	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "read value of");
	if (isfld(vp) && !donefld)
		fldbld();
	else if (isrec(vp) && !donerec)
		recbld();
	r = 0;
	t = vp->tval;
	if(isptr(vp))
		updateptr(vp);
	if(!isnum(vp)){
		if((s = vp->sval) == EMPTY){
			*type = STR;
			return ZV;
		}
		if(r = is_valid_number(s, true, &no_trailing, NULL, &v)){
			if (no_trailing && (t & CON) == 0)
				t |= r;	/* make NUM only sparingly */
			if(r & FLT)
				t |= FLT;
			else
				t &= ~FLT;
		}else{
			v.i = 0;
			t &= ~FLT;
		}
		if(!isptr(vp)){
			vp->tval = t;
			vp->val = v;
		}
	}else
		v = vp->val;
	*type = t;
	return v;
}

Awknum getival(Cell *vp)	/* get int val of a Cell */
{
	short t;
	Value v;

	v = getval(vp, &t);
	if(dbg){
		if(!isptr(vp))
			DPRINTF("getival %p: %s = %lld,%g, t=%o\n",
				(void*)vp, NN(vp->nval), v.i, v.f, t);
		else
			DPRINTF("getival %p: [%lld] = %lld,%g, t=%o\n",
				(void*)vp, (Awknum)vp->nval, v.i, v.f, t);
	}
	return (t & FLT) == 0 ? v.i : v.f;
}

Awkfloat getfval(Cell *vp)	/* get float val of a Cell */
{
	short t;
	Value v;

	v = getval(vp, &t);
	if(dbg){
		if(!isptr(vp))
			DPRINTF("getfval %p: %s = %lld,%g, t=%o\n",
				(void*)vp, NN(vp->nval), v.i, v.f, t);
		else
			DPRINTF("getfval %p: [%lld] = %lld,%g, t=%o\n",
				(void*)vp, (Awknum)vp->nval, v.i, v.f, t);
	}
	return (t & FLT) == 0 ? v.i : v.f;
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

/* FIXME: tostring usage: detect if this is a throwaway value,
 * only do it if we plan to keep the string around */
static inline void update_str_val(Cell *vp)
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
		snprintf(s, sizeof(s), *OFMT, vp->val.f);
	vp->sval = STRDUP(s);
	if(!isptr(vp))
		vp->tval &= ~DONTFREE;
	vp->tval |= STR;
}

/* not updating string automatically if OFMT changed. use printf. */
static char *get_str_val(Cell *vp)        /* get string val of a Cell */
{
	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "read value of");
	if (isfld(vp) && ! donefld)
		fldbld();
	else if (isrec(vp) && ! donerec)
		recbld();
	if (!isstr(vp) || isptr(vp) && updateptr(vp))
		update_str_val(vp);
	if(dbg){
		if(!isptr(vp))
			DPRINTF("getsval %p: %s = \"%s (%p)\", t=%o\n",
				(void*)vp, NN(vp->nval), vp->sval, (void*)vp->sval, vp->tval);
		else
			DPRINTF("getsval %p: [%lld] = \"%s (%p)\", t=%o\n",
				(void*)vp, (Awknum)vp->nval, vp->sval, (void*)vp->sval, vp->tval);
	}
	return vp->sval;
}

char *getsval(Cell *vp)       /* get string val of a Cell */
{
      return get_str_val(vp);
}

char *getpssval(Cell *vp)     /* get string val of a Cell for print */
{
      return get_str_val(vp);
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
	c = setsymtab(buf, NULL, ZV, CON|STR|DONTFREE, symtab);
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

int compileawk(int argc, char **argv)
{
	compile_time = COMPILING;
	setlocale(LC_CTYPE, "");
	setlocale(LC_NUMERIC, "C"); /* for parsing cmdline & prog */
	initpool();
	catchfpe();
	yyin = NULL;
	cmdname = argv[0];
	lexprog = argv[1];
	symtab = makesymtab(NSYMTAB/NSYMTAB);
	recinit(recsize);
	syminit();
	arginit(0, NULL);
	yyparse();
	if (errorflag == 0)
		compile_time = RUNNING;
	else
		bracecheck();
	return(errorflag);
}

void runawk(void)
{
	run(winner);
}

int awkmain(int argc, char **argv)
{
	const char *fs = NULL;
	char *fn, *vn;

	setlocale(LC_CTYPE, "");
	setlocale(LC_NUMERIC, "C"); /* for parsing cmdline & prog */
	initpool();
	cmdname = argv[0];
	if (argc == 1) {
		fprintf(stderr,
		  "usage: %s [-F fs] [-v var=value] [-f progfile | 'prog'] [file ...]\n",
		  cmdname);
		exit(1);
	}
	catchfpe();
	init_genrand64(srand_seed);
	yyin = NULL;
	symtab = makesymtab(NSYMTAB/NSYMTAB);
	while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
		if (strcmp(argv[1], "-version") == 0 || strcmp(argv[1], "--version") == 0) {
			printf("strawk %s\n", version);
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
			printf("strawk %s\n", version);
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
	if (fs){
		xfree(*FS);
		*FS = qstring(fs, '\0');
	}
	DPRINTF("errorflag=%d\n", errorflag);
	if (errorflag == 0) {
		compile_time = RUNNING;
		run(winner);
	} else
		bracecheck();
	return(errorflag);
}
