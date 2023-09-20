#!/usr/bin/env python3
# assume input is 0-indexed, and already ordered correctly
# format different from plaintext version for readability:
# file: hdr[] levels[] nodes[] edges[]
# hdr: nnodes[8] nedges[8] nlevel[8]
# levels: { noff[8] eoff[8] nnodes[8] nedges[8] }[nlevel]
# nodes: { u[8] s[8] weight[8] }[nnodes + nsuper]
# edges: { i[8] }[] }	# variable number at every level: nedges + sth

from os import system, getpid
import sys
import fileinput
from struct import pack

off = 0

def w64(f, v):
	global off
	f.write(pack("q", int(v)))
	sys.stderr.write("["+str(v)+"]")
	off += 8
def w32(f, v):
	global off
	f.write(pack("I", int(v)))
	sys.stderr.write("<"+str(v)+">")
	off += 4

# FIXME: same tmpfile naming as the awk stuff (general framework)
fname = f"/tmp/coarse.{getpid()}.bin"
fd = open(fname + "_0", "wb")
fn = open(fname + "_1", "wb")
fe = open(fname + "_2", "wb")
lidx = []
nv = ns = ne = 0
s = -1
off0 = 0
for r in fileinput.input():
	p = r.strip("\n \t").split()
	if p[0][0] == "#":
		continue
	elif p[0] == "level":
		lidx.append((nv, ne, int(p[1]), int(p[2])))
	elif p[0] == "node":
		w64(fn, int(p[1], base=16))	# u
		w64(fn, int(p[2], base=16))	# node[u]
		w64(fn, int(p[3], base=16))	# s
		w64(fn, int(p[4], base=16))	# weight[u]
		ns = max(ns, s)
		nv += 1
	elif p[0] == "edge":
		w64(fe, int(p[1], base=16))	# j
		ne += 1
	elif p[0] == "push":
		sys.stderr.write("\n")
		off0 = off
	else:
		sys.stderr.write("unknown verb " + s[0] + "\n")
		exit()
fn.close()
fe.close()
f = sys.stdout.buffer
off = 0
w64(f, nv)	# total including supers
#w64(f, ns)
#w64(f, ns + 1 - nv)	# neither probably make sense w/o first knowing nv
w64(f, ne)	# same as the one in index
w64(f, len(lidx))
noff = off + len(lidx) * 4 * 8
eoff = noff + nv * 4 * 8
for l in lidx:
	w64(f, noff + l[0] * 8 * 4)
	w64(f, eoff + l[1] * 8)
	w64(f, l[2])
	w64(f, l[3])
sys.stdout.flush()
system(f"cat {fname}_* && rm {fname}_*")
sys.stderr.write("\n")
