#include "strpg.h"
#define	SOKOL_GLCORE33
//#define	NDEBUG
#include "lib/sokol_app.h"
#include "lib/sokol_gfx.h"
#include "lib/sokol_log.h"
#include "lib/sokol_glue.h"
/* include nuklear.h before the sokol_nuklear.h implementation */
#define	NK_INCLUDE_FIXED_TYPES
#define	NK_INCLUDE_STANDARD_IO
#define	NK_INCLUDE_DEFAULT_ALLOCATOR
#define	NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define	NK_INCLUDE_FONT_BAKING
#define	NK_INCLUDE_DEFAULT_FONT
#define	NK_INCLUDE_STANDARD_VARARGS
//#define	NK_INCLUDE_ZERO_COMMAND_MEMORY
#include "lib/nuklear.h"
#include "lib/sokol_nuklear.h"
#include "ui.h"
#include "cmd.h"
#include "drw.h"

static int butts;
static int prompting;
static char ptext[1024];
static int plen;

enum{
	NKpopt = NK_EDIT_BOX |
		NK_EDIT_SIG_ENTER |
		NK_EDIT_GOTO_END_ON_ACTIVATE,
	NKwopt = NK_WINDOW_SCALABLE |
		NK_WINDOW_MOVABLE |
		NK_WINDOW_SCALE_LEFT |
		NK_WINDOW_MINIMIZABLE,
};

/* FIXME: prompting requires mouse action onto the prompt area */
void
resetprompt(void)
{
	ptext[0] = 0;
	plen = 0;
}
void
prompt(Rune)
{
}

/* must be called after a new frame was started and before flushing */
void
_drawui(struct nk_context *ctx)
{
	nk_flags e;

	if(selected.type != Onil)
		showobj(&selected);
	if(nk_begin(ctx, "Prompt", nk_rect(10, 10, 410, 74), NKwopt)){
		nk_layout_row_dynamic(ctx, 400, 1);
		e = nk_edit_string(ctx, NKpopt, ptext, &plen, sizeof ptext-1, nk_filter_default);
		prompting = (e & NK_EDIT_ACTIVE) != 0;
		if((e & NK_EDIT_COMMITED) != 0){
			ptext[plen] = 0;
			pushcmd("%s", ptext);
		}
	}
	nk_end(ctx);
}

void
drawui(void)
{
}

static void
mouseposev(float x, float y, float Δx, float Δy)
{
	if(butts != 0){
		if(mouseevent(V(x, y, 0.0f), V(Δx, Δy, 0.0f), butts) < 0)
			warn("mouseevent: %s\n", error());
	}
}

static void
mousebutev(float x, float y, float Δx, float Δy, int b, int down)
{
	int m;

	switch(b){
	case SAPP_MOUSEBUTTON_LEFT: m = Mlmb; break;
	case SAPP_MOUSEBUTTON_MIDDLE: m = Mmmb; break;
	case SAPP_MOUSEBUTTON_RIGHT: m = Mrmb; break;
	default: warn("mousebutev: unhandled mouse button %d\n", b); return;
	}
	if(down)
		butts = butts | m;
	else
		butts = butts & ~m;
	mouseposev(x, y, Δx, Δy);
}

static int
keyev(sapp_keycode k, uint32_t mod, int down)
{
	Rune r;

	if(prompting || !down)
		return 0;
	/* FIXME: more generic interface, make keyevent return ate or not */
	r = 0;
	switch(k){
	case SAPP_KEYCODE_UP: r = KBup; break;
	case SAPP_KEYCODE_DOWN: r = KBdown; break;
	case SAPP_KEYCODE_LEFT: r = KBleft; break;
	case SAPP_KEYCODE_RIGHT: r = KBright; break;
	case SAPP_KEYCODE_ESCAPE: r = KBescape; break;
	case SAPP_KEYCODE_L: r = 'l'; break;
	case SAPP_KEYCODE_R: r = 'r'; break;
	case SAPP_KEYCODE_Q: sapp_quit(); return 1;
	case SAPP_KEYCODE_MINUS: r = '-'; break;
	case SAPP_KEYCODE_EQUAL: if((mod & SAPP_MODIFIER_SHIFT) != 0) r = '+'; break;
	default:;
	}
	if(r != 0){
		if(keyevent(r) < 0)
			warn("keyev: invalid input key %d\n", r);
		return 1;
	}
	return 0;
}

static int inline
special(const sapp_event* ev)
{
	switch (ev->type){
	case SAPP_EVENTTYPE_MOUSE_MOVE: mouseposev(ev->mouse_x, ev->mouse_y,
		ev->mouse_dx, ev->mouse_dy); break;
	case SAPP_EVENTTYPE_MOUSE_DOWN: mousebutev(ev->mouse_x, ev->mouse_y,
		ev->mouse_dx, ev->mouse_dy, ev->mouse_button, 1); break;
	case SAPP_EVENTTYPE_MOUSE_UP: mousebutev(ev->mouse_x, ev->mouse_y,
		ev->mouse_dx, ev->mouse_dy, ev->mouse_button, 0); break;
	case SAPP_EVENTTYPE_KEY_DOWN: return keyev(ev->key_code, ev->modifiers, 1);
	case SAPP_EVENTTYPE_KEY_UP: return keyev(ev->key_code, ev->modifiers, 0);
	case SAPP_EVENTTYPE_RESIZED: reqdraw(Reqresetdraw); break;
	case SAPP_EVENTTYPE_QUIT_REQUESTED: sapp_quit(); break;
	default:;
	}
	return 0;
}

void
event(const sapp_event* ev)
{
	pollcmd();
	if(special(ev))
		return;
	snk_handle_event(ev);
}

void
initsysui(void)
{
}
