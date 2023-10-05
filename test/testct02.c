#include "strpg.h"
#include "fs.h"
#include "em.h"

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

void
threadmain(int, char**)
{
	char *path;
	EM *em;

	debug = -1ULL;
	path = touch(Maxchunk);	// next test: maxgreed then beyond
	em = emclone(path);
	if(empput64(em, Maxchunk/8, 0xdeadbeefcafebabeULL) < 0)
		sysfatal("emput64: %s", error());
	if(empget64(em, Maxchunk/8) != 0xdeadbeefcafebabeULL)
		sysfatal("empget64: %s", error());
	emclose(em, 0);
	assert(access(path, AEXIST) == 0);
	remove(path);
	sysquit();
}
