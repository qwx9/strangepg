#include "strpg.h"
#include "cmd.h"
#include "em.h"

int haxx0rz;

static int intype;
static char **filev;

void
quit(void)
{
	sysquit();
}

static void
loadinputs(char **files,  int type)
{
	char *s;

	if(files == nil)
		sysfatal("no input files");
	while((s = *files++) != nil){
		if(loadfs(s, type) < 0)
			sysfatal("loadfs: could not load %s: %r\n", s);
	}
}

static void
run(void)
{
	loadinputs(filev, intype);
	if(noui)
		quit();
	evloop();
}

static void
usage(void)
{
	sysfatal("usage: %s [-bins] [-l layout] [-m 16-63] [FILE]\n", argv0);
}

static int
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
	filev = argv;
	return 0;
}

static void
init(void)
{
	initem();
	initcmd();
	initfs();
	initlayout();
	initrend();
	if(noui)
		return;
	initsysdraw();
	initui();
	resetui(1);
}

/* note: npe already sets mainstacksize higher before renaming main */
int
main(int argc, char **argv)
{
	sysinit();
	srand(time(nil));
	if(parseargs(argc, argv) < 0)
		sysfatal("usage");
	init();
	run();
	sysquit();
	return 0;
}
