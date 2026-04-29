#include "strpg.h"
#include "fs.h"

int
sysfdopen(File *f, int fd, int)
{
	f->aux = (void *)(intptr)fd;
	return 0;
}

int
sysopen(File *f, char *path, int mode)
{
	int fd;

	if(mode != OWRITE && (mode & OTRUNC) == 0){
		if((fd = open(path, mode)) < 0)
			return -1;
	}else{
		if((fd = create(path, mode, 0644)) < 0)
			return -1;
	}
	return sysfdopen(f, fd, mode);
}

int
sysfd(int which)
{
	switch(which){
	case 0 ... 2: return which;
	default: werrstr("invalid fd"); return -1;
	}
}

void
sysclose(File *f)
{
	int fd;

	fd = (intptr)f->aux;
	close(fd);
	f->aux = nil;
}

void
sysflush(File *)
{
}

int
syswstatlen(File *f, vlong n)
{
	int fd;
	Dir d;

	nulldir(&d);
	d.length = n;
	fd = (intptr)f->aux;
	if(dirfwstat(fd, &d) < 0)
		return -1;
	return 0;
}

int
sysseek(File *f, vlong off)
{
	int fd;

	fd = (intptr)f->aux;
	return seek(fd, off, 0);
}

vlong
sysftell(File *f)
{
	int fd;

	fd = (intptr)f->aux;
	return seek(fd, 0, 1);
}

/* FIXME: rename fsremove and use File*; sysremove should be in sys */
/* FIXME: don't mix fs and fd logic => fs* functions, sys* functions: buffered vs unbuffered io */
void
sysremove(char *path)
{
	if(remove(path) < 0)
		warn("remove: %r\n");
}

int
sysmktmp(void)
{
	char s[] = "_strpg.XXXXXXXXXXX";

	mktemp(s);
	if(strncmp(s, "/", sizeof s) == 0)
		return -1;
	return create(s, ORDWR|ORCLOSE, 0600);
}

int
syswrite(File *f, void *buf, int n)
{
	int fd;

	fd = (intptr)f->aux;
	return write(fd, buf, n);
}

int
sysread(File *f, void *buf, int n)
{
	int fd;

	fd = (intptr)f->aux;
	return read(fd, buf, n);
}
