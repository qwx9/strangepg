#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "drw.h"
#include "cmd.h"

/* restrictions:
 * - NO internationalization
 * - no quoting, no escape sequences,',' is always a separator
 * - currently first column is implied to be segment labels
 */

/* FIXME: error check *object* names (valid GFA name regex + awk varname */
/* FIXME: special characters: delete or error? */
/* FIXME: handle crnl */
/* FIXME: quoting? */

static char **
csvheader(File *f, int *wait)
{
	char c, w;
	char *p, *s, **tags;

	*wait = w = 0;
	if(readline(f) == nil){
		werrstr("empty file");
		return nil;
	}
	tags = nil;
	/* FIXME: currently node-only (use cistrcmp) */
	if(nextfield(f) == nil){
		werrstr("no header");
		return nil;
	}
	for(; (s = nextfield(f)) != nil;){
		while(isspace(*s))
			s++;
		for(p=s; (c=*p)!=0; p++){
			if(isspace(c))
				continue;
			if(p == s){
				if(!isalpha(c)){
					werrstr("loadcsv: invalid tag name \"%s\" in header", s);
					return nil;
				}
			}else if(!isalnum(c)){
				*p = 0;
				warn("loadcsv: truncating tag name \"%s\" on invalid character \'%c\'\n", s, c);
				break;
			}
		}
		/* don't wait for csv to load if there are  no layout tags */
		if(cistrncmp(s, "color", 5) == 0)
			s = "CL";
		else if(strlen(s) == 2
		&& (s[0] == 'f' && (s[1] == 'x' || s[1] == 'y' || s[1] == 'z')
		|| s[1] == '0' && (s[0] == 'x' || s[0] == 'y' || s[0] == 'z')))
			w++;
		p = estrdup(s);
		dypush(tags, p);
	}
	if(w)
		*wait = 1;
	return tags;
}

/* FIXME:
 * - delete any \r
 * - reject "" in unquoted fields
 * - internationalization: handle quotes, commas for decimals?
 */

/* FIXME: lilu dallas mooltigraph?? */
static void
loadcsv(void *arg)
{
	int r, nf, nr, wait;
	char *s, *path, *tag, **tags, name[512];
	File *f;

	path = arg;
	DPRINT(Debugfs, "loadcsv %s", path);
	if((f = openfs(path, OREAD)) == nil){
		warn("loadcsv %s: %s\n", path, error());
		free(path);
		return;
	}
	r = -1;
	nr = 0;
	splitfs(f, ',');
	if((tags = csvheader(f, &wait)) == nil)
		goto end;
	if(gottagofast)
		wait = 0;
	if(!wait && (debug & Debugload) == 0){
		pushcmd("cmd(\"FHJ142\")");
		flushcmd();
	}
	nr++;
	name[sizeof name-1] = 0;
	/* beyond the tag names in the header, we leave all input validation to strawk */
	while(readline(f) != nil){
		*name = 0;
		for(nf=0; (s = nextfield(f)) != nil; nf++){
			if(nf == 0){
				strecpy(name, name+sizeof name, s);
				continue;
			}else if(nf-1 >= dylen(tags)){
				warn("readcsv: line %d: ignoring extra columns\n", nr);
				break;
			}
			tag = tags[nf-1];
			setnamedtag(tag, name, s);
		}
		nr++;
	}
	r = 0;
end:
	if(r < 0)
		logerr(va("loadcsv %s: %s\n", path, error()));
	else
		logmsg(va("loadcsv: done, read %d records\n", nr));
	if(debug & Debugload)
		pushcmd("cmd(\"FJJ142\")");
	else if(wait)
		pushcmd("cmd(\"FHJ142\")");
	else
		pushcmd("loadbatch()");
	flushcmd();
	for(nf=0; nf<dylen(tags); nf++)
		free(tags[nf]);
	dyfree(tags);
	USED(tags);
	freefs(f);
	free(path);
}

static Filefmt ff = {
	.name = "csv",
	.load = loadcsv,
};

Filefmt *
regcsv(void)
{
	return &ff;
}
