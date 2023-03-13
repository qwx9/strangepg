#include "strpg.h"

// FIXME: utf8 handling

static int	(*mousefn)(Event, Vertex);
static int	(*keyfn)(char);


// FIXME: these look generic to me
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
mousemenu(Event)	/* just use Event instead of Mouse */
{
	return 0;
}

int
mouseprompt(char *label, char *buf, int bufsz)
{
	USED(label);
	USED(buf);
	USED(bufsz);
	return 0;
}

static int
defaultmouse(Event e, Vertex Δ)
{
	USED(e);
	USED(Δ);
	return 0;
}

static int
defaultkey(int)
{
	return 0;
}

Event
waitevent(void)
{
	Event e = {0};

	return e;
}

void
initsysui(void)
{
}
