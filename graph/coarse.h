typedef struct CNode CNode;

enum{
	FCNvisited = 1<<0,
};
/* FIXME: some of the properties of the nodes would be nice to keep as read-
 * only for recovery or undoing changes; but a lot of those could just be
 * kept in the gfa and re-read as well, that's relatively cheap and can be
 * done asynchronously already */
struct CNode{
	ioff idx;		/* correspondence in visible nodes[] */
	ioff eoff;
	ioff nedges;
	ioff parent;
	ioff child;
	ioff sibling;
	uchar flags;	/* FIXME */
	int length;		/* LN[id] value before coarsening */	/* FIXME: uint? */
};
extern CNode *cnodes;	/* immutable */
extern ioff *cedges;	/* immutable */
extern ioff nnodes, nedges;	/* FIXME: rename */

int	initcoarse(void);
