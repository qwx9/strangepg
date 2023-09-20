typedef struct Filefmt Filefmt;

struct File{
	char *path;
	void *aux;
	char *s;
	char *fld[16];	/* FIXME: why */
	int nf;
	int nr;
	int err;
	vlong foff;		/* file offset at *start of record* */
};

struct Filefmt{
	char *name;
	Graph* (*load)(char *);
	int (*save)(Graph*);
	void (*nuke)(Graph*);
};

Filefmt*	reggfa(void);
Filefmt*	regindex(void);
char*	readrecord(File*);
void	regfs(Filefmt*);

#define	GBIT8(p)	(((uchar*)(p))[0])
#define	GBIT16(p)	(((uchar*)(p))[0]|(((uchar*)(p))[1]<<8))
#define	GBIT32(p)	(((uchar*)(p))[0]|(((uchar*)(p))[1]<<8)|\
				(((uchar*)(p))[2]<<16)|(((uchar*)(p))[3]<<24))
#define	GBIT64(p)	((u32int)(((uchar*)(p))[0]|(((uchar*)(p))[1]<<8)|\
	(((uchar*)(p))[2]<<16)|(((uchar*)(p))[3]<<24)) |\
	((uvlong)(((uchar*)(p))[4]|(((uchar*)(p))[5]<<8)|\
	(((uchar*)(p))[6]<<16)|(((uchar*)(p))[7]<<24)) << 32))
#define	PBIT8(p,v)	do{(p)[0]=(v);}while(0)
#define	PBIT16(p,v)	do{(p)[0]=(v);(p)[1]=(v)>>8;}while(0)
#define	PBIT32(p,v)	do{(p)[0]=(v);(p)[1]=(v)>>8;(p)[2]=(v)>>16;(p)[3]=(v)>>24;}while(0)
#define	PBIT64(p,v)	do{(p)[0]=(v);(p)[1]=(v)>>8;(p)[2]=(v)>>16;(p)[3]=(v)>>24;\
			   (p)[4]=(v)>>32;(p)[5]=(v)>>40;(p)[6]=(v)>>48;(p)[7]=(v)>>56;}while(0)

void	freefs(File*);
int	openfs(File*, char*, int);
int	fdopenfs(File*, int, int);
File*	graphopenfs(Graph*, char*, int);
int	chlevel(Graph*, int);
int	readfs(File*, void*, int);
int	setsizefs(File*, vlong);
int	writefs(File*, void*, int);
void	flushfs(File*);
vlong	seekfs(File*, vlong);
vlong	tellfs(File*);
int	opentmpfs(File*);
void	closefs(File*);
u8int	get8(File*);
u16int	get16(File*);
u32int	get32(File*);
u64int	get64(File*);
double	getdbl(File*);
int	put8(File*, u8int);
int	put16(File*, u16int);
int	put32(File*, u32int);
int	put64(File*, u64int);

int	sysopen(File*, int);
int	sysfdopen(File*, int, int);
int	syswrite(File*, void*, int);
int	sysread(File*, void*, int);
int	syswstatlen(File*, vlong);
vlong	sysftell(File*);
vlong	sysseek(File*, vlong);
void	sysremove(char*);
char*	sysmktmp(void);
void	sysflush(File*);
void	sysclose(File*);
