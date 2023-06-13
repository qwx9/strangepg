#include "strpg.h"
#include <pthread.h>
#include <time.h>
#include "sokol_gfx.h"
#include "sokol_log.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

void
evloop(void)
{
	struct timespec t;

	for(;;){
		t.tv_sec = 0;
		t.tv_nsec = 1000000;
		nanosleep(&t, NULL);
	}
}

void
initui(void)
{
}
