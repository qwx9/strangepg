#include "strpg.h"
#include "cmd.h"

int epfd[2] = {-1, -1};

// FIXME: client-side; batching; should be by graph as well

void
parseresponse(char *s)
{
	int n;
	char *fld[16];

	if((n = getfields(s, fld, nelem(fld), 1, " \t")) < 1)
		sysfatal("getfields: short read: %s", error());
	USED(n);
}

void
pushcmd(char *fmt, ...)
{
	ssize ai;
	char c, *f, sb[1024], *sp, *as;
	va_list arg;

	if(epfd[1] < 0){
		warn("pushcmd: pipe closed\n");
		return;
	}
	va_start(arg, fmt);
	for(f=fmt, sp=sb; sp<sb+sizeof sb;){
		if((c = *f++) == 0){
			*sp = 0;
			break;
		}else if(c != '%'){
			*sp++ = c;
			continue;
		}
		if((c = *f++) == 0){
			warn("pushcmd: malformed command %s", fmt);
			*sp = 0;
			break;
		}
		switch(c){
		case 's':
			as = va_arg(arg, char*);
			sp = strecpy(sp, sb+sizeof sb, as);
			break;
		case 'd':
			ai = va_arg(arg, ssize);
			sp = seprint(sp, sb+sizeof sb, "%zd", ai);
			break;
		default:
			warn("pushcmd: unknown format %c", c);
			*sp++ = c;
			break;
		}
	}
	va_end(arg);
	sendcmd(sb);
}
char *fns = " \
function addnode(id, label){\n \
	node[label] = id\n \
	lnode[id] = label\n \
	print \"n\", id, label\n \
}\n \
function addnnode(id, label){\n \
	node[label] = id\n \
	lnode[id] = label\n \
	print \"N\", id, label\n \
}\n \
function addnedge(id, u, v, d1, d2){\n \
	edge[id] = node[u] \"\x1c\" node[v]\n \
	dir[id,1] = d1\n \
	dir[id,2] = d2\n \
	print \"E\", id\n \
}\n \
function addedge(id, u, v, d1, d2){\n \
	edge[id] = u \"\x1c\" v\n \
	dir[id,1] = d1\n \
	dir[id,2] = d2\n \
	print \"e\", id\n \
}\n \
function expandnode(id){ return \"expandnode(\" id \")\" }\n \
function retractnode(id){ return \"retractnode(\" id \")\" }\n \
function select(id){ return \"select(\" id \")\" }\n \
function hidenode(id){ return \"hidenode(\" id \")\" }\n \
function hideedge(id){ return \"hideedge(\" id \")\" }\n \
function shownode(id){ return \"shownode(\" id \")\" }\n \
function showedge(id){ return \"showedge(\" id \")\" }\n \
function colornode(id, c){ return \"colornode(\" id \",\" c \")\" }\n \
function coloredge(id, c){ return \"coloredge(\" id \",\" c \")\" }\n \
{\n \
	if($1 == \"n\")\n \
		addnode($2, $3)\n \
	else if($1 == \"N\")\n \
		addnnode($2, $3)\n \
	else if($1 == \"e\")\n \
		addedge($2, $3, $4, $5, $6)\n \
	else if($1 == \"E\")\n \
		addnedge($2, $3, $4, $5, $6)\n \
	else\n \
		print \"nope\"\n \
}\n \
";

int
initcmd(void)
{
	int n, fd;

	if(pipe(epfd) < 0)
		return -1;
	// FIXME: temp file
	if((fd = create("/tmp/lib.awk", OWRITE, 0644)) < 0)
		return -1;
	n = strlen(fns);
	if(write(fd, fns, n) != n)
		return -1;
	close(fd);
	if(sysinitcmd() < 0)
		return -1;
	close(epfd[0]);
	write(epfd[1], fns, strlen(fns));
	return 0;
}
