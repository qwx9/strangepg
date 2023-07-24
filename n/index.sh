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
from struct import pack
nodeidx = open("'$tmp.2'", "wb")
nodedeg = open("'$tmp.3'", "wb")
edges = open("'$tmp.4'", "wb")
hdr = open("'$tmp.1'", "wb")
i = 0
ne = 0
for l in fileinput.input():
	s = l.rstrip("\n").split()
	d = int(s[0])
	u = int(s[1])
	nodeidx.write(pack("Q", i))
	nodedeg.write(pack("Q", d))
	for i in range(2, 2+d):
		edges.write(pack("Q", int(s[i])))
		ne += 1
	i += 1
hdr.write(pack("Q", i))
hdr.write(pack("Q", ne))
hdr.close()
nodeidx.close()
nodedeg.close()
edges.close()
'
cp $tmp.elist "$1.idxe"
cp $tmp.ulist "$1.idxu"
cat $tmp.[0-9]* > "$1.idx"
rm $tmp.*
