#include <u.h>
#include <libc.h>
#include <geometry.h>

typedef intptr	ssize;
typedef	void	thret_t;

#define	const
#include "lib/khash.h"
#include "dat.h"
#include "fns.h"
#include "dynar.h"

u64int	nanosec(void);

#pragma	varargck	argpos	warn	1
#pragma	varargck	argpos	dprint	2
