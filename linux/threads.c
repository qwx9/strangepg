#include "strpg.h"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int mainstacksize;	/* FIXME: set it */

static _Atomic(u32int) *fux;

Thread *
newthread(thret_t (*fp)(void*), void *arg, uint stacksize)
{
	ssize st;
	pthread_attr_t a;
	pthread_t p;
	Thread *t;

	if((r = pthread_attr_init(&a)) != 0)
		sysfatal("newthread: pthread_attr_init: %d %s", r, error());
	if((r = pthread_attr_getstacksize(&a, &st)) != 0)
		sysfatal("newthread: pthread_attr_getstacksize: %d %s", r, error());
	if(st < stacksize){
		st = stacksize;
		if((r = pthread_attr_setstacksize(&a, &st)) != 0)
			sysfatal("newthread: pthread_attr_setstacksize: %d %s", r, error());
	}
	t = emalloc(sizeof *t);
	t->arg = arg;
	t->aux = emalloc(sizeof p);
	(pthread_t *)*t->aux = p;
	if((r = pthread_create(t->aux, &a, fp, t)) != 0)
		sysfatal("newthread: pthread_create: %d %s", r, error());
	if((r = pthread_attr_destroy(&a)) != 0)
		sysfatal("newthread: pthread_attr_destroy: %d %s", r, error());
	return t;
}

void
initthread(Thread *t, char *s)
{
	int r;

	if((r = pthread_name_np(t->aux, s)) != 0)
		sysfatal("pthread_name_np: %d %s", r, error());
	t->pid = getpid();
}

void
killthread(Thread *t)
{
	if(t == nil)
		return;
	pthread_cancel(t->aux);
	//waitpid(t->aux, NULL, 0);
	free(t->aux);
	free(t);
}

void
exitthread(Thread *t, char *err)
{
	if(err)
		warn("thread %d: %s\n", t->pid, err);
	free(t->aux);
	free(t);
	pthread_exit(nil);
}

/* futex(2) */
static int
futex(_Atomic(u32int) *addr, int op, uint32_t val)
{
	return syscall(SYS_futex, addr, op, val, NULL, NULL, 0);
}

FIXME: rlock/runlock
void
coffeetime(void)
{
	u32int one;

	one = 1;
	for(;;){
		if(atomic_compare_exchange_strong(fux, &one, 0))
			break;
		if(futex(fux, FUTEX_WAIT, 0) < 0 && errno != EAGAIN)
			sysfatal("waitcoffeetime: futex wait");
	}
}

void
coffeeover(void)
{
	u32int zilch;

	zilch = 0;
	if(atomic_compare_exchange_strong(fux, &zilch, 1)){
		if(futex(fux, FUTEX_WAKE, 1) < 0)
			sysfatal("coffeeover: futex wake");
	}
}

void
threadsetname(char *)
{
	/* FIXME: use pthread_setname_np */
}

int
proccreate(void (*f)(void *arg), void *arg, uint)
{
	int r;
	pthread_t th;

	/* FIXME: incompatible function type, return void vs void* */
	if((r = pthread_create(&th, NULL, (void*(*)(void*))f, arg)) != 0)
		return -1;
	return 0;
}

void
threadexits(char *)
{
	pthread_exit(nil);
}

Channel *
chancreate(int elsize, int nel)
{
	USED(elsize);
	return chan_init(nel);
}

void *
recvp(Channel *c)
{
	void *p;

	if(chan_recv(c, &p) < 0)
		return nil;
	return p;
}

int
nbsendp(Channel *c, void *p)
{
	int r;

	if(chan_select(nil, 0, nil, &c, 1, &p) < 0)
		return 0;
	return 1;
}

void
initthread(void)
{
	if((fux = mmap(NULL, sizeof(*fux), PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED)
			sysfatal("sysinit: mmap wake");
	*fux = 1;
}
