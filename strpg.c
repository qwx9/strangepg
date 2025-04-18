#include "strpg.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "fs.h"
#include "ui.h"

int gottagofast = 0;

/* FIXME: doesn't need to be a global */
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
deferred(char **argv)
{
	while(*argv != nil)
		pushcmd(*argv++);
	flushcmd();
}

static void
help(void)
{
	warn("usage: %s [-AHMWZbhvw] [-f FILE] [-l ALG] [-n FILE] [-s LEN WIDE] [-t N] [-c FILE] FILE [CMD..]\n", argv0);
	warn(
		"-b             White-on-black color theme\n"
		"-c FILE        Load tags from csv FILE\n"
		"-f FILE        Load layout from FILE\n"
		"-h             Print usage information and exit\n"
		"-l ALG         Set layouting algorithm (default: pfr)\n"
		"-n FILE        Run layouting headless, saving to FILE periodically\n"
		"-s LEN WIDE    Set node length and width (max: 40, default: 1.0 1.0)\n"
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

static char **
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
		else if(strcmp(s, "ui") == 0)
			debug |= Debugui;
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
	case 's':
		drawing.nodesz = atof(EARGF(usage()));
		drawing.fatness = atof(EARGF(usage()));
		if(drawing.nodesz <= 0.0f || drawing.nodesz >= Maxsz
		|| drawing.fatness <= 0.0f || drawing.fatness >= Maxsz){
			warn("invalid node dimensions\n");
			usage();
		}
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
	pushfile(*argv++, type);
	return argv;
}

/* note: npe already sets mainstacksize higher before renaming main */
int
main(int argc, char **argv)
{
	char **d;

	initsys();
	initlog();
	initrand();
	initcmd();	/* fork repl before starting other threads */
	initlayout();
	d = parseargs(argc, argv);
	/* FIXME: shouldn't we hold off of loading any draw/render shit until
	 * after we're done validating, configuring and loading? */
	initdrw();	/* load default drawing state before files override it */
	initfs();
	load();
	if(drawing.flags & DFnope || debug & Debugload)
		exportforever();
	/* FIXME: view stuff doesn't belong in ui any more */
	initview();
	initui();
	waitforit();
	deferred(d);
	evloop();
	quit();
	return 0;
}
