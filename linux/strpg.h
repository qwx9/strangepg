#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

/* u.h */
//#define nil		((void*)0)
typedef	unsigned short	ushort;
typedef	unsigned char	uchar;
typedef unsigned int	uint;
typedef unsigned long	ulong;
typedef signed char	schar;
typedef	long long	vlong;
typedef	unsigned long long uvlong;
typedef long long	intptr;
typedef unsigned long long uintptr;
typedef	uint		Rune;

typedef size_t usize;
typedef ssize_t ssize;
typedef int8_t s8int;
typedef int16_t s16int;
typedef int32_t s32int;
typedef int64_t s64int;
typedef uint8_t u8int;
typedef uint16_t u16int;
typedef uint32_t u32int;
typedef uint64_t u64int;

/* libc.h */
#define	nelem(x)	(sizeof(x)/sizeof((x)[0]))
#define USED(x) (void)(x)
#define SET(x)	(x = *(&(x)))
#define PI	M_PI
#define	OREAD	O_RDONLY
#define	OWRITE	O_WRONLY
#define ORDWR	O_RDWR
#define OCREAT	(O_CREAT|O_TRUNC)
#define AEXIST	R_OK
#define IOUNIT	(1<<16)

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
				while(_argc = *_args++)\
				switch(_argc)
#define	ARGEND		SET(_argt);USED((_argt,_argc,_args));}USED((argv, argc));
#define	ARGF()		(_argt=_args, _args="",\
				(*_argt? _argt: argv[1]? (argc--, *++argv): 0))
#define	EARGF(x)	(_argt=_args, _args="",\
				(*_argt? _argt: argv[1]? (argc--, *++argv): ((x), abort(), (char*)0)))

#define	ARGC()		_argc

int nrand(int);
int	lrand(void);
int	getfields(char*, char**, int, int, char*);
char*	seprint(char*, char*, char*, ...);
char*	strecpy(char*, char*, char*);

//#define const
#define nil NULL	// better for them to be interchangeable here

enum{
	KF = 0xF000,	/* Rune: beginning of private Unicode space */
	Spec = 0xF800,
	Kview = Spec|0x00,
	Khome = KF|0x0D,
	Kup = KF|0x0E,
	Kdown = Kview,
	Kpgup = KF|0x0F,
	Kprint = KF|0x10,
	Kleft = KF|0x11,
	Kright = KF|0x12,
	Kpgdown = KF|0x13,
	Kins = KF|0x14,
	Kalt = KF|0x15,
	Kshift = KF|0x16,
	Kctl = KF|0x17,
	Kend = KF|0x18,
	Kscroll = KF|0x19,
	Kscrolloneup = KF|0x20,
	Kscrollonedown = KF|0x21,
	Ksoh = 0x01,
	Kstx = 0x02,
	Ketx = 0x03,
	Keof = 0x04,
	Kenq = 0x05,
	Kack = 0x06,
	Kbs = 0x08,
	Knack = 0x15,
	Ketb = 0x17,
	Kdel = 0x7f,
	Kesc = 0x1b,
	Kbreak = Spec|0x61,
	Kcaps = Spec|0x64,
	Knum = Spec|0x65,
	Kaltgr = Spec|0x67,
	Kmouse = Spec|0x100,
};

#include "lib/khash.h"
#include "dat.h"
#include "fns.h"
#include "dynar.h"

void	werrstr(char*, ...);
void	sysfatal(char*, ...);

void	lsleep(vlong);
vlong	seek(int, vlong, int);
int	create(char*, int, int);
int	dupfd(int, int);

// FIXME
#define sleep(x)	lsleep((x) * 1000000)
#define snprint	snprintf
#define print	printf
#define	dup	dupfd
