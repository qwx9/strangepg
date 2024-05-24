BEGIN{
	red = "0xff0000"
	green = "0x00ff00"
	blue = "0x0000ff"
	orange = "0xffff00"
	yellow = "0xff7f00"
	grey = "0xffff00"
	brown = "0x7f3f00"
	black = "0x0"
	white = "0xffffff"
	lightblue = "0x1f78b4"
	lightorange = "0xff7f00"
	lightgreen = "0x33a02c"
	lightred = "0xe31a1c"
	violet = "0x6a3d9a"
	purple = violet
	lightbrown = "0xb15928"
	paleblue = "0x8080ff"
	palegreen = "0x8ec65e"
	palered = "0xc76758"
	paleorange = "0xca9560"
	paleviolet = "0xc893f0"
	greybrown = "0x7f5f67"
	lightviolet = "0xb160c9"
	palebluegreen = "0x5fc69f"
	pinkviolet = "0xc96088"
	cyan = "0x8dd3c7"
	paleyellow = "0xffffb3"
	greyblue = "0xbebada"
	lightorange2 = "0xfb8072"
	lightblue2 = "0x80b1d3"
	lightorange2 = "0xfdb462"
	lightgreen2 = "0xb3de69"
	greypink = "0xfccde5"
	lightgrey = "0xd9d9d9"
	lightviolet = "0xbc80bd"
	greygreen = "0xccebc5"
	lightyellow = "0xffed6f"
	lightblue2 = "0xa6cee3"
	lightgreen3 = "0xb2df8a"
	lightpink = "0xfb9a99"
	lightorange3 = "0xfdbf6f"
	greyviolet = "0xcab2d6"
	paleyellow = "0xffff99"
}
function addnode(id, label, color){
	# remove placeholder for nodes spawned from out of order links
	if(id in lnode)
		delete node[lnode[id]]
	node[label] = id
	lnode[id] = label
	if(color != "")
		CL[id] = color
}
function nodecolor(id, color){
	if(!(id in lnode)){
		print "E no such node", id
		return
	}
	CL[id] = color
	print "c", id, color
}
function delnode(id){
	if(!(id in lnode)){
		print "E no such node", id
		return
	}
	delete node[lnode[id]]
	delete lnode[id]
}
function addedge(id, u, urev, v, vrev, 	pair){
	pair = u (urev ? "-" : "+") "\x1c" v (vrev ? "-" : "+")
	edge[pair] = id
	ledge[id] = pair
}
function deledge(id){
	if(!(id in ledge)){
		print "E no such edge", id
		return
	}
	pair = ledge[id]
	delete edge[pair]
	delete ledge[id]
}
function deledgeuv(u, urev, v, vrev){
	pair = u urev "\x1c" v vrev
	if(!(pair in edge)){
		print "E no such edge", u urev "," v vrev
		return
	}
	id = edge[u urev,v vrev]
	delete ledge[id]
	delete edge[u,v]
}
function cmd(code){
	if(code == "FGD135")	# wing attack plan R
		crm114 = 1
}
function readcsv(f){
	print "f", f
}
function selectnode(){
}
function selectedge(){
}
function fixnode(id, x, y){
	if(y == "")
		y = 0
	fx[id] = x
	fy[id] = y
	print "x", id, x, y
}
{
	eval("{" $0 "}")
}
