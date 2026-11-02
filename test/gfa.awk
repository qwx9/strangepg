#!/bin/awk -f
function printnodes(	id, k, eid, v){
	print (nnodes+1), "nodes:"
	for(id=0; id<nnodes; id++){
		printf "%d\t%s: %d adj: ", id, label[id], degree[id]
		for(k=0; k<degree[id]; k++){
			eid = adj[id,k]
			v = edgev[eid]
			if(id == v)
				v = edgeu[eid]
			printf "%s%d", (k>0?",":""), v
		}
		printf "; edges: "
		for(k=0; k<degree[id]; k++){
			printf "%s%d", (k>0?",":""), adj[id,k]
		}
		printf "\n"
	}
}
function printedges(	id, k, eid, v, s){
	print (nedges+1), "edges:"
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
		t = !d1
		d1 = !d2
		d2 = t
	}
	d1 = d1 ? "-" : "+"
	d2 = d2 ? "-" : "+"
	l = u d1 v d2
	if(l in edges){
		print "line " NR ": duplicate edge, previous definition: ", edges[l]
		next
	}
	edges[l] = NR
	id = nedges++
	edgeu[id] = u
	edgev[id] = v
	edge[id] = l
	d = degree[u]++
	nadj++
	adj[u,d] = id
	if(v != u){
		d = degree[v]++
		adj[v,d] = id
		nadj++
	}
}
END{
	printnodes()
	printedges()
}
