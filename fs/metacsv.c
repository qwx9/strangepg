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
csvheader(File *f, int *wait, int *nwarn)
{
	int r;
	char c, w, *p, *s, **tags;

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
		r = 0;
		while(isspace(*s))
			s++;
		for(p=s; (c=*p)!=0; p++){
			if(isspace(c)){
				*p = '_';
				r = 1;
				continue;
			}
			if(p == s){
				if(!isalpha(c)){
					werrstr("loadcsv: invalid tag name \"%s\" in header", s);
					return nil;
				}
			}else if(!isalnum(c)){
				*p = 0;
				r = 1;
				logmsg(va("loadcsv: truncating tag name \"%s\" on invalid character \'%c\'\n", s, c));
				break;
			}
		}
		for(p--; *p=='_'; p--)
			*p = 0;
		/* don't wait for csv to load if there are no special tags */
		if(cistrncmp(s, "color", 5) == 0){
			s = "CL";
			r = 1;
			w = 1;
		}else if(strlen(s) == 2
		&& (s[0] == 'C' && s[1] == 'L'
		|| s[0] == 'f' && (s[1] == 'x' || s[1] == 'y' || s[1] == 'z')
		|| s[1] == '0' && (s[0] == 'x' || s[0] == 'y' || s[0] == 'z')))
			w = 1;
		if(r){
			DPRINT(Debuginfo, "loadcsv: tag renamed to \"%s\"", s);
			(*nwarn)++;
		}
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
	int r, nf, nr, wait, nwarn;
	char *s, *path, *tag, **tags, name[512];
	File *f;

	path = arg;
	DPRINT(Debugfs, "loadcsv %s", path);
	if((f = openfs(path, OREAD)) == nil){
		logerr(va("loadcsv %s: %s\n", path, error()));
		free(path);
		return;
	}
	r = -1;
	nr = 0;
	nwarn = 0;
	splitfs(f, ',');
	if((tags = csvheader(f, &wait, &nwarn)) == nil)
		goto end;
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
				logmsg(va("readcsv: line %d: ignoring extra columns\n", nr));
				break;
			}
			tag = tags[nf-1];
			if(setnamedtag(tag, name, s) < 0){
				DPRINT(Debuginfo, "loadcsv: line %d: %s", f->nr, error());
				nwarn++;
			}
		}
		nr++;
	}
	r = 0;
end:
	if(wait || debug & Debugload){
		pushcmd("cmd(\"FHJ142\")");
		flushcmd();
	}
	if(r < 0)
		logerr(va("loadcsv %s: %s\n", path, error()));
	else
		logmsg(va("loadcsv: done, read %d records\n", nr));
	if(nwarn > 0 && (debug & Debuginfo) == 0)
		warn("loadcsv: suppressed %d warning messages (use -W to display them)\n", nwarn);
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
