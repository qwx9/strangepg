#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "lib/sokol_app.h"
#include "lib/sokol_gfx.h"
#include "lib/sokol_log.h"
#include "lib/sokol_glue.h"
#define	NK_INCLUDE_FIXED_TYPES
#define	NK_INCLUDE_STANDARD_IO
#define	NK_INCLUDE_DEFAULT_ALLOCATOR
#define	NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define	NK_INCLUDE_FONT_BAKING
#define	NK_INCLUDE_DEFAULT_FONT
#define	NK_INCLUDE_STANDARD_VARARGS
#include "lib/nuklear.h"
#include "lib/sokol_nuklear.h"
#include "sokol.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"

typedef struct nk_context nk_context;
typedef struct nk_color nk_color;
typedef struct nk_text_edit nk_text_edit;

enum{
	Pbox = 1<<0,
	Pnodesz = 1<<1,
	Pnodew = 1<<2,
	Pminsz = 1<<3,
	Pmaxsz = 1<<4,
	Pfedge = 1<<5,
};
static nk_text_edit nkprompt;
static int prompting;
static char ptext[8192];
static int plen;
enum{
	NKOnodesz,
	NKOnodew,
	NKOmaxsz,
	NKOminsz,
	NKOfedge,
	NKOend,
};
static char nkopt[NKOend][64];
static int nkoptn[NKOend];

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
	NKxopt =
		NK_EDIT_SELECTABLE |
		NK_EDIT_MULTILINE |
		NK_EDIT_ALLOW_TAB |
		NK_EDIT_CLIPBOARD |
		NK_EDIT_READ_ONLY |
		NK_EDIT_GOTO_END_ON_ACTIVATE,
	NKfopt =
		NK_EDIT_SIMPLE |
		NK_EDIT_SELECTABLE |
		NK_EDIT_SIG_ENTER,
	Fonth = 13,
	Padh = 3,
	Colh = Fonth + Padh,
};

/* FIXME: clear text on escape while prompt active */
void
resetprompt(void)
{
	memset(ptext, 0, plen);
	plen = 0;
	nk_str_clear(&nkprompt.string);
}
void
prompt(Rune)
{
}

static void
pasteprompt(const char *s)
{
	if(s == nil)
		return;
	plen = nk_str_len_char(&nkprompt.string);
	s = strecpy(ptext+plen, ptext+sizeof ptext, s);
	plen = s - ptext;
}

static inline int
validfloat(double *fp, double min, double max, char *s)
{
	double f;
	char *p;

	f = strtod(s, &p);
	*fp = f;
	return *p == 0 && f >= min && f <= max;
}

static int
drawoptions(nk_context *ctx)
{
	double f;
	nk_flags e;

	if(!nk_tree_push(ctx, NK_TREE_TAB, "Drawing", NK_MINIMIZED))
		return 0;
	nk_layout_row_dynamic(ctx, 2 * Fonth, 2);
	/* FIXME: sliders? */
	nk_label(ctx, "Node length (0.01-50):", NK_TEXT_LEFT);
	if((e = nk_edit_string(ctx, NKfopt, nkopt[NKOnodesz],
	&nkoptn[NKOnodesz], sizeof nkopt[NKOnodesz], nk_filter_default))){
		if(e & NK_EDIT_COMMITED){
			prompting &= ~Pnodesz;
			nkopt[NKOnodesz][nkoptn[NKOnodesz]] = 0;
			if(validfloat(&f, 0.01, 50.0, nkopt[NKOnodesz])){
				nk_edit_unfocus(ctx);
				drawing.nodesz = f;
				drawing.wflags |= DFstalelen;
				reqdraw(Reqflags|Reqshape);
				reqlayout(Lreinit);
			}else
				logerr("invalid node length\n");
		}else if(e & NK_EDIT_ACTIVE)
			prompting |= Pnodesz;
		else
			prompting &= ~Pnodesz;
	}else
		prompting &= ~Pnodesz;
	nk_label(ctx, "Node width (0.01-50):", NK_TEXT_LEFT);
	if((e = nk_edit_string(ctx, NKfopt, nkopt[NKOnodew],
	&nkoptn[NKOnodew], sizeof nkopt[NKOnodew], nk_filter_default))){
		if((e & NK_EDIT_COMMITED) != 0){
			prompting &= ~Pnodew;
			nkopt[NKOnodew][nkoptn[NKOnodew]] = 0;
			if(validfloat(&f, 0.01, 50.0, nkopt[NKOnodew])){
				nk_edit_unfocus(ctx);
				drawing.fatness = f;
				reqdraw(Reqshape);
			}else
				logerr("invalid node width\n");
		}else if(e & NK_EDIT_ACTIVE)
			prompting |= Pnodew;
		else
			prompting &= ~Pnodew;
	}else
		prompting &= ~Pnodew;
	nk_label(ctx, "Min.node length (0.01-100):", NK_TEXT_LEFT);
	if((e = nk_edit_string(ctx, NKfopt, nkopt[NKOminsz],
	&nkoptn[NKOminsz], sizeof nkopt[NKOminsz], nk_filter_default))){
		if((e & NK_EDIT_COMMITED) != 0){
			prompting &= ~Pminsz;
			nkopt[NKOminsz][nkoptn[NKOminsz]] = 0;
			if(!validfloat(&f, 0.01, 100.0, nkopt[NKOminsz]))
				logerr("invalid min length\n");
			else if(f > drawing.maxsz)
				logerr("must be lesser or equal to max length\n");
			else if(f != drawing.minsz){
				nk_edit_unfocus(ctx);
				drawing.minsz = f;
				drawing.wflags |= DFstalelen;
				reqdraw(Reqflags);
				reqlayout(Lreinit);
			}
		}else if(e & NK_EDIT_ACTIVE)
			prompting |= Pminsz;
		else
			prompting &= ~Pminsz;
	}else
		prompting &= ~Pminsz;
	nk_label(ctx, "Max.node length (0.01-100.0):", NK_TEXT_LEFT);
	if((e = nk_edit_string(ctx, NKfopt, nkopt[NKOmaxsz],
	&nkoptn[NKOmaxsz], sizeof nkopt[NKOmaxsz], nk_filter_default))){
		if((e & NK_EDIT_COMMITED) != 0){
			prompting &= ~Pmaxsz;
			nkopt[NKOmaxsz][nkoptn[NKOmaxsz]] = 0;
			if(!validfloat(&f, 0.01, 100.0, nkopt[NKOmaxsz]))
				logerr("invalid max length\n");
			else if(f < drawing.minsz)
				logerr("must be greater or equal to min length\n");
			else if(f != drawing.maxsz){
				nk_edit_unfocus(ctx);
				drawing.maxsz = f;
				drawing.wflags |= DFstalelen;
				reqdraw(Reqflags);
				reqlayout(Lreinit);
			}
		}else if(e & NK_EDIT_ACTIVE)
			prompting |= Pmaxsz;
		else
			prompting &= ~Pmaxsz;
	}else
		prompting &= ~Pmaxsz;
	nk_label(ctx, "Edge length factor (1-5000):", NK_TEXT_LEFT);
	if((e = nk_edit_string(ctx, NKfopt, nkopt[NKOfedge],
	&nkoptn[NKOfedge], sizeof nkopt[NKOfedge], nk_filter_default))){
		if((e & NK_EDIT_COMMITED) != 0){
			prompting &= ~Pfedge;
			nkopt[NKOfedge][nkoptn[NKOfedge]] = 0;
			if(!validfloat(&f, 1.0, 5000.0, nkopt[NKOfedge]))
				logerr("invalid edge length factor\n");
			else if(f != drawing.fedge){
				nk_edit_unfocus(ctx);
				drawing.fedge = f;
				drawing.wflags |= DFstalelen;
				reqdraw(Reqflags);
				reqlayout(Lreinit);
			}
		}else if(e & NK_EDIT_ACTIVE)
			prompting |= Pfedge;
		else
			prompting &= ~Pfedge;
	}else
		prompting &= ~Pfedge;
	nk_tree_pop(ctx);
	return 1;
}

/* must be called after a new frame was started and before flushing */
void
drawui(nk_context *ctx)
{
	int i, sz;
	float h;
	nk_flags e;
	struct nk_rect r;
	struct nk_panel *p;
	struct nk_style_edit *s;

	if(nk_begin(ctx, "Console", nk_rect(8, 8, sapp_width() / 4.5, 16*Colh), NKwopt)){
		s = &ctx->style.edit;
		r = nk_window_get_bounds(ctx);
		nk_layout_row_dynamic(ctx, 8, 1);
		nk_label(ctx, selstr[0] == 0 ? "" : selstr, NK_TEXT_LEFT);
		nk_label(ctx, hoverstr[0] == 0 ? "" : hoverstr, NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 3 * Fonth, 1);
		if(nk_group_begin(ctx, "last", NK_WINDOW_NO_SCROLLBAR)){
			nk_layout_row_dynamic(ctx, 8, 1);
			for(i=0; i<3; i++){
				if(!iserrmsg[i])
					nk_label(ctx, lastmsg[i][0] != 0 ? lastmsg[i] : "", NK_TEXT_LEFT);
				else
					nk_label_colored(ctx, lastmsg[i], NK_TEXT_LEFT, nk_rgb(160,0,0));
			}
			nk_group_end(ctx);
		}
		h = MAX(r.h - (12*Colh - Padh) - s->padding.y - s->border, 24);
		if(nk_tree_push(ctx, NK_TREE_TAB, "Log", NK_MINIMIZED)){
			nk_layout_row_dynamic(ctx, 6 * Colh, 1);
			/* nk_text and nk_label do not handle newlines */
			if(nk_group_begin(ctx, "all", 0)){
				sz = logsz;
				nk_layout_row_dynamic(ctx, 10 + nlog * (Fonth+2), 1);
				nk_edit_string(ctx, NKxopt, (char *)logbuf, &sz, logsz, nk_filter_default);
				nk_group_end(ctx);
			}
			nk_tree_pop(ctx);
		}
		drawoptions(ctx);
		if(nk_tree_push(ctx, NK_TREE_TAB, "Prompt", NK_MAXIMIZED)){
			nk_layout_row_dynamic(ctx, h, 1);
			e = nk_edit_buffer(ctx, NKpopt, &nkprompt, nk_filter_default);
			if((e & NK_EDIT_COMMITED) != 0){
				prompting &= ~Pbox;
				plen = nk_str_len_char(&nkprompt.string);
				ptext[plen] = 0;
				pushcmd("%s", ptext);
				flushcmd();
				nk_edit_unfocus(ctx);
			}else if(e & NK_EDIT_ACTIVE)
				prompting |= Pbox;
			else
				prompting &= ~Pbox;
			nk_tree_pop(ctx);
		}else
			prompting &= ~Pbox;
		promptbox = (Box){r.x, r.y, r.x + r.w, r.y + r.h};
	}else{	/* minimized */
		if((p = nk_window_get_panel(ctx)) != nil){
			r = nk_window_get_bounds(ctx);
			promptbox = (Box){r.x, r.y, r.x + r.w, r.y + p->header_height};
		}
	}
	nk_end(ctx);
}

static void
mouseposev(float x, float y, float Δx, float Δy)
{
	if(mouseevent(x, y, Δx, Δy) < 0)
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
	case SAPP_EVENTTYPE_CLIPBOARD_PASTED: pasteprompt(sapp_get_clipboard_string()); break;
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

	if((drawing.flags & DFhaxx0rz) == 0){
		nk_default_color_style[NK_COLOR_TEXT] = (nk_color){0x00, 0x00, 0x00, 0xcc};
		nk_default_color_style[NK_COLOR_WINDOW] = (nk_color){0xf7, 0xf7, 0xf7, 0xcc};
		nk_default_color_style[NK_COLOR_HEADER] = (nk_color){0xe0, 0xe0, 0xe0, 0xcc};
		nk_default_color_style[NK_COLOR_TAB_HEADER] = nk_default_color_style[NK_COLOR_HEADER];
		nk_default_color_style[NK_COLOR_BORDER] = nk_default_color_style[NK_COLOR_HEADER];
		nk_default_color_style[NK_COLOR_COMBO] = nk_default_color_style[NK_COLOR_HEADER];
		nk_default_color_style[NK_COLOR_EDIT] = (nk_color){0xff, 0xff, 0xff, 0xff};
		nk_default_color_style[NK_COLOR_EDIT_CURSOR] = (nk_color){0x00, 0x00, 0x00, 0xcc};
		nk_default_color_style[NK_COLOR_SCROLLBAR] = nk_default_color_style[NK_COLOR_HEADER];
		nk_default_color_style[NK_COLOR_SCROLLBAR_CURSOR] = (nk_color){0xd0, 0xd0, 0xd0, 0xcc};
		nk_default_color_style[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = (nk_color){0xc0, 0xc0, 0xb0, 0xcc};
		nk_default_color_style[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = (nk_color){0xb0, 0xb0, 0xb0, 0xcc};
	}
	snk_setup(&(snk_desc_t){
		.dpi_scale = sapp_dpi_scale(),
		.logger.func = slog_func,
		.enable_set_mouse_cursor = true,
	});
	ctx = snk_get_context();
	nk_style_hide_cursor(ctx);
	nk_textedit_init_fixed(&nkprompt, ptext, sizeof ptext-1);
	nkoptn[NKOnodesz] = snprint(nkopt[NKOnodesz], sizeof nkopt[NKOnodesz], "%.2f", drawing.nodesz);
	nkoptn[NKOnodew] = snprint(nkopt[NKOnodew], sizeof nkopt[NKOnodew], "%.2f", drawing.fatness);
	nkoptn[NKOminsz] = snprint(nkopt[NKOminsz], sizeof nkopt[NKOminsz], "%.2f", drawing.minsz);
	nkoptn[NKOmaxsz] = snprint(nkopt[NKOmaxsz], sizeof nkopt[NKOmaxsz], "%.2f", drawing.maxsz);
	nkoptn[NKOfedge] = snprint(nkopt[NKOfedge], sizeof nkopt[NKOfedge], "%.2f", drawing.fedge);
}

void
initsysui(void)
{
}
