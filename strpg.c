#include "strpg.h"
#include "strawk/mt19937-64.h"
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
xsrand(u64int s)
{
	init_genrand64(s);
}

float
xfrand(void)
{
	return genrand64_real2();	/* [0,1) */
}

u32int
xlrand(void)
{
	return genrand64_int63() & (1ULL<<32)-1;
}

u32int
xnrand(u32int lim)
{
	return genrand64_int64() % lim;
}

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
	warn("usage: %s [-AHMWZbhvw] [-f FILE] [-l ALG] [-n FILE] [-t N] [-c FILE] FILE\n", argv0);
	warn(
		"-b             White-on-black color theme\n"
		"-c FILE        Load tags from csv FILE\n"
		"-f FILE        Load layout from FILE\n"
		"-h             Print usage information and exit\n"
		"-l ALG         Set layouting algorithm (default: pfr)\n"
		"-n FILE        Run layouting headless, saving to FILE periodically\n"
		"-t N           Set number of layouting threads (1-1024, default: 4)\n"
		"-v             Print version and exit\n"
		"-w             Do not wait for all files to load to start layouting\n"
		"-A             Disable transparency (for performance)\n"
		"-H             Enable Hi-DPI mode\n"
		"-M             Enable 4x multisample anti-aliasing (MSAA)\n"
		"-Z             Minimize node depth (z-axis) offsets in 2d layouts\n"
		"-W             Do not suppress warning messages\n"
		"ALG may be one of:\n"
		" fr            Parallelized variant of Fruchterman-Reingold (default)\n"
		" 3d            Experimental 3d version of the above\n"
	);
	quit();
}

static void
usage(void)
{
	sysfatal("usage: %s [-AHMWZbhvw] [-f FILE] [-l ALG] [-n FILE] [-t N] [-c FILE] FILE", argv0);
}

static void
parseargs(int argc, char **argv)
{
	int type;
	char *s;

	type = FFgfa;
	ARGBEGIN{
	case 'A': drawing.flags |= DFnoalpha; break;
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
		else if(strcmp(s, "em") == 0)
			debug |= Debugem;
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
	case 'W': debug |= Debuginfo; break;
	case 'Z': drawing.flags |= DFnodepth; break;
	case 'b': drawing.flags |= DFhaxx0rz; break;
	case 'c': pushfile(EARGF(usage()), FFcsv); break;
	case 'f': pushfile(EARGF(usage()), FFlayout); break;
	case 'h': help(); break;
	case 'l':
		s = EARGF(usage());
		if(strcmp(s, "fr") == 0)
			deflayout = LLpfr;
		else if(strcmp(s, "3d") == 0)
			deflayout = LLpfr3d;
		else
			sysfatal("unknown layout type");
		break;
	case 'n':
		pushcmd("layfile=\"%s\"", EARGF(usage()));
		drawing.flags |= DFnope;
		break;
	case 't':
		nlaythreads = atoi(EARGF(usage()));
		if(nlaythreads <= 0 || nlaythreads > 1024){
			warn("invalid number of threads %d ∉ [1,1024]\n", nlaythreads);
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

/* note: npe already sets mainstacksize higher before renaming main */
int
main(int argc, char **argv)
{
	init_genrand64(time(nil));
	initsys();
	initlog();
	initlayout();
	initcmd();	/* fork repl before starting other threads */
	parseargs(argc, argv);
	initdrw();	/* load default drawing state before files override it */
	initfs();
	load();
	if(drawing.flags & DFnope || debug & Debugload)
		for(;;){
			sleep(10);
			if(drawing.flags & DFnope){
				pushcmd("exportlayout(layfile)");
				flushcmd();
			}
		}
	initui();
	waitforit();
	evloop();
	quit();
	return 0;
}
