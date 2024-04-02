#include <thread.h>

typedef	void	thret_t;
typedef	struct Thread Thread;
struct Thread{
	int tid;
	void *arg;
};

Thread*	newthread(thret_t (*)(void*), void*, uint);
void	namethread(Thread*, char*);
void	killthread(Thread*);

#define	exitthread(t, s)	do{ \
	free((t)); \
	threadexits((s)); \
}while(0)
