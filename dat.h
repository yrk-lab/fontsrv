typedef struct Xfont Xfont;

enum {
	PageSize = 32,
	PageMask = (1<<24)/PageSize - 1,
	NPAGE = (1<<24)/PageSize,
};

struct Xfont
{
	char	*name;
	char	*fontfile;
	int	index;
	int	loaded;
	int	npage;
	uchar	page[NPAGE];
	double	ptheight;	// of 1pt
	double	ptascent;
	double	ptxmax;
	int	size;		// set with ftscale()
	int	height;
	int	ascent;
	int	xmax;
};

void	xfontinit(void);
char* xfload(Xfont*);
void xfscale(Xfont*, int);
Memsubfont*	xfsubfont(Xfont*, char*, int, int, int);

extern Xfont *xfont;
extern int nxfont;
