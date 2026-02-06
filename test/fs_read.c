#include "strpg.h"
#include "fs.h"

int
main(int argc, char **argv)
{
	char *s;
	vlong ns, ne, nl;
	File *f;

	if(argc < 2)
		sysfatal("usage: %s GFA", argv[0]);
	else if(argc > 2)
		debug |= Debugfs;
	if((f = openfs(argv[1], OREAD)) == nil)
		sysfatal("loadgfa: %s", error());
	ns = ne = nl = 0;
	while((s = readline(f)) != nil){
		nl++;
		if(s[0] == 'S')
			ns++;
		else if(s[0] == 'L')
			ne++;
	}
	freefs(f);
	warn("%lld nodes %lld edges %lld lines\n", ns, ne, nl);
	return 0;
}
