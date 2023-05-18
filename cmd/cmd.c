#include "strpg.h"

static uchar cmdbuf[1024], *bufp = cmdbuf;

/* nil-terminated strings with 1-character command code */
int
flushcmd(void)
{
	uchar c, *p, *q;
	usize sz;
	dprint("flushcmd: %zd bytes\n", bufp - cmdbuf);

	p = cmdbuf;
	while(p < bufp){
		sz = GBIT64(p);
		p += 8;
		dprint("cmd: %lld bytes:\n", sz);
		if(debug){
			for(q=p; q<p+sz+2+1; q++){
				char c = *q;
				if(isprint(c))
					dprint("%c  ", c);
				else
					dprint("%02ux ", c);
				if((uintptr)(q - p) % 16 == 0)
					dprint("\n");
			}
			dprint("\n");
		}
		dprint("\n");
		switch(c = *p++){
		case COMload:
			dprint("flushcmd: COMload\n");
			switch(c = *p++){
			case FFgfa:
				dprint("flushcmd: FFgfa\n");
				if(loadfs(FFgfa, (char *)p) == nil)
					sysfatal("loadfs: %r");
				rendernew();
				break;
			case FFindex:
				dprint("flushcmd: FFindex\n");
				if(loadfs(FFindex, (char *)p) == nil)
					sysfatal("loadfs: %r");
				rendernew();
				break;
			default: warn("unknown file format %c\n", c);
			}
			break;
		case COMredraw:
			dprint("flushcmd: redraw\n");
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

	dprint("pushcmd %c n=%zd arg=%d\n", cmd, sz, arg1);
	if(sz + 1 > cmdbuf + nelem(cmdbuf) - bufp){
		flushcmd();
		if(sz + 1 > nelem(cmdbuf) - 1){
			werrstr("pushcmd: message too long");
			return -1;
		}
	}
	p = bufp;
	PBIT64(p, sz);	/* excluding header */
	p += 8;
	*p++ = cmd;
	*p++ = arg1;
	memmove(p, cbuf, sz);
	p += sz;
	*p++ = 0;
	bufp = p;
	return 0;
}
