#!/bin/rc
N=10
pd=0.1
pi=0.2
while(~ $1 -?){
	switch($1){
	case -d
		pd=$2
		shift 2
	case -i
		pi=$2
		shift 2
	}
	shift 1
}
if(! ~ $#* 0){
	N=$1
	shift
}

awk \
	-v 'N='^$N \
	-v 'pd='^$pd \
	-v 'pi='^$pi \
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
