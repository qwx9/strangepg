#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

ssize
emsysopen(char *path)
{
	return open(path, O_RDWR);
}

ssize
emsyswrite(EM *em, uchar *buf, ssize n)
{
	return write(em->fd, buf, n);
}

ssize
emsysread(EM *em, uchar *buf, ssize n)
{
	return read(em->fd, buf,  n);
}

int
emsysseek(EM *em, vlong off, int mode)
{
	return lseek(em->fd, off, mode);
}