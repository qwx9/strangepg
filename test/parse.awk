#!/bin/awk -f
BEGIN{
	tagfn["CL"] = "nodecolor"
}
function isindex(s)
{
	gsub("^[ 	]+|[	 ]+$", "")
	return s ~ /^[0-9]+$/ || s ~ /^"[^"]+"$/ \
		|| s ~ /^[A-Za-z_][0-9A-Za-z_]+/ && s != "i"
}
function namedtag(s)
{
	if(s == "color")
		return "CL"
	return s
}
function subexpr(s, 	t, p, n, i, j, sp, fp, stk){
	t = s
	p = 1
	while((i = index(t, "[")) != 0){
		p += i
		lb[length(lb)+1] = p
		t = substr(t, i+1)
	}
	sp = 0
	fp = 1
	t = s
	p = 1
	while((i = index(t, "]")) != 0){
		p += i
		while(fp <= length(lb) && lb[fp] < p)
			stk[++sp] = fp++
		j = stk[sp--]
		rb[j] = p - 1 - lb[j]
		t = substr(t, i+1)
	}
	#for(i=1; i<=length(lb); i++)
	#	print i, lb[i], rb[i], "\"" substr(s, lb[i], rb[i]) "\""
}
{
	assign = match($0, "\][ 	]*=[^=]")	# just print or changeval
	subexpr($0)
	new = ""
	ni = 1
	for(i=1; i<=length(lb); i++){
		for(j=lb[i]-2; j>=1; j--)
			if(substr($0, j, 1) !~ /[0-9A-Za-z_]/)	# goddamn it awk
				break
		if(++j >= lb[i] - 1)
			continue
		new = new substr($0, ni, j - 1 - ni)
		#ni = rb[i]
		print "\"" j "\"", substr($0, j, lb[i]-1-j)
		t = substr($0, j, lb[i]-1-j)
		tag = namedtag(t)
		ss = substr($0, lb[i], rb[i])
		#if(tag in SYMTAB){	# FIXME: not symtab, but TAGS[]
			# FIXME: rhs + close brace
			if(!isindex(ss)){
				# FIXME: also need to exec side effect
				if(ss == "i")	# special case, everything
					new = new "for(i in " tag "){ " tag"["i"] = "
				else
					new = new "for(i in " tag ") if("ss"){ XXX"
				n1 = rb[i]+1
			}else if(tag in tagfn){
				new = new tagfn[tag] "("ss", XXX)"
				n1 = rb[i]+1
			}
		#}
	}
	if(n1 != length($0))
		$0 = new substr($0, n1)
	else
		$0 = new
	print $0
	delete lb
	delete rb
}
