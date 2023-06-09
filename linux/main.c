#include "strpg.h"
#include <time.h>

int
main(int argc, char **argv)
{
	srand(time(NULL));
	if(parseargs(argc, argv) < 0)
		sysfatal("usage");
	run();
	return 0;
}
