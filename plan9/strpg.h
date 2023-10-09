#include <u.h>
#include <libc.h>
#include <geometry.h>

typedef intptr ssize;

#define const
#include "lib/khash.h"
KHASH_MAP_INIT_STR(id, usize)
#include "dat.h"
#include "fns.h"
#include "dynar.h"

void	startdrawclock(void);

#pragma	varargck	argpos	warn	1
#pragma	varargck	argpos	dprint	2
