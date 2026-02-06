#!/bin/awk -f
# pass nolabs=1 and/or notags=1 as arg
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
