#include "strpg.h"
#include "fs.h"
#include "em.h"

enum{
	/* must be powers of two */
	Poolsz = 1ULL<<30,
	Bshift = 24,
	Banksz = 1<<8,
	Bmask = Banksz - 1,
	Pshift = 16,
	Pagesz = 1<<16,
	Pmask = Pagesz - 1,
};
static uchar buf[Pagesz];

char *
touch(void)
{
	int fd;
	u64int v;
	uchar *p;
	static char *path;

	path = sysmktmp();
	if((fd = create(path, OWRITE, 0644)) < 0)
		sysfatal("create: %s", error());
	for(p=buf, v=0; p<buf+sizeof buf; v++, p+=8)
		PBIT64(p, v);
	if(write(fd, buf, sizeof buf) != sizeof buf)
		sysfatal("write: %s", error());
	close(fd);
	return path;
}

int
main(int argc, char **argv)
{
	char *path;
	u64int v, w;
	EM *em;

	path = touch();
	if((em = emopen(path, 0)) == nil)
		sysfatal("emopen: %s", error());
	if((w = emr64(em, 0)) != 0)
		sysfatal("got %llux instead of %llux", w, 0ULL);
	for(v=1; v<nelem(buf)/8; v++){
		if((w = emr64(em, v)) != v)
			sysfatal("got %llux instead of %llux", w, v);
	}
	emclose(em);
	remove(path);
	free(path);
	sysquit();
}
