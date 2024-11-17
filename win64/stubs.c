#include "strpg.h"

pid_t
fork(void)
{
	return -1;
}

ssize_t
pread(int fd, void *buf, size_t count, off_t offset)
{
	return -1;
}

ssize_t
pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	return -1;
}

int
pipe(int pipefd[2])
{
	return -1;
}
