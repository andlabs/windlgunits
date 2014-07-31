// 31 july 2014
#include "windlgunits.h"

WCHAR *modenames[nModes] = {
	[mode_noSystem_GetTextExtentPoint32] = L"GetTextExtentPoint32()",
	[mode_noSystem_tmAveCharWidth] = L"tmAveCharWidth",
	[mode_System_GetTextExtentPoint32] = L"System + GetTextExtentPoint32()",
	[mode_System_tmAveCharWidth] = L"System + tmAveCharWidth",
	[mode_MapDialogRect] = L"MapDialogRect()",
};

static WCHAR *colnames[] = {
	L"Mode",
	L"Width",
	L"Height",
	NULL,
};

void initResultsListView(HWND hwnd)
{
	LVCOLUMNW col;
	int i;

	for (i = 0; colnames[i] != NULL; i++) {
		ZeroMemory(&col, sizeof (LVCOLUMNW));
		col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
		col.fmt = LVCFMT_LEFT;
		col.pszText = colnames[i];
		col.iSubItem = i;
		col.iOrder = i;
		if (SendMessageW(hwnd, LVM_INSERTCOLUMN, (WPARAM) i, (LPARAM) (&col)) == (LRESULT) -1)
			abort();//TODO
	}
	refreshResultsListView(hwnd);
}

void refreshResultsListView(HWND hwnd)
{
	if (SendMessageW(hwnd, LVM_SETITEMCOUNT, (WPARAM) nModes, 0) == 0)
		abort();//TODO
}

struct baseunits {
	int xbase_gtep32;
	int xbase_acw;
	int ybase;
};

struct dlgunitsargs {
	int x;
	int y; 
	int *xs;
	int *ys;
	HWND hwnd;
	HDC dc;
	HFONT font;
	HFONT prevfont;
	struct baseunits fontb;
	struct baseunits sysb;
};

static void noSystem(int i, struct dlgunitsargs *args)
{
	int xbase;

	xbase = args->fontb.xbase_gtep32;
	if (i == mode_noSystem_tmAveCharWidth)
		xbase = args->fontb.xbase_acw;
	args->xs[i] = MulDiv(args->x, xbase, 4);
	args->ys[i] = MulDiv(args->y, args->fontb.ybase, 8);
}

static void withSystem(int i, struct dlgunitsargs *args)
{
	int xbase, sysxbase;

	xbase = args->fontb.xbase_gtep32;
	sysxbase = args->sysb.xbase_gtep32;
	if (i == mode_System_tmAveCharWidth) {
		xbase = args->fontb.xbase_acw;
		sysxbase = args->sysb.xbase_acw;
	}
	args->xs[i] = 2 * args->x * (((double) xbase) / ((double) sysxbase));
	args->ys[i] = 2 * args->y * (((double) (args->fontb.ybase)) / ((double) (args->sysb.ybase)));
}

static void mapDialogRect(int i, struct dlgunitsargs *args)
{
	RECT r;

	r.left = 0;
	r.top = 0;
	r.right = args->x;
	r.bottom = args->y;
	if (MapDialogRect(args->hwnd, &r) == 0)
		abort();//TODO
	if (r.left != 0 || r.top != 0)
		abort();//TODO sanity check
	args->xs[i] = r.right;
	args->ys[i] = r.bottom;
}

static void (*modefuncs[nModes])(int, struct dlgunitsargs *) = {
	[mode_noSystem_GetTextExtentPoint32] = noSystem,
	[mode_noSystem_tmAveCharWidth] = noSystem,
	[mode_System_GetTextExtentPoint32] = withSystem,
	[mode_System_tmAveCharWidth] = withSystem,
	[mode_MapDialogRect] = mapDialogRect,
};

#define GTEP32STR L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

static void getAverages(HDC dc, struct baseunits *out)
{
	SIZE extents;
	TEXTMETRIC tm;

	if (GetTextExtentPoint32(dc, GTEP32STR, 52, &extents) == 0)
		abort();//TODO
	out->xbase_gtep32 = extents.cx;
	if (GetTextMetrics(dc, &tm) == 0)
		abort();//TODO
	out->xbase_acw = tm.tmAveCharWidth;
	out->ybase = tm.tmHeight;
}

void runCalculations(HWND hwnd, HFONT font, int x, int y, int *xs, int *ys)
{
	struct dlgunitsargs args;
	int i;

	ZeroMemory(&args, sizeof (struct dlgunitsargs));
	args.x = x;
	args.y = y;
	args.xs = xs;
	args.ys = ys;
	args.hwnd = hwnd;
	args.dc = GetDC(hwnd);
	if (args.dc == NULL)
		abort();//TODO
	// get averages for the System font now while it's selected into the DC
	getAverages(args.dc, &args.sysb);
	args.font = font;
	args.prevfont = SelectObject(args.dc, args.font);
	if (args.prevfont == NULL)
		abort();//TODO
	getAverages(args.dc, &args.fontb);

	for (i = 0; i < nModes; i++)
		(*modefuncs[i])(i, &args);

	if (SelectObject(args.dc, args.prevfont) != args.font)
		abort();//TODO
	if (ReleaseDC(args.hwnd, args.dc) == 0)
		abort();//TODO
}
