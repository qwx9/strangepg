$1 == "n"{
	node[$3] = $2
	lnode[$2] = $3
	next
}
$1 == "e"{
	edge[$2,1] = $3
	edge[$2,2] = $4
	edge[$2,3] = $5
	edge[$2,4] = $6
	ledge[$3,$4] = $2
	next
}
$1 == "d"{
	delete node[lnode[$2]]
	delete lnode[$2]
	next
}
$1 == "c"{
	id = edge[$2,1] "\x1c" edge[$2,2]
	delete ledge[id]
	delete edge[$2,1]
	delete edge[$2,2]
	delete edge[$2,3]
	delete edge[$2,4]
	next
}
$1 == "C"{
	id = ledge[$2,$3]
	delete ledge[id]
	delete edge[id,1]
	delete edge[id,2]
	delete edge[id,3]
	delete edge[id,4]
	next
}
{
	system("fortune")
}
