#!/bin/awk -f
# one round: from given set of edges, output result
# to avoid awk bullshit, we 1-index but debug print 0-indexed
function exists(a, b,	p){
	if(!(a in s2j))
		return 0
	for(p=s2j[a]; p<=NE; p++){
		if(edgeu[jump[p]] != a)
			break
		else if(edgev[jump[p]] == b)
			return 1
	}
	return 0
}
function insert(s,	p, q){
	if(s in s2j){
		for(p=s2j[s]; p<NE; p++)
			if(edgeu[jump[p]] != s)
				break
	}else{
		p = NE
		s2j[s] = p
	}
	jump[NE] = jump[p]
	jump[p] = NE
}
{
	u = $1
	v = $2
	if(!(u in visited)){
		visited[u] = ++S
		print "S", u-1, "←", S-1 >> "/fd/2"
		topdog = u
	}
	s = visited[u]
	# unvisited right node
	if(!(v in visited)){
		# vassals may not annex nodes on their own
		if(u == topdog){
			visited[v] = s
			print "M", v-1, "←", s-1 >> "/fd/2"
		}else
			print "_", v-1, ":", u-1, "not", s-1 >> "/fd/2"
	# self edge
	}else if(u == v)
		print "↔", u-1, v-1 >> "/fd/2"
	# mirrored internal edge
	else if((t = visited[v]) == s)
		print "↔", u-1, v-1 >> "/fd/2"
	# redundant external edge
	# FIXME: one way should do for both
	else if(exists(s,t) || exists(t,s))
		print "R", s-1, t-1 >> "/fd/2"
	# external edge, retained for next iteration
	else{
		NE++
		edgeu[NE] = s
		edgev[NE] = t
		insert(s)
		print "E", s-1, t-1 >> "/fd/2"
	}
}
END{
	# unordered
	for(i=1; i<=length(edgeu); i++)
		print edgeu[i] "\t" edgev[i]
}
