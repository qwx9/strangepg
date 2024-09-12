#include "strpg.h"
#include "drw.h"
#include "fs.h"
#include "em.h"
#include "lib/khashl.h"

KHASHL_MAP_INIT(KH_LOCAL, colormap, cmap, u32int, Color*, kh_hash_uint32, kh_eq_generic)
static colormap *cmap;

u32int *theme;

static u32int theme1[Cend] = {
	[Cbg]	= 0x000000ff,
	[Ctext]	= 0xbb1100ff,
	[Cnode]	= 0x2222ff90,
	[Cedge]	= 0xbbbbbb30,
	[Cemph]	= 0xff0000ff,
};
static u32int theme2[Cend] = {
	[Cbg]	= 0xffffffff,
	[Ctext]	= 0xeeee00ff,
	[Cnode]	= 0xdddd009f,
	[Cedge]	= 0x77777730,
	[Cemph]	= 0xff0000ff,
};

static u32int colors[] = {
	/* some bandage */
	0x8080ff,	/* purpleblue */
	0x8ec65e,	/* green */
	0xc76758,	/* grey red */
	0xca9560,	/* light brown */
	0xc893f0,	/* pale purple */
	0x7f5f67,	/* grey brown */
	0xb160c9,	/* purple */
	0x5fc69f,	/* blue green */
	0xc96088,	/* dark pink */
	/* 12 class paired */
	0x1f78b4,	/* blue */
	0xff7f00,	/* orange */
	0x33a02c,	/* dark green */
	0xe31a1c,	/* red */
	0x6a3d9a,	/* dark purple */
	0xb15928,	/* brown */
	/* 12 class set3 */
	0x8dd3c7,	/* light teal */
	0xeaf47f,	/* pale yellow */
	0xbebada,	/* grey blue */
	0xfb8072,	/* light red */
	0x80b1d3,	/* light blue */
	0xfdb462,	/* light orange */
	0xd5f65f,	/* light green */
	0xfccde5,	/* light pink */
	0xd9d9d9,	/* light grey */
	0xbc80bd,	/* light purple */
	0xccebc5,	/* grey green */
	0xffed6f,	/* yellow */
	/* 12 class paired, pale counterparts */
	0xa6cee3,	/* pale blue */
	0xb2df8a,	/* pale green */
	0xfb9a99,	/* pale red */
	0xfdbf6f,	/* pale orange */
	0xcab2d6,	/* pale violet */
	0x7acdcd,	/* cyan */
};

Color *
color(u32int v)
{
	int abs;
	khint_t k;
	Color *c;

	k = cmap_get(cmap, v);
	if(k == kh_end(cmap)){
		c = newcolor(v);
		k = cmap_put(cmap, v, &abs);
		kh_val(cmap, k) = c;
	}else
		c = kh_val(cmap, k);
	return c;
}

u32int
somecolor(ioff i)
{
	return colors[i % nelem(colors)] << 8 | 0x90;
}

void
settheme(void)
{
	if((view.flags & VFhaxx0rz) != 0)
		theme = theme1;
	else
		theme = theme2;
}

void
initcol(void)
{
	cmap = cmap_init();
}
