$1 == "n"{
	node[$3] = $2
	lnode[$2] = $3
	print "new node id", $2, "name", $3
	next
}
$1 == "e"{
	e = $3 "\x1c" $4
	edge[e] = $2
	ledge[$2] = e
	next
}
$1 == "d"{
	delete node[lnode[$2]]
	delete lnode[$2]
	next
}
$1 == "c"{
	id = ledge[$2]
	delete edge[id]
	delete ledge[$2]
	next
}
$1 == "C"{
	id = edge[$2,$3]
	delete ledge[id]
	delete edge[$2,$3]
	next
}
$1 == "s"{
	print $2"["$3"] ← "$4"\t("lnode[$2]")"
	next
}
{
	print "?", $0
	system("fortune")
}
