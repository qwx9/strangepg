#include <pthreads.h>
#include "lib/chan.h"

extern int mainstacksize;

typedef struct chan_t Channel;
Channel*	chancreate(int, int);
void*	recvp(Channel*);
int	nbsendp(Channel*, void*);

int	proccreate(void (*)(void*), void*, uint);
typedef pthread_rwlock_t RWLock;
#define rlock	pthread_rwlock_rdlock
#define runlock	pthread_rwlock_unlock
#define wlock	pthread_rwlock_wrlock
#define wunlock	pthread_rwlock_unlock
// FIXME
void	threadsetname(char*);
void	threadexits(char*);

extern RWLock renderlock;
