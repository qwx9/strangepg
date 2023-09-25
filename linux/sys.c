#include "strpg.h"
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/time.h>

int noui, debug;

char *argv0;

static char errbuf[1024];
static _Atomic(u32int) *fux;

// FIXME: kill it
char *
shitprint(int, void *)
{
	return "";
}

void
sysquit(void)
{
	pthread_exit(NULL);
}

int
errstr(char *err, uint nerr)
{
	uint n;

	n = sizeof errbuf < nerr ? sizeof errbuf : nerr;
	strncpy(err, errbuf, n - 1);
	err[n-1] = 0;
	*errbuf = 0;
	return 0;
}

void
werrstr(char *fmt, ...)
{
	va_list arg;
	memset(errbuf, 0, sizeof errbuf);
	va_start(arg, fmt);
	vsnprintf(errbuf, sizeof errbuf, fmt, arg);
	va_end(arg);
}

void
vawarn(char *fmt, va_list arg)
{
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

void
sysfatal(char *fmt, ...)
{
	va_list arg;

	perror("fatal");
	va_start(arg, fmt);
	vawarn(fmt, arg);
	exit(EXIT_FAILURE);
}

char *
error(void)
{
	return strerror(errno);
}

/* FIXME: check */
vlong
msec(void)
{
	struct timeval tv;

	if(gettimeofday(&tv, NULL) < 0){
		perror("gettimeofday");
		return -1;
	}
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void
lsleep(vlong ns)
{
	struct timespec t = {0};

	t.tv_nsec = ns;
	nanosleep(&t, NULL);
}

void
yield(void)
{
}

char *
estrdup(char *s)
{
	char *p;

	if((p = strdup(s)) == NULL)
		sysfatal("estrdup");
	return p;
}

void *
erealloc(void *p, usize n, usize oldn)
{
	if((p = realloc(p, n)) == NULL){
		if(n == 0)
			return NULL;
		sysfatal("erealloc");
	}
	if(n > oldn)
		memset((uchar *)p + oldn, 0, n - oldn);
	return p;
}

void *
emalloc(usize n)
{
	void *p;

	if((p = calloc(1, n)) == NULL){
		if(n == 0)
			return NULL;
		sysfatal("emalloc");
	}
	return p;
}

/* futex(2) */
static int
futex(_Atomic(u32int) *addr, int op, uint32_t val)
{
	return syscall(SYS_futex, addr, op, val, NULL, NULL, 0);
}

void
coffeetime(void)
{
	u32int one;

	one = 1;
	warn("one %#x\n", one);
	for(;;){
		if(atomic_compare_exchange_strong(fux, &one, 0))
			break;
		if(futex(fux, FUTEX_WAIT, 0) < 0 && errno != EAGAIN)
			sysfatal("waitcoffeetime: futex wait");
	}
}

void
coffeeover(void)
{
	u32int zilch;

	zilch = 0;
	warn("zilch %#x\n", zilch);
	if(atomic_compare_exchange_strong(fux, &zilch, 1)){
		if(futex(fux, FUTEX_WAKE, 1) < 0)
			sysfatal("coffeeover: futex wake");
		warn("wakezilch %#x\n", zilch);
	}
}

void
sysinit(void)
{
	if((fux = mmap(NULL, sizeof(*fux), PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED)
			sysfatal("sysinit: mmap wake");
	*fux = 1;
}
