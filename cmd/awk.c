#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include <locale.h>
#include <signal.h>
#include "strawk/awk.h"
#include "strawk/proto.h"

/* [0] is read, [1] is write to cater to windows' _pipe */
int infd[2] = {-1, -1}, outfd[2] = {-1, -1};

static inline char *
getnodelabel(ioff i, char **il)
{
	static char lab[16];
	Cell *c;
	Array *a;

	snprint(lab, sizeof lab, "%d", i);
	if(il != nil)
		*il = lab;
	if((c = lookup("label", symtab)) == nil){
		werrstr("no extant labels");
		return nil;
	}else if(!isarr(c)){
		werrstr("bug: scalar label");
		return nil;
	}
	a = (Array *)c->sval;
	if((c = lookup(lab, a)) == nil){
		werrstr("doesn\'t exist");
		return nil;
	}
	return c->sval;
}

static inline Cell *
getcell(char *tab, char *lab, Array **ap)
{
	Cell *c;
	Array *a;
	Value v = {.i = 0};

	if((c = lookup(tab, symtab)) == nil){
		c = setsymtab(tab, NULL, v, ARR, symtab);
		a = makesymtab(NSYMTAB);
		c->sval = (char *)a;
		c->tval |= ARR;
		c = nil;	/* array is empty */
	}else if(!isarr(c)){
		if (freeable(c))
			xfree(c->sval);
		a = makesymtab(NSYMTAB);
		c->tval &= ~(STR|NUM|DONTFREE);
		c->tval |= ARR;
		c->sval = (char *)a;
		c = nil;	/* array is empty */
	}else
		a = (Array *)c->sval;
	if(ap != nil)
		*ap = a;
	return c == nil ? nil : lookup(lab, a);
}

int
awknamedstr(char *tab, char *lab, char *val)
{
	Cell *c;
	Array *a;
	Value v = {.i = 0};

	if((c = getcell(tab, lab, &a)) == nil)
		setsymtab(lab, val, v, STR, a);
	else
		setsval(c, val);
	return 0;
}

int
awknamedfloat(char *tab, char *lab, double val)
{
	Cell *c;
	Array *a;
	Value v;

	if((c = getcell(tab, lab, &a)) == nil){
		v.f = val;
		setsymtab(lab, NULL, v, NUM|FLT, a);
	}else
		setfval(c, val);
	return 0;
}

int
awknamedint(char *tab, char *lab, s64int val)
{
	Cell *c;
	Array *a;
	Value v;

	if((c = getcell(tab, lab, &a)) == nil){
		v.i = val;
		setsymtab(lab, NULL, v, NUM, a);
	}else
		setival(c, val);
	return 0;
}

int
awkstr(char *tab, ioff i, char *val)
{
	char *name;

	if((name = getnodelabel(i, nil)) == nil)
		return -1;
	return awknamedstr(tab, name, val);
}

int
awkfloat(char *tab, ioff i, double val)
{
	char *name;

	if((name = getnodelabel(i, nil)) == nil)
		return -1;
	return awknamedfloat(tab, name, val);
}

int
awkint(char *tab, ioff i, s64int val)
{
	char *name;

	if((name = getnodelabel(i, nil)) == nil)
		return -1;
	return awknamedint(tab, name, val);
}

static noreturn void fpecatch(int n
#ifdef SA_SIGINFO
	, siginfo_t *si, void *uc
#endif
)
{
#ifdef SA_SIGINFO
	const char *mesg = NULL;

	switch (si->si_code) {
	case FPE_INTDIV:
		mesg = "Integer divide by zero";
		break;
	case FPE_INTOVF:
		mesg = "Integer overflow";
		break;
	case FPE_FLTDIV:
		mesg = "Floating point divide by zero";
		break;
	case FPE_FLTOVF:
		mesg = "Floating point overflow";
		break;
	case FPE_FLTUND:
		mesg = "Floating point underflow";
		break;
	case FPE_FLTRES:
		mesg = "Floating point inexact result";
		break;
	case FPE_FLTINV:
		mesg = "Invalid Floating point operation";
		break;
	case FPE_FLTSUB:
		mesg = "Subscript out of range";
		break;
	case 0:
	default:
		mesg = "Unknown error";
		break;
	}
#endif
	FATAL("floating point exception"
#ifdef SA_SIGINFO
		": %s", mesg
#endif
	    );
}

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
#ifdef SA_SIGINFO
	{
		struct sigaction sa;
		sa.sa_sigaction = fpecatch;
		sa.sa_flags = SA_SIGINFO;
		sigemptyset(&sa.sa_mask);
		(void)sigaction(SIGFPE, &sa, NULL);
	}
#else
	(void)signal(SIGFPE, fpecatch);
#endif
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
	newthread(awk, nil, nil, nil, "strawk", mainstacksize);
	return 0;
}
