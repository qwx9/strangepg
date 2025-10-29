#include "strpg.h"
#include "fs.h"
#include <sys/stat.h>

void
sysclose(File *f)
{
	int fd;

	if((fd = (intptr)f->aux) < 0)
		return;
	close(fd);
	fd = -1;
	f->aux = (void *)(intptr)fd;
}

int
sysfdopen(File *f, int fd, int)
{
	f->aux = (void *)(intptr)fd;
	return 0;
}

int
sysopen(File *f, char *path, int flags)
{
	int fd;

	if(flags & (ORDWR|OWRITE))
		flags |= O_CREAT | O_TRUNC;	/* FIXME: not for ORDWR? */
	if((fd = open(path, flags, 0644)) < 0)
		return -1;
	return sysfdopen(f, fd, 0);
}

int
sysfd(int which)
{
	switch(which){
	case 0: return STDIN_FILENO;
	case 1: return STDOUT_FILENO;
	case 2: return STDERR_FILENO;
	default: werrstr("invalid fd"); return -1;
	}
}

void
sysflush(File *)
{
}

/* there has to be a better wstat equivalent than this */
int
syswstatlen(File *f, vlong n)
{
	int fd;
	struct stat st;

	if((fd = (intptr)f->aux) < 0)
		return 0;
	fstat(fd, &st);
	if(n <= st.st_size)
		return 0;
	sysseek(f, n);
	syswrite(f, &n, 1);
	return 0;
}

int
sysseek(File *f, vlong off)
{
	int fd;

	if((fd = (intptr)f->aux) < 0)
		return 0;
	if(lseek(fd, off, SEEK_SET) < 0)
		return -1;
	return 0;
}

vlong
sysftell(File *f)
{
	int fd;
	off_t n;

	if((fd = (intptr)f->aux) < 0)
		return 0;
	if((n = lseek(fd, 0, SEEK_CUR)) < 0)
		return -1;
	return n;
}

/* FIXME: error check in callers */
void
sysremove(char *path)
{
	if(remove(path) < 0)
		warn("remove %s: %s\n", path, error());
}

/* avoid hidden files, especially on error */
int
sysmktmp(void)
{
	int fd;
	char tmp[] = "_strpg.XXXXXX";

	if((fd = mkstemp(tmp)) < 0)
		return -1;
	if(unlink(tmp) < 0)
		warn("sysmktmp: %s\n", error());
	return fd;
}

int
syswrite(File *f, void *buf, int n)
{
	int fd, m;

	if((fd = (intptr)f->aux) < 0)
		return 0;
	if((m = write(fd, buf, n)) != n)
		return -1;
	return m;
}

int
sysread(File *f, void *buf, int n)
{
	int fd;

	if((fd = (intptr)f->aux) < 0)
		return 0;
	return read(fd, buf, n);
}
