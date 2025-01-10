enum{
	LFstop = 1<<0,
	LFnuke = 1<<1,
};

typedef struct Target Target;
enum{
	LLfr,
	LLpfr,
	LLpfr3d,
	LLlinear,
	LLcirc,
	LLnil,
};
struct Target{
	char *name;
	void* (*init)(void);
	void* (*new)(void);
	void (*update)(void*);
	void (*cleanup)(void*);
	/* FIXME: ??????????????? clang needs volatile or it optimizes
	 * the access away or something */
	int (*compute)(void*, volatile int*, int);
};
extern int deflayout;
extern int nlaythreads;

Target*	regfr(void);
Target*	regpfr(void);
Target*	regpfr3d(void);
Target*	reglinear(void);
Target*	regcirc(void);

enum{
	Lidle,
	Lstop,
	Lstart,
	Lreset,
};
void	initlayout(void);
int	newlayout(Graph*, int);
int reqlayout(int);

#define SETDIR(dir, dx, dy)	do{\
	if((dx) != 0.0f) \
		(dir) = V((dx), (dy), 0.0f); \
}while(0)
