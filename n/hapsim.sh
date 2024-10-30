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
	return n < 1 ? 0 : int((n-1) * rand() + 0.5)
}
function wrhdr(){
	print "H", "VN:Z:1.0"
}
function wrsegs(	i, m, t){
	t = ""
	for(i=1; i<=N; i++)
		print "S", i, "*" t
}
function wrlink(left, right, ldir, rdir){
	print "L", left, ldir, right, rdir, "0M"
}
function newlink(n){
	right = n
	rdir = rand() < pi ? "-" : "+"
	wrlink(left, right, ldir, rdir)
	left = right
	ldir = rdir
}
BEGIN{
	OFS = "\t"
	srand()
	wrhdr()
	wrsegs()
	frag--
	if(frag >= 1)
		k = N / 2 + nrand(N / 2 - frag)
	else
		k = N
	left = 1
	ldir = "+"
	for(n=2; n<=k; n++){
		while(rand() < pd)
			newlink(1 + nrand(N))
		newlink(n)
	}
	if(k == N)
		exit
	# FIXME: not quite
	ldir = "+"
	for(n=k; n<=N-1;){
		l = left = 1 + nrand(k)
		c = 1 + nrand(N - n)
		m = n + c - 1
		while(n <= m)
			newlink(n++)
		#if(n < N)
			newlink(l + c)
		ldir = rand() < pi ? "-" : "+"
	}
	newlink(N)
}
'
