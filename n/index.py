#!/bin/python
# -*- coding: utf-8 -*-
import sys
import os
import shutil
from struct import pack

"""
breadth-first coarsening hierarchy index, first naive attempt
- pass 1: load entire gfa first (red flag!), count incoming and outgoing
  degrees for each node → buckets of degree n
- pass 2: while more than one node remains or cannot reduce further (bug),
  apply one of 3 heuristics to merge nodes into supernodes (and redundant
  edges into superedges)
	* if 2 or more nodes in buckets 0 or 1 are adjacent, they form a linear
	  chain that can be compacted → new "unchopped" zoom level
	* if not, if there exist 1 or more simple bubble, pop them (merge
	  branches into a supernode) ← unimplemented, but easy
	* if not, take all nodes in the highest degree bucket and merge them
	  with their adjacencies
- proceeding from the entire graph (all "leaf" nodes), one of the 3 heuristics
  will be applied at each iteration, producing a list of nodes and edges to
  merge for a new "zoom out" level
- desired output:  all complexity offloaded here, minimum effort online:
	index: hdr[] dict[] nodes[] edges[] meta[]
	hdr: nnode[8] nedge[8] nlevel[8] noff[8] eoff[8] metaoff[8]
	dict: {
		noff[8] nlvl[8] ntotal[8]
		eoff[8] elvl[8] etotal[8]
	}[nlevel]
	nodes: {
		nout[8] nin[8] weight[8] parent[8]
	}[nnode]
	nodes: {
		head[8] tail[8] weight[8] parent[8]
	}[nedge]
	meta: unspecified; collection of nul terminated strings
- note: ]---[head  (node)  tail]---[
- at each iteration, nodes and edges to be merged on the upper zoom level
  are directly written to disk rather than stored in memory
- thus higher zoom levels are prepended to lower ones;  we cannot prepend
  writes to file, only overwrite;  therefore, create 64mb sparse file,
  write backwards into it until space runs out, then create a new one; at
  the end, concatenate files in reverse order, noting that the first one
  is sparse
- next, write a "dictionary" indexing spans of nodes and edges for each
  level; it is written directly to the outfile following a simple header;
  at the very end, node and edge arrays are appended one after the other
- observe that once a leaf node is placed at any zoom level, it will not be
  touched again when zooming in: it is "fixated";  observe also that a
  supernode can only be merged further (zoom out) or burst (zoom in), thus a
  modification replaces rather than modifies it;  node and edge references
  thus remain valid irrespective of zoom level, so random access is possible;
  fast queries for objects is important for the ui
- what is actually stored per level is only the children of whichever nodes
  are burst;  the parent pointers indicate which parent to remove, online
  it can simply be skipped;  in a way, they exist only between the moment
  they are declared and the moment they spawn children
- this tries hard to keep memory usage low and favoring disk access;  for
  higher scale, we can avoid loading the graph entirely or even counting
  by using shell tools;  presort gfa by link records by head node, forward
  first (sort -u +0dr -1 +1d -2 +2nr dicks.gfa), then count outgoing per
  node (using sort +0n -1 +1d;  both can be done in a single awk script)
- supernodes and superedges right now are individual nodes/edges created
  in addition to the preexisting ones;  this can be avoided by reusing
  the slots of merged nodes/edges, since there will never be more supernodes/
  superedges than nodes/edges in the initial graph;  already know how, but
  too tricky for now
"""

def put64(f, v):
	f.write(pack("Q", v))


# FIXME: are we actually relabeling nodes/edges when fixating them?

Blksz = 64 * 1024 * 1024

class Writer:
	def __init__(self, prefix):
		self.prefix = prefix
		self.bank = 0
		self.f = None
		self.lastsz = 0			# last record's size in bytes
		self.cursz = 0
		self.off = -1			# in current block only
	def startblock(self):
		if not self.f is None:
			self.f.close()
		self.f = self.mkblock()
	def mkblock(self, suffix=""):
		path = self.prefix + ".%03d" % self.bank + suffix
		f = open(path, "wb")
		f.seek(Blksz - 1)
		f.write("\00")
		f.seek(-8, 1)
		self.off = Blksz - 8		# writing quads only
		self.bank += 1
		return f
	def write(self, v):
		if self.off < 0:
			self.startblock()
		self.f.write(pack("Q", v))
		self.f.seek(-16, 1)
		self.off -= 8
		self.cursz += 8
	def endrecord(self):
		self.lastsz = self.cursz
		self.cursz = 0
	def cat(self, f):
		for i in range(0, self.bank):
			path = self.prefix + ".%03d" % i
			ff = open(path, "rb")
			# first file is sparse
			if i == 0:
				ff.seek(self.off)
			shutil.copyfileobj(ff, f)	# default length = 16k??
			ff.close()
			os.remove(path)

class Node:
	def __init__(self, w=1):
		self.ein = set()
		self.eout = set()
		self.w = w
		self.parent = None
	def __str__(self):
		return str(self.ein) + ":" + str(self.eout) + " w=" + str(self.w) + \
			" par=" + str(self.parent)
	def write(self, f):
		f.write(len(self.eout))
		f.write(len(self.ein))
		f.write(self.w)
		f.write(self.parent)

class Edge:
	def __init__(self, head, dh, tail, dt):
		dh = 1 if dh == "-" else 0
		dt = 1 if dt == "-" else 0
		self.h = head << 1 | dh
		self.t = tail << 1 | dt
		self.w = 1
		self.parent = None
		self.recordsize = 4 * 8
	def __str__(self):
		return ("<" if self.h & 1 else ">") + str(self.h>>1) \
			+ ("<" if self.t & 1 else ">") + str(self.t>>1) \
			+ " w=" + str(self.w) + " par=" + str(self.parent)
	def remap(self, h, t):
		self.h = h << 1 | self.h & 1
		self.t = t << 1 | self.t & 1
	# defensive functions
	def head(self):
		return self.h >> 1
	def tail(self):
		return self.t >> 1
	def write(self, f):
		f.write(self.h)
		f.write(self.t)
		f.write(self.w)
		f.write(self.parent)

class Seymourbutz:
	def __init__(self, path):
		self.path = path
		self.nodes = []
		self.edges = []
		self.llist = []
		self.sac = {}
		self.nlevel = 0
		self.nodetot = 0
		self.edgetot = 0
		self.fnode = Writer(path + ".node")
		self.fedge = Writer(path + ".edge")
		self.outpath = path + ".idx"
		self.readgfa(self.path)
	def addnode(self, id, seq, nmap):
		nmap[id] = len(self.nodes)
		self.nodes.append(Node())
		#id
		#seq
		print "node", len(self.nodes)-1, id, self.nodes[len(self.nodes)-1]
	def addedge(self, u, du, v, dv, overlap, nmap):
		eid = len(self.edges)
		u = nmap[u]
		v = nmap[v]
		self.nodes[u].eout.add(eid)
		self.nodes[v].ein.add(eid)
		self.edges.append(Edge(u, du, v, dv))
		#overlap
		print "edge", len(self.edges)-1, self.edges[len(self.edges)-1]
	def readgfa(self, path):
		nmap = {}
		f = open(path)
		while True:
			s = f.readline()
			if not s:
				break
			s = s.strip().split()
			if s[0] == 'S':
				self.addnode(s[1], s[2], nmap)
			elif s[0] == 'L':
				self.addedge(s[1], s[2], s[3], s[4], s[5], nmap)
		f.close()

	def writemeta(self, f):
		pass
	def writeedges(self, f):
		self.fedge.cat(f)
		return f.tell()
	def writenodes(self, f):
		self.fnode.cat(f)
		return f.tell()
	def writedict(self, f):
		noff = 0	# offsets are relative to start of section
		eoff = 0
		nn = 0
		ne = 0
		for i in self.llist:
			nn += i[0]
			ne += i[1]
			put64(f, noff)
			put64(f, i[0])
			put64(f, nn)
			put64(f, eoff)
			put64(f, i[2])
			put64(f, ne)
			noff += i[1]
			eoff += i[3]
		return f.tell()
	def writehdr2(self, f, noff, eoff, moff):
		put64(f, self.nodetot)		# repetition, defensive
		put64(f, self.edgetot)
		put64(f, self.nlevel)
		put64(f, noff)
		put64(f, eoff)
		put64(f, moff)
	def writehdr(self, f):
		put64(f, self.nodetot)
		put64(f, self.edgetot)
		put64(f, self.nlevel)
		# placeholders: offsets to sections
		put64(f, 0)
		put64(f, 0)
		put64(f, 0)
	def mkindex(self):
		self.fedge.f.close()
		self.fnode.f.close()
		f = open(self.outpath, "wb")
		self.writehdr(f)
		dn = self.writedict(f)
		de = self.writenodes(f)
		dm = self.writeedges(f)
		self.writemeta(f)
		self.writehdr2(f, dn, de, dm)
		f.close()

	def addlevel(self, nodes, edges):
		for n in nodes:
			self.nodes[n].write(self.fnode)
		self.fnode.endrecord()
		for e in edges:
			self.edges[e].write(self.fedge)
		self.fedge.endrecord()
		self.llist.append(( \
			len(nodes), self.fnode.lastsz, len(edges), self.fedge.lastsz))
		self.nodetot += len(nodes)
		self.edgetot += len(edges)
		self.nlevel += 1

	# FIXME: paper model assumes only one entry per edge,
	# not one for each node
	def merge(self, l, lvln, lvle):
		print "merge:", l
		s = Node(0)
		sid = len(self.nodes)
		self.nodes.append(s)
		# it's not possible for two distinct nodes to use the same in/out edge
		# as another, ie. there will never be duplicates within ein or eout,
		# only between them
		ein = set()
		eout = set()
		for i in l:
			u = self.nodes[i]
			u.parent = sid
			s.w += u.w
			c = max(len(u.ein), len(u.eout))
			print c, self.sac[c]
			self.sac[c].discard(i)
			s.ein |= u.ein
			s.eout |= u.eout
			lvln.append(i)
		# report all edges; internal ones are fixated; external ones need
		# relabeling and may be contracted into hyperedges
		dup = s.ein & s.eout
		s.ein -= dup
		s.eout -= dup
		for e in dup:
			self.edges[e].parent = sid
			lvle.append(e)
		# contract now redundant edges
		# FIXME: ignoring orientation here (both sides)
		dc = {}
		for e in s.eout:
			print "checking edge ", str(e), ": ", str(self.edges[e])
			lvle.append(e)
			print lvle
			e = self.edges[e]
			e.parent = sid
			n = e.tail
			# multiple incoming edges from the same node need to be contracted
			if n in dc:
				e.w += 1
			else:
				dc[n] = 1
				e.remap(sid, e.tail())
		for e in s.ein:
			print "checking edge ", str(e), ": ", str(self.edges[e])
			lvle.append(e)
			print lvle
			e = self.edges[e]
			e.parent = sid
			n = e.head
			if n in dc:
				e.w += 1
			else:
				dc[n] = 1
				e.remap(e.head(), sid)
		self.sac[max(len(u.ein), len(u.eout))].add(len(self.nodes)-1)
		for i in lvln:
			print "lvln", self.nlevel, i, self.nodes[i]
		for i in lvle:
			print "lvle", self.nlevel, i, self.edges[i]
		return True
	def fuse(self):
		print "try fuse"
		c = max(self.sac.keys())
		# if level is empty:
		# get nodes with highest total degree
		# merge into new supernode
		# insert supernode in hierarchical tree
		return None
	def pop(self):
		print "try pop"
		return None
	# FIXME: 5+5+, 5+5-, 5+4+, 4-5+ and the like, make sure this still works
	# this IGNORES orientation; merge does not
	def unchop(self):
		# stupidest possible approach to start with
		# unchopping two consecutive nodes is only possible if both
		# have only one outgoing and one incoming edge and share a
		# common edge; in the set of nodes with max degree 1, there
		# can be no two nodes that share the same outgoing or
		# incoming edge, the only overlaps may be between in and out
		# → intersect in and out
		print "try unchop"
		for i in self.sac:
			print "sac", i, self.sac[i]	# nid:eid
		if not 1 in self.sac or len(self.sac[1]) == 0:
			return
		sin = {}	# nid:eid[]
		sout = {}	# nid:eid[]
		for u in self.sac[1]:
			# there can be only one, in either direction
			if len(self.nodes[u].ein) > 0:		# or source
				sin.update({u:list(self.nodes[u].ein)[0]})
			if len(self.nodes[u].eout) > 0:		# or sink
				sout.update({u:list(self.nodes[u].eout)[0]})
		dupe = set(sin.values()) & set(sout.values())
		print "sub ein", sin
		print "sub eout", sout
		print "intersection", dupe
		if not dupe:
			return False
		I = set([ self.edges[e].tail() for e in dupe ])
		O = set([ self.edges[e].head() for e in dupe ])
		m = I & O
		print "I", I
		print "O", O
		print "M", m
		s = []
		while dupe:
			e = dupe.pop()
			u = self.edges[e].head()
			v = self.edges[e].tail()
			x = [u, v]
			while u in m:	# no need to edit m, there's only one such entry
				ew = sin[u]	# w,u: want in[u], there can be only one also
				w = self.edges[ew].head()
				x.append(w)
				dupe.discard(ew)
				u = w
			while v in m:
				ew = sout[v]	# u,v
				w = self.edges[ew].tail()
				x.append(w)
				dupe.discard(ew)
				v = w
			s.append(x)
		print s
		assert(len(s) > 0)
		return s
	def countdeg(self):
		for i in range(len(self.nodes)):
			c = max(len(self.nodes[i].ein), len(self.nodes[i].eout))
			try:
				self.sac[c].add(i)
			except:
				self.sac[c] = set([i])
	def mktree(self):
		while True:
			l = self.unchop()
			# FIXME: unimplemented
			if not l:
				l = self.pop()
			# FIXME: unimplemented
			if not l:
				l = self.fuse()
			if not l:
				print "cannot reduce further??"
				break
			lvln = []
			lvle = []
			for i in l:
				self.merge(i, lvln, lvle)
			print lvln, lvle
			self.addlevel(lvln, lvle)

for i in sys.argv[1:]:
	print "processing", i
	s = Seymourbutz(i)
	s.countdeg()	# avoidable
	s.mktree()
	s.mkindex()
