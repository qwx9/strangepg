#include "strpg.h"

int
idput(Htab *h, usize k, usize v)
{
	int ret;
	khiter_t g;

	dprint("idput %#p k=%zd v=%zd\n", h, k, v);
	g = kh_put(id, h, k, &ret);
	if(ret < 0){
		werrstr("idmap %#p: could not insert %zd:%zd\n", h, k, v);
		return -1;
	}
	kh_value(h, g) = v;
	warn("k=%zd v=%zd kh=%zd\n", k, v, kh_value(h, g));
	return 0;
}

int
idget(Htab *h, usize k, usize *v)
{
	khiter_t g;

	assert(v != nil);
	dprint("idget %#p k=%zd\n", h, k);
	g = kh_get(id, h, k);
	if(g == kh_end(h)){
		werrstr("idmap %#p: no such key %zd\n", h, k);
		return -1;
	}
	*v = kh_value(h, g);
	return 0;
}
