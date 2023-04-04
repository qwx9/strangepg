</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=strpg
OFILES=\
	main.$O\
	cmd/cmd.$O\
	draw/draw.$O\
	draw/geom.$O\
	fs/fs.$O\
	fs/gfa.$O\
	graph/fertex.$O\
	graph/graph.$O\
	graph/vertex.$O\
	layout/force.$O\
	layout/layout.$O\
	layout/conga.$O\
	rend/rend.$O\
	plan9/draw.$O\
	plan9/fs.$O\
	plan9/sys.$O\
	plan9/ui.$O\
	ui/ui.$O\
	util/vec.$O\

HFILES=\
	dat.h\
	fns.h\
	khash.h\
	kvec.h\
	cmd/cmd.h\
	draw/drawprv.h\
	draw/vdraw.h\
	fs/fs.h\
	fs/fsprv.h\
	layout/layoutprv.h\
	plan9/strpg.h\
	rend/rend.h\
	ui/ui.h\

</sys/src/cmd/mkone

CFLAGS=$CFLAGS -p -D__plan9__ -D__${objtype}__ \
	-I/sys/include/npe -Iplan9 \
	-I. -Icmd -Idraw -Ifs -Igraph -Ilayout -Irend -Iui \

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c

CLEANFILES=$OFILES
