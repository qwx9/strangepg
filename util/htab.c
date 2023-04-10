#include "strpg.h"

int
idput(Htab *h, usize k, usize v)
{
	int ret;
	khiter_t g;

	g = kh_put(id, h, v, &ret);
	if(ret < 0){
		werrstr("idmap %#p: could not insert %zd:%zd\n", h, k, v);
		return -1;
	}
	kh_value(h, g) = v;
	return 0;
}

int
idget(Htab *h, usize k, usize *v)
{
	assert(v != nil);
	*v = kh_get(id, h, k);
	if(*v == kh_end(h)){
		werrstr("idmap %#p: no such key %zd\n", h, k);
		return -1;
	}
	return 0;
}
