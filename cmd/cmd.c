#include "strpg.h"

static char cmdbuf[1024], *bufp;


/* FIXME: command pack/unpack; good idea? varargs prob. mandatory
	problem: alignment!
void
vunpack(char *fmt, va_list a)
{
	long n;
	void *p;

	for(;;){
		switch(*fmt++){
		default: sysfatal("unknown format %c", fmt[-1]);
		case 0: return;
		case 'b': *va_arg(a, int*) = get8(outbf); break;
		case 'B': *va_arg(a, u8int*) = get8(outbf); break;
		case 'w': *va_arg(a, int*) = get16(outbf); break;
		case 'W': *va_arg(a, u16int*) = get16(outbf); break;
		case 'd': *va_arg(a, int*) = get32(outbf); break;
		case 's': *va_arg(a, int*) = (s16int)get16(outbf); break;
		case 'S': *va_arg(a, int*) = (s32int)get32(outbf); break;
		case 'n':
			p = va_arg(a, void*);
			n = va_arg(a, long);
			eread(outbf, p, n);
			break;
		}
	}
}

void
unpack(char *fmt, ...)
{
	va_list a;

	va_start(a, fmt);
	vunpack(fmt, a);
	va_end(a);
}
*/

int
flushcmd(void)	/* execute queued commands */
{
	/* FIXME: ... unpack, read buffer or sth ... */
	static int done;
	char *path = "";

	if(done > 0)
		return 0;
	loadfs(FFgfa, path);
	done++;
	return 0;
}

void
vpack(char *fmt, va_list a)
{
	USED(fmt);
	USED(a);
/*
	int c;
	long n;
	void *p;
	u32int v;
	uchar u[4];

	// FIXME: this fucking sucks; see sce code
	for(;;){
		break;
		switch(*fmt++){
		default: sysfatal("unknown format %c", fmt[-1]);
		case 0: return;
		case 'F':
			c = va_arg(a, int);
			n += sizeof *fmt + sizeof c;
			switch(c){
			case COMload:
				s = va_arg(a, char*);
				n += strlen(s) + 1;
				assert(p + n <= cmdbuf + nelem(cmdbuf));
				bufp = memmove(bufp, &c, sizeof c);
			}
			
		out8:
			ewrite(outbf, u, sizeof(u8int));
			break;
		out16:
			PBIT16(u, v);
			ewrite(outbf, u, sizeof(u16int));
			break;
		case 'b':
			u[0] = va_arg(a, int);
			goto out8;
		case 'B':
			u[0] = va_arg(a, u8int);
			goto out8;
		case 'w':
			v = va_arg(a, int);
			goto out16;
		case 'W':
			v = va_arg(a, u16int);
			goto out16;
		case 'd':
			v = va_arg(a, int);
			PBIT32(u, v);
			ewrite(outbf, u, sizeof(u32int));
			break;
		case 'n':
			p = va_arg(a, void*);
			n = va_arg(a, long);
			ewrite(outbf, p, n);
			break;
		}
	}
	*/
}
//#pragma	varargck	type	"F"	int

int
pushcmd(int cmd, char *fmt, ...)
{
	va_list a;

	USED(cmd);
	va_start(a, fmt);
	vpack(fmt, a);
	va_end(a);
	return 0;
}
