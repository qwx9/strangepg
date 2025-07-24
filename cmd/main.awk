BEGIN{
	black = 0x00000000
	blue = 0x1f78b400
	bluegreen = 0x5fc69f00
	brightblue = 0x0000ff00
	brightgreen = 0x00ff0000
	brightpurple = 0x00ffff00
	brightred = 0xff000000
	brightyellow = 0xffff0000
	brown = 0xb1592800
	cyan = 0x7acdcd00
	darkblue = 0x03328500
	darkgray = 0x3f3f3f00
	darkgrey = darkgray
	darkgreen = 0x33a02c00
	darkpink = 0xc9608800
	darkpurple = 0x6a3d9a00
	darksalmon = 0xe9967a00
	gray = 0x77777700
	grayblue = 0xbebada00
	graybrown = 0x7f5f6700
	graygreen = 0xccebc500
	grayred = 0xc7675800
	grey = gray
	greyblue = grayblue
	greybrown = graybrown
	greygreen = graygreen
	greyred = grayred
	green = 0x8ec65e00
	lightblue = 0x80b1d300
	lightbrown = 0xca956000
	lightgray = 0xd9d9d900
	lightgreen = 0xd5f65f00
	lightgrey = lightgray
	lightorange = 0xfdb46200
	lightpink = 0xfccde500
	lightpurple = 0xbc80bd00
	lightred = 0xfb807200
	lightteal = 0x8dd3c700
	orange = 0xff7f0000
	paleblue = 0xa6cee300
	palegray = 0xdcdcdc00
	palegreen = 0xb2df8a00
	paleorange = 0xfdbf6f00
	palepurple = 0xc893f000
	palered = 0xfb9a9900
	paleviolet = 0xcab2d600
	paleyellow = 0xffffb300
	purple = 0xb160c900
	purpleblue = 0x8080ff00
	red = 0xe31a1c00
	teal = 0x00969600
	violet = purple
	white = 0xffffff00
	yellow = 0xffed6f00
	translucent = 0xeeeeee30
	cmap[n++] = blue
	cmap[n++] = green
	cmap[n++] = red
	cmap[n++] = purple
	cmap[n++] = brown
	cmap[n++] = orange
	cmap[n++] = bluegreen
	cmap[n++] = yellow
	cmap[n++] = purpleblue
	cmap[n++] = greyred
	cmap[n++] = lightbrown
	cmap[n++] = palepurple
	cmap[n++] = greybrown
	cmap[n++] = darkpink
	cmap[n++] = darkgreen
	cmap[n++] = darkpurple
	cmap[n++] = lightteal
	cmap[n++] = paleyellow
	cmap[n++] = greyblue
	cmap[n++] = lightred
	cmap[n++] = lightblue
	cmap[n++] = lightorange
	cmap[n++] = lightgreen
	cmap[n++] = lightpink
	cmap[n++] = lightgrey
	cmap[n++] = lightpurple
	cmap[n++] = greygreen
	cmap[n++] = paleblue
	cmap[n++] = palegreen
	cmap[n++] = palered
	cmap[n++] = paleorange
	cmap[n++] = paleviolet
	cmap[n] = cyan
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
	}else if(code == "FJJ142"){	# mission completed, returning
		die = 1
		cmd("FHJ142")
	}else if(code == "OPL753"){	# wing to contact base immediately
		loadall()
		print "R"
		crm114 = 1
	}else if(code == "FGD135"){	# wing attack plan R
		if(crm114 || fnr < nd + 1)
			return
		crm114 = 1
		loadall()
		if(die)
			quit()
		print (noreset ? "r" : "R")
	}
}
function exportlayout(f){
	print "o", f
}
function importlayout(f){
	if(crm114 == 1)
		print "i", f
	else{
		deferred[++nd] = "i\t" f "\n"
		noreset = 1
	}
}
function readcsv(f){
	if(crm114 == 1)
		print "f", f
	else
		deferred[++nd] = "f\t" f "\n"
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
function edgeinfostr(i,	e, u, v, a, b, s){
	e = realedge(i)
	u = e >> 32
	v = e >> 2 & 0x3fffffff
	a = e & 1 ? "-" : "+"
	b = e & 2 ? "-" : "+"
	if(!checknodeid(u) || !checknodeid(v))
		return
	s = label[u] a label[v] b
	if(i in cigar)
		s = s ", CIGAR=" cigar[i]
	return s
}
function edgeinfo(i, s){
	if((s = edgeinfostr(i)) == "")
		return
	info("Edge: " s)
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
function showselected(){
	selinfo = selinfostr()
	refresh()
}
function selectnodebyid(id, noshow){
	if(!checknodeid(id) || id in selected)
		return
	selected[id] = CL[label[id]]
	if(!noshow)
		showselected()
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
# FIXME: can't be variadic, unless in C
#function collapse(id){
#	if(id != ""){
#		if(!checknodeid(id))
#			return
#		collapse1(id)
#	}else{
#		for(id in selected)
#			collapse1(id)
#	}
#	commit()
#	deselect()
#}
function expand(id){
	if(id != ""){
		if(!checknodeid(id))
			return
		expand1(id)
	}else if(length(selected) > 0){
		for(id in selected)
			expand1(id)
	}else
		expandall()
	commit()
	deselect()
}
# FIXME: randomize
function mkcolormap(	i, colors){
	for(i in CL){
		c = CL[i]
		if(!(c in colors)){
			colors[c] = ""
			cmap[n++] = c
		}
	}
	delete colors
}
function groupby(tag, incl, 	acc, n, m){
	if(length(cmap) == 0)
		mkcolormap()
	if((n = length(cmap)) <= 0){
		print "E", "no colors to hand"
		return
	}
	delete acc
	m = 0
	# FIXME: error handling in nested evals?
	if(incl == "")
		eval("{for(i in "tag"){ c = "tag"[i]; if(!(c in acc)) acc[c] = m++; nodecolor(i, cmap[acc[c] % n]) } }")
	else
		eval("{for(i in CL){ if(!(i in "tag")){ c = translucent }else{ c = "tag"[i]; if(c !~ /"incl"/) c = translucent; else{ if(!(c in acc)) acc[c] = m++; c = cmap[acc[c] % n] }}; nodecolor(i, c)}}")
	if(m > n)
		print "E", "more categories than colors"
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
function initz(name, z){
	if(!checknodename(name))
		return
	z0[name] = z
	print "z", node[name], z
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
function fixz(name, z){
	if(!checknodename(name))
		return
	fz[name] = z
	print "Z", node[name], z
}
function quit(){
	print "!"
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
	i = ""
	eval("{" $0 "}")
}
