#!/bin/awk -f
# one round: from given set of edges, output result
# to avoid awk bullshit, we 1-index but debug print 0-indexed
{
	u = $1
	v = $2
	if(!(u in visited)){
		visited[u] = ++S
		top = u
		printf "NEWSUPER %x → %x [%d]\n", u, s, weight[u+1] >>"/fd/2"
	}
	s = visited[u]
	if(!(v in visited)){
		if(u == top){
			visited[v] = s
			printf "MERGERIGHT %x,%x (%x,%x)\n", s, t, u, x >>"/fd/2"
		}else
			printf "SKIPOUTER %x,%x (%x,%x)\n", s, t, u, v >>"/fd/2"
	}else if(u == v)
		printf "SELF %x,%x (%x,%x)\n", s, t, u, v >>"/fd/2"
	else if((t = visited[v]) == s)
		printf "MIRROR %x,%x (%x,%x)\n", s, t, u, v >>"/fd/2"
	else if((s,t) in edges || (t,s) in edges)
		printf "REDUNDANT %x,%x (%x,%x)\n", s, t, u, v >>"/fd/2"
	else{
		edges[s,t] = 1
		edgei[++M] = s"\x1c"t
		printf "EXT %x,%x (%x,%x)\n", s, t, u, v >>"/fd/2"
	}
}
END{
	for(i=1; i<=M; i++){
		split(edgei[i], a, "\x1c")
		print a[1] "\t" a[2]
	}
}
