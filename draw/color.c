#include "strpg.h"
#include "drw.h"
#include "fs.h"
#include "em.h"
#include "lib/khashl.h"

KHASHL_MAP_INIT(KH_LOCAL, colormap, cmap, u32int, Color*, kh_hash_uint32, kh_eq_generic)
static colormap *cmap;

u32int *theme;

static u32int theme1[Cend] = {
	[Cbg]	0x000000ff,
	[Ctext]	0xbb1100ff,
	[Cnode]	0x2222ff90,
	[Cedge]	0xbbbbbb30,
	[Cemph]	0xff0000ff,
};
static u32int theme2[Cend] = {
	[Cbg]	0xffffffff,
	[Ctext]	0xeeee00ff,
	[Cnode]	0xdddd009f,
	[Cedge]	0x77777730,
	[Cemph]	0xff0000ff,
};

static u32int colors[] = {
	/* 12 class paired */
	0x1f78b490,	/* light blue */
	0xff7f0090,	/* light orange */
	0x33a02c90,	/* light green */
	0xe31a1c90,	/* light red */
	0x6a3d9a90,	/* violet */
	0xb1592890,	/* light brown */
	/* some bandage */
	0x8080ff90,	/* pale blue */
	0x8ec65e90,	/* pale green */
	0xc7675890,	/* pale red */
	0xca956090,	/* pale orange */
	0xc893f090,	/* pale violet */
	0x7f5f6790,	/* grey brown */
	0xb160c990,	/* light violet */
	0x5fc69f90,	/* pale blueish green */
	0xc9608890,	/* pink violet */
	/* 12 class set3 */
	0x8dd3c790,	/* cyan */
	0xffffb390,	/* pale yellow */
	0xbebada90,	/* grey blue */
	0xfb807290,	/* light orange (2) */
	0x80b1d390,	/* light blue (2) */
	0xfdb46290,	/* light orange (2) */
	0xb3de6990,	/* light green (2) */
	0xfccde590,	/* grey pink */
	0xd9d9d990,	/* light grey */
	0xbc80bd90,	/* light violet */
	0xccebc590,	/* grey green */
	0xffed6f90,	/* light yellow */
	/* 12 class paired, pale counterparts */
	0xa6cee390,	/* light blue (2) */
	0xb2df8a90,	/* light green (3) */
	0xfb9a9990,	/* light pink */
	0xfdbf6f90,	/* light orange (3) */
	0xcab2d690,	/* grey violet */
	0xffff9990,	/* pale yellow */
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
	return colors[i % nelem(colors)];
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
