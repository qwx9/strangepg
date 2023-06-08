#include "strpg.h"
#include <SDL2/SDL.h>

// rotate.c/shape.c or w/e: reshape nodes; SDL rect has rotation code
// bezier.c: bezier curves in c+SDL, see bookmarks
// NOTE: we can scale images using SDL_...
// NOTE: we can translate all images using SDL_BlitSurface's arguments

struct{
	SDL_Window *win;
	SDL_Renderer *rend;
	SDL_Texture *tex;
} sdl;

static long lastflip;	/* ms */

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

int
drawquad(Quad q, int)
{
	SDL_Rect rr;

	if(SDL_RenderFillRect(sdl.rend, &rr) < 0){
		//SDL_GetError
		return -1;
	}
	// render to pointer image (probably not Rend function?
	return 0;
}

int
drawquad2(Quad, Quad, int, int)
{
	return 0;
}

int
drawline(Quad q, double w, int)
{
	SDL_Rect rr;

	if(SDL_RenderDrawLine(sdl.rend, q.o.x, q.o.y, q.v.x, q.v.y) < 0){
		//SDL_GetError
		return -1;
	}
	// render to pointer image (probably not Rend function?
	return 0;
}

int
drawbezier(Quad, double)
{
	return 0;
}

void
flushdraw(void)
{
	// draw shape
	// take care of select() data structure: draw into index fb
	// flip prematurely if taking too long since last update
	//lastflip = msec();
	SDL_RenderClear(sdl.rend);	// ?
	// might be unnecessary; we should just draw directly on screen
	// the rest is what is kept in memory
	//SDL_UpdateTexture ..
	SDL_RenderCopy(sdl.rend, sdl.tex, NULL, NULL);	// ?
	SDL_RenderPresent(sdl.rend);
}

static void
flipview(void)
{
}

void
cleardraw(void)
{
	SDL_RenderClear(sdl.rend);
}

int
resetdraw(void)
{
	// reset renderer? for now, leave as is
	// reallocate pointer sdl_surface
	// reallocate pointer array
	// do not grow beyond what is currently needed
	// less than 256 objects -> u8int
	return 0;
}

int
initdrw(void)
{
	int w, h;

	w = Vdefw;
	h = Vdefh;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		sysfatal("SDL_Init: %s\n", SDL_GetError());
	if((sdl.win = SDL_CreateWindow(
		"strpg",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		w, h,
		SDL_WINDOW_ALLOW_HIGHDPI)) == NULL)
		sysfatal("SDL_CreateWindow: %s\n", SDL_GetError());
	if((sdl.rend = SDL_CreateRenderer(
		sdl.win, -1, SDL_RENDERER_PRESENTVSYNC)) == NULL)
		sysfatal("SDL_CreateRenderer: %s\n", SDL_GetError());
	if((sdl.tex = SDL_CreateTexture(
		sdl.rend,
		/* FIXME: use CMAP8 + palette: SDL_Surface, SDL_PixelFormat */
		SDL_PIXELFORMAT_INDEX8, SDL_TEXTUREACCESS_STATIC,	// FIXME: static or streaming?
		w, h)) == NULL)
		sysfatal("SDL_CreateTexture: %s\n", SDL_GetError());
	SDL_GetWindowSize(sdl.win, &w, &h);
	view.dim.v.x = w;
	view.dim.v.y = h;
	return resetdraw();
}
