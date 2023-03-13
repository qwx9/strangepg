#include "strpg.h"
#include <SDL2/SDL.h>

struct{
	SDL_Window *win;
	SDL_Renderer *rend;
	SDL_Texture *tex;
} sdl;

View
initsysdisp(void)
{
	View v = {0};

	v.w = Vdefw;
	v.h = Vdefh;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		sysfatal("SDL_Init: %s\n", SDL_GetError());
	if((sdl.win = SDL_CreateWindow(
		"strpg",
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		v.w, v.h,
		SDL_WINDOW_ALLOW_HIGHDPI)) == NULL)
		sysfatal("SDL_CreateWindow: %s\n", SDL_GetError());
	if((sdl.rend = SDL_CreateRenderer(
		sdl.win, -1, SDL_RENDERER_PRESENTVSYNC)) == NULL)
		sysfatal("SDL_CreateRenderer: %s\n", SDL_GetError());
	if((sdl.tex = SDL_CreateTexture(
            sdl.rend,
            SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
            v.w, v.h)) == NULL)
		sysfatal("SDL_CreateTexture: %s\n", SDL_GetError());
	return v;
}
