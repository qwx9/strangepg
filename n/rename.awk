#!/bin/env -S awk -f
$1 == "S"{
	node[$2] = ++nn
	$2 = node[$2]
	print
	next
}
$1 == "L"{
	if(!($2 in node) || !($4 in node)){
		late[++ne] = $0
		next
	}
	$2 = node[$2]
	$4 = node[$4]
	print
	next
}
$1 == "P"{
	if(length(late) != 0){
		for(i in late){
			n = split(late[i], a, "\t")
			late[2] = node[late[2]]
			late[4] = node[late[4]]
			printf late[1]
			for(j=1; j<=n; j++)
				printf "\t"late[n]
			printf "\n"
		}
		delete late
	}
	n = split($3, a, ",")
	printf $1 "\t" $2 "\t"
	for(i=1; i<=n; i++){
		e = substr(a[i], 1, length(a[i])-1)
		printf sep node[e] substr(a[i], length(a[i]))
		sep = ","
	}
	delete a
	printf "\n"
	next
}
{
	print
}
END{
	if(length(late) != 0){
		for(i in late){
			n = split(late[i], a, "\t")
			late[2] = node[late[2]]
			late[4] = node[late[4]]
			printf late[1]
			for(j=1; j<=n; j++)
				printf "\t"late[n]
			printf "\n"
		}
		delete late
	}
}
