function addnode(id, label){
	node[label] = id
	lnode[id] = label
	print "n", id, label
}
function addnnode(id, label){
	node[label] = id
	lnode[id] = label
	print "N", id, label
}
function addnedge(id, u, v, d1, d2){
	edge[id] = node[u] "\x1c" node[v]
	dir[id,1] = d1
	dir[id,2] = d2
	print "E", id
}
function addedge(id, u, v, d1, d2){
	edge[id] = u "\x1c" v
	dir[id,1] = d1
	dir[id,2] = d2
	print "e", id
}
function expandnode(id){ return "expandnode(" id ")" }
function retractnode(id){ return "retractnode(" id ")" }
function select(id){ return "select(" id ")" }
function hidenode(id){ return "hidenode(" id ")" }
function hideedge(id){ return "hideedge(" id ")" }
function shownode(id){ return "shownode(" id ")" }
function showedge(id){ return "showedge(" id ")" }
function colornode(id, c){ return "colornode(" id "," c ")" }
function coloredge(id, c){ return "coloredge(" id "," c ")" }
{
	if($1 == "n")
		addnode($2, $3)
	else if($1 == "N")
		addnnode($2, $3)
	else if($1 == "e")
		addedge($2, $3, $4, $5, $6)
	else if($1 == "E")
		addnedge($2, $3, $4, $5, $6)
	else
		print "nope"
}
