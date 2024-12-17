#include <pthread.h>
#include "lib/chan.h"

extern int mainstacksize;

typedef pthread_rwlock_t RWLock;
typedef struct chan_t Channel;

Channel*	chancreate(int, int);
int	sendul(Channel*, ulong);
void*	recvp(Channel*);
ulong	recvul(Channel*);
int	nbsendp(Channel*, void*);
int	nbsendul(Channel*, ulong);
ulong	nbrecvul(Channel*);
void*	nbrecvp(Channel*);
#define	sendp	chan_send

#define rlock	pthread_rwlock_rdlock
#define runlock	pthread_rwlock_unlock
#define wlock	pthread_rwlock_wrlock
#define wunlock	pthread_rwlock_unlock
