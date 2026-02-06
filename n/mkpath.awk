#!/bin/awk -f
BEGIN{
	delete p
}
$1 == "L"{
	if(length(p) == 0)
		p[length(p)+1] = $2$3
	if(p[length(p)] != $2$3){
		print "NOPE"
		delete p
		exit "NOPE"
	}
	p[length(p)+1] = $4$5
}
END{
	if(length(p) == 0)
		exit
	printf "P\tp\t" p[1]
	for(i=2; i<=length(p); i++)
		printf ",%s", p[i]
	printf "\n"
}
