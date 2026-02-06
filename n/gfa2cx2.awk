#!/bin/awk -f
BEGIN{
	print "[{\"CXVersion\":\"2.0\", \"hasFragments\":true},"
}
function addnode(l,	id){
	if(l in nodes)
		id = nodes[l]
	else{
		id = ++nnodes
		nodes[l] = id
	}
	label[id] = l
	return id
}
$1 == "S"{
	id = addnode($2)
	#if($3 != "*")
	#	seq[id] = $3
	for(i=4; i<=NF; i++){
		n = split($i, a, ":")
		tags[a[1]] = a[2] == "f" ? "double" \
			: a[2] == "i" ? "integer" \
			: "string"
		tag[a[1],id] = a[3]
	}
}
$1 == "L"{
	u = addnode($2)
	v = addnode($4)
	d1 = $3 == "-"
	d2 = $5 == "-"
	if(u > v || u == v && d1){
		t = u
		u = v
		v = t
		t = !d1
		d1 = !d2
		d2 = t
	}
	d1 = d1 ? "-" : "+"
	d2 = d2 ? "-" : "+"
	l = u d1 v d2
	if(l in edges){
		print "line " NR ": duplicate edge, previous definition: id=", edges[l] >"/dev/stderr"
		next
	}
	id = ++nedges
	edges[l] = id
	edgeu[id] = u
	edgev[id] = v
	edgeU[id] = d1
	edgeV[id] = d2
	ovl[id] = $6
	for(i=7; i<=NF; i++){
		n = split($i, a, ":")
		etags[a[1]] = a[2] == "f" ? "double" \
			: a[2] == "i" ? "integer" \
			: "string"
		etag[a[1],id] = a[3]
	}
}
$1 == "P"{
	n = split($3,a,",")
	p = ""
	for(i=1; i<n; i++){
		if(i == 1)
			k = length(a[i])
		else
			k = m
		u = substr(a[i],1,k-1)
		U = substr(a[i],k)
		m = length(a[i+1])
		v = substr(a[i+1],1,m-1)
		V = substr(a[i+1],m)



		l = nodes[u] U nodes[v] V
		if(!(l in edges)){
			U = U == "+" ? "-" : "+"
			V = V == "+" ? "-" : "+"
			l = nodes[v] V nodes[u] U
			if(!(l in edges)){
				print "line " NR ": no such edge: ", l >"/dev/stderr"
				continue
			}
		}
		if(p == "")
			p = edges[l]
		else
			p = p "," edges[l]
	}
	paths[$2] = p
	p = ""
}
function printattr(	i){
	print "{\"attributeDeclarations\":[{"
	print "\t\"nodes\":{"
	print "\t\t\"label\":{\"d\":\"string\", \"a\":\"l\"},"
	printf "\t\t\"seq\":{\"d\":\"string\",\"a\":\"s\",\"v\":\"*\"}"
	for(i in tags)
		printf ",\n\t\t\"%s\":{\"d\":\"%s\"}", i, tags[i]
	print "\n\t},\n\t\"edges\":{"
	print "\t\t\"srev\":{\"d\":\"boolean\", \"a\":\"S\"},"
	print "\t\t\"trev\":{\"d\":\"boolean\", \"a\":\"T\"},"
	printf "\t\t\"cigar\":{\"d\":\"string\", \"a\":\"o\", \"v\":\"0M\"}"
	for(i in etags)
		printf ",\n\t\t\"%s\":{\"d\":\"%s\"}", i, etags[i]
	print "\n\t},\n\t\"paths\":{"
	print "\t\t\"label\":{\"d\":\"string\", \"a\":\"l\"},"
	print "\t\t\"edges\":{\"d\":\"list_of_long\", \"a\":\"e\"}"
	print "\t}\n}]},"
}
function printnodes(	s, x, i, id){
	print "{\"nodes\":["
	s = ""
	for(id=1; id<=nnodes; id++){
		printf "%s\t{\"id\":%d, \"v\":{\"l\":\"%s\"", s, id, label[id]
		if(id in seq)
			printf ", \"s\":\"%s\"", seq[id]
		for(i in tags){
			x = i "\x1c" id
			if(!(x in tag))
				continue
			if(tags[i] == "string")
				printf ", \"%s\":\"%s\"", i, tag[x]
			else
				printf ", \"%s\":%s", i, tag[x]
		}
		printf "}}"
		s = ",\n"
	}
	print "\n]},"
}
function printedges(){
	print "{\"edges\":["
	s = ""
	for(id=1; id<=nedges; id++){
		printf "%s\t{\"id\":%d, \"s\":%d, \"t\":%d",
			s, id, edgeu[id], edgev[id]
		printf ", \"v\":{\"S\":%s, \"T\":%s, \"o\":\"%s\"",
			(edgeU[id] == "-" ? "true" : "false"),
			(edgeV[id] == "-" ? "true" : "false"),
			ovl[id]
		for(i in etags){
			x = i "\x1c" id
			if(!(x in etag))
				continue
			if(etags[i] == "string")
				printf ", \"%s\":\"%s\"", i, etag[x]
			else
				printf ", \"%s\":%s", i, etag[x]
		}
		printf "}}"
		s = ",\n"
	}
	print "\n]},"
}
function printpaths(){
	print "{\"paths\":["
	s = ""
	for(i in paths){
		printf "%s\t{\"l\":%s, \"e\":[%s]}", s, i, paths[i]
		s = ",\n"
	}
	print "\n]},"
}
END{
	printattr()
	printnodes()
	printedges()
	printpaths()
	print "{\"status\":[{\"success\":true}]}]"
}
