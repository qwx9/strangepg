function addnode(id, label){
	node[label] = id
	lnode[id] = label
	printf "n %s %s", id, label
}
function addnnode(id, label){
	node[label] = id
	lnode[id] = label
	printf "N %s %s", id, label
}
function addedge(id, u, v, d1, d2){
	id = u "" v
	edge[id] = node[u] "" node[v]
	dir[id,1] = d1
	dir[id,2] = d2
	printf "e %s %s %d %d", u, v, d1, d2
}
function addnedge(id, u, v, d1, d2){
	id = u "" v
	edge[id] = node[u] "" node[v]
	dir[id,1] = d1
	dir[id,2] = d2
	printf "E %s %s %d %d", u, v, d1, d2
}
function delnode(id){
	delete node[lnode[id]]
	delete lnode[id]
	printf "d %s", id
}
function delnnode(id){
	delete node[lnode[id]]
	delete lnode[id]
	printf "D %s", id
}
function cutedge(u, v,	id){
	id = u "" v
	delete dir[id,1]
	delete dir[id,2]
	delete edge[id]
	printf "c %s %s", u, v
}
function cutnedge(u, v,	id){
	id = u "" v
	delete dir[id,1]
	delete dir[id,2]
	delete edge[id]
	printf "C %s %s", u, v
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
	else if($1 == "d")
		delnode($2)
	else if($1 == "D")
		delnnode($2)
	else if($1 == "c")
		cutedge($2, $3)
	else if($1 == "C")
		cutnedge($2, $3)
	else
		print "nope"
	fflush()
}
