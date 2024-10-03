BEGIN{
	black = "0x000000"
	blue = "0x1f78b4"
	bluegreen = "0x5fc69f"
	brightblue = "0x0000ff"
	brightgreen = "0x00ff00"
	brightpurple = "0x00ffff"
	brightred = "0xff0000"
	brightyellow = "0xffff00"
	brown = "0xb15928"
	cyan = "0x00ffff"
	darkgray = "0x3f3f3f"
	darkgreen = "0x33a02c"
	darkpink = "0xc96088"
	darkpurple = "0x6a3d9a"
	gray = "0x777777"
	green = "0x8ec65e"
	greyblue = "0xbebada"
	greybrown = "0x7f5f67"
	greygreen = "0xccebc5"
	greyred = "0xc76758"
	lightblue = "0x80b1d3"
	lightbrown = "0xca9560"
	lightgray = "0xc0c0c0"
	lightgreen = "0xd5f65f"
	lightgrey = "0xd9d9d9"
	lightorange = "0xfdb462"
	lightpink = "0xfccde5"
	lightpurple = "0xbc80bd"
	lightred = "0xfb8072"
	lightteal = "0x8dd3c7"
	orange = "0xff7f00"
	paleblue = "0xa6cee3"
	palegray = "0xdcdcdc"
	palegreen = "0xb2df8a"
	paleorange = "0xfdbf6f"
	palepurple = "0xc893f0"
	palered = "0xfb9a99"
	paleviolet = "0xcab2d6"
	paleyellow = "0xffffb3"
	purple = "0xb160c9"
	purpleblue = "0x8080ff"
	red = "0xe31a1c"
	teal = "0x009696"
	violet = purple
	white = "0xffffff"
	yellow = "0xffed6f"
	srand()
}
function addnode(id, name, color){
	# remove placeholder for nodes spawned from out of order links
	if(id in label){
		delete node[label[id]]
		delete label[id]
	}
	node[name] = id
	label[id] = name
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
	delete label[node[name]]
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
function nodeinfo(id){
	name = label[id]
	s = name
	if(name in LN)
		s = s ", length=" LN[name]
	print "I", "Node:", s
}
function edgeinfo(id){
	name = ledge[id]
	s = name
	sub("\x1c", " ", s)
	if(name in cigar)
		s = s ", CIGAR=\"" cigar[name] "\""
	print "I", "Edge:", s
}
function initx(name, x){
	if(!(name in node)){
		print "E no such node", name
		return
	}
	x0[name] = x
	print "x", node[name], x
}
function inity(name, y){
	if(!(name in node)){
		print "E no such node", name
		return
	}
	y0[name] = y
	print "y", node[name], y
}
function fixx(name, x){
	if(!(name in node)){
		print "E no such node", name
		return
	}
	fx[name] = x
	print "X", node[name], x
}
function fixy(name, y){
	if(!(name in node)){
		print "E no such node", name
		return
	}
	fy[name] = y
	print "Y", node[name], y
}
# too complicated if nested, and temporary anyway; would ideally expand
# other tags into tag[i]
function subexpr(s, fn,	i, j, t, pred){
	i = match(s, "\][ 	]*=[^=]")
	# error check: i not 0; only sub expression up to } or ; etc
	pred = substr(s, 1, i-1)
	s = substr(s, RSTART+RLENGTH-1)
	$0 = "for(i in node) if(" pred "){ " fn "(i, " s ")}"
}
crm114 && /^[	 ]*[A-Za-z][A-Za-z0-9 ]*\[.*\] *= */{
	i = index($0, "[")
	v = substr($0, 1, i - 1)
	s = substr($0, i + 1)
	if(v == "CL" || v ~ /[Cc][Oo][Ll][Oo][Rr]/)
		subexpr(s, "nodecolor")
	else if(v == "x0")
		subexpr(s, "initx")
	else if(v == "y0")
		subexpr(s, "inity")
	else if(v == "fx")
		subexpr(s, "fixx")
	else if(v == "fy")
		subexpr(s, "fixy")
}
{
	eval("{" $0 "}")
}
