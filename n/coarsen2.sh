#!/usr/bin/env -S awk -f
# plaintext coarsening tree for debugging and testing
# input: plaintext index.sh output (wide format edge list)
# NOTE for this implementation:
# - mounting precision issues + no external memory → small graphs only
# - requires write access to /tmp
# - input is 1-indexed, output is 0-indexed
# - no input sanity checks
BEGIN{
	OFS = "\t"
	tmpdir = "/tmp"
	"uuidgen" | getline f
	tmp = tmpdir "/coarse2." f
}
{
	d = $1
	u = $2 - 1
	node[u] = u
	weight[u] = 1
	if(NV < u + 1)
		NV = u + 1
	# NOTE: this includes mirrored edges, ie the real NE is half, initially
	for(i=3; i<=NF; i++){
		edgeu[++NE] = u
		edgev[NE] = $i - 1
		edge[NE] = NE
	}
}
END{
	printgraph()
	S = NV - 1	# nodemap is 0-indexed
	w = S
	top = -1	# cannot happen
	while(NE > 0){
		ne = NE
		newlevel()
		for(i=1; i<=ne; i++){
			e = edge[i]
			u = edgeu[e]
			v = edgev[e]
			s = node[u]
			t = node[v]
			# unvisited node: make new supernode
			if(s <= w){
				top = u
				lastm[s] = ++S
				s = S
				last[s-w] = i
				outputsuper()
				node[u] = s
			}
			t = node[v]
			# unvisited adjacency or self: merge internal edges
			if(t <= w && lastm[t] <= w || t == s && u != v){
				# edges not starting from the top node are mirrors, skip them
				if(u == top){
					outputnode(v)
					weight[u] += weight[v]
					weight[v] = weight[u]
					t = node[v] = s
					outputedge()
				}
			# adjacency previously merged elsewhere: fold external edges
			}else{
				if(last[t-w] >= last[s-w] && (u != v || nl > 1)){
					# already retained one edge, discard following redundant ones
					printf "discarding redundant edge: t:%d >= s:%d\n",
						last[t-w], last[s-w] >> "/dev/stderr"
					outputedge()
				}else{
					# retain edge for next round
					edge[++NE] = edge[i]
					printf "retain edge[%d] %x,%x in slot %d\n",
						i, s, t, NE >> "/dev/stderr"
					last[t-w] = i
				}
			}
		}
		outputlevel()
		w = S
	}
	# add a top node
	outputtop()
	# reverse order from coarsest to full
	reverse()
}
function reverse(){
	for(i=nl; i>=1; i--){
		f = sprintf(tmp ".%08x", i)
		system("cat " f "* && rm " f "*")
	}
}
function endlevel(){
	printf "level %d %d\n", NV, NE2 >> fname "_0"
	close(fname "_0")
	close(outf)
	close(eoutf)
}
function newlevel(){
	NV = NE2 = NE = 0
	fname = sprintf(tmp ".%08x", ++nl)
	outf = sprintf(fname "_1")
	eoutf = sprintf(fname "_2")
}
function outputsuper(){
	printf "new supernode %x: ", s >> "/dev/stderr"
	outputnode(u)
}
function outputnode(n){
	printf "node %x %x %x %d\n", n, node[n], s, weight[n] >> outf
	printf "merge node %x → %x, weight %d\n", n, s, weight[n] >> "/dev/stderr"
	NV++
}
function outputedge(){
	printf "edge %x\n", edge[i]-1 >> eoutf
	printf "discard edge i=%d mapping to %x,%x (u,v %x,%x)\n", i-1, s, t, u, v >> "/dev/stderr"
	NE2++
}
function outputtop(	u){
	newlevel()
	u = top
	s = ++S
	outputsuper()
	outputlevel()
}
function outputlevel(	i){
	printf "push\n" >> eoutf
	printf "ending level with NV=%d NE=%d\n", NV, NE >> "/dev/stderr"
	endlevel()
	delete last
	for(i=NE+1; i<=ne; i++)
		delete edge[i]
	printgraph()
}
function printgraph(	i, e, u, v){
	for(i=1; i<=NE; i++){
		e = edge[i]
		u = edgeu[e]
		v = edgev[e]
		printf "E[%d] e %d u[%d] %x v[%d] %x\n", i, e, u, node[u], v, node[v] >> "/dev/stderr"
	}
	# unordered, but complete (for debugging)
	for(i in node){
		u = node[i]
		printf "V[%d] u %x w %d\n", i, u, weight[i] >> "/dev/stderr"
	}
}
