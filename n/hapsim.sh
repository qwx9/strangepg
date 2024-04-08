#!/bin/bash
N=10	# N nodes
pd=0.1	# duplication ratio
pi=0.05	# inversion ratio
frag=1	# M connected components of N nodes (linked by single edge)
usage(){
	echo usage: "$0 [-f NCOMP] [-d %DUP] [-i %INV] [NNODES]" 1>&2
	exit 1
}
while [[ $# -gt 1 ]]; do
	case $1 in
	-d) shift; pd=$1;;
	-f) shift; frag=$1;;
	-i) shift; pi=$1;;
	*) usage;;
	esac
	shift
done
if [[ $# -ne 0 ]]; then
	N=$1
fi
awk \
	-v "N=$N" \
	-v "pd=$pd" \
	-v "pi=$pi" \
	-v "frag=$frag" \
'
function nrand(n){
	return 1 + int((n-1) * rand() + 0.5)
}
function wrhdr(){
	print "H", "VN:Z:1.0"
}
function wrsegs(	i, m, t){
	i = 1
	for(m=1; m<=frag; m++){
		t = "\tfx:f:" i*8*N
		for(i=i; i<=m*N; i++){
			print "S", i, "*" t
			t = ""
		}
	}
}
function wrlink(u, v, i, j){
	print "L", u, i, v, j, "0M"
}
function newlink(n){
	v = n
	j = rand() < pi ? "-" : "+"
	wrlink(u, v, i, j)
	u = v
	i = j
}
BEGIN{
	OFS = "\t"
	srand()
	wrhdr()
	wrsegs()
	u = 1
	i = "+"
	n = 2
	for(m=1; m<=frag; m++){
		k += N
		newlink(n)
		for(n=n; n<=k; n++){
			while(rand() < pd)
				newlink((m-1)*N + nrand(N-1))
			newlink(n)
		}
	}
}
'
