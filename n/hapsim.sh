#!/bin/bash
pd=0.3
pi=0.2
usage(){
	echo usage: "$0 [-d %DUP] [-i %INV] NNODES" 1>&2
	exit 1
}
while [[ $# -gt 1 ]]; do
	case $1 in
	-d) shift; pd=$1;;
	-i) shift; pi=$1;;
	*) usage;;
	esac
	shift
done
if [[ $# -ne 1 ]]; then
	usage
fi
N=$1
awk \
	-v "N=$N" \
	-v "pd=$pd" \
	-v "pi=$pi" \
'
function nrand(n){
	return 1 + int((n-1) * rand() + 0.5)
}
function wrhdr(){
	print "H", "VN:Z:1.0"
}
function wrsegs(	i){
	for(i=1; i<=N; i++)
		print "S", i, "*"
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
	for(n=2; n<=N-1; n++){
		done = 0;
		while(rand() < pd)
			newlink(nrand(N-1))
		newlink(n)
	}
	newlink(N)
}
'
