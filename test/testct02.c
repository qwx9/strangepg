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

	EMrdonly = 1<<0,
};
char *
touch(uint n)
{
	int fd;
	uchar u[1];
	static char *path;

	path = sysmktmp();
	if((fd = create(path, OWRITE, 0644)) < 0)
		sysfatal("create: %s", error());
	if(seek(fd, n-1, 0) < 0)	// next tests: non-aligned
		sysfatal("seek: %s", error());
	if(write(fd, u, 1) != 1)
		sysfatal("write: %s", error());
	close(fd);
	return path;
}

int
main(int argc, char **argv)
{
	char *path;
	EM *em;

	path = touch(Pagesz);	// next test: maxgreed then beyond
	if((em = emopen(path, 0)) == nil)
		sysfatal("emopen: %s", error());
	emw64(em, Pagesz/8, 0xdeadbeefcafebabeULL);
	if(emr64(em, Pagesz/8) != 0xdeadbeefcafebabeULL)
		sysfatal("emr64: %s", error());
	emclose(em);
	assert(access(path, AEXIST) == 0);
	remove(path);
	sysquit();
}
