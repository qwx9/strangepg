#!/bin/bash -e
# as explicit as possible, can certainly be shortened;
# adding mirrors makes sure that all edges u,v can be processed like this,
# labeling every node, and making sure indices are consistent between u and v

# FIXME: general usage; document
uuid=`uuidgen`
tmpdir=$TMPDIR
out=$1
if [[ "x$tmpdir" == x ]]; then tmpdir=$HOME/tmp; fi
if [[ "x$out" == "x-" ]]; then out="stdin.$uuid"; fi
mkdir -p $tmpdir
tmp="$tmpdir/$uuid"

awk 'BEGIN{OFS="\t"}$1=="L"{print $2, $4 "\n" $4, $2}' "$1" \
	| sort -k1,2V -u -T $tmpdir -S 10G \
	| awk 'BEGIN{OFS="\t"}{if(u!=$1) n++; u=$1; print n, $1, $2}' \
	> $tmp.ulist

# FIXME: is this strictly necessary?? check for duplicate work with named
# non sequential links/nodes
sort -k3V -k2V $tmp.ulist -T $tmpdir -S 10G \
	| paste $tmp.ulist - \
	| awk '{if(u!=$1){u=$1; printf "%s%s\t", (NR==1?"":"\n"), u}; printf "%s\t", $4}END{printf "\n"}' \
	| awk 'BEGIN{OFS="\t"}{print NF-1, $0}' \
	| sort -k1V -k2V -k3V -T $tmpdir -S 10G \
	> $tmp.elist

rm -f $tmp.ulist

<$tmp.elist /usr/bin/env python3 -c '
import fileinput
import sys
from struct import pack
edges = open("'$tmp'", "wb")
nn = ne = 0
for l in fileinput.input():
	s = l.rstrip("\n").split()
	d = int(s[0])
	u = int(s[1]) - 1
	nn += 1	
	for e in range(2, 2+d):
		v = int(s[e]) - 1
		edges.write(pack("Q", u))
		edges.write(pack("Q", v))
		ne += 1
f = sys.stdout.buffer
f.write(pack("Q", nn))
f.write(pack("Q", ne))
edges.close()
' > "$out.idx"
cat $tmp >> "$out.idx" && rm $tmp
mv $tmp.elist "$out.idxe"
