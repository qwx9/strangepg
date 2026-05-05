#!/bin/awk -f
# pass nolabs=1, notags=1, nopaths=1 as arg
function rename(x){
	if(!(x in nodes))
		nodes[x] = length(nodes) + 1
	return nodes[x]
}
BEGIN{
	OFS = "\t"
}
$1 == "H"{
	print
}
$1 == "S"{
	for(i=4; i<=NF; i++)
		if($i ~ /^LN:/)
			break
	if(i > NF){
		$3 = "*\tLN:i:"length($3)
		FS = FS
	}else
		$3 = "*"
	if(nolabs)
		$2 = rename($2)
	if(notags)
		print $1, $2, $3
	else
		print
}
$1 == "L"{
	if(nolabs){
		$2 = rename($2)
		$4 = rename($4)
	}
	if(notags)
		print $1, $2, $3, $4, $5, "*"
	else
		print
}
$1 == "P"{
	if(nopaths)
		next
	if(nolabs){
		n = split($3,a,",")
		for(i=1; i<n; i++){
			if(i == 1)
				k = length(a[i])
			else
				k = m
			u = substr(a[i],1,k-1)
			U = substr(a[i],k)
			l = nodes[u] U
			if(i == 1)
				p = l
			else
				p = p "," l
		}
	}else
		p = $3
	print $1, $2, p
	p = ""
}
