#!/bin/awk -f
# one round: from given set of edges, output result
# to avoid awk bullshit, we 1-index but debug print 0-indexed
{
	u = $1
	v = $2
	if(!(u in visited)){
		visited[u] = ++S
		print "S", u-1, "←", S-1 >> "/fd/2"
		top = u
	}
	s = visited[u]
	if(!(v in visited)){
		# only the sovereign node may annex; if not it
		# just wait and see
		if(u == top){
			visited[v] = s
			print "M", v-1, "←", s-1 >> "/fd/2"
		}else
			print "_", v-1, ":", u-1, "not", s-1 >> "/fd/2"
	}else if(u == v){
		# self edge
		print "↔", u-1, v-1 >> "/fd/2"
	}else if((t = visited[v]) == s){
		# ignore mirror internal edge
		print "↔", u-1, v-1 >> "/fd/2"
	# external edge
	}else if((s,t) in edges || (t,s) in edges){
		# ignore redundant edges
		print "R", s-1, t-1 >> "/fd/2"
	}else{
		edges[s,t] = 1
		edgei[length(edges)] = s "\t" t
		print "E", s-1, t-1 >> "/fd/2"
	}
}
END{
	# unordered
	for(i=1; i<=length(edgei); i++)
		print edgei[i]
}
