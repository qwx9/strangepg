#!/bin/sh -e
# as explicit as possible, can certainly be shortened;
# adding mirrors makes sure that all edges u,v can be processed like this,
# labeling every node, and making sure indices are consistent between u and v
# final format:
#	index: hdr[] elist[]
#	hdr: [nn][ne]
# 	elist: {[u][v]}[ne]
# umap is literally a map u→s; ordering is set in the ct, here it's useless
# elist is sorted by degree(u), then u, then v

# FIXME: general usage
#tmpdir=/tmp
tmpdir=$HOME/tmp
tmp="$tmpdir/`uuidgen`"

awk 'BEGIN{OFS="\t"}$1=="L"{print $2, $4 "\n" $4, $2}' "$1" \
	| sort -k1,2V -u -T $tmpdir -S 11G \
	| awk 'BEGIN{OFS="\t"}{if(u!=$1) n++; u=$1; print n, $1, $2}' \
	> $tmp.ulist

# FIXME: is this strictly necessary?? check for duplicate work with named
# non sequential links/nodes
sort -k3V -k2V $tmp.ulist -T $tmpdir -S 11G \
	| paste $tmp.ulist - \
	| awk '{if(u!=$1){u=$1; printf "%s%s\t", (NR==1?"":"\n"), u}; printf "%s\t", $4}END{printf "\n"}' \
	| awk 'BEGIN{OFS="\t"}{print NF-1, $0}' \
	| sort -k1V -k2V -k3V -T $tmpdir -S 11G \
	> $tmp.elist

rm -f $tmp.ulist

/bin/time -f "py: %E usr %U sys %S max %Mk avg %Kk fs %O" \
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
' > "$1.idx"
cat $tmp >> "$1.idx" && rm $tmp
mv $tmp.elist "$1.idxe"
