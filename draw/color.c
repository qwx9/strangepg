#include "strpg.h"
#include "drw.h"
#include "fs.h"
#include "threads.h"
#include "lib/khashl.h"

u32int *theme;

static u32int theme1[Cend] = {
	[Cbg]	= 0x000000ff,
	[Ctext]	= 0xbb1100ff,
	[Cedge]	= 0xaaaaaa80,
	[Cemph]	= 0xff0000ff,
	[Chigh] = 0xffffccff,
	[Cxaxis] = 0xffccccff,
	[Cyaxis] = 0xccffccff,
	[Czaxis] = 0xccccffff,
};
static u32int theme2[Cend] = {
	[Cbg]	= 0xffffffff,
	[Ctext]	= 0xeeee00ff,
	[Cedge]	= 0xcccccc77,
	[Cemph]	= 0xff0000ff,
	[Chigh] = 0x000000ff,
	[Cxaxis] = 0xff0000ff,
	[Cyaxis] = 0x00ff00ff,
	[Czaxis] = 0x0000ffff,
};

/* D65 */
#define	Xn	95.0489
#define	Yn	100.0
#define Zn	108.8840
#define δ	(6.0 / 29.0)
#define δ3	(δ * δ * δ)
#define R	(1.0 / 3400850)

static inline double
iFn(double t)
{
	return t > δ ? t * t * t : 3.0 * δ * δ * (t - 4.0 / 29.0);
}

static inline void
lab2xyz(double c[3])
{
	double l, a, b;

	l = c[0];
	a = c[1];
	b = c[2];
	c[0] = Xn * iFn((l + 16.0) / 116.0 + a / 500.0);
	c[1] = Yn * iFn((l + 16.0) / 116.0);
	c[2] = Zn * iFn((l + 16.0) / 116.0 - b / 200.0);
}

static inline void
xyz2rgb(double c[3])
{
	double x, y, z;
	static double f[9] = {
		8041697 * R, -3049000 * R, -1591847 * R,
		-1752003 * R, 4851000 * R, 301853 * R,
		17697 * R, -49000 * R, 3432153 * R,
	};

	x = c[0];
	y = c[1];
	z = c[2];
	c[0] = f[0] * x + f[1] * y + f[2] * z;
	c[1] = f[3] * x + f[4] * y + f[5] * z;
	c[2] = f[6] * x + f[7] * y + f[8] * z;
}

static inline void
ilinear(double c[3])
{
	double r, g, b;

	r = c[0];
	g = c[1];
	b = c[2];
	c[0] = r > 0.0031308 ? 1.055 * pow(r, 1.0 / 2.4) - 0.055 : 12.92 * r;
	c[1] = g > 0.0031308 ? 1.055 * pow(g, 1.0 / 2.4) - 0.055 : 12.92 * g;
	c[2] = b > 0.0031308 ? 1.055 * pow(b, 1.0 / 2.4) - 0.055 : 12.92 * b;
}

static inline void
lab2rgb(double c[3])
{
	lab2xyz(c);
	xyz2rgb(c);
	ilinear(c);
}

static inline void
linear(double c[3])
{
	double K, a, γ, r, g, b;

	r = c[0];
	g = c[1];
	b = c[2];
	K = 0.04045;
	a = 0.055;
	γ = 2.4;
	c[0] = r > K ? pow((r + a) / (1 + a), γ) : r / 12.92;
	c[1] = g > K ? pow((g + a) / (1 + a), γ) : g / 12.92;
	c[2] = b > K ? pow((b + a) / (1 + a), γ) : b / 12.92;
}

static inline void
rgb2xyz(double c[3])
{
	double r, g, b;

	r = c[0];
	g = c[1];
	b = c[2];
	c[0] = 0.49000 * r + 0.31000 * g + 0.20000 * b;
	c[1] = 0.17697 * r + 0.81240 * g + 0.01063 * b;
	c[2] = 0.00000 * r + 0.01000 * g + 0.99000 * b;
}

static inline double
Fn(double t)
{
	return t > δ3 ? pow(t, 1.0 / 3.0)
		: (1.0 / 3.0) * t * (1.0 / (δ * δ)) + 4.0 / 29.0;
}

static inline void
rgb2lab(double c[3])
{
	double x, y, z;

	linear(c);
	rgb2xyz(c);
	x = c[0];
	y = c[1];
	z = c[2];
	c[0] = 116.0 * Fn(y / Yn) - 16;
	c[1] = 500.0 * (Fn(x / Xn) - Fn(y / Yn));
	c[2] = 200.0 * (Fn(y / Yn) - Fn(z / Zn));
}

static void
mixcolors(float *cols, u32int v)
{
	double c1[3], c2[3];

	c1[0] = cols[0];
	c1[1] = cols[1];
	c1[2] = cols[2];
	c2[0] = (double)(v >> 16 & 0xff) / 255.0;
	c2[1] = (double)(v >> 8 & 0xff) / 255.0;
	c2[2] = (double)(v & 0xff) / 255.0;
	rgb2lab(c1);
	rgb2lab(c2);
	c1[0] = (c1[0] + c2[0]) / 2.0;
	c1[1] = (c1[1] + c2[1]) / 2.0;
	c1[2] = (c1[2] + c2[2]) / 2.0;
	lab2rgb(c1);
	cols[0] = c1[0];
	cols[1] = c1[1];
	cols[2] = c1[2];
}

void
highlightnode(ioff idx)
{
	RNode *r;

	r = rnodes + idx;
	mixcolors(r->col, theme[Chigh] >> 8);
}

void
setcolor(float *f, u32int c)
{
	f[3] = (c & 0xff) / 255.f;
	f[0] = f[3] * ((c >> 24 & 0xff) / 255.f);
	f[1] = f[3] * ((c >> 16 & 0xff) / 255.f);
	f[2] = f[3] * ((c >> 8 & 0xff) / 255.f);
}

u32int
setdefalpha(u32int c)
{
	if((c & 0xff) == 0){
		c &= ~0xff;
		c |= (drawing.flags & DFnoalpha ? 0xff
			: drawing.flags & DFhaxx0rz ? 0x70 : 0xc0);
	}
	return c;
}

void
settheme(void)
{
	if((drawing.flags & DFhaxx0rz) != 0)
		theme = theme1;
	else
		theme = theme2;
}
