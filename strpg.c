#include "strpg.h"
#include "cmd.h"
#include "em.h"
#include "drw.h"
#include "layout.h"
#include "fs.h"
#include "ui.h"

static int intype;

void
quit(void)
{
	sysquit();
}

static void
load(char **files)
{
	char *s;

	while((s = *files++) != nil){
		if(loadfs(s, intype) < 0)
			sysfatal("loadfs: could not load %s: %r\n", s);
	}
}

static void
usage(void)
{
	sysfatal("usage: %s [-bi] [-l layout] [-m 16-63] [-t 1-128] FILE [FILE..]\n", argv0);
}

static char **
parseargs(int argc, char **argv)
{
	char *s;

	/* FIXME: remove intype and -i, add an optional format specifier in filename:
	 *	strpg ass.gfa; strpg i~ass.bct */
	/* FIXME: we won't try to guess format, but we should validate it */
	/* FIXME: lilu dallas mooltigraph */
	intype = FFgfa;
	ARGBEGIN{
	case 'D':
		s = EARGF(usage());
		if(strcmp(s, "cmd") == 0)
			debug |= Debugcmd;
		else if(strcmp(s, "draw") == 0)
			debug |= Debugdraw;
		else if(strcmp(s, "render") == 0)
			debug |= Debugrender;
		else if(strcmp(s, "layout") == 0)
			debug |= Debuglayout;
		else if(strcmp(s, "fs") == 0)
			debug |= Debugfs;
		else if(strcmp(s, "coarse") == 0)
			debug |= Debugcoarse;
		else if(strcmp(s, "extmem") == 0)
			debug |= Debugextmem;
		else if(strcmp(s, "perf") == 0)
			debug |= Debugperf;
		else if(strcmp(s, "meta") == 0)
			debug |= Debugmeta;
		else if(strcmp(s, "all") == 0)
			debug |= Debugtheworld;
		else{
			warn("unknown debug component %s\n", s);
			usage();
		}
		break;
	case 'b': view.flags |= VFhaxx0rz; break;
	case 'i': intype = FFindex; break;
	case 'l':
		s = EARGF(usage());
		if(strcmp(s, "random") == 0)
			deflayout = LLrandom;
		else if(strcmp(s, "conga") == 0)
			deflayout = LLconga;
		else if(strcmp(s, "linear") == 0)
			deflayout = LLlinear;
		else if(strcmp(s, "fr") == 0)
			deflayout = LLfr;
		else if(strcmp(s, "pfr") == 0)
			deflayout = LLpfr;
		else if(strcmp(s, "pline") == 0)
			deflayout = LLpline;
		else if(strcmp(s, "circ") == 0)
			deflayout = LLcirc;
		else if(strcmp(s, "bo") == 0)
			deflayout = LLbo;
		else
			sysfatal("unknown layout type");
		break;
	case 'm':
		multiplier = atoi(EARGF(usage()));
		break;
	case 't':
		nlaythreads = atoi(EARGF(usage()));
		if(nlaythreads <= 0 || nlaythreads > 128){
			warn("invalid number of threads %d ∉ [1,128]\n", nlaythreads);
			usage();
		}
		break;
	default: usage();
	}ARGEND
	if(argv == nil)
		usage();
	return argv;
}

static void
init(void)
{
	initcmd();	/* fork repl before starting other threads */
	initem();
	initfs();
	initlayout();
	initdrw();
	initui();
}

/* note: npe already sets mainstacksize higher before renaming main */
int
main(int argc, char **argv)
{
	sysinit();
	srand(time(nil));
	argv = parseargs(argc, argv);
	init();
	load(argv);
	evloop();
	sysquit();
}
