BEGIN{
	black = 0x000000
	blue = 0x1f78b4
	bluegreen = 0x5fc69f
	brightblue = 0x0000ff
	brightgreen = 0x00ff00
	brightpurple = 0x00ffff
	brightred = 0xff0000
	brightyellow = 0xffff00
	brown = 0xb15928
	cyan = 0x7acdcd
	darkgray = 0x3f3f3f
	darkgreen = 0x33a02c
	darkpink = 0xc96088
	darkpurple = 0x6a3d9a
	gray = 0x777777
	green = 0x8ec65e
	greyblue = 0xbebada
	greybrown = 0x7f5f67
	greygreen = 0xccebc5
	greyred = 0xc76758
	lightblue = 0x80b1d3
	lightbrown = 0xca9560
	lightgray = 0xc0c0c0
	lightgreen = 0xd5f65f
	lightgrey = 0xd9d9d9
	lightorange = 0xfdb462
	lightpink = 0xfccde5
	lightpurple = 0xbc80bd
	lightred = 0xfb8072
	lightteal = 0x8dd3c7
	orange = 0xff7f00
	paleblue = 0xa6cee3
	palegray = 0xdcdcdc
	palegreen = 0xb2df8a
	paleorange = 0xfdbf6f
	palepurple = 0xc893f0
	palered = 0xfb9a99
	paleviolet = 0xcab2d6
	paleyellow = 0xffffb3
	purple = 0xb160c9
	purpleblue = 0x8080ff
	red = 0xe31a1c
	teal = 0x009696
	violet = purple
	white = 0xffffff
	yellow = 0xffed6f
	srand()
	OFS = "\t"
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
	if(!checknodename(name))
		return
	if(length(color) == 0){
		print "E no such color for node: " name
		return
	}
	CL[name] = color
	printf "c\t%s\t0x%x\n", node[name], color
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
function cmd(code){
	if(code == "FGD135"){	# wing attack plan R
		if(!noreset && relayout)
			deferred = deferred "R\n"
		if(deferred != ""){
			print deferred
			deferred = ""
		}
		crm114 = 1
	}else if(!crm114){
		if(code == "FHJ142"){	# wing to proceed to targets
			relayout = 1
			# FIXME
			print "R"
		}
	}
}
function exportlayout(f){
	print "o", f
}
function importlayout(f){
	if(crm114 == 1)
		print "i", f
	else{
		deferred = deferred "i\t" f "\n"
		noreset = 1
	}
}
function readcsv(f){
	if(crm114 == 1)
		print "f", f
	else{
		deferred = deferred "f\t" f "\n"
		relayout = 1
	}
}
function checknodeid(id){
	if(!(id in label)){
		print "E no such nodeid: " id
		return 0
	}
	return 1
}
function checknodename(name){
	if(!(name in node)){
		print "E no such node: " name
		return 0
	}
	return 1
}
function edgeinfo(id,	s, u, v, a, b){
	if(!((id "\x1c" 1) in edge)){
		print "E no such edge id: " id
		return
	}
	a = edge[id,1]
	b = edge[id,2]
	u = a >> 1
	v = b >> 1
	if(!checknodeid(a >> 1) || !checknodeid(b >> 1))
		return
	u = label[a >> 1] (a & 1 ? "-" : "+")
	v = label[b >> 1] (b & 1 ? "-" : "+")
	s = "Edge: " u v
	if(id in cigar)
		s = s ", CIGAR=" cigar[id]
	print "I", s
}
function findnode(name,	id){
	if(!checknodename(name))
		return
	id = node[name]
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
			s = s (length(s) == 0 ? "" : ",") name
			m = length(s)
		}else if(n == 0){
			n++
			s = s ",..."
		}
	}
	if(length(selected) == 1)
		return s ", length=" l
	return s "; total length=" l " in " length(selected) " nodes"
}
function nodeinfostr(id,	s){
	name = label[id]
	s = name
	if(name in LN)
		s = s ", length=" LN[name]
	return s
}
function selinfo(){
	if(length(selected) != 0)
		print "s", selinfostr()
	#else
	#	print "E bug: nothing selected"
}
function nodeinfo(id,	name, s){
	if(!checknodeid(id))
		return
	print "I", "Node: " nodeinfostr(id)
}
function deselect(dontkill,	id){
	if(length(selected) == 0)
		return
	for(id in selected)
		printf "c\t%s\t0x%x\n", id, CL[label[id]]
	delete selected
	if(!dontkill)
		print "s"
}
function deselectnodebyid(id){
	if(!checknodeid(id))
		return
	if(!(id in selected)){
		print "E deselect: not selected: " id
		return
	}
	if(length(selected) == 1){
		deselect()
		return
	}
	delete selected[id]
	printf "c\t%s\t0x%x\n", id, CL[label[id]]
}
function deselectnode(name,	id){
	if(!checknodename(name))
		return
	deselectnodebyid(node[name])
}
function selectnodebyid(id,	name, s, l){
	if(!checknodeid(id) || id in selected)
		return
	selected[id] = 1
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
	deselect(1)
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
