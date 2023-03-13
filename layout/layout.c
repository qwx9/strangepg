#include "strpg.h"
#include "layoutprv.h"

Layer ZL;
static Layout *ll[LLnil];

Layer
dolayout(Graph *g, int type)
{
	Layout *l;

	if(type < 0 || type >= LLnil){
		werrstr("invalid layout");
		return ZL;
	}
	l = ll[type];
	assert(l != nil);
	if(l->compute == nil){
		werrstr("unimplemented fs type");
		return ZL;
	}
	return l->compute(g);
}

void
initlayout(void)
{
	ll[LLconga] = regconga();
}
