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
	int (*save)(Graph*);
};
extern Filefmt fftab[FFnil];

Filefmt*	reggfa(void);
char*	readrecord(File*);
void	regfs(Filefmt*);
