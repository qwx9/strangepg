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

	Addr = 0x21ff86,
};
char *
touch(usize n)
{
	int fd;
	uchar u[1];
	static char *path = "_strpg.test04";

	if((fd = create(path, OWRITE, 0644)) < 0)
		sysfatal("create: %s", error());
	if(seek(fd, n-1, 0) < 0)
		sysfatal("seek: %s", error());
	if(write(fd, u, 1) != 1)
		sysfatal("write: %s", error());
	close(fd);
	return path;
}

int
main(int, char **)
{
	char *path;
	EM *em;

	initem();
	path = touch(Addr);
	if((em = emopen(path)) == nil)
		sysfatal("emopen: %s", error());
	emw64(em, Addr, 0xdeadbeefcafebabeULL);
	if(emr64(em, Addr) != 0xdeadbeefcafebabeULL)
		sysfatal("emr64: %s", error());
	emclose(em);
	assert(access(path, AEXIST) == 0);
	remove(path);
	sysquit();
	return 0;
}
