#include "strpg.h"
#include "fs.h"
#include "em.h"

static uchar buf[Chunksz];

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

void
threadmain(int, char**)
{
	char *path;
	u64int v, w;
	EM *em;

	path = touch();
	if((em = emclone(path)) == nil)
		sysfatal("emclone: %s", error());
	if((w = empget64(em, 0)) != 0)
		sysfatal("got %llux instead of %llux", w, 0ULL);
	for(v=1; v<nelem(buf)/8; v++)
		if((w = emget64(em)) != v)
			sysfatal("got %llux instead of %llux", w, v);
	emclose(em);
	remove(path);
	free(path);
	sysquit();
}
