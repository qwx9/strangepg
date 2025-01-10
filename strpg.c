#include "strpg.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "fs.h"
#include "ui.h"

int gottagofast = 0;

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
	flushcmd();
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
	warn("usage: %s [-HMZbhqvw] [-f FILE] [-l ALG] [-t N] [-c FILE] FILE\n", argv0);
	warn(
		"-b             White-on-black color theme\n"
		"-c FILE        Load tags from csv FILE\n"
		"-f FILE        Load layout from FILE\n"
		"-h             Print usage information and exit\n"
		"-l ALG         Set layouting algorithm (default: pfr)\n"
		"-t N           Set number of layouting threads (1-128, default: 4)\n"
		"-q             Suppress warning messages\n"
		"-v             Print version and exit\n"
		"-w             Do not wait for all files to load to start layouting\n"
		"-H             Enable Hi-DPI mode\n"
		"-M             Enable 4x multisample anti-aliasing (MSAA)\n"
		"-Z             Minimize node depth (z-axis) offsets in 2d layouts\n"
		"ALG may be one of:\n"
		" fr            Fruchterman-Reingold algorithm\n"
		" pfr           Parallelized variant of FR (default)\n"
		" pfr3d         Experimental 3d version of the above\n"
	);
	quit();
}

static void
usage(void)
{
	sysfatal("usage: %s [-HMZbhqvw] [-f FILE] [-l ALG] [-t N] [-c FILE] FILE", argv0);
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
		else if(strcmp(s, "awk") == 0)
			debug |= Debugawk;
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
		else if(strcmp(s, "graph") == 0)
			debug |= Debuggraph;
		else if(strcmp(s, "layout") == 0)
			debug |= Debuglayout;
		else if(strcmp(s, "load") == 0)
			debug |= Debugload | Debugperf;
		else if(strcmp(s, "meta") == 0)
			debug |= Debugmeta;
		else if(strcmp(s, "perf") == 0)
			debug |= Debugperf;
		else if(strcmp(s, "render") == 0)
			debug |= Debugrender;
		else if(strcmp(s, "strawk") == 0)
			debug |= Debugstrawk;
		else{
			warn("unknown debug component %s\n", s);
			usage();
		}
		break;
	case 'H': drawing.flags |= DFhidpi; break;
	case 'M': drawing.flags |= DFmsaa; break;
	case 'Z': drawing.flags |= DFnodepth; break;
	case 'b': drawing.flags |= DFhaxx0rz; break;
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
	case 'q':
		debug = 0;
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
	case 'w': gottagofast = 1; break;
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
	if(debug & Debugload)
		for(;;)
			sleep(60);
	evloop();
	quit();
	return 0;
}
