BEGIN{
	black = 0x000000c0
	blue = 0x1f78b4c0
	bluegreen = 0x5fc69fc0
	brightblue = 0x0000ffc0
	brightgreen = 0x00ff00c0
	brightpurple = 0x00ffffc0
	brightred = 0xff0000c0
	brightyellow = 0xffff00c0
	brown = 0xb15928c0
	cyan = 0x7acdcdc0
	darkgray = 0x3f3f3fc0
	darkgreen = 0x33a02cc0
	darkpink = 0xc96088c0
	darkpurple = 0x6a3d9ac0
	gray = 0x777777c0
	green = 0x8ec65ec0
	greyblue = 0xbebadac0
	greybrown = 0x7f5f67c0
	greygreen = 0xccebc5c0
	greyred = 0xc76758c0
	lightblue = 0x80b1d3c0
	lightbrown = 0xca9560c0
	lightgray = 0xc0c0c0c0
	lightgreen = 0xd5f65fc0
	lightgrey = 0xd9d9d9c0
	lightorange = 0xfdb462c0
	lightpink = 0xfccde5c0
	lightpurple = 0xbc80bdc0
	lightred = 0xfb8072c0
	lightteal = 0x8dd3c7c0
	orange = 0xff7f00c0
	paleblue = 0xa6cee3c0
	palegray = 0xdcdcdcc0
	palegreen = 0xb2df8ac0
	paleorange = 0xfdbf6fc0
	palepurple = 0xc893f0c0
	palered = 0xfb9a99c0
	paleviolet = 0xcab2d6c0
	paleyellow = 0xffffb3c0
	purple = 0xb160c9c0
	purpleblue = 0x8080ffc0
	red = 0xe31a1cc0
	teal = 0x009696c0
	violet = purple
	white = 0xffffffc0
	yellow = 0xffed6fc0
	srand()
	OFS = "\t"
}
function cmd(code){
	if(code == "FHJ142"){	# wing to proceed to targets
		if(++fnr == nd + 1)
			cmd("FGD135")
		else{
			print deferred[fnr]
			loadbatch()
		}
	}else if(code == "FGD135"){	# wing attack plan R
		if(fnr < nd + 1)
			return
		crm114 = 1
		loadall()
		if(die)
			quit()
		if(noreset)
			print "r"
		else
			print "R"
		noreset = 0
	}else if(code == "FJJ142"){	# mission completed, returning
		die = 1
		cmd("FHJ142")
	}else if(code == "OPL753"){	# wing to contact base immediately
		loadall()
		print "R"
	}
}
function exportlayout(f){
	print "o", f
}
function importlayout(f, force){
	if(crm114 == 1 || force)
		print "i", f
	else{
		deferred[++nd] = "i\t" f "\n"
		noreset = 1
	}
}
function readcsv(f, force){
	if(crm114 == 1 || force)
		print "f", f
	else
		deferred[++nd] = "f\t" f "\n"
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
function delnode(name){
	if(!checknodename(name))
		return
	delete label[node[name]]
	delete node[name]
}
function addedge(id, u, v){
	edge[id,1] = u << 1 & (v & 1)
	edge[id,2] = v >> 1
}
function checknodeid(id){
	if(!(id in label)){
		print "E\tno such nodeid: " id
		return 0
	}
	return 1
}
function checknodename(name){
	if(!(name in node)){
		print "E\tno such node: " name
		return 0
	}
	return 1
}
function edgeinfostr(id,	s, u, v, a, b){
	if(!((id "\x1c" 1) in edge)){
		print "E\tno such edge id: " id
		return ""
	}
	a = edge[id,1]
	b = edge[id,2]
	u = a >> 1
	v = b >> 1
	if(!checknodeid(a >> 1) || !checknodeid(b >> 1))
		return
	u = label[a >> 1] (a & 1 ? "-" : "+")
	v = label[b >> 1] (b & 1 ? "-" : "+")
	s = u v
	if(id in cigar)
		s = s ", CIGAR=" cigar[id]
	return s
}
function edgeinfo(id,	s){
	if((s = edgeinfostr(id)) == "")
		return
	info("Edge :" s)
}
function findnode(name,	id){
	if(!checknodename(name))
		return
	id = node[name]
	selectnodebyid(id)
	print "N", id
}
function selinfostr(	id, name, l, n, m, s){
	if(length(selected) == 0)
		return ""
	l = 0
	s = ""
	n = m = 0
	for(id in selected){
		name = label[id]
		if(name in LN)
			l += LN[name]
		if(m < 28){
			m = length(s)
			s = s (m == 0 ? "" : ",") name
		}else if(n == 0){
			n++
			s = s ",..."
		}
	}
	s = "Selected: " s
	if(length(selected) == 1)
		return s ", length=" l
	return s "; total length=" l " in " length(selected) " nodes"
}
function nodeinfo(id,	name, s){
	if(!checknodeid(id))
		return
	name = label[id]
	s = "Node: " name
	if(name in LN)
		s = s ", length=" LN[name]
	info(s)
}
function deselect(	id){
	selinfo = ""
	for(id in selected)
		unshow(id, selected[id])
	delete selected
}
function deselectnodebyid(id,	col){
	if(!checknodeid(id))
		return
	if(!(id in selected)){
		print "E\tdeselect: not selected: " id
		return
	}
	if(length(selected) == 1){
		deselect()
		return
	}
	col = selected[id]
	delete selected[id]
	selinfo = selinfostr()
	unshow(id, col)
}
function deselectnode(name,	id){
	if(!checknodename(name))
		return
	deselectnodebyid(node[name])
}
function selectnodebyid(id, noshow){
	if(!checknodeid(id) || id in selected)
		return
	selected[id] = CL[label[id]]
	if(!noshow){
		selinfo = selinfostr()
		refresh()
	}
}
function selectnode(name, id){
	if(!checknodename(name))
		return
	id = node[name]
	selectnodebyid(id)
}
function toggleselect(id){
	if(!checknodeid(id))
		return
	if(id in selected)
		deselectnodebyid(id)
	else
		selectnodebyid(id)
}
function reselectnode(id){
	if(!checknodeid(id))
		return
	deselect()
	selectnodebyid(id)
}
function initx(name, x){
	if(!checknodename(name))
		return
	x0[name] = x
	print "x", node[name], x
}
function inity(name, y){
	if(!checknodename(name))
		return
	y0[name] = y
	print "y", node[name], y
}
function fixx(name, x){
	if(!checknodename(name))
		return
	fx[name] = x
	print "X", node[name], x
}
function fixy(name, y){
	if(!checknodename(name))
		return
	fy[name] = y
	print "Y", node[name], y
}
function quit(){
	print "!"
	exit
}
# too complicated if nested, and temporary anyway; would ideally expand
# other tags into tag[i]
function subexpr(s, v, fn,	i, j, t, pred){
	i = match(s, "\][ 	]*=[^=]")
	# error check: i not 0; only sub expression up to } or ; etc
	pred = substr(s, 1, i-1)
	s = substr(s, RSTART+RLENGTH-1)
	# my god man, implement pointers
	eval("{w = length(" v ") == 0 ? \"node\" : v}")
	$0 = "for(i in " w ") if(" pred "){ " fn "(i, " s ")}"
}
#crm114 && $1 == function"{
#	eval($0)
#	next
#}
crm114 && /^[	 ]*[A-Za-z][A-Za-z0-9 ]*\[.*\] *= */{
	i = index($0, "[")
	v = substr($0, 1, i - 1)
	s = substr($0, i + 1)
	if(v == "CL" || v ~ /[Cc][Oo][Ll][Oo][Rr]/)
		subexpr(s, v, "nodecolor")
	else if(v == "x0")
		subexpr(s, v, "initx")
	else if(v == "y0")
		subexpr(s, v, "inity")
	else if(v == "fx")
		subexpr(s, v, "fixx")
	else if(v == "fy")
		subexpr(s, v, "fixy")
}
{
	eval("{" $0 "}")
}
