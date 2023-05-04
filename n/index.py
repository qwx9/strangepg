#!/bin/python
# -*- coding: utf-8 -*-
import sys

def pass2(sac, es, ein, eout):
	def merge(x):
		"""
		# FIXME: write merge function: create supernode and fix links
		then update tree
		do not remove edges, only replace them
			... repair links? point to/from new edge from all neighbors
			ein[u].discard(... edge id ...)
			ein[v].discard(... edge id ...)
			... remove (all) merged edges ...
			... retain remainder summed set of edges ...
			parent[u] = n
			parent[v] = n
		"""
		pass
	def untangle():
		pass
		# get nodes with highest total degree
		# merge into new supernode
		# insert supernode in hierarchical tree

	def pop():
		pass

	def unchop():
		# stupidest possible approach to start with
		# unchopping two consecutive nodes is only possible if both
		# have only one outgoing and one incoming edge and share a
		# common edge; in the set of nodes with max degree 1, there
		# can be no two nodes that share the same outgoing or
		# incoming edge, the only overlaps may be between in and out
		# → intersect in and out
		if not 1 in sac or len(sac[1]) == 0:	# nodes
			return
		# FUCK IT
		EDI = {}
		EDO = {}
		EI = set()
		EO = set()
		print "sac[1]", sac[1]
		for n in sac[1]:
			EDI.update({n:ein[n]})		# node → edge id's
			EDO.update({n:eout[n]})
			print "n", n, "in", ein[n], "out", eout[n]
			EI |= ein[n]				# edge id's; to
			EO |= eout[n]				# from
		d = EI & EO
		print "EI", EI
		print "EO", EO
		print "EDI", EDI
		print "EDO", EDO
		print "d", d
#		if not d:
#			return
		I = set([ es[e][1] for e in EI ])	# nodes; to (in)
		O = set([ es[e][0] for e in EO ])	# from (out)
		m = I & O
		print "I", I
		print "O", O
		print "M", m
		s = []
		while d:
			e = d.pop()
			u = es[e][1]
			v = es[e][0]
			x = [u, v]
			while u in m:	# no need to edit m, there's only one such entry
				ew = EDI[u]	# w,u: want in[u], there can be only one also
				w = es[ew][0]
				x.append(w)
				d.discard(ew)
				u = w
			while v in m:
				ew = EDO[v]	# u,v
				w = es[ew][1]
				x.append(w)
				d.discard(ew)
				v = w
			s.append(x)
		print s

		# FIXME: possibility: 	5+4+, 4-5+ and the like
		# we have to choose whether to ignore orientation or not, i would

		for x in s:
			merge(x)

	while True:
		unchop()
		pop()
		untangle()
		break

def pass1(gfa):
	nodes = []
	nid = {}
	seq = []
	# fuck it
	es = []
	ein = {}
	eout = {}
	sac = {}
	f = open(gfa)
	while True:
		s = f.readline()
		if not s:
			break
		s = s.strip().split()
		if s[0] == 'S':
			i = len(nodes)
			ein[i] = set()
			eout[i] = set()
			nodes += [s[1]]
			nid[s[1]] = i
			#seq[nn] = s[2]		# FIXME: write out directly
		elif s[0] == 'L':
			u = nid[s[1]]
			v = nid[s[3]]
			e = (u, v, s[2], s[4])
			eout[u].add(len(es))
			ein[v].add(len(es))
			es.append(e)
			#ovlap[...]			# FIXME: write out directly
	f.close()
	for i in range(len(nodes)):
		c = max(len(ein[i]), len(eout[i]))
		try:
			sac[c].add(i)
		except:
			sac[c] = set([i])
	return sac, es, ein, eout

if len(sys.argv) < 2:
	print "usage: $0 GFA"
	sys.exit(1)
sac, es, ein, eout = pass1(sys.argv[1])
pass2(sac, es, ein, eout)
