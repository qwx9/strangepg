$1 == "n"{
	if($2 in lnode)
		delete node[lnode[$2]]
	node[$3] = $2
	lnode[$2] = $3
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
# FIXME: not distinguishing between node/edge? should we?
$1 == "s"{
	if($2 == "NLN"){ NLN[$3] = $4 }
	else if($2 == "cig"){ CIG[$3] = $4 }
	# maybe this alone is sufficient?
	else{
		k = $2 "\1xc" $3
		if(!(k in sym)){
			lhs[$2]++
			rhs[$2] = rhs[$2] "\1xc" $3
		}
		sym[k] = $4
	}
	next
}
# tokenize and eval expression in the same way?
/^node\[[^\]]+\]$/{
	gsub("node\\[|\\]", "", $1)
	s = lnode[$1]
	if($1 in NLN)
		s = s ", LN=" NLN[$1]
	print "node", $1, s
	next
}
{
	print "?", $0 >>"/fd/2"
	system("fortune")
}
