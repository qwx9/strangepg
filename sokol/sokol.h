typedef struct Vparam Vparam;
typedef struct Fparam Fparam;
typedef struct Render Render;

/* FIXME: use ctype in shader */
struct Vparam{
	HMM_Mat4 mvp;
};
struct Fparam{
	float color[4];
};

struct Render{
	int caching;	/* rendering into picking buffer */
	int moving;		/* inhibit picking while moving things */
	int stalepick;	/* picking cache stale */
	sg_pass_action clearscreen;
	sg_pass_action nothing;
	sg_attachments offscrfb;
	sg_pipeline nodepipe;
	sg_pipeline offscrnodepipe;
	sg_pipeline edgepipe;
	sg_pipeline offscredgepipe;
	sg_pipeline offscrpipe;
	sg_bindings nodebind;
	sg_bindings edgebind;
	sg_bindings offscrbind;
	u32int *pickfb;
	int nnodev;
	int nedgev;
	Fparam edgefs;
	Vparam cam;
};
extern Render render;

/* FIXME: provide aligned data as per std140? */
struct Params{
	HMM_Mat4 mvp;
};

struct Color{
	float col[4];
};

extern char *node_vertsh, *node_fragsh, *nodeidx_vertsh, *nodeidx_fragsh;
extern char *edge_vertsh, *edge_fragsh, *edgeidx_vertsh, *edgeidx_fragsh;
extern char *scr_vertsh, *scr_fragsh;

void	updateview(void);
void	resize(void);
void	initnk(void);
void	initgl(void);
