enum{
	LFstop = 1<<0,
};

typedef struct Shitkicker Shitkicker;
enum{
	LLrandom,
	LLconga,
	LLfr,
	LLpfr,
	LLlinear,
	LLcirc,
	LLbo,
	LLnil,
};
struct Shitkicker{
	char *name;
	void* (*new)(Graph*);
	void (*update)(Graph*, void*);
	void (*cleanup)(void*);
	/* FIXME: ??????????????? clang needs volatile or it optimizes
	 * the access away or something */
	int (*compute)(void*, volatile int*, int);
};
extern int deflayout;
extern int nlaythreads;

Shitkicker*	regrandom(void);
Shitkicker*	regconga(void);
Shitkicker*	regfr(void);
Shitkicker*	regpfr(void);
Shitkicker*	reglinear(void);
Shitkicker*	regcirc(void);
Shitkicker*	regbo(void);

void	initlayout(void);
int	runlayout(Graph*, int);
int	resetlayout(Graph*);
int	haltlayout(Graph*);
int	stoplayout(Graph*);
int	updatelayout(Graph*);
int	togglelayout(Graph*);

#define SETDIR(dir, dx, dy)	do{\
	if((dx) != 0.0f) \
		(dir) = V((dx), (dy), 0.0f); \
}while(0)
