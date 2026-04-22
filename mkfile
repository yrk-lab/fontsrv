</$objtype/mkfile

TARG=fontsrv
OFILES=\
	fontsrv.$O \
	freetype.$O \
	fc.$O\
	pjw.$O\

LIBDIR=libfreetype
#LIBDIR=/usr/glenda/src/freetype

LIB=$LIBDIR/libfreetype.a$O
LIBHFILES=$LIBDIR/builds/plan9/p9ftopt.h

HFILES=dat.h $LIBHFILES

CFLAGS=$CFLAGS -p -I$LIBDIR/builds/plan9 -I$LIBDIR/include \
	-D'FT_CONFIG_OPTIONS_H=<p9ftopt.h>' \
	-D'FT_CONFIG_STANDARD_LIBRARY_H=<p9lib.h>' \

BIN=/$objtype/bin

</sys/src/cmd/mkone

$LIB $LIBHFILES:
	cd $LIBDIR; mk install

install:V:	$BIN/$TARG /sys/lib/fontsrv.map /sys/man/4/fontsrv.4
/sys/lib/fontsrv.map:
	cp lib/fontsrv /sys/lib/fontsrv.map
/sys/man/4/fontsrv.4: man/4/fontsrv.4
	cp man/4/fontsrv.4 /sys/man/4/fontsrv.4

clean nuke:V:
	@{ cd $LIBDIR; mk $target }
	rm -f *.[$OS] [$OS].out $TARG

release:V:
	mk nuke
	rm -rf freetype-*

FT=VER-2-14-3

freetype-$FT:
	hget https://github.com/freetype/freetype/archive/refs/tags/$FT.tar.gz | tar xz

vendor:V: freetype-$FT
	rm -rf libfreetype; mkdir -p libfreetype 
	dircp port libfreetype	# upstreamed
	disk/mkfs -s freetype-$FT -d libfreetype vendor.proto
	sed -f port/builds/plan9/fixint.sed freetype-$FT/src/truetype/ttgload.c > libfreetype/src/truetype/ttgload.c # 2.13.3-2.14.3 # upstreamed
	sed '/#include <stddef.h>/d' freetype-$FT/include/freetype/fttypes.h > libfreetype/include/freetype/fttypes.h # upstreamed
