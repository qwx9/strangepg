#!/bin/awk -f
# strpg/n/longplain.awk $gfa | strpg/test/ctfull.awk 10 >[2]/dev/null
BEGIN{
	OFS="\t"
	if(ARGC <= 1){
		err = "usage: " ARGV[0] " NEDGES [EDGELIST]"
		exit err
	}
	# fucking hell
	nedges = int(ARGV[1])
	if(ARGC == 2)
		ARGV[1] = "-"
	else
		delete ARGV[1]
	S = nedges		# 1-indexed
	w = S
	print "===== level " lvl++ >>"/fd/2"
}
function gprint(	i, a){
	print "level", lvl, e
	for(i=1; i<=M; i++){
		split(edgei[i], a, "\x1c")
		printf "%x\t%x\n", a[1]-1, a[2]-1
	}
}
function trymerge(u, v, u0, v0){
	printf ">> [%03d] EDGE %x,%x\n", ++ep, u-1, v-1 >>"/fd/2"
	if(!(u in visited)){
		s = visited[u] = ++S
		top = u
		printf "NEWSUPER %x → %x [%x]\n", u-1, s-1, weight[u] >>"/fd/2"
	}
	s = (u in visited) ? visited[u] : u
	t = (v in visited) ? visited[v] : v
	if(!(v in visited)){
		if(u == top){
			visited[v] = s
			printf "MERGERIGHT %x,%x (%x,%x)\n", s-1, t-1, u-1, v-1 >>"/fd/2"
		}else
			printf "SKIPOUTER %x,%x (%x,%x)\n", s-1, t-1, u-1, v-1 >>"/fd/2"
	}else if(u == v)
		printf "SELF %x,%x (%x,%x)\n", s-1, t-1, u-1, v-1 >>"/fd/2"
	else if((t = visited[v]) == s)
		printf "MIRROR %x,%x (%x,%x)\n", s-1, t-1, u-1, v-1 >>"/fd/2"
	else if((s,t) in edges || (t,s) in edges)
		printf "REDUNDANT %x,%x (%x,%x)\n", s-1, t-1, u-1, v-1 >>"/fd/2"
	else{
		edges[s,t] = lvl == 1 ? (u"\x1c"v) : (u0"\x1c"v0)
		edgei[++e] = s"\x1c"t
		printf "EXT %x,%x (%x,%x) [%x,%x]\n", s-1, t-1, u-1, v-1, u0-1, v0-1 >>"/fd/2"
	}
}
{
	trymerge($1, $2, $1, $2)
}
END{
	if(err != ""){
		print err >>"/fd/2"
		exit err
	}
	M = e
	gprint()
	while(M > 0){
		print "===== level " lvl++ >>"/fd/2"
		e = 0
		for(i=1; i<=M; i++){
			split(edgei[i], a, "\x1c")
			split(edges[edgei[i]], b, "\x1c")
			delete edges[edgei[i]]
			trymerge(a[1], a[2], b[1], b[2])
		}
		M = e
		gprint()
	}
}
