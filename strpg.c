#include "strpg.h"
#include "em.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "fs.h"
#include "ui.h"

int gottagofast = 1;

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
		/* defer anything that must be loaded after the graph(s) */
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
	warn("usage: %s [-Zbhvw] [-f FILE] [-l ALG] [-t N] [-c FILE] FILE\n", argv0);
	warn(
		"-b             White-on-black color theme\n"
		"-c FILE        Load tags from csv FILE\n"
		"-f FILE        Load layout from FILE\n"
		"-h             Print usage information and exit\n"
		"-l ALG         Set layouting algorithm (default: pfr)\n"
		"-t N           Set number of layouting threads (1-128, default: 4)\n"
		"-v             Print version and exit\n"
		"-w             Force layouting to wait until all inputs are loaded\n"
		"-Z             Minimize node depth (z-axis) offsets in 2d layouts\n"
		"ALG may be one of:\n"
		" fr            Fruchterman-Reingold algorithm\n"
		" pfr           Parallelized variant of FR (default)\n"
		" pfr3d         Experimental 3d version of the above\n"
		" linear        Linear layout with fixed-position nodes (wip)\n"
		" circ          Circular layout with fixed-position nodes (wip)\n"
	);
	quit();
}

static void
usage(void)
{
	sysfatal("usage: %s [-Zbhvw] [-f FILE] [-l ALG] [-t N] [-c FILE] FILE", argv0);
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
		if(strcmp(s, "all") == 0)
			debug |= Debugtheworld;
		else if(strcmp(s, "cmd") == 0)
			debug |= Debugcmd;
		else if(strcmp(s, "coarse") == 0)
			debug |= Debugcoarse;
		else if(strcmp(s, "draw") == 0)
			debug |= Debugdraw;
		else if(strcmp(s, "extmem") == 0)
			debug |= Debugextmem;
		else if(strcmp(s, "fs") == 0)
			debug |= Debugfs;
		else if(strcmp(s, "layout") == 0)
			debug |= Debuglayout;
		else if(strcmp(s, "meta") == 0)
			debug |= Debugmeta;
		else if(strcmp(s, "perf") == 0)
			debug |= Debugperf;
		else if(strcmp(s, "render") == 0)
			debug |= Debugrender;
		else{
			warn("unknown debug component %s\n", s);
			usage();
		}
		break;
	case 'F': gottagofast = 0; break;
	case 'Z': view.flags |= VFnodepth; break;
	case 'b': view.flags |= VFhaxx0rz; break;
	case 'c': pushfile(EARGF(usage()), FFcsv); break;
	case 'f': pushfile(EARGF(usage()), FFlayout); break;
	case 'h': help(); break;
	case 'l':
		s = EARGF(usage());
		if(strcmp(s, "linear") == 0)
			deflayout = LLlinear;
		else if(strcmp(s, "fr") == 0)
			deflayout = LLfr;
		else if(strcmp(s, "pfr") == 0)
			deflayout = LLpfr;
		else if(strcmp(s, "pfr3d") == 0)
			deflayout = LLpfr3d;
		else if(strcmp(s, "circ") == 0)
			deflayout = LLcirc;
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
}

/* note: npe already sets mainstacksize higher before renaming main */
int
main(int argc, char **argv)
{
	sysinit();
	srand(time(nil));
	parseargs(argc, argv);
	init();
	initlayout();
	load();
	initui();
	evloop();
	quit();
	return 0;
}
