#include <u.h>
#include <libc.h>
#include <keyboard.h>
#include <ctype.h>
#include <math.h>	/* npe: float math functions */

typedef intptr	ssize;

#define	const
#include "dat.h"
#include "fns.h"
#include "dynar.h"

u64int	nanosec(void);

#pragma	varargck	argpos	warn	1
#pragma	varargck	argpos	dprint	2

void	initp9draw(void);

#define	initqlock(x)
#define	initrwlock(x)
#define	nukerwlock(x)
#define	strtof(x,y)	strtod(x,y)
