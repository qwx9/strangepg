typedef struct Filefmt Filefmt;
typedef struct Dat Dat;

enum{
	FFgfa,
	FFgfa2,
	FFnil,
};
struct Filefmt{
	char *name;
	Graph* (*load)(char *);
	int (*save)(Graph*);
};
extern Filefmt fftab[FFnil];

struct Dat{
	int type;
	Filefmt *ff;
	void *p;
	usize sz;
};

void	initfs(void);
void	regfs(Filefmt*);
Graph*	loadfs(int, char*);
