enum{
	LLconga,
	LLrandom,
	LLforce,
	LLfr,
	LLlinear,
	LLnil,
};
struct Layout{
	char *name;
	void (*compute)(Graph*);
	void (*init)(Graph*);
};
extern int deflayout;

Layout*	regconga(void);
Layout*	regforce(void);
Layout*	reglinear(void);
Layout*	regfr(void);
Layout*	regrandom(void);
void	putnode(Node*, int, int);
