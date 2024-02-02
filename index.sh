#!/bin/bash -e
# as explicit as possible, can certainly be shortened;
# adding mirrors makes sure that all edges u,v can be processed like this,
# labeling every node, and making sure indices are consistent between u and v

# FIXME: general usage; document
uuid=`uuidgen`
tmpdir=$TMPDIR
out=${1/.gfa/}
if [[ "x$tmpdir" == x ]]; then tmpdir=$HOME/tmp; fi
if [[ "x$out" == "x-" ]]; then out="stdin.$uuid"; fi
mkdir -p $tmpdir
tmp="$tmpdir/$uuid"

rm -f "$out".{s,e,u,bs,be,hs,he}

# FIXME: slow af, redundancies; python doubles runtime, but
# first step is even more expensive: 1e5: 1.3s + 0.2s + 1s
# as opposed to 1.0s total
# FIXME: tmpdir

# save indices of original elements and reorder directly by left node
awk -v out="$out" '
BEGIN{OFS="\t"; e=0; off=0; i0=0; nn=0; ne=0}
{
	off0 = off
	ll = length($0)
	off += ll
}$1 == "S"{
	# S u off len idx
	print "S", $2, off0, ll, nn++
}$1 == "L"{
	# L u v o off len idx
	print "L", $2, $4, $3 $5, off0, ll, ne
	o = ($3=="+"?"-":"+") ($5=="+"?"-":"+"_)
	# L v u o "x" len idx
	print "L", $4, $2, o, "x", ll, ne++
}' "$1" \
	| sort +1V -2 +0d -1 +2V -3 +3d -4 \
	| awk -v out="$out" '
BEGIN{OFS="\t"; d=0; u=0; s=0; nn=0}
# S U off len idx d s
$1 == "S"{ print $0, d, s; s+=d; d=0; u=$5; nn++ }
# L U V o off len
$1 == "L"{ print $1, $2, $3, $4, $5, $6; d++ }
' \
	| sort +1V -2 +0rd -1 +2V -3 +3d -4 \
	| awk -v out="$out" '
BEGIN{OFS="\t"}
# d i ei off len
$1 == "S"{ print $6, $5, $7, $3, $4 | "sort +0V -1 +1V -2 +2V -3 >>" out ".s"; u=$5 }
# 
$1 == "L"{ print u, $2, $3, $4, $5, $6 }
' \
	> "$out.u"
sort +2V -3 +1V -2 +3d -3 "$out.u" \
	| paste "$out.u" - \
	| awk '
BEGIN{OFS="\t"}
# i j o off len
{ print $1, $7, $4, $5, $6 }
' > "$out.e"

rm "$out.u"

/usr/bin/env python3 -c '
import fileinput
import sys
from struct import pack
out = "'$out'"
ne = 0
nse = 0
nn = 0
# binarize sorted edge list
of = open(out+".e", "r")
f = open(out+".be", "wb")
for l in of:
	s = l.rstrip("\n").split()
	assert(len(s) == 5)
	if s[2] == "++":
		s[2] = 0
	elif s[2] == "-+":
		s[2] = 1
	elif s[2] == "+-":
		s[2] = 2
	elif s[2] == "--":
		s[2] = 3
	if s[3] == "x":
		s[3] = -1
	else:
		ne += 1
	nse += 1
	# sorted by u, v, o
	f.write(pack("n", int(s[0])))	# i		u int id
	f.write(pack("n", int(s[1])))	# j		v int id	
	f.write(pack("n", int(s[2])))	# dir	orientation (should be B)
	f.write(pack("n", int(s[3])))	# off	off/len in gfa
	f.write(pack("n", int(s[4])))	# len	(should be L)
f.close()
of.close()
of = open(out+".s", "r")
f = open(out+".bs", "wb")
for l in of:
	s = l.rstrip("\n").split()
	assert(len(s) == 5)
	# sorted by d, i, off
	f.write(pack("n", int(s[0])))	# d		degree (should be L)
	f.write(pack("n", int(s[1])))	# i		u int id
	f.write(pack("n", int(s[2])))	# ei	(first edge in $out.be)
	f.write(pack("n", int(s[3])))	# off	off/len in gfa
	f.write(pack("n", int(s[4])))	# len	(should be L)
	nn += 1
of.close()
f.close()
f = open(out+".hs", "wb")
f.write(pack("Q", nn))
f.close()
f = open(out+".he", "wb")
f.write(pack("Q", ne))
f.write(pack("Q", nse))
f.close()
'
# FIXME: stdout pipeline; print/read beforehand
cat "$out.hs" "$out.bs" > "$out.bs.1" && mv "$out.bs.1" "$out.bs"
cat "$out.he" "$out.be" > "$out.be.1" && mv "$out.be.1" "$out.be"
rm "$out.hs" "$out.he"
