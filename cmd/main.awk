BEGIN{
	opx = "[!-#%-\\/:-<>-@\\[-\\^`\\{-~]"
	namex = "[^0-9" opx "][^" opx "]*"
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
# for now, doing everything with id's, not names
function nodecolor(id, c){
	if(!(id in lnode)){
		print "E no such node"
		return
	}
	color[lnode[id]] = c
	print "C", id, c
}

function tokenize(){
	gsub("[ 	]", "")
	gsub("==", " == ")
	gsub(opx, " & ")
	gsub("[\\(\\[\\],#]", " & ")
}
function parse(){
}
$1 == "n"{
	if($2 in lnode)
		delete node[lnode[$2]]
	node[$3] = $2
	lnode[$2] = $3
	if(NF > 3)
		color[$2] = $4
	next
}
$1 == "e"{
	e = $3 "\x1c" $4
	edge[e] = $2
	ledge[$2] = e
	next
}
$1 == "d"{
	delete node[lnode[$2]]
	delete lnode[$2]
	next
}
$1 == "c"{
	id = ledge[$2]
	delete edge[id]
	delete ledge[$2]
	next
}
$1 == "C"{
	id = edge[$2,$3]
	delete ledge[id]
	delete edge[$2,$3]
	next
}
# FIXME: not distinguishing between node/edge? should we?
$1 == "s"{
	if($2 == "NLN"){ NLN[$3] = $4 }
	else if($2 == "cig"){ cig[$3] = $4 }
	else if($2 == "color"){ color[$3] = $4 }
	# maybe this alone is sufficient?
	# FIXME: useless if eval works
	else{
		k = $2 "\1xc" $3
		if(!(k in sym)){
			lhs[$2]++
			rhs[$2] = rhs[$2] "\1xc" $3
		}
		sym[k] = $4
	}
	next
}
# tokenize and eval expression in the same way? combined with
# not distinguishing between node/edge/etc, pretty generic; in
# the end this is going to be our repl...
/^[ \t]*node\[[^\]]+\][ \t]*$/{
	gsub(OFS"|node\\[|\\]", "")
	s = lnode[$1]
	if($1 in NLN)
		s = s ", LN=" NLN[$1]
	if($1 in color)
		s = s ", color=" color[$1]
	print "node", $1, s
	next
}
/^[ \t]*edge\[[^\]]+\][ \t]*$/{
	gsub(OFS"|edge\\[|\\]", "")
	s = ledge[$1]
	if($1 in cig)
		s = s ", overlap=" cig[$1]
	print "edge", $1, s
	next
}
/^FGD135$/{
	crm114 = 1
	next
}
{
	#tokenize()
	eval("{" $0 "}")
}
