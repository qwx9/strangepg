#include "strpg.h"
#include "drw.h"

u32int *theme;

static u32int theme1[Cend] = {
	[Cbg]	0x000000,
	[Ctext]	0xbb1100,
	[Cnode]	0x2222ff,
	[Cedge]	0xbbbbbb,
	[Cemph]	0xff0000,
};
static u32int theme2[Cend] = {
	[Cbg]	0xffffff,
	[Ctext]	0xeeee00,
	[Cnode]	0xdddd00,
	[Cedge]	0x777777,
	[Cemph]	0xff0000,
};

static u32int colors[] = {
	/* 12 class paired */
	0x1f78b4,	/* light blue */
	0xff7f00,	/* light orange */
	0x33a02c,	/* light green */
	0xe31a1c,	/* light red */
	0x6a3d9a,	/* violet */
	0xb15928,	/* light brown */
	/* some bandage */
	0x8080ff,	/* pale blue */
	0x8ec65e,	/* pale green */
	0xc76758,	/* pale red */
	0xca9560,	/* pale orange */
	0xc893f0,	/* pale violet */
	0x7f5f67,	/* grey brown */
	0xb160c9,	/* light violet */
	0x5fc69f,	/* pale blueish green */
	0xc96088,	/* pink violet */
	/* 12 class set3 */
	0x8dd3c7,	/* cyan */
	0xffffb3,	/* pale yellow */
	0xbebada,	/* grey blue */
	0xfb8072,	/* light orange (2) */
	0x80b1d3,	/* light blue (2) */
	0xfdb462,	/* light orange (2) */
	0xb3de69,	/* light green (2) */
	0xfccde5,	/* grey pink */
	0xd9d9d9,	/* light grey */
	0xbc80bd,	/* light violet */
	0xccebc5,	/* grey green */
	0xffed6f,	/* light yellow */
	/* 12 class paired, pale counterparts */
	0xa6cee3,	/* light blue (2) */
	0xb2df8a,	/* light green (3) */
	0xfb9a99,	/* light pink */
	0xfdbf6f,	/* light orange (3) */
	0xcab2d6,	/* grey violet */
	0xffff99,	/* pale yellow */
};

khash_t(cmap) *cmap;

Color *
color(u32int v)
{
	int ret;
	khiter_t k;
	Color *c;

	k = kh_get(cmap, cmap, v);
	if(k == kh_end(cmap)){
		c = newcolor(v);
		k = kh_put(cmap, cmap, v, &ret);
		kh_val(cmap, k) = c;
	}else
		c = kh_val(cmap, k);
	return c;
}

Color *
somecolor(ioff i)
{
	assert(i >= 0);
	return color(colors[i % nelem(colors)]);
}

void
settheme(void)
{
	if((view.flags & VFhaxx0rz) != 0)
		theme = theme1;
	else
		theme = theme2;
}
