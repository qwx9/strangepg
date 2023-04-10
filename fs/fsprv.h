typedef struct Filefmt Filefmt;

struct File{
	char *path;
	void *aux;
	char *s;
	char *fld[64];
	int nf;
	int nr;
	int err;
};

struct Filefmt{
	char *name;
	Graph* (*load)(char *);
	int (*save)(Graph*);
};
extern Filefmt fftab[FFnil];

Filefmt*	reggfa(void);
char*	readrecord_(File*);
void	regfs(Filefmt*);
