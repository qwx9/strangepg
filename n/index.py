#!/bin/python
# -*- coding: utf-8 -*-
import sys
import os
import shutil
from struct import pack

"""
NOTE: this is now full(er) of lies

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
		tail[8] head[8] weight[8] parent[8]
	}[nedge]
	meta: unspecified; collection of nul terminated strings
- note: ]---[tail  (node)  head]---[
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
  by using shell tools;  presort gfa by link records by tail node, forward
  first (sort -u +0dr -1 +1d -2 +2nr dicks.gfa), then count outgoing per
  node (using sort +0n -1 +1d;  both can be done in a single awk script)
- supernodes and superedges right now are individual nodes/edges created
  in addition to the preexisting ones;  this can be avoided by reusing
  the slots of merged nodes/edges, since there will never be more supernodes/
  superedges than nodes/edges in the initial graph;  already know how, but
  too tricky for now
"""

def put64(f, v):
	f.write(pack("q", v))

sizeofNode = 4 * 8
sizeofEdge = 4 * 8

class Node:
	def __init__(self, w=1.0):
		self.ein = set()
		self.eout = set()
		self.w = w
		self.parent = None
		self.tag = None
	def __str__(self):
		return str(self.ein) + ":" + str(self.eout) + " w=" + str(self.w) + \
			" par=" + str(self.parent) + " as=" + str(self.tag)
	def write(self, f):
		print "write node", self
		put64(f, len(self.eout))
		put64(f, len(self.ein))
		put64(f, self.w)
		put64(f, self.parent if self.parent else -1)

class Edge:
	def __init__(self, tail, dt, head, dh):
		dt = 1 if dt == "-" else 0
		dh = 1 if dh == "-" else 0
		self.t = tail << 1 | dt
		self.h = head << 1 | dh
		self.w = 1
		self.parent = None
		self.recordsize = 4 * 8
		self.tag = None
	def __str__(self):
		return ("<" if self.t & 1 else ">") + str(self.t>>1) \
			+ ("<" if self.h & 1 else ">") + str(self.h>>1) \
			+ " w=" + str(self.w) + " par=" + str(self.parent)
	def remap(self, t, h):
		self.t = t << 1 | self.t & 1
		self.h = h << 1 | self.h & 1
	# defensive functions
	def head(self):
		return self.h >> 1
	def tail(self):
		return self.t >> 1
	def write(self, f):
		print "write edge", self
		put64(f, self.t)
		put64(f, self.h)
		put64(f, self.w)
		put64(f, self.parent if self.parent else -1)

class Seymourbutz:
	def __init__(self, path):
		self.tag = 0
		self.path = path
		self.nodes = []
		self.edges = []
		self.sac = {}
		self.df = 0
		self.nlevel = 0
		self.didx = []
		self.nidx = []
		self.eidx = []
		self.outpath = path + ".idx"

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
	def readgfa(self):
		nmap = {}
		f = open(self.path)
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
	def countdeg(self):
		for i in range(len(self.nodes)):
			c = max(len(self.nodes[i].ein), len(self.nodes[i].eout))
			try:
				self.sac[c].add(i)
			except:
				self.sac[c] = set([i])
			self.df += 1

	# going left to right, there should never be a reference to anything that
	# doesn't exist yet at a given level
	def writemeta(self, f):
		pass
	def gettag(self, n):
		if n is None:
			return None
		tag = self.nodes[n].tag
		assert(not tag is None)
		return tag
	def writeedges(self, f):
		print "writeedges"
		print self.eidx
		for i,e in enumerate(self.eidx):
			# ARGH
			e.remap(self.gettag(e.tail()), self.gettag(e.head()))
			e.write(f)
		return f.tell()
	# tagging: change of coordinates system, from `n' (nodes 1..N and
	# appended new supernodes) to `i' (from first to last uncovered node
	# of any type)
	def writenodes(self, f):
		print "writenodes"
		print self.nidx
		for i,n in enumerate(self.nidx):
			u = self.nodes[n]
			u.tag = i
			u.parent = self.gettag(u.parent)
			u.write(f)
		for i in self.nidx:
			print self.nodes[i]
		return f.tell()
	def writedict(self, f):
		noff = 0	# offsets are relative to start of section
		eoff = 0
		ntot = 0
		etot = 0
		print "writedict"
		for i in self.didx:
			ntot += i[0]
			put64(f, noff)
			put64(f, i[0])
			put64(f, ntot)
			noff += i[0] * sizeofNode
			etot += i[1]
			put64(f, eoff)
			put64(f, i[1])
			put64(f, etot)
			eoff += i[1] * sizeofEdge
		return f.tell()
	def writehdr2(self, f, dd, dn, de, dm):
		print "writehdr2"
		f.seek(0)
		put64(f, len(self.nidx))
		put64(f, len(self.eidx))
		put64(f, self.nlevel)
		put64(f, dd)
		put64(f, dn)
		put64(f, de)
		put64(f, dm)
	def writehdr(self, f):
		print "writehdr"
		put64(f, len(self.nidx))
		put64(f, len(self.eidx))
		put64(f, self.nlevel)
		# placeholders: offsets to sections (avoiding precalculating them)
		put64(f, 0)
		put64(f, 0)
		put64(f, 0)
		put64(f, 0)
		return f.tell()
	def mkindex(self):
		f = open(self.outpath, "wb")
		dd = self.writehdr(f)
		dn = self.writedict(f)
		de = self.writenodes(f)
		assert(de == dn + sizeofNode * len(self.nidx))
		dm = self.writeedges(f)
		assert(dm == de + sizeofEdge * len(self.eidx))
		self.writemeta(f)
		self.writehdr2(f, dd, dn, de, dm)
		f.close()
	def addlevel(self, nids, eids):
		self.didx = [(len(nids), len(eids))] + self.didx
		self.nidx = nids + self.nidx
		self.eidx = eids + self.eidx
		self.nlevel += 1

	# FIXME: paper model assumes only one entry per edge,
	# not one for each node
	def merge(self, l, lvln, lvle):
		print "merge:", l
		s = Node()
		sid = len(self.nodes)
		self.nodes.append(s)
		# it's not possible for two distinct nodes to use the same in/out edge
		# as another, ie. there will never be duplicates within ein or eout,
		# only between them
		for i in l:
			u = self.nodes[i]
			u.parent = sid
			s.w += u.w
			c = max(len(u.ein), len(u.eout))
			print c, self.sac[c]
			self.sac[c].discard(i)
			self.df -= 1
			s.ein |= u.ein
			s.eout |= u.eout
			lvln.append(i)
		# report all edges; internal ones are fixated; external ones need
		# relabeling and may be contracted into hyperedges
		dup = s.ein & s.eout
		s.ein -= dup
		s.eout -= dup
		for e in dup:
			e = self.edges[e]
			e.parent = sid
			lvle.append(e)
		# contract now redundant edges
		# FIXME: ignoring orientation here (both sides)
		dc = {}
		for e in s.eout:
			print "checking edge ", str(e), "::", self.edges[e]
			e = self.edges[e]
			e.parent = sid
			z = Edge(e.tail(), e.t&1, e.head(), e.h&1)
			z.parent = e.parent
			z.w = e.w
			lvle.append(z)
			print lvle
			n = e.head()
			# multiple incoming edges from the same node need to be contracted
			if n in dc:
				e.w += 1
			else:
				dc[n] = 1
				e.remap(sid, e.head())
		for e in s.ein:
			print "checking edge ", str(e), "::", self.edges[e]
			e = self.edges[e]
			e.parent = sid
			z = Edge(e.tail(), e.t&1, e.head(), e.h&1)
			z.parent = e.parent
			z.w = e.w
			lvle.append(z)
			print lvle
			n = e.tail()
			if n in dc:
				e.w += 1
			else:
				dc[n] = 1
				e.remap(e.tail(), sid)
		self.sac[max(len(u.ein), len(u.eout))].add(len(self.nodes)-1)
		self.df += 1
		for i in lvln:
			print "lvln", self.nlevel, i, self.nodes[i]
		for e in lvle:
			print "lvle", self.nlevel, e
		return True

	# FIXME: merge NON INTERSECTING, or all together
	def fuse(self):
		print "try fuse"
		# if level is empty, just fudge it and get nodes with highest tota
		# degree to merge into new supernodes; do so with *non-intersecting*
		# ones first, else merge all into one ultranode
		c = max(self.sac.keys())
		#l = []
		l = set()
		for i in self.sac[c]:
			s = set((i,))
			s |= set([self.edges[e].head() for e in self.nodes[i].eout])
			s |= set([self.edges[e].tail() for e in self.nodes[i].ein ])
			print "fuse", i, "neighborhood:", s
			#l.append(s)
			l |= s
		return [l]

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
		I = set([ self.edges[e].head() for e in dupe ])
		O = set([ self.edges[e].tail() for e in dupe ])
		m = I & O
		print "I", I
		print "O", O
		print "M", m
		s = []
		while dupe:
			e = dupe.pop()
			u = self.edges[e].tail()
			v = self.edges[e].head()
			x = [u, v]
			while u in m:	# no need to edit m, there's only one such entry
				ew = sin[u]	# w,u: want in[u], there can be only one also
				w = self.edges[ew].tail()
				x.append(w)
				dupe.discard(ew)
				u = w
			while v in m:
				ew = sout[v]	# u,v
				w = self.edges[ew].head()
				x.append(w)
				dupe.discard(ew)
				v = w
			s.append(x)
		print s
		assert(len(s) > 0)
		return s
	def mktree(self):
		print "df", self.df
		# FIXME: while more than one remain, unless break captures all
		while self.df > 1:
			l = self.unchop()
			# FIXME: unimplemented
			if not l:
				l = self.pop()
			# FIXME: unimplemented
			if not l:
				l = self.fuse()
			if not l:
				print "cannot reduce further??"
				self.addlevel(range(len(self.nodes)), range(len(self.edges)))
				break
			lvln = []
			lvle = []
			for i in l:
				self.merge(i, lvln, lvle)
			print lvln, lvle
			self.addlevel(lvln, lvle)
		if self.df == 1:
			self.addlevel([len(self.nodes)-1], list())

for i in sys.argv[1:]:
	print "processing", i
	s = Seymourbutz(i)
	s.readgfa()
	s.countdeg()	# avoidable
	s.mktree()
	s.mkindex()
