#include <pthread.h>
#include "lib/chan.h"

extern int mainstacksize;

typedef struct chan_t Channel;
Channel*	chancreate(int, int);
void*	recvp(Channel*);
int	nbsendp(Channel*, void*);
int	nbsendul(Channel*, ulong);
ulong	nbrecvul(Channel*);
#define	sendul	chan_send_int32
#define	sendp	chan_send

int	proccreate(void (*)(void*), void*, uint);
typedef pthread_rwlock_t RWLock;
#define rlock	pthread_rwlock_rdlock
#define runlock	pthread_rwlock_unlock
#define wlock	pthread_rwlock_wrlock
#define wunlock	pthread_rwlock_unlock

typedef struct Thread Thread;
struct Thread{
	pthread_t p;
	void *arg;
};

#define	exitthread(t, s)	do{ \
	if(t != nil){ \
		if((s) != nil) \
			warn("exitthread: %s\n", (s)); \
	} \
	pthread_exit(nil); \
	free(t); \
	return nil; \
}while(0)

Thread*	newthread(thret_t (*)(void*), void*, uint);
void	namethread(Thread*, char*);
void	killthread(Thread*);
