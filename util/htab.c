#include "strpg.h"

/* the original api is imho horrible and conducive to errors; while it
 * would be hard or impossible to improve upon it given the limitations
 * of c99, we don't need it to be fully generic and can try to have a
 * more familiar and simpler interface. even when inlining these, we
 * may be impacting the ht's performance on its own, but it would
 * hardly be a bottleneck globally unless something has gone very wrong.
 */

void
iddump(Htab *h)
{
	khiter_t g;
	char *kk;
	usize vv;

	kh_foreach(h, kk, vv, {warn("%s:%zd\n", kk, vv);});
}

void
idnuke(Htab *h)
{
	char *k;
	usize v;

	kh_foreach(h, k, v, {(v); free(k);});
	kh_destroy(id, h);
}

int
idput(Htab *h, char *k, usize v)
{
	int ret;
	khiter_t g;

	g = kh_put(id, h, k, &ret);
	if(ret < 0){
		werrstr("idmap %#p: could not insert %s:%zd\n", h, k, v);
		return -1;
	}
	kh_value(h, g) = v;
	return 0;
}

int
idget(Htab *h, char *k, usize *v)
{
	khiter_t g;

	assert(v != nil);
	g = kh_get(id, h, k);
	if(g == kh_end(h)){
		werrstr("idmap %#p: no such key %s\n", h, k);
		return -1;
	}
	*v = kh_value(h, g);
	return 0;
}
