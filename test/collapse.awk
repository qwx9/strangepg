#!/usr/bin/env -S strawk -f
# fn t { dot -Tps /dev/snarf >/tmp/out.ps && page /tmp/out.ps }
function getadj(id, k,	eid){
	eid = adj[id,k]
	#return eid < 0 ? edgeu[-eid] : edgev[eid]
	return eid < 0 ? edge[-eid] : edge[eid]
}
function printnodes(	id, k, eid, l){
	print nnodes, "nodes:"
	for(id=0; id<nnodes; id++){
		printf "%d\t%s: %d adj: ", id, label[id], degree[id]
		for(k=0; k<degree[id]; k++){
			l = getadj(id, k)
			printf "%s%s", (k>0?",":""), l
		}
		printf "\n"
	}
}
function printedges(	id){
	print nedges, "edges:"
	for(id=0; id<nedges; id++)
		printf "%d\t%s\n", id, edge[id]
}
function addnode(l,	id){
	if(l in nodeid)
		id = nodeid[l]
	else{
		id = nnodes++
		nodeid[l] = id
	}
	label[id] = l
	return id
}
BEGIN{
	OFS = "\t"
}
$1 == "S"{
	addnode($2)
}
$1 == "L"{
	u = addnode($2)
	v = addnode($4)
	d1 = $3 == "-"
	d2 = $5 == "-"
	if(u > v || u == v && d1){
		t = u
		u = v
		v = t
		t = d1
		d1 = !d2
		d2 = !t
	}
	d1 = d1 ? "-" : "+"
	d2 = d2 ? "-" : "+"
	l = u d1 v d2
	if(l in edges){
		#print "line " NR ": duplicate edge, previous definition: ", edges[l]
		next
	}
	edges[l] = NR
	id = nedges++
	edgeu[id] = u
	edgev[id] = v
	edged1[id] = d1
	edged2[id] = d2
	edge[id] = l
	d = degree[u]++
	nadj++
	adj[u,d] = id
	nei[u,d] = v
	if(v != u){
		d = degree[v]++
		adj[v,d] = -id
		nei[v,d] = u
		nadj++
	}
}
function exportgfa(	id, U){
	print "H", "VN:Z:1.0"
	for(id in nodeid){
		U = nodeid[id]
		print "S", id, "*", "dg:i:" degree[U],
			"cp:i:" (U in parent ? parent[U] : -1),
			"cc:i:" (U in child ? child[U] : -1),
			"cs:i:" (U in sibling ? sibling[U] : -1)
	}
	for(id in edge)
		print "L", edgeu[id], edged1[id], edgev[id], edged2[id], "0M"
}
function printctab(	U){
	for(U=0; U<nnodes; U++){
		print (U in parent ? parent[U] : -1),
			(U in child ? child[U] : -1),
			(U in sibling ? sibling[U] : -1)
	}
}
function printtree(	id){
	print "digraph {"
	for(id in edge)
		print "\t" edgeu[id] " -> " edgev[id]
	print "}"
}
function printdot(	U){
	print "digraph {"
	for(U in parent)
		print "\t" parent[U] " -> " U
	#for(U in child)
	#	print "\t" child[U] " -> " U " [color=grey]"
	#for(U in sibling)
	#	print "\t" U " -> " sibling[U] " [style=dashed, color=blue]"
	print "}"
}
function getlastchild(U,	W){
	if(!(U in child))
		W = ""
	else{
		W = child[U]
		while(W in sibling)
			W = sibling[W]
	}
	return W
}
function printsorted(	n, i, e, ee, d, id, eid, v){
	if((n = length(idx)) == 0)
		return
	print n " sorted nodes:"
	for(i=e=0; i<n; i++){
		id = idx[i]
		d = degree[id]
		printf "[%d] %d\t%s: %d adj: ", i, id, label[id], d
		s = ""
		for(ee=e+d; e<ee; e++){
			v = idxe[e]
			printf "%s%d", s, v
			if(s == "")
				s = ":"
		}
		printf "\n"
	}
}
function sortedges(	id, i, d, n, ep, p, v, bucket){
	max = 0
	for(id in degree){
		if((d = degree[id]) > max)
			max = d
		bucket[d]++
	}
	if(max == 0)
		return
	for(n=d=0; d<=max; d++){
		if(!(d in bucket))
			continue
		off[d] = p
		offe[d] = ep
		n = bucket[d]
		p += n
		ep += n * d
	}
	delete bucket
	for(id=0; id<nnodes; id++){
		if(!(id in degree))
			continue
		d = degree[id]
		i = off[d]++
		idx[i] = id
		i = offe[d]
		offe[d] += d
		for(k=0; k<d; k++)
			idxe[i++] = nei[id,k]
	}
	delete off
	delete offe
}
function top(u){
	while(u in parent)
		u = parent[u]
	return u
}
function lastchild(u){
	u = child[u]
	while(u in sibling)
		u = sibling[u]
	return u
}
function buildct(	r, u, v, i, k, id, e, ee, t){
	sortedges()
	delete nei
	while(length(idx) > 0){
		#print "collapse: round " ++r ": " length(idx) " nodes, " length(idxe) " edges"
		#printctab()
		#printsorted()
		for(i=e=0; i<length(idx); i++){
			u = idx[i]
			d = degree[u]
			u = top(u)
			k = u in deg2 ? deg2[u] : 0
			for(ee=e+d; e<ee; e++){
				v = idxe[e]
				if((v = top(v)) == u)
					continue
				if(!(v in visited)){
					visited[v] = 1
					parent[v] = u
					if(!(u in child))
						child[u] = v
					else
						sibling[lastchild(u)] = v
				}
				if(u""v in neigh)	# keep mirrors
					continue
				nei[u,k++] = v		# v's edges will be added on its turn
				neigh[u,v] = 1
				deg2[u]++
			}
			visited[u] = 1
		}
		delete degree
		for(d in deg2)
			degree[d] = deg2[d]
		delete deg2
		delete idx
		delete idxe
		delete neigh
		delete visited
		sortedges()
		delete nei
	}
}
END{
	#printnodes()
	#printedges()
	buildct()
	printdot()
	#printctab()
}
