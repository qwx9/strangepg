#include "strpg.h"
#define	SOKOL_GLCORE
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
#define	NK_IMPLEMENTATION
#include "lib/nuklear.h"
#define	SOKOL_NUKLEAR_IMPL
#include "lib/sokol_nuklear.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"

typedef struct nk_context nk_context;
typedef struct nk_color nk_color;
typedef struct nk_text_edit nk_text_edit;

static nk_text_edit nkprompt;
static nk_color nktheme[NK_COLOR_COUNT];
static int prompting;
static char ptext[1024];
static int plen;

enum{
	NKpopt =
		NK_EDIT_BOX |
		NK_EDIT_SIG_ENTER |
		NK_EDIT_GOTO_END_ON_ACTIVATE |
		NK_EDIT_CTRL_ENTER_NEWLINE,
	NKwopt =
		NK_WINDOW_SCALABLE |
		NK_WINDOW_MOVABLE |
		NK_WINDOW_BORDER |
		NK_WINDOW_MINIMIZABLE |
		NK_WINDOW_SCROLL_AUTO_HIDE,
};

/* FIXME: clear text on escape while prompt active */
/* FIXME: implement wrap or sth (see old nuklear repo issues) */
void
resetprompt(void)
{
	memset(ptext, 0, plen);
	plen = 0;
	nk_textedit_clear_state(&nkprompt, NK_TEXT_EDIT_MULTI_LINE, nk_filter_default);
	nk_str_clear(&nkprompt.string);
}
void
prompt(Rune)
{
}

/* must be called after a new frame was started and before flushing */
void
_drawui(nk_context *ctx)
{
	float h;
	nk_flags e;

	drawselected();
	if(nk_begin(ctx, "Prompt", nk_rect(0, 0, view.w, 78), NKwopt)){
		/* FIXME: no wrap */
		nk_layout_row_dynamic(ctx, 0, 1);
		e = nk_edit_buffer(ctx, NKpopt, &nkprompt, nk_filter_default);
		prompting = (e & NK_EDIT_ACTIVE) != 0;
		if((e & NK_EDIT_COMMITED) != 0){
			plen = nk_str_len_char(&nkprompt.string);
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
	if(mouseevent(V(x, y, 0.0f), V(Δx, Δy, 0.0f)) < 0)
		warn("mouseevent: %s\n", error());
}

static void
mousebutev(float x, float y, float Δx, float Δy, int b, int down)
{
	Rune r;

	r = 0;
	switch(b){
	case SAPP_MOUSEBUTTON_LEFT: r = Klmb; break;
	case SAPP_MOUSEBUTTON_MIDDLE: r = Kmmb; break;
	case SAPP_MOUSEBUTTON_RIGHT: r = Krmb; break;
	/* sokol_app doesn't distinguish between up/down or horizontal/vertical
	 * mouse scrolling */
	case -1:
		if(Δy != 0)
			r = Δy < 0 ? Kscrlup : Kscrldn;
		else if(Δx != 0)
			r = Δx < 0 ? Kscrlup : Kscrldn;
		break;
	default: warn("mousebutev: unhandled mouse button %d\n", b); return;
	}
	keyevent(r, down);
	mouseposev(x, y, Δx, Δy);
}

static int
keyev(sapp_keycode k, uint32_t mod, int down)
{
	Rune r;

	if(prompting)
		return 0;
	r = 0;
	switch(k){
	case SAPP_KEYCODE_UP: r = Kup; break;
	case SAPP_KEYCODE_DOWN: r = Kdown; break;
	case SAPP_KEYCODE_LEFT: r = Kleft; break;
	case SAPP_KEYCODE_RIGHT: r = Kright; break;
	case SAPP_KEYCODE_ESCAPE: r = Kesc; break;	/* WHY */
	case SAPP_KEYCODE_KP_ENTER: /* wet floor */
	case SAPP_KEYCODE_ENTER: r = '\n'; break;	/* WHY */
	case SAPP_KEYCODE_LEFT_SHIFT: /* wet floor */
	case SAPP_KEYCODE_RIGHT_SHIFT: r = Kshift; break;
	case SAPP_KEYCODE_LEFT_CONTROL: /* wet floor */
	case SAPP_KEYCODE_RIGHT_CONTROL: r = Kctl; break;
	case SAPP_KEYCODE_LEFT_ALT: /* wet floor */
	case SAPP_KEYCODE_RIGHT_ALT: r = Kalt; break;
	case SAPP_KEYCODE_Q: sapp_quit(); return 1;
	case SAPP_KEYCODE_EQUAL: if((mod & SAPP_MODIFIER_SHIFT) != 0) r = '+'; break;
	default:
		if(k >= 'A' && k <= 'Z')
			k = tolower(k);
		r = k;
		break;
	}
	if(r != 0){
		if(keyevent(r, down) < 0)
			warn("keyev: invalid input key %d\n", r);
		return 1;
	}
	return 0;
}

inline static int
special(const sapp_event* ev)
{
	switch (ev->type){
	case SAPP_EVENTTYPE_MOUSE_MOVE: mouseposev(ev->mouse_x, ev->mouse_y,
		ev->mouse_dx, ev->mouse_dy); break;
	case SAPP_EVENTTYPE_MOUSE_DOWN: mousebutev(ev->mouse_x, ev->mouse_y,
		ev->mouse_dx, ev->mouse_dy, ev->mouse_button, 1); break;
	case SAPP_EVENTTYPE_MOUSE_UP: mousebutev(ev->mouse_x, ev->mouse_y,
		ev->mouse_dx, ev->mouse_dy, ev->mouse_button, 0); break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL: mousebutev(ev->mouse_x, ev->mouse_y,
		ev->scroll_x, ev->scroll_y, -1, 1); break;
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
	if(special(ev))
		return;
	snk_handle_event(ev);
}

void
initnk(void)
{
	nk_context *ctx;

	snk_setup(&(snk_desc_t){
		.dpi_scale = sapp_dpi_scale(),
		.logger.func = slog_func,
	});
	ctx = snk_get_context();
	memcpy(nktheme, nk_default_color_style, sizeof nk_default_color_style);
	/* FIXME: inconsistent with draw/color.c */
	if((view.flags & VFhaxx0rz) == 0){
		nktheme[NK_COLOR_TEXT] = (nk_color){0x00, 0x00, 0x00, 0xcc};
		nktheme[NK_COLOR_WINDOW] = (nk_color){0xaa, 0xaa, 0xaa, 0xcc};
		nktheme[NK_COLOR_HEADER] = (nk_color){0x99, 0x99, 0x99, 0xcc};
		nktheme[NK_COLOR_BORDER] = nktheme[NK_COLOR_HEADER];
		nktheme[NK_COLOR_SELECT] = (nk_color){0xcc, 0xcc, 0xcc, 0xcc};
		nktheme[NK_COLOR_SELECT_ACTIVE] = (nk_color){0x66, 0x66, 0x66, 0xcc};
		nktheme[NK_COLOR_EDIT] = (nk_color){0xff, 0xff, 0xff, 0xff};
		nktheme[NK_COLOR_EDIT_CURSOR] = nktheme[NK_COLOR_TEXT];
		nktheme[NK_COLOR_SCROLLBAR] = nktheme[NK_COLOR_WINDOW];
		nktheme[NK_COLOR_SCROLLBAR_CURSOR] = (nk_color){0xcc, 0xcc, 0x8e, 0xcc};
		nktheme[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = (nk_color){0xee, 0xee, 0x9e, 0xcc};
		nktheme[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = (nk_color){0xee, 0xee, 0x9e, 0xcc};
		nktheme[NK_COLOR_TAB_HEADER] = nktheme[NK_COLOR_WINDOW];
		nk_style_from_table(ctx, nktheme);
	}
	nk_textedit_init_fixed(&nkprompt, ptext, sizeof ptext-1);
}

void
initsysui(void)
{
}
