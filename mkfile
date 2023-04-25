</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=strpg
OFILES=\
	main.$O\
	cmd/cmd.$O\
	draw/draw.$O\
	fs/fs.$O\
	fs/gfa.$O\
	graph/graph.$O\
	graph/vertex.$O\
	layout/conga.$O\
	layout/force.$O\
	layout/layout.$O\
	layout/random.$O\
	rend/rend.$O\
	plan9/draw.$O\
	plan9/fs.$O\
	plan9/sys.$O\
	plan9/ui.$O\
	ui/ui.$O\
	util/htab.$O\
	util/vec.$O\

HFILES=\
	dat.h\
	fns.h\
	draw/drw.h\
	fs/fs.h\
	layout/layout.h\
	plan9/strpg.h\
	util/khash.h\

</sys/src/cmd/mkone

CFLAGS=$CFLAGS -p -D__plan9__ -D__${objtype}__ \
	-I/sys/include/npe -Iplan9 \
	-I. -Icmd -Idraw -Ifs -Igraph -Ilayout -Irend -Iui -Iutil \

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c

CLEANFILES=$OFILES
