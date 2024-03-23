#include "strpg.h"
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

#undef dup

int noui, debug;

char *argv0;

static char errbuf[1024];

void
sysquit(void)
{
	exit(0);
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
vadebug(char *type, char *fmt, va_list arg)
{
	fprintf(stderr, "[%s] ", type);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	fprintf(stderr, "\n");
}

void
sysfatal(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vawarn(fmt, arg);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

char *
error(void)
{
	char *s;

	s = strerror(errno);
	errno = 0;
	return s;
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

vlong
seek(int fd, vlong off, int mode)
{
	return lseek(fd, off, mode);
}

int
create(char *path, int mode, int perm)
{
	return open(path, mode|O_CREAT|O_TRUNC, perm);	/* not just O_WRONLY */
}

int
dupfd(int oldfd, int newfd)
{
	return newfd < 0 ? dup(oldfd) : dup2(oldfd, newfd);
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

void
sysinit(void)
{
}
