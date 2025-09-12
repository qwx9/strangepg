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
	paleyellow = 0xffeb4900
	purple = 0xb160c900
	purpleblue = 0x8080ff00
	red = 0xe31a1c00
	tan = 0xd2b48c00
	teal = 0x00969600
	violet = purple
	white = 0xffffff00
	yellow = 0xfbf50000
	yellowgreen = 0xc5fa0000
	translucent = 0xeeeeee30
	gmap[n++] = blue
	gmap[n++] = green
	gmap[n++] = red
	gmap[n++] = purple
	gmap[n++] = brown
	gmap[n++] = orange
	gmap[n++] = bluegreen
	gmap[n++] = yellow
	gmap[n++] = purpleblue
	gmap[n++] = greyred
	gmap[n++] = lightbrown
	gmap[n++] = palepurple
	gmap[n++] = greybrown
	gmap[n++] = darkpink
	gmap[n++] = darkgreen
	gmap[n++] = darkpurple
	gmap[n++] = lightteal
	gmap[n++] = paleyellow
	gmap[n++] = greyblue
	gmap[n++] = lightred
	gmap[n++] = lightblue
	gmap[n++] = lightorange
	gmap[n++] = lightgreen
	gmap[n++] = lightpink
	gmap[n++] = lightgrey
	gmap[n++] = lightpurple
	gmap[n++] = greygreen
	gmap[n++] = paleblue
	gmap[n++] = palegreen
	gmap[n++] = palered
	gmap[n++] = paleorange
	gmap[n++] = paleviolet
	gmap[n] = cyan
	n = 0
	cmap[n++] = purpleblue
	cmap[n++] = green
	cmap[n++] = greyred
	cmap[n++] = lightbrown
	cmap[n++] = palepurple
	cmap[n++] = greybrown
	cmap[n++] = purple
	cmap[n++] = bluegreen
	cmap[n++] = darkpink
	cmap[n++] = blue
	cmap[n++] = orange
	cmap[n++] = darkgreen
	cmap[n++] = red
	cmap[n++] = darkpurple
	cmap[n++] = brown
	cmap[n++] = lightteal
	cmap[n++] = greyblue
	cmap[n++] = tan
	cmap[n++] = lightgreen
	cmap[n++] = lightred
	cmap[n++] = lightblue
	cmap[n++] = lightorange
	cmap[n++] = lightpurple
	cmap[n++] = lightgrey
	cmap[n++] = lightpink
	cmap[n++] = greygreen
	cmap[n++] = yellow
	cmap[n++] = paleblue
	cmap[n++] = palered
	cmap[n++] = palegreen
	cmap[n++] = paleorange
	cmap[n++] = paleviolet
	cmap[n] = cyan
	OFS = "\t"
}
function setdefcols(	n, i){
	if((n = length(cmap)) <= 0){
		print "E", "no colors to hand"
		return
	}
	for(i in CL)
		if(!(i in CL))
			CL[i] = cmap[i % n]
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
		loadbatch()
		if(!crm114)
			setdefcols()
		print "R"
		crm114 = 1
	}else if(code == "FGD135"){	# wing attack plan R
		if(crm114 || fnr < nd + 1)
			return
		crm114 = 1
		loadbatch()
		setdefcols()
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
function readctab(f){
	if(crm114 == 1)
		print "t", f
	else
		deferred[++nd] = "t\t" f "\n"
}
function readcsv(f){
	if(crm114 == 1)
		print "f", f
	else
		deferred[++nd] = "f\t" f "\n"
}
function checknodeid(i){
	# FIXME: test id in node
	return 1
}
function checknodename(name){
	if(!(name in id)){
		print "E\tno such node: " name
		return 0
	}
	return 1
}
function edgeinfostr(u, v, a, b,	s){
	s = node[u] a node[v] b
	if(i in cigar)
		s = s ", CIGAR=" cigar[i]
	return s
}
function edgeinfo(e,	u, v, a, b){
	u = e >> 32
	v = e >> 2 & 0x3fffffff
	if(!checknodeid(u) || !checknodeid(v))
		return
	a = e & 1 ? "-" : "+"
	b = e & 2 ? "-" : "+"
	info("Edge: " edgeinfostr(u, v, a, b))
}
function findnode(name,	i){
	if(!checknodename(name))
		return
	i = id[name]
	selectnodebyid(i)
	print "N", i
}
# FIXME: cache it?
function nodeinfostr(i,	a, t){
	s = node[i] ", LN=" LN[i]
	xxx = ""	# has to be global for eval
	for(a in ATTACHED){
		if(a == "degree" || a == "LN" || a == "CL")
			continue
		eval("{ xxx = (i in " a ") ? " a "[i] : \"\" }")
		if(xxx != "")
			s = s " " a "=" xxx
	}
	return s
}
function selinfostr(	i, l, n, m, s){
	if(length(selected) == 0)
		return ""
	# very deliberate and assuming nodes for now
	if(length(selected) == 0)
		for(i in selected)
			return nodeinfostr(i)
	l = 0
	s = ""
	n = m = 0
	for(i in selected){
		l += LN[i]
		if(m < 5){
			m = length(s)
			s = s (m == 0 ? "" : ",") node[i]
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
function nodeinfo(i,	name, s){
	if(!checknodeid(i))
		return
	s = "Node: " nodeinfostr(i)
	info(s)
}
function deselect(	i){
	selinfo = ""
	for(i in selected)
		CL[i] = CL[i]
	delete selected
}
function deselectnodebyid(i,	col){
	if(!checknodeid(i))
		return
	if(!(i in selected)){
		print "E\tdeselect: not selected: " i
		return
	}
	if(length(selected) == 1){
		deselect()
		return
	}
	delete selected[i]
	selinfo = selinfostr()
	CL[i] = CL[i]
}
function deselectnode(name,	i){
	if(!checknodename(name))
		return
	deselectnodebyid(id[name])
}
function showselected(){
	selinfo = selinfostr()
	refresh()
}
function selectnodebyid(i, noshow){
	if(!checknodeid(i) || i in selected)
		return
	selected[i] = 0
	if(!noshow)
		showselected()
}
function selectall(i){
	for(i in LN)
		selectnodebyid(i, 1)
}
function selectnode(name){
	if(!checknodename(name))
		return
	selectnodebyid(id[name], 1)	# FIXME
}
function toggleselect(i){
	if(!checknodeid(i))
		return
	if(i in selected)
		deselectnodebyid(i)
	else
		selectnodebyid(i)
}
function reselectnode(i){
	if(!checknodeid(i))
		return
	deselect()
	selectnodebyid(i)
}
# FIXME: can't be variadic, unless in C
#function collapse(i){
#	if(i != ""){
#		if(!checknodeid(i))
#			return
#		collapse1(i)
#	}else{
#		for(i in selected)
#			collapse1(i)
#	}
#	commit()
#	deselect()
#}
function expand(i){
	if(i != ""){
		if(!checknodeid(i))
			return
		expand1(i)
	}else if(length(selected) > 0){
		for(i in selected)
			expand1(i)
	}else
		expandall()
	commit()
	selectall()
	deselect()
}
function groupby(tag, incl, 	acc, n, m){
	if((n = length(gmap)) <= 0){
		print "E", "no colors to hand"
		return
	}
	delete acc
	m = 0
	# FIXME: error handling in nested evals?
	if(incl == "")
		eval("{for(i in "tag"){ c = "tag"[i]; if(!(c in acc)) acc[c] = m++; CL[i] = gmap[acc[c] % n] } }")
	else
		eval("{for(i in CL){ if(!(i in "tag")){ c = translucent }else{ c = "tag"[i]; if(c !~ /"incl"/) c = translucent; else{ if(!(c in acc)) acc[c] = m++; c = gmap[acc[c] % n] }}; CL[i] = c}}")
	if(m > n)
		print "E", "more categories than colors"
}
function initx(name, x){
	if(!checknodename(name))
		return
	x0[name] = x
	print "x", id[name], x
}
function inity(name, y){
	if(!checknodename(name))
		return
	y0[name] = y
	print "y", id[name], y
}
function initz(name, z){
	if(!checknodename(name))
		return
	z0[name] = z
	print "z", id[name], z
}
function fixx(name, x){
	if(!checknodename(name))
		return
	fx[name] = x
	print "X", id[name], x
}
function fixy(name, y){
	if(!checknodename(name))
		return
	fy[name] = y
	print "Y", id[name], y
}
function fixz(name, z){
	if(!checknodename(name))
		return
	fz[name] = z
	print "Z", id[name], z
}
function quit(){
	print "!"
}
# too complicated if nested, and temporary anyway; would ideally expand
# other tags into tag[i]
function subexpr(s, v, fn,	i, pred){
	i = match(s, "\][ 	]*=[^=]")
	# error check: i not 0; only sub expression up to } or ; etc
	pred = substr(s, 1, i-1)
	s = substr(s, RSTART+RLENGTH-1)
	if(fn != "")
		$0 = "for(i in " v ") if(" pred "){ " fn "(i, " s ")}"
	else
		$0 = "for(i in " v ") if(" pred "){ " v "[i]=" s "}"
}
crm114 && /^[	 ]*[A-Za-z][A-Za-z0-9 ]*\[.*\] *= */{
	i = index($0, "[")
	v = substr($0, 1, i - 1)
	s = substr($0, i + 1)
	if(v == "x0")
		subexpr(s, v, "initx")
	else if(v == "y0")
		subexpr(s, v, "inity")
	else if(v == "fx")
		subexpr(s, v, "fixx")
	else if(v == "fy")
		subexpr(s, v, "fixy")
	else
		subexpr(s, v)
}
{
	eval("{" $0 "}")
}
