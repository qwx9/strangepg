#include "strpg.h"

/* ids are `usize' indices into object vector, where bit 0 (LSB) has
 * a special and overloaded meaning in the general case:
 *	LSB = node ? 1 : 0, ie.  index = id << 1 | isnode.
 * the exception is walks, where only nodes are considered, ie.:
 *	index = id << 1 | 1
 * implicitely, while:
 *	LSB = inverted ? 1 : 0, ie. index = id << 1 | orientation
 */

/* stupid but forward-looking anti-brainfart code */
int
getgroup(Graph *g, usize id)
{
	


	switch(type){
	case Oedge: kv_A(g->obj, id).gid;
	}
}
