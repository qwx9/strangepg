function addnode(id, label){
	node[label] = id
	lnode[id] = label
}
function addnnode(id, label){
	node[label] = id
	lnode[id] = label
}
function addedge(id, u, v, d1, d2){
	id = u "" v
	edge[id] = node[u] "" node[v]
	dir[id,1] = d1
	dir[id,2] = d2
}
function addnedge(id, u, v, d1, d2){
	id = u "" v
	edge[id] = node[u] "" node[v]
	dir[id,1] = d1
	dir[id,2] = d2
}
function delnode(id){
	delete node[lnode[id]]
	delete lnode[id]
}
function delnnode(id){
	delete node[lnode[id]]
	delete lnode[id]
}
function cutedge(u, v,	id){
	id = u "" v
	delete dir[id,1]
	delete dir[id,2]
	delete edge[id]
}
function cutnedge(u, v,	id){
	id = u "" v
	delete dir[id,1]
	delete dir[id,2]
	delete edge[id]
}
function doprint(type){
	if(type == "edges"){
		for(i in edge) print "[" i "]", edge[i]
	}else if(type == "nodes"){
		for(i in nodes) print "[" i "]", nodes[i]
	}
	# FIXME: ...
}
function expandnode(id){ }
function retractnode(id){ }
function select(id){ }
function hidenode(id){ }
function hideedge(id){ }
function shownode(id){ }
function showedge(id){ }
function colornode(id, c){ }
function coloredge(id, c){ }
{
	# FIXME: [c] var ~ PAT: command (optional), table, regex to match against
	#	this works, can "define" regex to match against at runtime
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
	else if($1 == "p")
		doprint($2)
	else
		print "nope"
}
