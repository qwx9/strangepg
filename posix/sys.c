#include "strpg.h"
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
//#include <fenv.h>
#include "threads.h"
#include "cmd.h"

#undef dup

char *argv0;

static char errbuf[1024];

void
sysquit(void)
{
	killcmd();
	exit(EXIT_SUCCESS);
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
	errno = 0;
	va_list arg;
	memset(errbuf, 0, sizeof errbuf);
	va_start(arg, fmt);
	vsnprintf(errbuf, sizeof errbuf, fmt, arg);
	va_end(arg);
}

char *
va(char *fmt, ...)
{
	va_list arg;
	static char buf[256];

	va_start(arg, fmt);
	vsnprintf(buf, sizeof buf, fmt, arg);
	va_end(arg);
	return buf;
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
	killcmd();	/* for interrupting awk threads */
	exit(EXIT_FAILURE);
}

char *
error(void)
{
	char *s;

	if(errno != 0){
		s = strerror(errno);
		errbuf[0] = 0;
		return s;
	}
	return errbuf;
}

vlong
μsec(void)
{
	struct timeval tv;

	if(gettimeofday(&tv, NULL) < 0){
		perror("gettimeofday");
		return -1;
	}
	return (vlong)tv.tv_sec * 1000000 + tv.tv_usec;
}

void
lsleep(vlong ns)
{
	struct timespec t = {0};

	t.tv_nsec = ns;
	if(ns >= 1000000000){
		t.tv_nsec %= 1000000000;
		t.tv_sec = ns / 1000000000;
	}
	if(nanosleep(&t, NULL) < 0)
		sysfatal("lsleep: %s", error());
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
	if(n == 0)
		return NULL;
	if((p = realloc(p, n)) == NULL)
		sysfatal("erealloc");
	if(n > oldn)
		memset((uchar *)p + oldn, 0, n - oldn);
	return p;
}

void *
emalloc(usize n)
{
	void *p;

	if(n == 0)
		return NULL;
	if((p = calloc(1, n)) == NULL)
		sysfatal("emalloc");
	return p;
}

void
initsys(void)
{
	//feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
}
