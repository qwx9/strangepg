</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=\
	strpg\
	coarsen\

OFILES=\
	strpg.$O\
	draw/color.$O\
	draw/draw.$O\
	fs/fs.$O\
	fs/gfa.$O\
	fs/index.$O\
	fs/load.$O\
	graph/graph.$O\
	graph/vertex.$O\
	layout/conga.$O\
	layout/force.$O\
	layout/layout.$O\
	layout/random.$O\
	rend/rend.$O\
	plan9/draw.$O\
	plan9/fs.$O\
	plan9/layout.$O\
	plan9/sys.$O\
	plan9/ui.$O\
	ui/ui.$O\
	util/htab.$O\

HFILES=\
	dat.h\
	fns.h\
	draw/drw.h\
	fs/fs.h\
	layout/layout.h\
	lib/khash.h\
	plan9/strpg.h\
	util/dynar.h\

</sys/src/cmd/mkmany

CFLAGS=$CFLAGS -p -D__plan9__ -D__${objtype}__ \
	-I/sys/include/npe -Iplan9 \
	-I. -Icmd -Idraw -Ifs -Igraph -Ilayout -Irend -Iui -Iutil \

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c

$O.coarsen: n/coarsen.$O util/htab.$O plan9/sys.$O
	$LD $LDFLAGS -o $target $prereq

CLEANFILES=$OFILES
