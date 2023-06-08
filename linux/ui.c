#include "strpg.h"
#include <SDL2/SDL.h>

static int	(*mousefn)(SDL_Event, Vertex);
static int	(*keyfn)(char);


static int
mousepan(Vertex Δ)
{
	return panview(Δ);
}

static int
mouseselect(Vertex p)
{
	USED(p);
	return 0;
}

static int
mousemenu(void)
{
	return 0;
}

static int
mouseprompt(char *label, char *buf, int bufsz)
{
	USED(label);
	USED(buf);
	USED(bufsz);
	return 0;
}

int
evloop(void)
{
	SDL_Event e;

	for(;;){
		while(SDL_PollEvent(&e)){
		    if(e.type == SDL_MOUSEBUTTONDOWN){
				// FIXME: pan, menu, select
		    }else if(e.type == SDL_MOUSEMOTION){
		        //mx = e.motion.x;
		        //my = e.motion.y;
		        // FIXME: ...
		    }else if(e.type == SDL_KEYDOWN){
		        switch(e.key.keysym.sym){
		        case SDLK_ESCAPE:
		        case SDLK_q:
		            SDL_Quit();
					return 0;
		        default:;
		        }
		    }else if(e.type == SDL_QUIT)
		        return 0;
		}
		// FIXME: update world if needed
		// FIXME: pan/thrdhold
	}
}

void
initui(void)
{
}
