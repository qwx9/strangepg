#!/bin/awk -f
# one round: from given set of edges, output result
# to avoid awk bullshit, we 1-index but debug print 0-indexed
function exists(s, t){
	return (s,t) in adj
}
function pushedge(s, t)
{
	NE++
	if(!(s in head))
		head[s] = s"\x1c"t
	print "E", s-1, t-1 >> "/fd/2"
	adj[tail[s]] = s"\x1c"t
	tail[s] = s"\x1c"t
	adj[s,t] = -1
}
function setdegree(s, d)
{
	# TODO
}
function flatten(){
	# TODO: flatten via ordered degree list
	for(s in head){
		p = head[s]
		while(p != -1){
			split(p, a, "\x1c")
			print a[1] "\t" a[2]	# 1-indexed for awk
			p = adj[p]
		}
	}
}
{
	u = $1
	v = $2
	# new left node
	if(u != uo){
		# unvisited left node
		if(!(u in nmap)){
			topdog = u
			s = ++S
			nmap[u] = s
			uo = u
			print "S", u-1, "←", S-1 >> "/fd/2"
		}else
			s = nmap[u]
	}
	setdegree(s, +1)
	# unvisited right node
	if(!(v in nmap)){
		# vassals may not annex nodes on their own
		if(u == topdog){
			nmap[v] = s
			print "M", v-1, "←", s-1 >> "/fd/2"
			setdegree(s, -1)
		}else
			print "_", v-1, ":", u-1, "not", s-1 >> "/fd/2"
	# self edge (output it)
	}else if(u == v){
		print "↔", u-1, v-1 >> "/fd/2"
		setdegree(s, -1)
	# mirrored internal edge (skip it)
	}else if((t = nmap[v]) == s){
		print "↔", u-1, v-1 >> "/fd/2"
		setdegree(s, -1)
	# unique external edge, retained for next iteration with mirror
	}else if(!exists(s,t)){
		pushedge(s,t)
		if(!exists(t,s))
			pushedge(t,s)
	}
}
END{
	flatten()
}
