</$objtype/mkfile

TARG=fontsrv
OFILES=\
	fontsrv.$O \
	freetype.$O \
	fc.$O\
	pjw.$O\

HFILES=dat.h
LIBDIR=libfreetype

CFLAGS=$CFLAGS -p -I$LIBDIR/builds/plan9 -I$LIBDIR/include
LIB=$LIBDIR/libfreetype.a$O

BIN=/$objtype/bin

</sys/src/cmd/mkone

$LIB:V:
	cd $LIBDIR; mk -f builds/plan9/mkfile install

install:V:	$BIN/$TARG /sys/lib/fontsrv.map /sys/man/4/fontsrv.4
/sys/lib/fontsrv.map:
	cp lib/fontsrv /sys/lib/fontsrv.map
/sys/man/4/fontsrv.4: man/4/fontsrv.4
	cp man/4/fontsrv.4 /sys/man/4/fontsrv.4

clean nuke:V:
	@{ cd $LIBDIR; mk -f builds/plan9/mkfile $target }
	rm -f *.[$OS] [$OS].out $TARG

FT=VER-2-13-3

vendor:V:
	rm -rf libfreetype.new; mkdir -p libfreetype.new
	hget https://github.com/freetype/freetype/archive/refs/tags/$FT.tar.gz | tar xz
	disk/mkfs -s freetype-$FT -d libfreetype.new vendor.proto
	dircp port libfreetype.new
	sed -f fixint.sed freetype-$FT/src/truetype/ttgload.c > libfreetype.new/src/truetype/ttgload.c
	rm -rf libfreetype freetype-$FT
	mv libfreetype.new libfreetype
	mk clean

