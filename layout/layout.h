enum{
	LFstop = 1<<0,
	LFnuke = 1<<1,
};

typedef struct Target Target;
enum{
	LLpfr,
	LLpfr3d,
	LLnil,
};
struct Target{
	char *name;
	void* (*init)(void);
	void* (*new)(int);
	void (*update)(void*);
	void (*cleanup)(void*);
	/* FIXME: ??????????????? clang needs volatile or it optimizes
	 * the access away or something */
	int (*compute)(void*, volatile int*, int);
};
extern int deflayout;
extern int nlaythreads;

Target*	regpfr(void);
Target*	regpfr3d(void);

enum{
	Lidle,
	Lstop,
	Lstart,
	Lreset,
	Lexport,
};
void	initlayout(void);
int	newlayout(int);
int reqlayout(int);
void	exportforever(void);

#define SETDIR(dir, dx, dy)	do{\
	if((dx) != 0.0f) \
		(dir) = V((dx), (dy), 0.0f); \
}while(0)
