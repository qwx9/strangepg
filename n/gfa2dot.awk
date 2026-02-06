#!/bin/awk -f
BEGIN{ print "digraph x {" }
#BEGIN{ print "graph x {" }
$1 == "S"{
	#s[$2] = ++ns
	print $2, "[shape=box]"
}
$1 == "L"{
	#u = s[$2]
	#v = s[$4]
	u = $2
	v = $4
	#print u, "--", v
	if($3 == "+" && $5 == "+")
		print u, "->", v
	else if($3 == "+" && $5 == "-")
		print u, "->", v, "[style=dotted]"
	else if($3 == "-" && $5 == "+")
		print v, "->", u, "[style=dotted]"
	else if($3 == "-" && $5 == "-")
		print v, "->", u
	else{
		print "invalid edge", $0
		exit
	}
}
END{ print "}" }
