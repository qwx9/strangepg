#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>
#include <assert.h>

typedef size_t usize;
typedef unsigned char uchar;
typedef long long vlong;
#define const
#define nil NULL

#define USED(x) (void)(x)

extern int debug;

void	werrstr(char*, ...);

#include "khash.h"
#include "kvec.h"
#include "dat.h"
#include "fns.h"
#include "cmd.h"
#include "graph.h"
#include "layout.h"
#include "rend.h"
#include "view.h"
#include "fs.h"
#include "ui.h"

extern char *argv0;
#define	ARGBEGIN	for((argv0||(argv0=*argv)),argv++,argc--;\
			    argv[0] && argv[0][0]=='-' && argv[0][1];\
			    argc--, argv++) {\
				char *_args, *_argt;\
				char _argc;\
				_args = &argv[0][1];\
				if(_args[0]=='-' && _args[1]==0){\
					argc--; argv++; break;\
				}\
				_argc = 0;\
				while(*_args && (_args++))\
				switch(_argc)
#define	ARGEND		SET(_argt);USED((_argt,_argc,_args));}USED((argv, argc));
#define	ARGF()		(_argt=_args, _args="",\
				(*_argt? _argt: argv[1]? (argc--, *++argv): 0))
#define	EARGF(x)	(_argt=_args, _args="",\
				(*_argt? _argt: argv[1]? (argc--, *++argv): ((x), abort(), (char*)0)))

#define	ARGC()		_argc

void	sysfatal(char*, ...);
