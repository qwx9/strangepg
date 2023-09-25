#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

ssize
emsysopen(char *path)
{
	return open(path, O_RDONLY);
}

ssize
emsyscreate(char *path)
{
	return open(path, O_RDWR|O_CREAT|O_TRUNC, 0664);
}

ssize
emsyswrite(EM *em, uchar *buf, ssize n)
{
	ssize r;

	if((r = write(em->fd, buf, n)) > 0)
		em->lastoff += n;
	return r;
}

ssize
emsysread(EM *em, uchar *buf, ssize n)
{
	ssize r;

	if((r = read(em->fd, buf, n)) > 0)
		em->lastoff += n;
	return r;
}

vlong
emsysseek(EM *em, vlong off, int mode)
{
	return em->lastoff = lseek(em->fd, off, mode);
}

int
emsysclose(EM *em)
{
	return close(em->fd);
}
