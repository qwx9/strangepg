typedef struct Filefmt Filefmt;

struct File{
	char *path;
	void *aux;
	uchar buf[IOUNIT+1];
	int trunc;
	int nr;
	int err;	// FIXME: not here
	vlong foff;
};

struct Filefmt{
	char *name;
	void (*load)(void *);
	int (*save)(Graph*);
	void (*nuke)(Graph*);
};

enum{
	FFdead,	/* deallocated */
	FFgfa,
	FFcsv,
	FFlayout,
	FFnil,
};

Filefmt*	reggfa(void);
Filefmt*	regcsv(void);
int	readchar(File*);
char*	nextfield(File*, char*, int*, char);
char*	readfrag(File*, int*);
char*	readline(File*, int*);
void	regfs(Filefmt*);

int	importlayout(Graph*, char*);
int	exportlayout(Graph*, char*);
void	setnamedtag(char*, char*, char*);
void	settag(Node*, ioff, char*, char*);

int	sysopen(File*, int);
int	sysfdopen(File*, int, int);
int	syswrite(File*, void*, int);
int	sysread(File*, void*, int);
int	syswstatlen(File*, vlong);
vlong	sysftell(File*);
vlong	sysseek(File*, vlong);
void	sysremove(char*);
int	sysmktmp(void);
void	sysflush(File*);
void	sysclose(File*);

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

int	get8(File*, u8int*);
int	get16(File*, u16int*);
int	get32(File*, u32int*);
int	get64(File*, u64int*);
u8int	eget8(File*);
u16int	eget16(File*);
u32int	eget32(File*);
u64int	eget64(File*);
double	egetdbl(File*);
int	put8(File*, u8int);
int	put16(File*, u16int);
int	put32(File*, u32int);
int	put64(File*, u64int);

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
void	closefs(File*);
void	freefs(File*);
int	loadfs(char*, int);
void	initfs(void);
