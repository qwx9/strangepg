#include "strpg.h"
#include "fs.h"
#include <bio.h>

int
sysfdopen(File *f, int fd, int mode)
{
	f->aux = emalloc(sizeof(Biobufhdr));	/* FIXME: danger zone */
	/* for line parsing, extra byte to always insures 0-terminated string */
	if(Binits(f->aux, fd, mode, f->buf, sizeof f->buf - 1) < 0)
		return -1;
	return 0;
}

int
sysopen(File *f, int mode)
{
	int fd;

	if((fd = open(f->path, mode)) < 0)
		return -1;
	return sysfdopen(f, fd, mode);
}

void
sysclose(File *f)
{
	assert(f->aux != nil);
	Bterm(f->aux);
	f->aux = nil;
}

void
sysflush(File *f)
{
	assert(f->aux != nil);
	Bflush(f->aux);
}

int
syswstatlen(File *f, vlong n)
{
	Dir d;
	Biobuf *bf;

	assert(f->aux != nil);
	nulldir(&d);
	d.length = n;
	bf = f->aux;
	if(dirfwstat(Bfildes(bf), &d) < 0)
		return -1;
	Bflush(bf);
	return 0;
}

vlong
sysseek(File *f, vlong off)
{
	return Bseek(f->aux, off, 0);
}

vlong
sysftell(File *f)
{
	return Boffset(f->aux);
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
	return Bwrite(f->aux, buf, n);
}

int
sysread(File *f, void *buf, int n)
{
	return Bread(f->aux, buf, n);
}

int
readchar(File *f)
{
	int c;
	Biobuf *bf;

	bf = f->aux;
	if((c = Bgetc(bf)) <= 0)
		return -1;
	return c;
}

/* FIXME: handle NULs in input? */
/* Brdline sucks but Brdstr will choke us when lines are very long;
 * so instead of removing bio, we break Brdline. */
char *
readfrag(File *f, int *len)
{
	int l;
	char *s;
	Biobuf *bf;

	assert(f != nil && f->aux != nil);
	if(len != nil)
		*len = 0;
	bf = f->aux;
	/* hack 1: force Brdline to discard truncated line and
	 * continue reading, which this bullshit should do itself */
	if(f->trunc){
		f->trunc = 0;
		bf->icount = 0;
	}
	s = Brdline(bf, '\n');
	l = Blinelen(bf);
	if(s == nil){
		/* hack 2: return truncated buffer as valid */
		if(l != 0){
			s = (char *)bf->gbuf;
			f->trunc = 1;
		}
	}else if(l > 0)
		s[--l] = 0;
	if(len != nil)
		*len = l;
	return s;
}
