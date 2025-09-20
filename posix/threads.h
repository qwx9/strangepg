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

static inline int
canrlock(RWLock *l)
{
	return pthread_rwlock_tryrdlock(l) == 0 ? 1 : 0;
}

static inline int
canwlock(RWLock *l)
{
	return pthread_rwlock_trywrlock(l) == 0 ? 1 : 0;
}

static inline void
rlock(RWLock *l)
{
	int r;

	if((r = pthread_rwlock_rdlock(l)) != 0)
		sysfatal("pthread_rwlock_rdlock: %s", strerror(r));
}

static inline void
wlock(RWLock *l)
{
	int r;

	if((r = pthread_rwlock_wrlock(l)) != 0)
		sysfatal("pthread_rwlock_wrlock: %s", strerror(r));
}

static inline void
wunlock(RWLock *l)
{
	int r;

	if((r = pthread_rwlock_unlock(l)) != 0)
		sysfatal("pthread_rwlock_unlock: %s", strerror(r));
}

static inline void
runlock(RWLock *l)
{
	int r;

	if((r = pthread_rwlock_unlock(l)) != 0)
		sysfatal("pthread_rwlock_unlock: %s", strerror(r));
}

static inline int
canqlock(QLock *l)
{
	return pthread_mutex_trylock(l) == 0 ? 1 : 0;
}

static inline void
qlock(QLock *l)
{
	int r;

	if((r = pthread_mutex_lock(l)) != 0)
		sysfatal("pthread_mutex_lock: %s", strerror(r));
}

static inline void
qunlock(QLock *l)
{
	int r;

	if((r = pthread_mutex_unlock(l)) != 0)
		sysfatal("pthread_mutex_unlock: %s", strerror(r));
}
