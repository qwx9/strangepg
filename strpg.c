#include "strpg.h"
#include "cmd.h"
#include "em.h"
#include "drw.h"
#include "layout.h"

int haxx0rz;

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
run(void)
{
	if(noui)
		quit();
	evloop();
}

static void
usage(void)
{
	sysfatal("usage: %s [-bins] [-l layout] [-m 16-63] [FILE]\n", argv0);
}

static char **
parseargs(int argc, char **argv)
{
	char *s;

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
		else if(strcmp(s, "all") == 0)
			debug |= Debugtheworld;
		else{
			warn("unknown debug component %s\n", s);
			usage();
		}
		break;
	case 'b': haxx0rz = 1; break;
	case 'i':
		intype = FFindex;
		break;
	case 'l':
		s = EARGF(usage());
		if(strcmp(s, "random") == 0)
			deflayout = LLrandom;
		else if(strcmp(s, "conga") == 0)
			deflayout = LLconga;
		else if(strcmp(s, "force") == 0)
			deflayout = LLforce;
		else if(strcmp(s, "linear") == 0)
			deflayout = LLlinear;
		else if(strcmp(s, "fr") == 0)
			deflayout = LLfr;
		else if(strcmp(s, "pfr") == 0)
			deflayout = LLpfr;
		else
			sysfatal("unknown layout type");
		break;
	case 'm':
		multiplier = atoi(EARGF(usage()));
		break;
	case 'n': noui = 1; break;
	case 's': drawstep = 1; break;
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
	initrend();
	if(noui)
		return;
	initdrw();
	initui();
	resetui(1);
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
	run();
	sysquit();
	return 0;
}
