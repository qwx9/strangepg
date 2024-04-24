#include "strpg.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "ui.h"
#include "cmd.h"
#include "drw.h"
#include "threads.h"

GLFWwindow* glfw_window(void);

typedef struct Mouse Mouse;
struct Mouse{
	double x;
	double y;
	double ox;
	double oy;
	int b;
};
static Mouse ms;

char*
enterprompt(Rune, char*)
{
	return nil;
}

static void
mouseposev(GLFWwindow *, double x, double y)
{
	double Δx, Δy;

	if(ms.b != 0){
		Δx = x - ms.ox;
		Δy = y - ms.oy;
		if(mouseevent(V(x, y, 0.0f), V(Δx, Δy, 0.0f), ms.b) < 0)
			warn("mouseev\n");
		ms.ox = x;
		ms.oy = y;
	}
	ms.x = x;
	ms.x = y;
}

static void
mousebutev(GLFWwindow *w, int b, int action, int mod)
{
	int m;
	double x, y;

	switch(b){
	case GLFW_MOUSE_BUTTON_LEFT: m = Mlmb; break;
	case GLFW_MOUSE_BUTTON_MIDDLE: m = Mmmb; break;
	case GLFW_MOUSE_BUTTON_RIGHT: m = Mrmb; break;
	default: warn("mouseev: unhandled mouse button %d\n", b); return;
	}
	if(action == GLFW_RELEASE)
		ms.b = ms.b & ~m;
	else
		ms.b = ms.b | m;
	glfwGetCursorPos(w, &x, &y);
	ms.ox = x;
	ms.oy = y;
	mouseposev(w, x, y);
}

/* could be split this way as well as per usual, but not sure it has
 * any advantage here besides not dicking with modifiers */
/*
static void
ukeyev(GLFWwindow *, unsigned int u)
{
	Rune r;

	switch(u){
	case 'q': glfwSetWindowShouldClose(glfw_window(), GLFW_TRUE); return;
	case '-': r = '-'; break;
	case '+': r = '+'; break;
	case 'R': r = 'R'; break;
	case 'a': r = 'a'; break;
	default: return;
	}
	if(keyevent(r) < 0)
		warn("invalid input key %d\n", r);
}
*/

static void
keyev(GLFWwindow *, int k, int, int action, int mod)
{
	Rune r;

	if(action == GLFW_RELEASE)
		return;
	r = 0;
	switch(k){
	case GLFW_KEY_DELETE: /* wet floor */
	case GLFW_KEY_Q: glfwSetWindowShouldClose(glfw_window(), GLFW_TRUE); return;
	case GLFW_KEY_UP: r = KBup; break;
	case GLFW_KEY_DOWN: r = KBdown; break;
	case GLFW_KEY_LEFT: r = KBleft; break;
	case GLFW_KEY_RIGHT: r = KBright; break;
	case GLFW_KEY_ESCAPE: r = KBescape; break;
	case GLFW_KEY_MINUS: r = '-'; break;
	case GLFW_KEY_EQUAL: if(mod & GLFW_MOD_SHIFT) r = '+'; break;
	case GLFW_KEY_R: r = 'R'; break;
	case GLFW_KEY_A: if(mod & GLFW_MOD_SHIFT) r ='a'; break;
	default: return;
	}
	if(keyevent(r) < 0)
		warn("keyev: invalid input key %d\n", k);
}

void
initsysui(void)
{
	//glfwSetCharCallback(glfw_window(), ukeyev);
	glfwSetKeyCallback(glfw_window(), keyev);
	glfwSetMouseButtonCallback(glfw_window(), mousebutev);
	glfwSetCursorPosCallback(glfw_window(), mouseposev);
}
