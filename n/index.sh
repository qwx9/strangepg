#!/bin/sh -e
# as explicit as possible, can certainly be shortened;
# adding mirrors makes sure that all edges u,v can be processed like this,
# labeling every node, and making sure indices are consistent between u and v
tmp="/tmp/`uuidgen`"

awk 'BEGIN{OFS="\t"}$1=="L"{print $2, $4 "\n" $4, $2}' "$1" \
	| sort -k1,2d -u \
	| awk 'BEGIN{OFS="\t"}{if(u!=$1) n++; u=$1; print n, $1, $2}' \
	> $tmp.ulist

	sort -k3d $tmp.ulist \
	| paste $tmp.ulist - \
	| awk '{if(u!=$1){u=$1; printf "%s%s\t", (NR==1?"":"\n"), u}; printf "%s\t", $4}END{printf "\n"}' \
	| awk 'BEGIN{OFS="\t"}{print NF-1, $0}' \
	| sort -k1V -k2V -k3V \
	> $tmp.elist

/bin/time -f "py: %E usr %U sys %S max %Mk avg %Kk fs %O" \
	<$tmp.elist /usr/bin/env python3 -c '
import fileinput
from os import write as outb
from struct import pack
nodes = open("'$tmp.2'", "wb")
edges = open("'$tmp.4'", "wb")
nodedeg = open("'$tmp.3'", "wb")
degsize = open("'$tmp.5'", "wb")
hdr = open("'$tmp.1'", "wb")
nd = 0
ne = 0
nn = 0
od = 0
odn = 0
d = 0
ulen = 0
uoff = 0
for l in fileinput.input():
	s = l.rstrip("\n").split()
	d = int(s[0])
	u = int(s[1])

	# degree interval indices
	if ne == 0:
		od = d
	elif od != d:
		degsize.write(pack("I", od))
		degsize.write(pack("Q", ulen))
		od = d
		ulen = 0
		nd += 1
	ulen += d

	# _,v node indices into node array
	for i in range(2, 2+d):
		edges.write(pack("Q", int(s[i])))
	ne += d

	# u,_ offset into edge array
	nodes.seek(u * 8)
	nodes.write(pack("Q", uoff))
	uoff += 8 * d
	nn += 1

	# node degree array
	nodedeg.seek(u * 8)
	nodedeg.write(pack("Q", d))

degsize.write(pack("I", d))
degsize.write(pack("Q", nd))
hdr.write(pack("Q", nn))
hdr.write(pack("Q", ne))
hdr.write(pack("Q", nd))
hdr.close()
degsize.close()
nodedeg.close()
edges.close()
nodes.close()
#outb(1, pack("Q", ne))
'
cp $tmp.elist "$1.elist"
cp $tmp.ulist "$1.ulist"
cat $tmp.[0-9]* > "$1.idx"
ls -l "$1".* $tmp.[0-9]*
#rm $tmp.*
