#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>
#include <assert.h>
#include <fcntl.h>

/* u.h */
#define nil		((void*)0)
typedef	unsigned short	ushort;
typedef	unsigned char	uchar;
typedef unsigned int ulong;	/* assumed 32 bit */
typedef unsigned int	uint;
typedef signed char	schar;
typedef	long long	vlong;
typedef	unsigned long long uvlong;
typedef long long	intptr;
typedef unsigned long long uintptr;
typedef	uint		Rune;

typedef size_t usize;
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
#define	OREAD	O_RDONLY

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

/* /sys/include/geometry.h */
typedef struct Point2 Point2;

struct Point2 {
	double x, y, w;
};

/* utils */
double flerp(double, double, double);
double fclamp(double, double, double);

/* Point2 */
Point2 Pt2(double, double, double);
Point2 Vec2(double, double);
Point2 addpt2(Point2, Point2);
Point2 subpt2(Point2, Point2);
Point2 mulpt2(Point2, double);
Point2 divpt2(Point2, double);
Point2 lerp2(Point2, Point2, double);
double dotvec2(Point2, Point2);
double vec2len(Point2);
Point2 normvec2(Point2);
int edgeptcmp(Point2, Point2, Point2);
int ptinpoly(Point2, Point2*, ulong);

//#define const
//#define nil NULL

#include "khash.h"
#include "dat.h"
#include "fns.h"

void	werrstr(char*, ...);
void	sysfatal(char*, ...);
