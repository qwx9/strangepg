/* based on /sys/src/libc/port/getfields.c */
#include "strpg.h"

int
getfields(char *str, char **args, int max, int mflag, char *set)
{
	char r;
	int nr, intok, narg;

	if(max <= 0)
		return 0;
	narg = 0;
	args[narg] = str;
	if(!mflag)
		narg++;
	intok = 0;
	for(;; str+=nr){
		nr = 1;
		r = *str;
		if(r == 0)
			break;
		if(strchr(set, r) != nil){
			if(narg >= max)
				break;
			*str = 0;
			intok = 0;
			args[narg] = str + nr;
			if(!mflag)
				narg++;
		}else{
			if(!intok && mflag)
				narg++;
			intok = 1;
		}
	}
	return narg;
}
