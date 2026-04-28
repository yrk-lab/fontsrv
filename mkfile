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

$LIB $LIBHFILES: $LIBDIR
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

freetype-%:
	hget https://github.com/freetype/freetype/archive/refs/tags/$stem.tar.gz | tar xz
	f=freetype-$stem/src/truetype/ttgload.c
		sed -f port/builds/plan9/fixint.sed $f >$f.new && mv $f.new $f
	f=freetype-$stem/include/freetype/fttypes.h
		sed '/#include <stddef.h>/d' $f >$f.new && mv $f.new $f
	f=freetype-$stem/include/freetype/config/ftmodule.h
		sed '/(sdf|svg)_renderer_class/d' $f >$f.new && mv $f.new $f
	dircp port freetype-$stem
	sed 1q freetype-$stem/README	# version

FT=VER-2-14-3

vendor:V: freetype-$FT
	rm -rf libfreetype; mkdir -p libfreetype 
	disk/mkfs -s freetype-$FT -d libfreetype vendor.proto
	sed 1q libfreetype/README	# version
