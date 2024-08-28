BEGIN{
	red = "0xff000090"
	green = "0x00ff0090"
	blue = "0x0000ff90"
	orange = "0xffff0090"
	yellow = "0xff7f0090"
	grey = "0xffff0090"
	brown = "0x7f3f0090"
	black = "0x090"
	white = "0xffffff90"
	lightblue = "0x1f78b490"
	lightorange = "0xff7f0090"
	lightgreen = "0x33a02c90"
	lightred = "0xe31a1c90"
	violet = "0x6a3d9a90"
	purple = violet
	lightbrown = "0xb1592890"
	paleblue = "0x8080ff90"
	palegreen = "0x8ec65e90"
	palered = "0xc7675890"
	paleorange = "0xca956090"
	paleviolet = "0xc893f090"
	greybrown = "0x7f5f6790"
	lightviolet = "0xb160c990"
	palebluegreen = "0x5fc69f90"
	pinkviolet = "0xc9608890"
	cyan = "0x8dd3c790"
	paleyellow = "0xffffb390"
	greyblue = "0xbebada90"
	lightorange2 = "0xfb807290"
	lightblue2 = "0x80b1d390"
	lightorange2 = "0xfdb46290"
	lightgreen2 = "0xb3de6990"
	greypink = "0xfccde590"
	lightgrey = "0xd9d9d990"
	lightviolet = "0xbc80bd90"
	greygreen = "0xccebc590"
	lightyellow = "0xffed6f90"
	lightblue2 = "0xa6cee390"
	lightgreen3 = "0xb2df8a90"
	lightpink = "0xfb9a9990"
	lightorange3 = "0xfdbf6f90"
	greyviolet = "0xcab2d690"
	paleyellow = "0xffff9990"
}
function addnode(id, name, color){
	# remove placeholder for nodes spawned from out of order links
	if(id in lnode){
		delete node[lnode[id]]
		delete lnode[id]
	}
	node[name] = id
	lnode[id] = name
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
function addedge(id, name){
	edge[name] = id
	ledge[id] = name
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
	if(code == "FGD135"){	# wing attack plan R
		if(deferred != ""){
			print deferred
			deferred = ""
		}
		crm114 = 1
	}else if(code == "KDH037")	# all planes to report position
		standby++
	else if(code == "OPL753")	# wing to contact base immediately
		deferred = deferred "R\n"
	else if(code == "FJJ142"){	# mission completed returning
		if(--standby == 0)
			print "R"
	}
	#else if(code == "FHJ142")	# wing to proceed to targets
	#else if(code == "OPL753")	# wing to contact base immediately
}
function exportlayout(f){
	print "o", f
}
function importlayout(f){
	if(crm114 == 1)
		print "i", f
	else{
		deferred = deferred "i " f "\n"
		cmd("KDH037")
	}
}
function readcsv(f){
	if(crm114 == 1)
		print "f", f
	else{
		deferred = deferred "f " f "\n"
		cmd("KDH037")
	}
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
