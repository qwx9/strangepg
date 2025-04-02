#include <pthread.h>
#include "lib/chan.h"

extern int mainstacksize;

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

typedef pthread_mutex_t	QLock;
typedef pthread_rwlock_t RWLock;

void	initqlock(QLock*);
void	initrwlock(RWLock*);
void	nukerwlock(RWLock*);

static inline void
rlock(RWLock *l)
{
	if(pthread_rwlock_rdlock(l) != 0)
		sysfatal("pthread_rwlock_rdlock: %s", error());
}

static inline void
wlock(RWLock *l)
{
	if(pthread_rwlock_wrlock(l) != 0)
		sysfatal("pthread_rwlock_wrlock: %s", error());
}

static inline void
wunlock(RWLock *l)
{
	if(pthread_rwlock_unlock(l) != 0)
		sysfatal("pthread_rwlock_unlock: %s", error());
}

static inline void
runlock(RWLock *l)
{
	if(pthread_rwlock_unlock(l) != 0)
		sysfatal("pthread_rwlock_unlock: %s", error());
}

static inline void
qlock(QLock *l)
{
	if(pthread_mutex_lock(l) != 0)
		sysfatal("pthread_mutex_lock: %s", error());
}

static inline void
qunlock(QLock *l)
{
	if(pthread_mutex_unlock(l) != 0)
		sysfatal("pthread_mutex_unlock: %s", error());
}
