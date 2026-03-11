#include <u.h>
#include <libc.h>
#include <keyboard.h>
#include <ctype.h>
#include <math.h>	/* npe: float math functions */

typedef intptr	ssize;

#define	VARGPOS(fp, ap)

#define	const
#include "dat.h"
#include "fns.h"
#include "dynar.h"

u64int	nanosec(void);

#pragma	varargck	argpos	warn	1
#pragma	varargck	argpos	va	1
#pragma	varargck	argpos	dprintnl	2
#pragma	varargck	argpos	dprint	2
#pragma	varargck	argpos	die	1
#pragma	varargck	argpos	errmsg	1

void	initp9draw(void);

#define	initqlock(x)
#define	initrwlock(x)
#define	nukerwlock(x)
#define	strtof(x,y)	strtod(x,y)
