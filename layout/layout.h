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
	LLpline,
	LLcirc,
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
Shitkicker*	regpline(void);
Shitkicker*	regcirc(void);

void	initlayout(void);
int	runlayout(Graph*, int);
int	resetlayout(Graph*);
int	haltlayout(Graph*);
int	updatelayout(Graph*);
int	togglelayout(Graph*);

#define ROTATENODE(rot, dir, dx, dy)	do{\
	float θ; \
	θ = atan2((dy), (dx)); \
	(rot)->z = θ; \
	(dir)->x = cos(θ); \
	(dir)->y = -sin(θ); \
}while(0)
