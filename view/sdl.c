#include "strpg.h"
#include <SDL2/SDL.h>

// rotate.c/shape.c or w/e: reshape nodes; SDL rect has rotation code
// bezier.c: bezier curves in c+SDL, see bookmarks
// 

struct{
	SDL_Window *win;
	SDL_Renderer *rend;
	SDL_Texture *tex;
} sdl;

int	SDL_RenderFillRect(SDL_Renderer*, SDL_Rect*);
int	SDL_RenderDrawLine(SDL_Renderer*, int, int, int, int);

/* NOTE: index must be shifted right one */

View view;

static long lastflip;	/* ms */

static void
flipview(void)
{
	// might be unnecessary; we should just draw directly on screen
	// the rest is what is kept in memory
	//SDL_UpdateTexture ..
	SDL_RenderClear(sdl.rend);
	SDL_RenderCopy(sdl.rend, sdl.tex, NULL, NULL);	// ?
	SDL_RenderPresent(sdl.rend);
}

static void
onscreen(void)
{
	// draw shape
	// take care of select() data structure: draw into index fb
	// flip prematurely if taking too long since last update
}

static void
setcol(usize id)
{
	int gid;
	SDL_Color c;

	//gid = getgroup(id);
	//

	//SDL_SetRenderDrawBlendMode(...);	// value for rects or lines
	//SDL_SetRenderDrawColor(sdl.rend, );	// value for *object group*
	SDL_SetRenderDrawColor(sdl.rend, 0xff, 0, 0, SDL_ALPHA_OPAQUE);
}

static int
drawline(usize id, Vrect *r)
{
	SDL_Rect rr;

	if(SDL_RenderDrawLine(sdl.rend, r->u.x, r->u.y, r->v.x, r->v.y) < 0){
		//SDL_GetError
		return -1;
	}
	// render to pointer image (probably not Rend function?
	return 0;
}

static int
drawrect(usize id, Vrect *r)
{
	SDL_Rect rr;

	if(SDL_RenderFillRect(sdl.rend, &rr) < 0){
		//SDL_GetError
		return -1;
	}
	// render to pointer image (probably not Rend function?
	return 0;
}

// NOTE: we can scale images using SDL_...
// NOTE: we can translate all images using SDL_BlitSurface's arguments
// NOTE: we *CANNOT* redraw statically using the renderer, any time we
//	need to just draw what we had, we have to start over with blank fb

static void
drawshapes(Graph *g, Render *rd)
{
	Shape *s, *se;
	Vrect r;
	SDL_Rect rr;

	for(s=&kv_A(rd->shapes, 0), se=s+kv_size(rd->shapes); s<se; s++){
		r.u = addvx(scalevx(s->r.u, view.zoom), view.pan);
		r.v = addvx(scalevx(s->r.v, view.zoom), view.pan);
		//rr = (SDL_Rect){r.u.x, r.u.y, dxvx(r), dyvx(r)};
		setcol(s->id);
		if((s->id & SHrect) == SHrect)
			drawrect(s->id >> 1, &r);
		else
			drawline(s->id >> 1, &r);
	}
}

void
show(Graph *g, Render *r)
{
	//lastflip = msec();
	SDL_RenderClear(sdl.rend);
	drawshapes(g, r);
	SDL_RenderPresent(sdl.rend);
}

void
resetview(void)
{
	// reset renderer? for now, leave as is
	// reallocate pointer sdl_surface
	// reallocate pointer array
		// do not grow beyond what is currently needed
		// less than 256 objects -> u8int
}

void
initview(void)
{
	View v = {0};

	v.w = Vdefw;
	v.h = Vdefh;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		sysfatal("SDL_Init: %s\n", SDL_GetError());
	if((sdl.win = SDL_CreateWindow(
		"strpg",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		v.w, v.h,
		SDL_WINDOW_ALLOW_HIGHDPI)) == NULL)
		sysfatal("SDL_CreateWindow: %s\n", SDL_GetError());
	if((sdl.rend = SDL_CreateRenderer(
		sdl.win, -1, SDL_RENDERER_PRESENTVSYNC)) == NULL)
		sysfatal("SDL_CreateRenderer: %s\n", SDL_GetError());
	if((sdl.tex = SDL_CreateTexture(
		sdl.rend,
		/* FIXME: use CMAP8 + palette: SDL_Surface, SDL_PixelFormat */
		SDL_PIXELFORMAT_INDEX8, SDL_TEXTUREACCESS_STATIC,	// FIXME: static or streaming?
		v.w, v.h)) == NULL)
		sysfatal("SDL_CreateTexture: %s\n", SDL_GetError());
	SDL_GetWindowSize(sdl.win, &v.w, &v.h);
	view = v;
	resetview();
}
