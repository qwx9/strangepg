#include <u.h>
#include <libc.h>
#include <keyboard.h>

typedef intptr	ssize;

#define	const
#include "lib/khash.h"
#include "dat.h"
#include "fns.h"
#include "dynar.h"

u64int	nanosec(void);

#pragma	varargck	argpos	warn	1
#pragma	varargck	argpos	dprint	2
