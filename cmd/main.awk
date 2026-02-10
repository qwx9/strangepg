BEGIN{
	black = 0x00000000
	blue = 0x1f78b400
	bluegreen = 0x00cc7700
	brightblue = 0x0000ff00
	brightcyan = 0x00ffff00
	brightgreen = 0x00ff0000
	brightpurple = 0xff00ff00
	brightred = 0xff000000
	brightyellow = 0xffff0000
	brown = 0xb1592800
	cyan = 0x4ddfdf00
	darkblue = 0x03328500
	darkgray = 0x3f3f3f00
	darkgreen = 0x549f2c00
	darkgrey = darkgray
	darkpink = 0xd7548900
	darkpurple = 0x6a3d9a00
	darksalmon = 0xe9967a00
	gray = 0x77777700
	grayblue = 0xbebada00
	graybrown = 0x7f5f6700
	graygreen = 0xa5e39500
	grayred = 0xc7675800
	green = 0x74d04100
	grey = gray
	greyblue = grayblue
	greybrown = graybrown
	greygreen = graygreen
	greyred = grayred
	lightblue = 0x80b1d300
	lightbrown = 0xca956000
	lightgray = 0xb6b6b600
	lightgreen = 0x88f03b00
	lightgrey = lightgray
	lightorange = 0xfba72400
	lightpink = 0xfbacd600
	lightpurple = 0xbc80bd00
	lightred = 0xfa3e4300
	lightteal = 0x7fe0cf00
	olive = 0x7ab64100
	orange = 0xff7f0000
	paleblue = 0xa6cee300
	palegray = 0xdcdcdc00
	palegreen = 0x94fb6a00
	paleorange = 0xfdbf6f00
	palepurple = 0xc893f000
	palered = 0xfb696700
	paleviolet = 0xcab2d600
	paleyellow = 0xffeb4900
	pink = 0xffb6c100
	purple = 0xb160c900
	purpleblue = 0x6481fc00
	red = 0xda181a00
	tan = 0xd2b48c00
	teal = 0x00bbbb00
	violet = purple
	white = 0xffffff00
	yellow = 0xe7d94100
	yellowgreen = 0xd5ef0000
	translucent = 0xdfdfdf60
	n = 0
	# named palettes courtesy of colorbrewer
	defgrp[n++] = greyblue
	defgrp[n++] = paleblue
	defgrp[n++] = lightblue
	defgrp[n++] = blue
	defgrp[n++] = purpleblue
	defgrp[n++] = darkblue
	defgrp[n++] = bluegreen
	defgrp[n++] = darkgreen
	defgrp[n++] = green
	defgrp[n++] = lightgreen
	defgrp[n++] = palegreen
	defgrp[n++] = greygreen
	defgrp[n++] = greyred
	defgrp[n++] = palered
	defgrp[n++] = lightred
	defgrp[n++] = red
	defgrp[n++] = orange
	defgrp[n++] = lightorange
	defgrp[n++] = paleorange
	defgrp[n++] = palepurple
	defgrp[n++] = paleviolet
	defgrp[n++] = lightpurple
	defgrp[n++] = purple
	defgrp[n++] = darkpurple
	defgrp[n++] = darkbrown
	defgrp[n++] = greybrown
	defgrp[n++] = brown
	defgrp[n++] = lightbrown
	defgrp[n++] = paleyellow
	defgrp[n++] = yellow
	defgrp[n++] = tan
	defgrp[n++] = lightteal
	defgrp[n++] = lightgrey
	n = 0
	set1[n++] = 0xe41a1c00
	set1[n++] = 0x377eb800
	set1[n++] = 0x4daf4a00
	set1[n++] = 0x984ea300
	set1[n++] = 0xff7f0000
	set1[n++] = 0xffff3300
	set1[n++] = 0xa5452800
	set1[n++] = 0xf781bf00
	set1[n++] = 0x99999900
	n = 0
	set2[n++] = 0x66c2a500
	set2[n++] = 0xfc8d6200
	set2[n++] = 0x8da0cb00
	set2[n++] = 0xe78ac300
	set2[n++] = 0xa6d85400
	set2[n++] = 0xffd92f00
	set2[n++] = 0xe4c49400
	set2[n++] = 0xb3b3b300
	n = 0
	set3[n++] = 0x8dd3c700
	set3[n++] = 0xffffb300
	set3[n++] = 0xbebada00
	set3[n++] = 0xfb807200
	set3[n++] = 0x80b1d300
	set3[n++] = 0xfdb46200
	set3[n++] = 0xb3de6900
	set3[n++] = 0xfccde500
	set3[n++] = 0xd9d9d900
	set3[n++] = 0xbc80bd00
	set3[n++] = 0xccebc500
	set3[n++] = 0xffed6f00
	n = 0
	paired[n++] = 0xa6cee300
	paired[n++] = 0x1f78b400
	paired[n++] = 0xb2df8a00
	paired[n++] = 0x33a02c00
	paired[n++] = 0xfb9a9900
	paired[n++] = 0xe31a1c00
	paired[n++] = 0xfdbf6f00
	paired[n++] = 0xff7f0000
	paired[n++] = 0xcab2d600
	paired[n++] = 0x6a3d9a00
	paired[n++] = 0xffff9900
	paired[n++] = 0xb1592800
	n = 0
	spectral[n++] = 0x5e4fa200
	spectral[n++] = 0x3288bd00
	spectral[n++] = 0x66c2a500
	spectral[n++] = 0xabdda400
	spectral[n++] = 0xe6f59800
	spectral[n++] = 0xffffbf00
	spectral[n++] = 0xfee08b00
	spectral[n++] = 0xfdae6100
	spectral[n++] = 0xf45d4300
	spectral[n++] = 0xd53e4f00
	spectral[n++] = 0x9e014200
	n = 0
	default[n++] = purpleblue
	default[n++] = green
	default[n++] = greyred
	default[n++] = lightbrown
	default[n++] = palepurple
	default[n++] = greybrown
	default[n++] = purple
	default[n++] = lightteal
	default[n++] = darkpink
	default[n++] = blue
	default[n++] = orange
	default[n++] = darkgreen
	default[n++] = red
	default[n++] = darkpurple
	default[n++] = brown
	default[n++] = lightteal
	default[n++] = greyblue
	default[n++] = tan
	default[n++] = darksalmon
	default[n++] = olive
	default[n++] = yellowgreen
	default[n++] = lightgreen
	default[n++] = lightred
	default[n++] = lightblue
	default[n++] = lightorange
	default[n++] = lightpurple
	default[n++] = lightgrey
	default[n++] = lightpink
	default[n++] = greygreen
	default[n++] = yellow
	default[n++] = paleblue
	default[n++] = palered
	default[n++] = palegreen
	default[n++] = paleorange
	default[n++] = paleviolet
	default[n++] = cyan
	OFS = "\t"
	Fgraph = 1<<0	# FIXME: make awk2c a preprocessor in awk,
	Fctab = 1<<1	#   then replace these as constants?
	Fflayout = 1<<2
	Ffctab = 1<<3
	Ffcsv = 1<<4
	Fdie = 1<<5
	Fcrm114 = 1<<15
}
function setdefcols(	n, i){
	n = length(default)
	for(i in CL)
		if(!(i in CL))
			CL[i] = default[i % n]
		else
			CL[i] = CL[i]
}
# FIXME: CL changes currently are still destructive, would be
# nice to be able to actually reset them to the initial values
# FIXME: use after free if using i as temp and not a param
function cmd(code, _i, __i){
	if(code == "OPL753"){		# wing to contact base immediately
		flags |= Fdie
		if(flags & Ffctab == 0)
			flags |= Fctab
	}else if(code == "FGG138"){	# wing report mission completed
		#if(flags & Fcrm114 == 0)
		#	nd--
	}else if(code == "JIK485"){	# mission completed returning
		if(flags & Fcrm114 == 0)
			nd--
		loadbatch()
	}else if(code == "FHJ142"){	# wing to proceed to targets
		if(flags & Fcrm114 == 0)
			nd--
		loadbatch()
	}else if(code == "HGI234"){	# wing holding at fail safe point
		flags |= Fctab
		if(flags & (Fcrm114|Ffctab) == Ffctab)
			nd--
	}else if(code == "FGD135"){	# wing attack plan R
		flags |= Fgraph
		nd = length(deferred)
		for(_i in deferred)
			print deferred[_i]
		delete deferred
		loadbatch()
	}
	if(nd <= 0 && flags & (Fcrm114|Fgraph|Fctab) == (Fgraph|Fctab)){
		loadbatch()
		if(flags & Fdie)
			quit()
		arm()
		setdefcols()	# FIXME: until rnodes are gone
		if(length(deferredexpr) != 0){	# FIXME: after arm because of rnodes
			for(_i=1; _i<=length(deferredexpr); _i++)
				eval("{"deferredexpr[_i]"}")
			delete deferredexpr
		}
		# must be done after arming rnodes
		if(length(layout) > 0){
			flags |= Fflayout
			for(_i=1; _i<=length(layout); _i++)
				print "i\t" layout[_i] "\n"
			delete layout
		}
		flags |= Fcrm114
		print (flags & Fflayout ? "r" : "R")
	}
}
function exportlayout(f){
	print "o", f
}
function importlayout(f){
	if(flags & Fcrm114)
		print "i", f
	else{
		layout[length(layout)+1] = f
		flags |= Fflayout
	}
}
function readctab(f){
	if(flags & Fcrm114)
		print "t", f
	else{
		deferred[length(deferred)+1] = "t\t" f "\n"
		flags |= Ffctab
	}
}
function readcsv(f){
	if(flags & Fcrm114)
		print "f", f
	else{
		deferred[length(deferred)+1] = "f\t" f "\n"
		flags |= Ffcsv
	}
}
function checkselected(	i){
	for(i in selected)
		if(typeof(selected[i]) == 3 && selected[i] == "")
			delete selected[i]
}
function checknodeid(i){
	if(i < 0 || i >= length(node)){
		print "E\id out of bounds: " i
		return 0
	}
	return 1
}
function checknodename(name){
	if(!(name in id)){
		print "E\tno such node: " name
		return 0
	}
	return 1
}
function edgeinfostr(u, v, a, b, e,	s){
	s = node[u] a node[v] b
	if(e == "")
		return s
	xxx = ""	# has to be global for eval
	xxe = e
	for(a in EATTACHED){
		eval("{ xxx = (xxe in " a ") ? " a "[xxe] : \"\" }")
		if(xxx != ""){
			if(a ~ /^e..$/)
				a = substr(a, 2)
			s = s " " a "=" xxx
		}
	}
	return s
}
function edgeinfo(w, e,	u, v, a, b){
	u = w >> 32
	v = w >> 2 & 0x3fffffff
	if(!checknodeid(u) || !checknodeid(v))
		return
	a = w & 1 ? "-" : "+"
	b = w & 2 ? "-" : "+"
	info("Edge: " edgeinfostr(u, v, a, b, e))
}
function findnode(name,	i){
	if(!checknodename(name))
		return
	i = id[name]
	expand1(i, 0)
	commit(1)
	reselectnode(i)
	print "N", i
}
# FIXME: cache it?
function nodeinfostr(i,	a, s){
	s = node[i] ", LN=" LN[i]
	xxx = ""	# has to be global for eval
	xxi = i
	for(a in ATTACHED){
		if(a == "degree" || a == "LN" || a == "CL")
			continue
		eval("{ xxx = (xxi in " a ") ? " a "[xxi] : \"\" }")
		if(xxx != "")
			s = s " " a "=" xxx
	}
	return s
}
function selinfostr(	x, i, l, n, m, s){
	if(length(selected) == 0)
		return ""
	# very deliberate and assuming nodes for now
	if(length(selected) == 1)
		for(i in selected)
			return "Selected: " nodeinfostr(selected[i])
	l = 0
	s = ""
	n = m = 0
	for(x in selected){
		i = selected[x]
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
function deselectnodebyid(i){
	if(!checknodeid(i) || !(i in selected))
		return
	delete selected[i]
}
function selectnodebyid(i){
	if(!checknodeid(i) || i in selected)
		return
	selected[i] = int(i)
}
function showselected(){
	selinfo = selinfostr()
	refresh()
}
function deselect(	x, i){
	selinfo = ""
	delete selected
}
function selectall(i){
	for(i in node)
		selectnodebyid(i)
	showselected()
}
function deselectnode(name,	i){
	if(!checknodename(name))
		return
	deselectnodebyid(id[name])
	showselected()
}
function selectnode(name){
	if(!checknodename(name))
		return
	selectnodebyid(id[name])
	showselected()
}
function reselectnode(i){
	if(!checknodeid(i))
		return
	if(length(selected) > 1 || !(i in selected))
		deselect()
	if(!(i in selected))
		selected[i] = int(i)
}
function toggleselect(i){
	if(!checknodeid(i))
		return
	if(i in selected)
		deselectnodebyid(i)
	else
		selectnodebyid(i)
}
function expand(name, full,	x, i){
	if(name != ""){
		expand1(id[i], !!full)
	}else if(length(selected) > 0){
		for(i in selected)
			expand1(selected[i], !!full)
	}else
		expandall()
	commit()
	deselect()
}
function redraw(){
	print "D"
}
function explode(d,	i){
	if(length(selected) == 0)
		return
	if(d == "" || float(d) <= 0)
		d = 8.0
	for(i in selected)
		explode1(selected[i], d)
	redraw()
}
function groupby(tag, incl, cm,	acc){
	delete acc
	m_ = n_ = 0	# have to be global for eval
	if(cm == "")
		cm = "defgrp"
	else if(!(cm in SYMTAB)){
		print "E\tno such color palette"
		return
	}
	eval("{n_ = length("cm")}")
	if(n_ <= 1){
		print "E\tinvalid color palette"
		return
	}
	if(incl == "")
		eval("{for(i in "tag"){ c = "tag"[i]; if(!(c in acc)){ if(int(c) != 0) acc[c] = c; else acc[c] = m_++}; CL[i] = "cm"[acc[c] % n_] } }")
	else
		eval("{for(i in CL){ if(!(i in "tag")){ c = translucent }else{ c = "tag"[i]; if(c !~ /"incl"/) c = translucent; else{ if(!(c in acc)) acc[c] = m_++; c = "cm"[acc[c] % n_] }}; CL[i] = c}}")
	if(m_ > n_)
		print "warning: more categories than colors"
	delete acc
}
function quit(){
	print "Q"
	exit
}
function longform(){
	n = index($0, "[")
	v = substr($0, 1, n - 1)
	s = substr($0, n + 1)
	n = match(s, "\][ \t]*=[^=]")
	pred = substr(s, 1, n-1)
	s = substr(s, RSTART+RLENGTH-1)
	# nested i won't be recognized either
	if(pred ~ /^[ \t]*i[ \t]*$/)
		$0 = "for(i in " v "){ " v "[i]=" s "}"
	else if(pred !~ /^[ \t]*("[^"]+"|[a-zA-Z0-9][a-zA-Z0-9_]*|[a-zA-Z0-9][a-zA-Z0-9_]*\[["a-zA-Z0-9_ \t]+\])[ \t]*$/)
		$0 = "for(i in " v ") if(" pred "){ " v "[i]=" s "}"
}
flags & Fcrm114 && /^[A-Za-z0-9_ \t]+\[.+\][ \t]*=[^=]/{
	longform()
}
$1 == "defer"{
	if(flags & Fcrm114)
		next
	$1 = ""
	if(/^[A-Za-z0-9_ \t]+\[.+\][ \t]*=[^=]/)
		longform()
	deferredexpr[length(deferredexpr)+1] = $0
	next
}
{
	ns = length(selected)
	eval("{" $0 "}")
	if(ns != length(selected) && length(selected) != 0)
		checkselected()
}
