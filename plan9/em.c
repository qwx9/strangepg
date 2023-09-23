#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

ssize
emsysopen(char *path)
{
	return open(path, OREAD);
}

ssize
emsyscreate(char *path)
{
	return create(path, ORDWR, 0644);
}

ssize
emsyswrite(EM *em, uchar *buf, ssize n)
{
	ssize r;

	if((r = write(em->fd, buf, n)) > 0)
		em->lastoff += r;
	return r;
}

ssize
emsysread(EM *em, uchar *buf, ssize n)
{
	ssize r;

	if((r = read(em->fd, buf, n)) > 0)
		em->lastoff += r;
	return r;
}

vlong
emsysseek(EM *em, vlong off, int mode)
{
	return em->lastoff = seek(em->fd, off, mode);
}

int
emsysclose(EM *em)
{
	return close(em->fd);
}
