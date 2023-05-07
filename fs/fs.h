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
char*	readrecord(File*);
void	regfs(Filefmt*);

u8int	get8(File*);
u16int	get16(File*);
u32int	get32(File*);
u64int	get64(File*);
