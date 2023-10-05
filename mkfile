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

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c

$O.coarsen: $OCOARSEN
	$LD $LDFLAGS -o $target $prereq

OTEST=\
	fs/em.$O\
	plan9/fs.$O\
	plan9/sys.$O\
	util/print.$O\

# FIXME: test/mkfile, test/Makefile
$O.testct01: $OTEST test/testct01.$O
	$LD $LDFLAGS -o $target $prereq

$O.testct02: $OTEST test/testct02.$O
	$LD $LDFLAGS -o $target $prereq

$O.testct03: $OTEST test/testct03.$O
	$LD $LDFLAGS -o $target $prereq

$O.testct04: $OTEST test/testct04.$O
	$LD $LDFLAGS -o $target $prereq

$O.testct05: $OTEST test/testct05.$O
	$LD $LDFLAGS -o $target $prereq

CLEANFILES=$OFILES\
	$OCOARSEN\
	$OTEST\
	test/testct01.$O\
	test/testct02.$O\
	test/testct03.$O\
	test/testct04.$O\
	test/testct05.$O\
