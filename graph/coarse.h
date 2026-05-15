typedef struct CNode CNode;

enum{
	FCNvisited = 1<<0,
	FCIseen = 1<<31,
};
enum{
	FCIhidden = 0xfefefefe,
};
/* FIXME: some of the properties of the nodes would be nice to keep as read-
 * only for recovery or undoing changes; but a lot of those could just be
 * kept in the gfa and re-read as well, that's relatively cheap and can be
 * done asynchronously already */
struct CNode{
	ioff idx;		/* correspondence in visible nodes[] */
	ioff eoff;		/* FIXME: should be voff to allow 1e9 */
	ushort nedges;	/* FIXME: get rid of it and use next eoff? */
	uchar flags;	/* FIXME */
	ioff parent;
	ioff child;
	ioff sibling;
};
extern CNode *cnodes;	/* immutable */
extern ioff *cedges;	/* immutable */

int	initcoarse(void);
