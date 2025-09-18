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
#include <stdlib.h>
#ifdef VERSION
#include "strpg.h"
#include "threads.h"
#endif
#include "awk.h"
#include AWKTAB

TNode *nodealloc(size_t n)
{
	TNode *u;

	u = defalloc(n * sizeof *u);
	u->lineno = lineno;
	u->nnext = NULL;
	return u;
}

TNode *exptostat(TNode *a)
{
	a->ntype = NSTAT;
	return(a);
}

TNode *node1(int a, TNode *b)
{
	TNode *x;

	x = nodealloc(1);
	x->nobj = a;
	x->narg[0]=b;
	return(x);
}

TNode *node2(int a, TNode *b, TNode *c)
{
	TNode *x;

	x = nodealloc(2);
	x->nobj = a;
	x->narg[0] = b;
	x->narg[1] = c;
	return(x);
}

TNode *node3(int a, TNode *b, TNode *c, TNode *d)
{
	TNode *x;

	x = nodealloc(3);
	x->nobj = a;
	x->narg[0] = b;
	x->narg[1] = c;
	x->narg[2] = d;
	return(x);
}

TNode *node4(int a, TNode *b, TNode *c, TNode *d, TNode *e)
{
	TNode *x;

	x = nodealloc(4);
	x->nobj = a;
	x->narg[0] = b;
	x->narg[1] = c;
	x->narg[2] = d;
	x->narg[3] = e;
	return(x);
}

TNode *stat1(int a, TNode *b)
{
	TNode *x;

	x = node1(a,b);
	x->ntype = NSTAT;
	return(x);
}

TNode *stat2(int a, TNode *b, TNode *c)
{
	TNode *x;

	x = node2(a,b,c);
	x->ntype = NSTAT;
	return(x);
}

TNode *stat3(int a, TNode *b, TNode *c, TNode *d)
{
	TNode *x;

	x = node3(a,b,c,d);
	x->ntype = NSTAT;
	return(x);
}

TNode *stat4(int a, TNode *b, TNode *c, TNode *d, TNode *e)
{
	TNode *x;

	x = node4(a,b,c,d,e);
	x->ntype = NSTAT;
	return(x);
}

TNode *op1(int a, TNode *b)
{
	TNode *x;

	x = node1(a,b);
	x->ntype = NEXPR;
	return(x);
}

TNode *op2(int a, TNode *b, TNode *c)
{
	TNode *x;

	x = node2(a,b,c);
	x->ntype = NEXPR;
	return(x);
}

TNode *op3(int a, TNode *b, TNode *c, TNode *d)
{
	TNode *x;

	x = node3(a,b,c,d);
	x->ntype = NEXPR;
	return(x);
}

TNode *op4(int a, TNode *b, TNode *c, TNode *d, TNode *e)
{
	TNode *x;

	x = node4(a,b,c,d,e);
	x->ntype = NEXPR;
	return(x);
}

TNode *celltonode(Cell *a, int b)
{
	TNode *x;

	a->ctype = OCELL;
	a->csub = b;
	x = node1(0, (TNode *) a);
	x->ntype = NVALUE;
	return(x);
}

TNode *rectonode(void)	/* make $0 into a TNode */
{
	extern Cell *literal0;
	return op1(INDIRECT, celltonode(literal0, CUNK));
}

TNode *makearr(TNode *p)
{
	Cell *cp;

	if (isvalue(p)) {
		cp = (Cell *) (p->narg[0]);
		if (isfcn(cp))
			SYNTAX( "%s is a function, not an array", cp->nval );
		else if (!isarr(cp)) {
			xfree(cp->sval);
			cp->sval = (char *) makesymtab(NSYMTAB);
			cp->tval &= ~(STR|NUM|FLT);
			cp->tval = ARR|DONTFREE;
		}
	}
	return p;
}

#define PA2NUM	50	/* max number of pat,pat patterns allowed */
int	paircnt;		/* number of them in use */
int	pairstack[PA2NUM];	/* state of each pat,pat */

TNode *pa2stat(TNode *a, TNode *b, TNode *c)	/* pat, pat {...} */
{
	TNode *x;

	x = node4(PASTAT2, a, b, c, itonp(paircnt));
	if (paircnt++ >= PA2NUM)
		SYNTAX( "limited to %d pat,pat statements", PA2NUM );
	x->ntype = NSTAT;
	return(x);
}

TNode *linkum(TNode *a, TNode *b)
{
	TNode *c;

	if (errorflag)	/* don't link things that are wrong */
		return a;
	if (a == NULL)
		return(b);
	else if (b == NULL)
		return(a);
	for (c = a; c->nnext != NULL; c = c->nnext)
		;
	c->nnext = b;
	return(a);
}

void defn(Cell *v, TNode *vl, TNode *st)	/* turn on FCN bit in definition, */
{					/*   body of function, arglist */
	TNode *p;
	int n;

	if (isarr(v)) {
		SYNTAX( "`%s' is an array name and a function name", v->nval );
		return;
	}
	if (isarg(v->nval) != -1) {
		SYNTAX( "`%s' is both function name and argument name", v->nval );
		return;
	}

	v->tval = FCN;
	v->sval = (char *) st;
	n = 0;	/* count arguments */
	for (p = vl; p; p = p->nnext)
		n++;
	v->val.i = n;
	DPRINTF("defining func %s (%d args)\n", v->nval, n);
}

int isarg(const char *s)		/* is s in argument list for current function? */
{			/* return -1 if not, otherwise arg # */
	extern TNode *arglist;
	TNode *p = arglist;
	Cell *cp;
	int n;

	for (n = 0; p != NULL; p = p->nnext, n++){
		cp = (Cell *)p->narg[0];
		if (isptr(cp) && !isarr(cp))
			FATAL("isarg: ptr argument, shouldn\'t happen");
		if (strcmp(cp->nval, s) == 0)
			return n;
	}
	return -1;
}

int ptoi(void *p)	/* convert pointer to integer */
{
	return (int) (intptr_t) p;	/* swearing that p fits, of course */
}

TNode *itonp(int i)	/* and vice versa */
{
	return (TNode *) (intptr_t) i;
}
