enum{
	LLconga,
	LLrandom,
	LLforce,
	LLfr,
	LLlinear,
	LLpfr,
	LLpline,
	LLnil,
};
struct Layout{
	char *name;
	void (*compute)(Graph*);
	void (*init)(Graph*);
	void (*cleanup)(void*);
};
extern int deflayout;

Layout*	regconga(void);
Layout*	regforce(void);
Layout*	reglinear(void);
Layout*	regfr(void);
Layout*	regpfr(void);
Layout*	regpline(void);
Layout*	regrandom(void);
void	putnode(Node*, int, int);
