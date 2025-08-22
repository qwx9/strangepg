#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "drw.h"
#include "cmd.h"
#include "coarse.h"

/* FIXME: errors here cause shit to hang after gfa is loaded, ie no draw */
/* FIXME: move buildct here and start it automatically once nodes are loaded if
 * no ctab is scheduled for load */

static inline int
getval(char *s, ioff *ip)
{
	char *p;
	ioff i;

	i = strtoll(s, &p, 10);	/* FIXME: use hex format? */
	if(p == s || i < -1 || i >= nnodes)
		return -1;
	*ip = i;
	return 0;
}

static void
loadct(void *arg)
{
	int r;
	char *s, *path;
	File *f;
	CNode *U, *UE;

	path = arg;
	if(cnodes != nil){
		logerr(va("loadctab %s: coarsening table already initialized\n", path));
		return;
	}
	if(initcoarse() < 0){
		logerr(va("loadctab %s: %s\n", error()));
		return;
	}
	DPRINT(Debugfs, "loadctab %s", path);
	if((f = openfs(path, OREAD)) == nil){
		logerr(va("loadctab %s: %s\n", path, error()));
		free(path);	/* FIXME: are we forgetting to free this in other impls? */
		return;
	}
	for(r=0, U=cnodes, UE=U+nnodes;; U++){
		if(readline(f) == nil)
			break;
		if(U >= UE){
			werrstr("more ctab records than nodes");
			r = -1;
			break;
		}
		if((s = nextfield(f)) == nil
		|| getval(s, &U->parent) < 0){
			werrstr("invalid record");
			r = -1;
			break;
		}else if((s = nextfield(f)) == nil
		|| getval(s, &U->child) < 0){
			werrstr("invalid record");
			r = -1;
			break;
		}else if((s = nextfield(f)) == nil
		|| getval(s, &U->sibling) < 0){
			werrstr("invalid record");
			r = -1;
			break;
		}
	}
	if(U != UE){
		werrstr("too few records in ctab, %zd < %zd", U-cnodes, UE-cnodes);
		r = -1;
	}
	pushcmd("cmd(\"FHJ142\")");	/* signal needed to continue, error or no */
	flushcmd();
	if(r < 0)
		logerr(va("loadctab %s: %s, line %d\n", path, error(), f->nr));
	else
		logmsg("loadctab: done\n");
	free(f);
	free(path);
}

static Filefmt ff = {
	.name = "ctab",
	.load = loadct,
};

Filefmt *
regctab(void)
{
	return &ff;
}
