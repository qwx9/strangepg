</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=strpg
OFILES=\
	main.$O\
	cmd/cmd.$O\
	fs/fs.$O\
	fs/gfa.$O\
	graph/graph.$O\
	graph/vertex.$O\
	layout/layout.$O\
	layout/conga.$O\
	rend/rend.$O\
	plan9/view.$O\
	plan9/fs.$O\
	plan9/render.$O\
	plan9/sys.$O\
	ui/ev.$O\
	ui/ui.$O\
	view/sdl.$O\

HFILES=\
	dat.h\
	fns.h\
	khash.h\
	kvec.h\
	cmd/cmd.h\
	fs/fs.h\
	fs/fsprv.h\
	graph/graph.h\
	layout/layout.h\
	layout/layoutprv.h\
	plan9/strpg.h\
	rend/rend.h\
	ui/ui.h\
	view/view.h\

</sys/src/cmd/mkone

CFLAGS=$CFLAGS -p -D__plan9__ -D__${objtype}__ \
	-I/sys/include/npe -Iplan9 \
	-I. -Icmd -Iview -Ifs -Igraph -Ilayout -Irend -Iui \

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c

CLEANFILES=$OFILES
