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
function addnode(id, name, color){
	# remove placeholder for nodes spawned from out of order links
	if(name in node)
		delete node[name]
	node[name] = id
	lnode[id] = label
	if(color != "")
		CL[name] = color
}
function nodecolor(name, color){
	if(!(name in node)){
		print "E no such node", name
		return
	}
	CL[name] = color
	print "c", node[name], color
}
function delnode(name){
	if(!(name in node)){
		print "E no such node", name
		return
	}
	delete lnode[node[name]]
	delete node[name]
}
function addedge(id, u, urev, v, vrev, 	pair){
	pair = u (urev ? "-" : "+") "\x1c" v (vrev ? "-" : "+")
	edge[pair] = id
	ledge[id] = pair
}
function deledgebyid(id){
	if(!(name in edge)){
		print "E no such edge", name
		return
	}
	delete edge[ledge[id]]
	delete ledge[id]
}
function deledge(u, urev, v, vrev){
	pair = u urev "\x1c" v vrev
	if(!(pair in edge)){
		print "E no such edge", u urev "," v vrev
		return
	}
	delete ledge[edge[pair]]
	delete edge[pair]
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
# FIXME: stupid
function fixnode(name, x, y){
	if(!(name in node)){
		print "E no such node", name
		return
	}
	if(y == "")
		y = 0
	fx[name] = x
	fy[name] = y
	print "X", node[name], x, y
}
function fixnodex(name, x){
	if(!(name in node)){
		print "E no such node", name
		return
	}
	BO[name] = x
	print "x", node[name], x
}
{
	eval("{" $0 "}")
}
