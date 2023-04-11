#include "strpg.h"

static int
loadgfacmd(uchar *arg, usize sz)
{
	assert(arg != nil);
	USED(sz);
	return loadfs(FFgfa, (char *)arg) == nil;
}

static uchar cmdbuf[1024], *bufp;

/* nil-terminated strings with 1-character command code */
int
flushcmd(void)
{
	uchar c, *p;
	usize sz;
	int (*fn)(uchar*, usize);

	p = cmdbuf;
	while(p < bufp){
		sz = GBIT64(p);
		p += 8;
		switch(c = *p++){
		case COMload:
			switch(c = *p++){
			case FFgfa: fn = loadgfacmd; break;
			default: fn = nil; warn("unknown file format %c\n", c);
			}
			break;
		default: fn = nil; warn("unknown command %c\n", c);
		}
		assert(p + sz < cmdbuf + nelem(cmdbuf) && p[sz] == 0);
		if(fn != nil && fn(p, sz) < 0)
			return -1;
		p += strlen((char *)p);
		assert(p < cmdbuf + nelem(cmdbuf));
	}
	bufp = cmdbuf;
	memset(cmdbuf, 0, nelem(cmdbuf));
	return 0;
}

/* [len][cmd][arg1][...] */
int
pushcmd(int cmd, usize sz, int arg1, uchar *cbuf)
{
	uchar *p;

	sz++;
	if(sz > cmdbuf + nelem(cmdbuf) - bufp){
		flushcmd();
		if(sz > nelem(cmdbuf) - 1){
			werrstr("pushcmd: message too long");
			return -1;
		}
	}
	p = bufp;
	PBIT64(p, sz);	/* excluding header */
	p += 8;
	*p++ = cmd;
	*p++ = arg1;
	bufp = memmove(p, cbuf, sz);
	return 0;
}
