#include "strpg.h"

/* the original api is imho horrible and conducive to errors; while it
 * would be hard or impossible to improve upon it given the limitations
 * of c99, we don't need it to be fully generic and can try to have a
 * more familiar and simpler interface. even when inlining these, we
 * may be impacting the ht's performance on its own, but it would
 * hardly be a bottleneck globally unless something has gone very wrong.
 */

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
