#include "strpg.h"

static uchar cmdbuf[1024], *bufp;

/* nil-terminated strings with 1-character command code */
int
flushcmd(void)
{
	uchar c, *p;
	usize sz;

	p = cmdbuf;
	while(p < bufp){
		sz = GBIT64(p);
		p += 8;
		switch(c = *p++){
		case COMload:
			switch(c = *p++){
			case FFgfa:
				if(loadfs(FFgfa, (char *)p) == nil)
					return -1;
				break;
			default: warn("unknown file format %c\n", c);
			}
			break;
		case COMredraw:
			redraw();
			break;
		default: warn("unknown command %c\n", c);
		}
		p += sz;
		assert(p < cmdbuf + nelem(cmdbuf) && *p == 0);
		p++;
	}
	bufp = cmdbuf;
	memset(cmdbuf, 0, sizeof cmdbuf);
	return 0;
}

// FIXME: get rid of this arg1 shit
/* [len][cmd][arg1][...] */
int
pushcmd(int cmd, usize sz, int arg1, uchar *cbuf)
{
	uchar *p;

	if(sz + 1 > cmdbuf + nelem(cmdbuf) - bufp){
		flushcmd();
		if(sz + 1 > nelem(cmdbuf) - 1){
			werrstr("pushcmd: message too long");
			return -1;
		}
	}
	warn("pushcmd %c n=%zd arg=%d\n", cmd, sz, arg1);
	p = bufp;
	PBIT64(p, sz);	/* excluding header */
	p += 8;
	*p++ = cmd;
	*p++ = arg1;
	p = memmove(p, cbuf, sz);
	*p++ = 0;
	bufp = p;
	return 0;
}
