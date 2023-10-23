</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=\
	strpg\
	coarsen\

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
	plan9/fs.$O\
	plan9/layout.$O\
	plan9/sys.$O\
	plan9/ui.$O\
	ui/ui.$O\
	util/htab.$O\
	util/print.$O\

OCOARSEN=\
	coarsen.$O\
	fs/em.$O\
	fs/fs.$O\
	plan9/fs.$O\
	plan9/sys.$O\
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

</sys/src/cmd/mkmany

CFLAGS=$CFLAGS -p -D__plan9__ -D__${objtype}__ \
	-I/sys/include/npe -Iplan9 \
	-I. -Icmd -Idraw -Ifs -Igraph -Iindex -Ilayout -Irend -Iui -Iutil \

#LDFLAGS=$LDFLAGS -p

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c

$O.coarsen: $OCOARSEN
	$LD $LDFLAGS -o $target $prereq

DIRS=\
	test\

clean:V:
	for(i in $DIRS) @{
		cd $i
		mk clean
	}
	rm -f *.[$OS] *.a[$OS] y.tab.? lex.yy.c y.debug y.output [$OS].??* $TARG $CLEANFILES
