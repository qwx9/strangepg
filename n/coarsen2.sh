#!/usr/bin/env -S bioawk -f
function output(u, parent){
	printf "%x:%x:%x,", u, parent, weight[u]
}
BEGIN{
	OFS = "\t"
}
{
	d = $1
	u = $2
	deg[u] = d
	n2s[u] = u
	nith[++NN] = u
	weight[u] = 1
	for(i=3; i<=NF; i++){
		off[++NE] = u	# for debug
		e2n[NE] = $i
	}
}
# FIXME: tables: e2n, nith//deg, n2s, weight ± off
# FIXME: must be backwards... → numbered temp files
#	warning: avoid locale problems
END{
	news = NN
	olds = NN
	for(i=1; i<=NE; i++){
		u = off[i]
		v = e2n[i]
		printf "e %d u %d v %d\tdeg %d w %d\n", i, n2s[u], n2s[v], deg[u], weight[u]
	}
	while(NE > 0){
		ne = NE
		i = 1
		newi = 1
		newe = 1
		for(e=1; e<=ne;){
			u = nith[i]
			s = n2s[u]
			print ">> e " e, "u " u, "deg " deg[u]
			olddeg = deg[u]
			deg[u] = 0
			# all the _,v adjacencies already exist elsewhere in
			# the edge list, and u already aliased v to itself
			if(s >= olds){
				printf "skip merged node %d part of %d\n", u, s
				e += olddeg
				i++
				continue
			}
			nn = 0
			nm = 0
			# FIXME: detect redundancies between older merges and
			# aliasing
			# (can leave it for now and just have extra edges)
			# could just mark adjacency somehow
			# we could store a bitwise adjacency matrix or sth
			# but maybe pointless, would still be like (1e9^2)/8 bytes
			# either find a trick to ignore duplicates, or sort the
			# list and look up? preprocessing maybe, like flag future
			# redundancies?
			for(d=0; d<olddeg; d++){
				vv = e2n[e+d]
				v = n2s[vv]
				print "	v " vv, "s " v, "e " (e+d), "u " u, "deg " deg[vv]
				if(nm++ == 0){
					s = news++
					n2s[u] = s
				}
				# write edges we *do not* remove
				if(v >= olds){	# already merged with some node
					if(nn++ == 0)
						nith[newi++] = u
					off[newe] = u
					e2n[newe++] = vv
					deg[u]++
					printf "add ext edge %d,%d (u=%d)\n", s, v, u
					continue
				}
				weight[u] += weight[vv]
				n2s[vv] = s
				printf "remove int edge %d,%d (u=%d)\n", s, v, u
			}
			e += d
			i++
		}
		olds = news
		NE = newe - 1
		NN = newi - 1
		break
	}
	for(i=1; i<=NE; i++){
		u = off[i]
		v = e2n[i]
		printf "e %d u %d v %d\tdeg %d w %d\n", i, n2s[u], n2s[v], deg[u], weight[u]
	}
}
