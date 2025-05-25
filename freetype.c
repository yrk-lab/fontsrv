#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <bio.h>
#include <ft2build.h>
#include 	FT_FREETYPE_H
#include "dat.h"

#define DBG	if(0)

static FT_Library  lib;
static int dpi = 96;

enum {
	Rnullpic = 0x2400,	// Graphic pictures for control codes
};

static int ftloadmemimage(Memimage*, Rectangle, FT_Bitmap*);
static void drawfcbox(Memimage*, Fontchar*, int, int);
static char* fterrstr(int);

static Xfont*
allocxfont(void)
{
	Xfont* font;

	xfont = realloc(xfont, (nxfont+1)*sizeof xfont[0]);
	font = &xfont[nxfont];
	memset(font, 0, sizeof *font);
	nxfont++;
	return font;
}

void
xfontinit(void)
{
	FT_Face face;
	FT_Error err;
	Biobuf *b;
	Xfont *xf;
	char *s, *file, *line, *f[3];
	int ntok;

	memimageinit();

	if((err = FT_Init_FreeType(&lib)) != 0){
		fprint(2, "freetype initialization failed: %s\n", fterrstr(err));
		exits("freetype failed");
	}

	file = "/sys/lib/fontsrv.map";
	b = Bopen(file, OREAD);
	while((line = Brdline(b, '\n')) != nil){
		line[Blinelen(b)-1] = 0;
		s = strchr(line, '#');
		if(s != nil && (s == line || s[-1] == ' ' || s[-1] == '\t'))
			*s = '\0'; 	/* chop comment iff after whitespace */
		ntok = tokenize(line, f, nelem(f));
		switch(ntok){
		case 1:
			if((err = FT_New_Face(lib, f[0], 0, &face)) != 0){
				fprint(2, "%s: load failed: %s\n", f[0], fterrstr(err));
				continue;
			}
			xf = allocxfont();
			xf->name = smprint("%s-%s", face->family_name, face->style_name);
			xf->fontfile = strdup(f[0]);
			FT_Done_Face(face);
			break;
		case 2:
			xf = allocxfont();
			xf->name = strdup(f[0]);
			xf->fontfile = strdup(f[1]);
			break;
		}
	}
	Bterm(b);
}

char*
xfload(Xfont *f)
{
	FT_Face face;
	FT_Error err;
	FT_ULong c;
	FT_UInt gi;
	int page;
	double k;

	if(f->loaded)
		return nil;

	err = FT_New_Face(lib, f->fontfile, f->index, &face);
	if(err != 0){
		fprint(2, "%s: load failed: %s\n", f->fontfile, fterrstr(err));
		return fterrstr(err);
	}

	if(!FT_IS_SCALABLE(face)){
		fprint(2, "%s: bitmap fonts are not supported\n", f->fontfile);
		FT_Done_Face(face);
		return "bitmap fonts are not supported";
	}

	f->ptheight = (face->ascender - face->descender) * dpi/72.0 / face->units_per_EM;
	f->ptascent = face->ascender * dpi/72.0 / face->units_per_EM;
	k = dpi/72.0 /face->units_per_EM;
	f->ptxmax = (int)(k*face->max_advance_width + 0.99999999);
	f->size = -1;
	
	for(c=FT_Get_First_Char(face, &gi); gi != 0; c=FT_Get_Next_Char(face, c, &gi)){
		if(c > Runemax)
			break;
		page = c/PageSize;
		if(!f->page[page]){
			f->page[page] = 1;
			f->npage++;
		}
	}

	FT_Done_Face(face);
	f->loaded = 1;
	return nil;
}

void
xfscale(Xfont *font, int size)
{
	if(font->size == size)
		return;
	font->size = size;
	font->height = size*font->ptheight  + 0.99999999;
	font->ascent = size*font->ptascent + 0.99999999;
	font->xmax = size*font->ptxmax + 0.99999999;
}

Memsubfont*
xfsubfont(Xfont *xf, char *name, int start, int n, int mono)
{
	FT_Error err;
	FT_Face face;
	FT_GlyphSlot glyph;
	FT_Bitmap *bits;
	int x, xmax, c, rune, height, ascent, lmode, chan;
	Fontchar *info, *i;
	Memimage *m, *m1, *mc;
	Memsubfont *sf;

	USED(name);

	if((err = FT_New_Face(lib, xf->fontfile, xf->index, &face)) != 0){
		werrstr("can't load: %s", fterrstr(err));
		return nil;
	}
	if((err = FT_Set_Char_Size(face, 0, xf->size<<6, dpi, dpi)) != 0){
		FT_Done_Face(face);
		werrstr("can't scale: %s", fterrstr(err));
		return nil;
	}

	chan = mono? GREY1:GREY8;
//	n = hi-lo+1;
	height = xf->height;
	ascent = xf->ascent;
	xmax = xf->xmax;
	if(xmax == 0)
		xmax = height;

	FT_Set_Pixel_Sizes(face, 0, height);		// just in case

	m = allocmemimage(Rect(0, 0, n*xmax, height), chan);
	mc = allocmemimage(Rect(0, 0, 3*xmax, height), chan);
	info = malloc((n+1) * sizeof info[0]);
	if(m == nil || mc == nil || info == nil) {
		werrstr("mksubfont: alloc: %r");
		freememimage(m);
		freememimage(mc);
		free(info);
		FT_Done_Face(face);
		return nil;
	}

	memfillcolor(m, DBlack);

	lmode = FT_LOAD_RENDER;
	if(mono)
		lmode |= FT_LOAD_TARGET_MONO;
	if(!mono && xf->size > 9)
		lmode |= FT_LOAD_NO_HINTING;

	info[0].x = 0;
	for(c=0; c<n; c++){
		i = &info[c];
		i->left = 0;
		i->top = 0;
		i->bottom = height;
		i->width = 0;
		(i+1)->x = i->x;

		rune = start+c;
		if(FT_Load_Char(face, rune, lmode) != 0){
			if(rune == 0){
				/* must have a valid fallback char */
				drawfcbox(m, i, height, ascent);
				continue;
			}else if(rune < 32){
				if(FT_Load_Char(face, rune+Rnullpic, lmode) != 0)
					continue;
			}else
				continue;
		}
		glyph = face->glyph;
		bits = &face->glyph->bitmap;

		i->left = glyph->bitmap_left;
		i->top = ascent - glyph->bitmap_top;
		i->bottom = i->top+bits->rows;
		if(i->bottom > height+1)
			i->bottom = height+1;
		i->width = (glyph->advance.x+(1<<5))>>6;
		(i+1)->x = i->x+bits->width;

		if(i->left < 0){
			/* fix artifacts when e.g. Symbola lets "j" on top of "e" in "ej" */
			i->width += -i->left;
			i->left = 0;
		}
		if(rune == 0 && (i->width == 0 || i->x == (i+1)->x)){
			/* must have a valid fallback char */
			drawfcbox(m, i, height, ascent);
			continue;
		}

		ftloadmemimage(mc, Rect(0, 0, bits->width, bits->rows), bits);
		memimagedraw(m, Rect(i->x, i->top, (i+1)->x, i->bottom), mc, mc->r.min, nil, ZP, S);
	}
	freememimage(mc);

	x = info[n].x;
	if(mono)
		x += -x & 31;
	else
		x += -x & 3;
	m1 = allocmemimage(Rect(0, 0, x, height), m->chan);
	if(m1 == nil){
		freememimage(m);
		free(info);
		FT_Done_Face(face);
		return nil;
	}
	memimagedraw(m1, m1->r, m, m->r.min, memopaque, ZP, S);
	freememimage(m);

	sf = allocmemsubfont(nil, n, height, ascent, info, m1);
	if(sf == nil){
		freememimage(m1);
		free(info);
		FT_Done_Face(face);
		return nil;
	}

	FT_Done_Face(face);
	return sf;
}

static int
ftloadmemimage(Memimage *i, Rectangle r, FT_Bitmap* bits)
{
	uchar *q;
	int n, ndata, y, bpl;

	switch(bits->pixel_mode){
	case FT_PIXEL_MODE_MONO:
		if(i->depth != 1){
			werrstr("bad pixel mode %d for depth %d", bits->pixel_mode, i->depth);
			return -1;
		}
		break;
	case FT_PIXEL_MODE_GRAY:
		if(i->depth != 8){
			werrstr("bad pixel mode %d for depth %d", bits->pixel_mode, i->depth);
			return -1;
		}
		break;
	}

	q = bits->buffer;
	bpl = bits->pitch;
	ndata = 0;

	for(y=r.min.y; y < r.max.y; y++, q += bpl){
		n = loadmemimage(i, Rect(r.min.x, y, r.max.x, y+1), q, bits->width);
		if(n < 0)
			return -1;
		ndata += n;
	}

	return ndata;
}

static void
drawfcbox(Memimage* m, Fontchar* i, int height, int ascent)
{
	int w;
	Rectangle r;

	w = height - ascent;
	if(w < 9)
		w = 9;
	(i+1)->x = i->x+w;
	i->width = w+1;
	i->left = 1;
	i->top = 0;
	i->bottom = ascent;

	r = Rect(i->x, i->top, (i+1)->x, i->bottom);
	memimagedraw(m, r, memwhite, ZP, nil, ZP, S);
	memimagedraw(m, insetrect(r, 2), memblack, ZP, nil, ZP, S);
}


/*
 * get the freetype error strings - lifted from /usr/inferno/libfreetype/freetype.c
 */

#define FT_NOERRORDEF_(l,c,t)
#define FT_ERRORDEF_(l,c,t)	c,t,

static struct FTerr {
	int		code;
	char*	text;
} fterrs[] = {
#include FT_ERROR_DEFINITIONS_H	/* "fterrdef.h" */
	-1, "",
};

static char*
fterrstr(int code)
{
	int i;

	if(code == 0)
		return nil;
	for(i = 0; fterrs[i].code > 0; i++) {
		if(fterrs[i].code == code)
			return fterrs[i].text;
	}
	return "unknown FreeType error";
}
