#include <pthread.h>
#include "lib/chan.h"

extern int mainstacksize;

typedef struct chan_t Channel;
Channel*	chancreate(int, int);
void*	recvp(Channel*);
ulong	recvul(Channel*);
int	nbsendp(Channel*, void*);
int	nbsendul(Channel*, ulong);
ulong	nbrecvul(Channel*);
#define	sendul	chan_send_int32
#define	sendp	chan_send

typedef pthread_rwlock_t RWLock;
#define rlock	pthread_rwlock_rdlock
#define runlock	pthread_rwlock_unlock
#define wlock	pthread_rwlock_wrlock
#define wunlock	pthread_rwlock_unlock
