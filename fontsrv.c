#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <thread.h>
#include <fcall.h>
#include <9p.h>
#include "dat.h"

char	*mtpt = "/mnt/font";
char	*service = "font";
Srv	fs;

Xfont	*xfont;
int	nxfont;

static char Enotfound[] = "file not found";

enum
{
	Qtopdir = 0,
	Qfontdir,
	Qsizedir,
	Qfontfile,
	Qsubfontfile,
};

#define QID(q,f,s,m,p)	(vlong)((q)|((f)<<4)|((s)<<20)|((m)<<28)|((vlong)(p)<<32))
#define FILE(q) 	(int)((q).path & 0xF)
#define FONT(q)	(int)(((q).path>>4) & 0xFFFF)
#define SIZE(q) 	(int)(((q).path>>20) & 0xFF)
#define MONO(q)	(int)(((q).path>>28) & 0x1)
#define PAGE(q)  	(int)(((q).path>>32) & PageMask)

static int sizes[] = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 24, 28 };

void
usage(void)
{
	fprint(2, "usage: fontsrv [-m mtpt] [-s service]\n");
	exits("usage");
}

static char*
fswalk1(Fid *f, char *name, Qid *qid)
{
	int i, n, k1, sz;
	char *p, *err;
	Qid q;
	Xfont *font;

	q = f->qid;
	q.vers = 0;

	switch(FILE(f->qid)) {
	default:
		return Enotfound;
	case Qtopdir:
		if(strcmp(name, "..") == 0)
			break;
		for(i=0; i<nxfont; i++){
			if(strcmp(xfont[i].name, name) == 0){
				q.path = QID(Qfontdir, i, 0, 0, 0);
				q.type = QTDIR;
				break;
			}
		}
		if(i == nxfont)
			return Enotfound;
		break;
	case Qfontdir:
		if(strcmp(name, "..") == 0){
			q.path = Qtopdir;
			q.type = QTDIR;
			break;
		}
		sz = strtol(name, &p, 10);
		if(sz == 0)
			return Enotfound;
		k1 = 1;
		if(*p == 'a'){
			k1 = 0;
			p++;
		}
		if(*p != 0)
			return Enotfound;
		q.path = QID(Qsizedir, FONT(f->qid), sz, k1, 0);
		q.type = QTDIR;
		break;
	case Qsizedir:
		if(strcmp(name, "..") == 0){
			q.path = QID(Qfontdir, FONT(f->qid), 0, 0, 0);
			q.type = QTDIR;
			break;
		}
		sz = SIZE(f->qid);
		k1 = MONO(f->qid);
		if(strcmp(name, "font") == 0){
			q.path = QID(Qfontfile, FONT(f->qid), sz, k1, 0);
			q.type = 0;
			break;
		}
		font = &xfont[FONT(f->qid)];
		err = xfload(font);
		if(err != nil)
			return err;
		p = name;
		if(*p != 'x')
			return Enotfound;
		p++;
		n = strtoul(p, &p, 16);
		i = (n/PageSize)&PageMask;
		if(p < name+5 || p > name+7 || strcmp(p, ".bit") != 0 || n%PageSize != 0 || !font->page[i])
			return Enotfound;
		q.path = QID(Qsubfontfile, FONT(f->qid), sz, k1, i);
		q.type = 0;
		break;
	}
	*qid = q;
	f->qid = q;
	return nil;
}

static void
filldir(Dir *d)
{
	Qid q;
	Xfont *font;
	char *err, buf[64];

	q.path = d->qid.path;
	q.type = 0;
	q.vers = 0;

	memset(d, 0, sizeof *d);
	d->mode= 0444;
	d->muid = estrdup9p("");
	d->uid = estrdup9p("font");
	d->gid = estrdup9p("font");

	switch(FILE(q)){
	default:
		sysfatal("filldir %#llux", q.path);
	case Qtopdir:
		q.type = QTDIR;
		d->name = estrdup9p("/");
		break;
	case Qfontdir:
		q.type = QTDIR;
		font = &xfont[FONT(q)];
		d->name = estrdup9p(font->name);
		break;
	case Qsizedir:
		q.type = QTDIR;
		snprint(buf, sizeof buf, "%d%s", SIZE(q), MONO(q) ? "" : "a");
		d->name = estrdup9p(buf);
		break;
	case Qfontfile:
		font = &xfont[FONT(q)];
		err = xfload(font);
		if(err == nil)
			d->length = 11+1+11+1+font->npage*(8+1+8+1+11+1);
		d->name = estrdup9p("font");
		break;
	case Qsubfontfile:
		snprint(buf, sizeof buf, "x%06x.bit", (int)PAGE(q)*PageSize);
		d->name = estrdup9p(buf);
		break;
	}
	d->qid = q;
	if(q.type == QTDIR)
		d->mode |= DMDIR | 0111;
}

int
rootgen(int i, Dir *d, void *)
{
	if(i >= nxfont)
		return -1;
	d->qid.path = QID(Qfontdir, i, 0, 0, 0);
	filldir(d);
	return 0;
}

int
fontgen(int i, Dir *d, void *v)
{
	Fid *f;
	
	f = v;
	if(i >= 2*nelem(sizes))
		return -1;
	d->qid.path = QID(Qsizedir, FONT(f->qid), sizes[i/2], i&1, 0);
	filldir(d);
	return 0;
}

int
sizegen(int i, Dir *d, void *v)
{
	Qid q;
	Fid *f;
	Xfont *font;
	int sz, k1, j;
	char* err;

	f = v;
	q = f->qid;
	sz = SIZE(f->qid);
	k1 = MONO(f->qid);

	if(i == 0) {
		d->qid.path = QID(Qfontfile, FONT(f->qid), sz, k1, 0);
		filldir(d);
		return 0;
	}
	i--;
	font = &xfont[FONT(q)];
	err = xfload(font);
	if(err != nil)
		return -1;
	for(j=0; j<nelem(font->page); j++){
		if(font->page[j] == 0)
			continue;
		if(i == 0){
			d->qid.path = QID(Qsubfontfile, FONT(f->qid), sz, k1, j);
			filldir(d);
			return 0;
		}
		i--;
	}
	return -1;
}

void
fsattach(Req *r)
{
	Qid q;

	q.path = Qtopdir;
	q.type = QTDIR;
	q.vers = 0;

	r->ofcall.qid = q;
	r->fid->qid = q;
	respond(r, nil);
}

void
fsopen(Req *r)
{
	if(r->ifcall.mode != OREAD) {
		respond(r, "permission denied");
		return;
	}
	r->ofcall.qid = r->fid->qid;
	respond(r, nil);
}

void
fsreadfont(Req *r)
{
	int i, lo, hi;
	Fid* f;
	Xfont* font;
	char *data, *err;
	Fmt fmt;

	f = r->fid;
	font = &xfont[FONT(f->qid)];

	if((err = xfload(font)) != nil){
		respond(r, err);
		return;
	}
	xfscale(font, SIZE(f->qid));

	fmtstrinit(&fmt);
	fmtprint(&fmt, "%11d %11d\n", font->height, font->ascent);
	for(i=0; i<nelem(font->page); i++){
		if(font->page[i] == 0)
			continue;
		lo = i*PageSize;
		hi = ((i+1)*PageSize) - 1;
		fmtprint(&fmt, "0x%06x 0x%06x x%06x.bit\n", lo, hi, lo);
	}
	data = fmtstrflush(&fmt);
	readstr(r, data);
	respond(r, nil);
	free(data);
}

static void
packinfo(Fontchar *fc, uchar *p, int n)
{
	int j;

	for(j=0;  j<=n;  j++){
		p[0] = fc->x;
		p[1] = fc->x>>8;
		p[2] = fc->top;
		p[3] = fc->bottom;
		p[4] = fc->left;
		p[5] = fc->width;
		fc++;
		p += 6;
	}
}

void
fsreadsubfont(Req *r)
{
	int size, i;
	Fid* f;
	char *data, *err, cbuf[12];
	Memsubfont *sf;
	Memimage *m;
	Xfont *xf;

	f = r->fid;
	xf = &xfont[FONT(f->qid)];
	if((err = xfload(xf)) != nil){
		respond(r, err);
		return;
	}
	if(f->aux == nil) {
		i = PAGE(f->qid);
		xfscale(xf, SIZE(f->qid));
		f->aux = xfsubfont(xf, nil, i*PageSize, PageSize, MONO(f->qid));
		if(f->aux == nil){
			responderror(r);
			return;
		}
	}
	sf = f->aux;
	m = sf->bits;
	if(r->ifcall.offset < 5*12){
		data = smprint("%11s %11d %11d %11d %11d ",
			chantostr(cbuf, m->chan), m->r.min.x, m->r.min.y, m->r.max.x, m->r.max.y);
		readstr(r, data);
		respond(r, nil);
		free(data);
		return;
	}
	r->ifcall.offset -= 5*12;
	size = bytesperline(m->r, chantodepth(m->chan)) * Dy(m->r);
	if(r->ifcall.offset < size) {
		readbuf(r, byteaddr(m, m->r.min), size);
		respond(r, nil);
		return;
	}
	r->ifcall.offset -= size;
	data = emalloc9p(3*12+6*(sf->n+1));
	sprint(data, "%11d %11d %11d ", sf->n, sf->height, sf->ascent);
	packinfo(sf->info, (uchar*)data+3*12, sf->n);
	readbuf(r, data, 3*12+6*(sf->n+1));
	respond(r, nil);
	free(data);
}

void
fsread(Req *r)
{
	Fid* f;

	f = r->fid;
	switch(FILE(f->qid)){
	default:
		respond(r, "internal error");
		return;
	case Qtopdir:
		dirread9p(r, rootgen, nil);
		break;
	case Qfontdir:
		dirread9p(r, fontgen, f);
		break;
	case Qsizedir:
		dirread9p(r, sizegen, f);
		break;
	case Qfontfile:
		fsreadfont(r);
		return;
	case Qsubfontfile:
		fsreadsubfont(r);
		return;
	}
	respond(r, nil);
}

void
fsdestroyfid(Fid *fid)
{
	Memsubfont *sf;

	sf = fid->aux;
	freememsubfont(sf);
	fid->aux = nil;
}

void
fsstat(Req *r)
{
	r->d.qid.path = r->fid->qid.path;
	filldir(&r->d);
	respond(r, nil);
}

void
threadmain(int argc, char **argv)
{
	ARGBEGIN{
	case 'D':
		chatty9p++;
		break;
	case 'm':
		mtpt = EARGF(usage());
		break;
	case 's':
		service = EARGF(usage());
		break;
	default:
		usage();
	}ARGEND

	if(argc != 0)
		usage();
	
	fmtinstall('R', Rfmt);
	fmtinstall('P', Pfmt);
	xfontinit();
	
	fs.attach = fsattach;
	fs.open = fsopen;
	fs.read = fsread;
	fs.stat = fsstat;
	fs.walk1 = fswalk1;
	fs.destroyfid = fsdestroyfid;

	threadpostmountsrv(&fs, service, mtpt, MREPL);
	threadexits(nil);
}
