#!/bin/bioawk -f
function output(u, parent){
	printf "%d:%d:%d,", u, parent, weight[u]
}
function merge(u, v, sid){
	# prioritizes highest degree and latest supernodes
	if(u > v){
		s = u
		t = v
	}else{
		s = v
		t = u
	}
	NE--
	deg[s]--
	if(node[lastn] != s){
		node[lastn] = s
		nmap[s] = sid
		off[s] = laste
		output(u, sid)
	}
	nmap[t] = s
	e = off[t]
	for(ee=e+deg[t]; e<ee; e++){
		w = edge[e]
		n = nmap[w]
		while(n != w && n < NN)
			w = nmap[w]
		if(w != s){
			output(nmap[edge[e]], sid)
			edge[laste++] = w
			deg[s]++
			weight[s] += weight[w]
		}
		nmap[edge[e]] = w	# lazy update
	}
	output(v, sid)
}
BEGIN{
	OFS = "\t"
}
{
	degend[d] = NN
	d = $1
	u = $2
	node[NN++] = u	# NOTE: 0-indexed!
	nmap[u] = u
	deg[u] = d
	off[u] = NE
	for(i=3; i<=NF; i++)
		edge[NE++] = $i
}
END{
	last = 0
	laste = 0
	id = 0
	u = node[id]
	for(d=deg[u]; d==0&&id!=degend[d]; u=node[id++])
		output(u, -1)
	e = 0
	lastsid = NN
	sid = lastsid
	ne = NE
	for(d=deg[u]; e<ne; d=deg[u]){
		u = node[id++]
		if(nmap[u] >= lastsid){
			e += d
			continue
		}
		for(ee=e+d; e<ee; e++){
			v = edge[e]
			if(nmap[v] >= lastsid)
				continue
			else if(deg[v] <= 2)
				merge(u, v, sid++)
			else if(id != laste)
				edge[laste++] = v
		}
		if(nmap[u] == sid)
			lastn++
	}
	n = 0
	FIXME: off unused?
	# FIXME: bubble rule probably works from 2 on
	for(d=deg[u]; d==2&&id!=degend[d]; u=node[++id]){
		if(nmap[u] >= lastsid)
			continue
		v = edge[u]
		if(deg[v] <= 2){
			merge(u, v, sid++)
			n++
		}
		if(
	}
	FIXME: fuse
	while(NE > 0){
		lastsid = sid
		lastn = 0
		laste = 0
		id = 0
		ee = NE
		for(e=0; e<ee; e++){
			u = ...
			FIXME: rules
			for(...
			if(n > 0){
				printf "|"
			}
			sid++
		}
	}



	# FIXME: after first pass, build a degree list of arrays, which
	# we update as we go; if too big, we can keep only lowest degree
	# or part of it, and look up, etc.
	# FIXME: write tobin.py or tobin.c to convert final result to
	#	binary format for display in strpg
	# FIXME: update strpg with coarsening + updateable layout


	e = 0
	while(deg[u] == 1){
		v = edge[e]
		if(deg[v] <= 2)
			merge(u, v)
		id++
		e++
		u = node[id]
	}
	while(deg[u] == 2){
		v = edge[e++]
		z = edge[e]
		if(deg[v] <= 2)
			merge(u, v)
		else if(deg[v] > 2){
			p = off[v]
			for(q=p+dev[v]; p<q; p++){
				w = edge[p]
				if(w == u)
					continue
				i = off[w]
				for(j=i+deg[w]; i<j; i++){
					if(edge[i] == z){
						mergelist[++m] = w
						break
					}
				}
			}
			if(len(mergelist) > 0){
				mergeall(u, v, z, mergelist)
				delete mergelist
				m = 0
			}
		}
		id++
		e++
		u = node[id]
	}
}
