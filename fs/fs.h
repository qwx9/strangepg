typedef struct Filefmt Filefmt;

struct File{
	char *path;
	void *aux;
	char *s;
	char *fld[64];
	int nf;
	int nr;
	int err;
	vlong foff;		/* file offset at *start of record* */
};

struct Filefmt{
	char *name;
	Graph* (*load)(char *);
	int (*chlev)(Graph*, int);
	int (*save)(Graph*);
};

Filefmt*	reggfa(void);
Filefmt*	regindex(void);
char*	readrecord(File*);
void	regfs(Filefmt*);
