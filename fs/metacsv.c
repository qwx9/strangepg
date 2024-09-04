#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "cmd.h"

/* restrictions:
 * - NO internationalization
 * - no quoting, no escape sequences,',' is always a separator
 * - column names are valid variable names, ie. cannot have spaces
 * - currently first column is implied to be segment labels
 */

/* FIXME: error check *object* names (valid GFA name regex + awk varname */
/* FIXME: special characters: delete or error? */
/* FIXME: handle crnl */
/* FIXME: quoting? */

static char **
csvheader(File *f)
{
	char c;
	int nf;
	char *p, *s, *t, **tags;

	if((s = readline(f, nil)) == nil){
		werrstr("empty file");
		return nil;
	}
	tags = nil;
	/* FIXME: currently node-only (use cistrcmp) */
	s = nextfield(f, s, nil, ',');
	for(nf=0; s!=nil; s=t, nf++){
		t = nextfield(f, s, nil, ',');
		for(p=s; (c=*p)!=0; p++){
			if(p == s){
				if(!isalpha(c)){
					werrstr("loadcsv: invalid tag name \"%s\" in header", s);
					return nil;
				}
			}else if(!isalnum(c)){
				*p = 0;
				warn("loadcsv: truncating tag name \"%s\" on invalid character %c\n", s, c);
				break;
			}
		}
		p = estrdup(s);
		dypush(tags, p);
	}
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
	int r, nf, nr;
	char *s, *t, *path, *tag, **tags, name[512];
	File *f;

	path = arg;
	DPRINT(Debugfs, "loadcsv %s", path);
	r = -1;
	tags = nil;
	f = emalloc(sizeof *f);
	if(openfs(f, path, OREAD) < 0)
		goto end;
	if((tags = csvheader(f)) == nil)
		goto end;
	nr = 1;
	name[sizeof name-1] = 0;
	/* beyond the tag names in the header, we leave all input validation to strawk */
	while((s = readline(f, nil)) != nil){
		*name = 0;
		for(nf=0; s!=nil; s=t, nf++){
			t = nextfield(f, s, nil, ',');
			if(nf == 0){
				strecpy(name, name+sizeof name, s);
				continue;
			}else if(nf-1 >= dylen(tags)){
				warn("readcsv: line %d: ignoring extra columns\n", nr);
				break;
			}
			tag = tags[nf-1];
			// FIXME: stupid
			if(strcmp(tag, "CL") == 0 || cistrcmp(tag, "color") == 0)
				pushcmd("nodecolor(\"%s\", %s)", name, s);
			else if(strcmp(tag, "BO") == 0)
				pushcmd("fixnodex(\"%s\", %s)", name, s);
			else
				pushcmd("%s[\"%s\"] = \"%s\"", tags[nf-1], name, s);
		}
		nr++;
	}
	r = 0;
end:
	pushcmd("cmd(\"FJJ142\")");
	for(nf=0; nf<dylen(tags); nf++)
		free(tags[nf]);
	dyfree(tags);
	USED(tags);
	freefs(f);
	if(r < 0)
		warn("loadcsv %s: %s\n", path, error());
	else
		warn("done reading csv\n");
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
