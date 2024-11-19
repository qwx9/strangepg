typedef struct File File;
typedef struct Filefmt Filefmt;

/* in case a field is in between reads in a long line,
 * leave one chance to save it */
enum{
	Readsz = IOUNIT,
};
struct File{
	char *path;
	void *aux;
	char buf[2 * Readsz + 1];
	char *end;	/* end of current read */
	int len;
	int trunc;
	vlong foff;	/* current token file offset*/
	int nr;		/* current line number */
	int nf;		/* current field number */
	char *tok;	/* if tokenizing, end of current token */
	int toksz;
	char sep;
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
char*	nextfield(File*);
char*	readfrag(File*);
char*	readline(File*);
void	splitfs(File*, char);
void	regfs(Filefmt*);

int	importlayout(Graph*, char*);
int	exportlayout(Graph*, char*);

int	sysopen(File*, char*, int);
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

File*	openfs(char*, int);
File*	fdopenfs(int, int);
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
