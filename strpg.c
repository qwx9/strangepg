#include "strpg.h"
#include "em.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "fs.h"
#include "ui.h"

typedef struct Input Input;
struct Input{
	int type;
	char *path;
};
static Input *files;

void
quit(void)
{
	sysquit();
}

static void
load(void)
{
	Input *in, *end;

	for(in=files, end=in+dylen(files); in!=end; in++){
		switch(in->type){
		case FFlayout:
			pushcmd("importlayout(\"%s\")", in->path);
			break;
		case FFcsv:
			pushcmd("readcsv(\"%s\")", in->path);
			break;
		default:
			if(loadfs(in->path, in->type) < 0)
				sysfatal("could not load file %s: %s", in->path, error());
			break;
		}
	}
	dyfree(files);
}

static void
pushfile(char *file, int type)
{
	Input in;

	if(file == nil || type == FFdead || type >= FFnil){
		warn("invalid input file\n");
		return;
	}
	if(access(file, AREAD) < 0)
		sysfatal("could not load file %s: %s", file, error());
	in = (Input){type, file};
	dypush(files, in);
}

static void
help(void)
{
	warn("usage: %s [-Rbhv] [-f FILE] [-l ALGO] [-t N] [-c FILE] FILE [FILE..]\n", argv0);
	warn(
		"-R             Do not reset layout once metadata is done loading\n"
		"-b             White-on-black theme\n"
		"-c FILE        Load tags from csv FILE\n"
		"-f FILE        Load layout from FILE\n"
		"-h             Print usage information and exit\n"
		"-l ALGO        Set layouting algorithm (default: pfr)\n"
		"-t N           Set number of layouting threads (1-128, default: 4)\n"
		"-v             Print version and exit\n"
		"ALGO may be one of:\n"
		" fr            Fruchterman-Reingold variant\n"
		" pfr           Parallel Fruchterman-Reingold variant (default)\n"
		" conga         Fixed linear layout based on segment order in input file\n"
		" random        Random fixed positions\n"
		" linear        Linear layout with fixed-position nodes (wip)\n"
		" circ          Circular layout with fixed-position nodes (wip)\n"
		" bo            Linear layout with initial positions\n"
	);
	quit();
}

static void
usage(void)
{
	sysfatal("usage: %s [-Rbhv] [-f FILE] [-l ALGO] [-t N] [-c FILE] FILE [FILE..]", argv0);
}

static void
parseargs(int argc, char **argv)
{
	int type;
	char *s;

	/* FIXME: remove intype and -i, add an optional format specifier in filename:
	 *	strpg ass.gfa; strpg i~ass.bct */
	/* FIXME: we won't try to guess format, but we should validate it */
	/* FIXME: lilu dallas mooltigraph */
	type = FFgfa;
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
	case 'R': noreset = 1; break;
	case 'b': view.flags |= VFhaxx0rz; break;
	case 'c': pushfile(EARGF(usage()), FFcsv); break;
	case 'f': pushfile(EARGF(usage()), FFlayout); break;
	case 'h': help(); break;
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
	case 'v':
		print(VERSION "\n");
		quit();
	default: usage();
	}ARGEND
	if(*argv == nil)
		help();
	for(; *argv!=nil; argv++)
		pushfile(*argv, type);
}

static void
init(void)
{
	initcmd();	/* fork repl before starting other threads */
	initem();
	initdrw();
	initfs();
	initlayout();
}

/* note: npe already sets mainstacksize higher before renaming main */
int
main(int argc, char **argv)
{
	sysinit();
	srand(time(nil));
	parseargs(argc, argv);
	init();
	load();
	initui();
	evloop();
	quit();
	return 0;
}
