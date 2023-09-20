</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=\
	strpg\
	coarsen2\

OFILES=\
	strpg.$O\
	draw/color.$O\
	draw/draw.$O\
	fs/em.$O\
	fs/fs.$O\
	fs/gfa.$O\
	fs/index.$O\
	fs/load.$O\
	graph/graph.$O\
	graph/vertex.$O\
	index/fs.$O\
	index/index.$O\
	layout/conga.$O\
	layout/force.$O\
	layout/layout.$O\
	layout/random.$O\
	rend/rend.$O\
	plan9/draw.$O\
	plan9/em.$O\
	plan9/fs.$O\
	plan9/layout.$O\
	plan9/sys.$O\
	plan9/ui.$O\
	ui/ui.$O\
	util/htab.$O\
	util/print.$O\

HFILES=\
	dat.h\
	fns.h\
	draw/drw.h\
	fs/em.h\
	fs/fs.h\
	index/index.h\
	layout/layout.h\
	lib/khash.h\
	plan9/strpg.h\
	util/dynar.h\

OCOARSEN=\
	n/coarsen2.$O\

</sys/src/cmd/mkmany

CFLAGS=$CFLAGS -p -D__plan9__ -D__${objtype}__ \
	-I/sys/include/npe -Iplan9 \
	-I. -Icmd -Idraw -Ifs -Igraph -Iindex -Ilayout -Irend -Iui -Iutil \

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c

$O.coarsen2: $OCOARSEN plan9/sys.$O plan9/fs.$O fs/fs.$O util/print.$O
	$LD $LDFLAGS -o $target $prereq

CLEANFILES=$OFILES $OCOARSEN
