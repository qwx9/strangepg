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
	return write(em->fd, buf, n);
}

ssize
emsysread(EM *em, uchar *buf, ssize n)
{
	return read(em->fd, buf, n);
}

vlong
emsysseek(EM *em, vlong off, int mode)
{
	return seek(em->fd, off, mode);
}
