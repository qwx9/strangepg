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
