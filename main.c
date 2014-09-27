// 31 july 2014
#include "windlgunits.h"

// #qo LDFLAGS: -luser32 -lkernel32 -lcomctl32 -lcomdlg32 -lgdi32

// TODO this needs some /major/ cleanup

LOGFONTW logfontMessageFont;
HFONT lfMessageFont;

struct mainwin {
	HWND hwnd;
	HWND hXCoord;
	HWND hYCoord;
	HWND hResults;
	HWND hCurFontLabel;
	int x;
	int y;
	int xs[nModes];
	int ys[nModes];
	HFONT font;
	BOOL freeFont;
	WCHAR *curfontmsg;
};

static void reportCurrentFont(struct mainwin *mainwin, LOGFONTW lf)
{
	LONG h;
	HDC dc;
	WCHAR *buf;
	DWORD_PTR args[5];

	dc = GetDC(mainwin->hwnd);
	if (dc == NULL)
		panic(mainwin->hwnd, "GetDC() in reportCurrentFont() failed");
	h = lf.lfHeight;
	if (h < 0) {
		// solve height = -MulDiv(points, base, 72dpi) for points to get this
		h = -h;
		h = MulDiv(h, 72, GetDeviceCaps(dc, LOGPIXELSY));
	} else if (h > 0) {
		TEXTMETRICW tm;
		HFONT prev;

		prev = SelectObject(dc, mainwin->font);
		if (prev == NULL)
			panic(mainwin->hwnd, "SelectObject() to select chosen font in reportCurrentFont() failed");
		if (GetTextMetricsW(dc, &tm) == 0)
			panic(mainwin->hwnd, "GetTextMetricsW() in reportCurrentFont() failed");
		if (SelectObject(dc, prev) != mainwin->font)
			panic(mainwin->hwnd, "SelectObject() to restore previous font in reportCurrentFont() failed");
		// solve the first formula in http://support.microsoft.com/kb/74299 for point size
		h = ((h - tm.tmInternalLeading) * 72) / GetDeviceCaps(dc, LOGPIXELSY);
	} else
		panic(mainwin->hwnd, "not sure how to convert 0 from lfHeight to points");
	if (ReleaseDC(mainwin->hwnd, dc) == 0)
		panic(mainwin->hwnd, "ReleaseDC() in reportCurrentFont() failed");
	args[0] = (DWORD_PTR) lf.lfFaceName;
	args[1] = (DWORD_PTR) h;
	args[2] = (DWORD_PTR) lf.lfWeight;
	args[3] = (DWORD_PTR) L"";
	if (lf.lfItalic)
		args[3] = (DWORD_PTR) L" Italic";
	args[4] = (DWORD_PTR) lf.lfCharSet;
	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_STRING,
		L"Current font: %1 %2!d! Weight %3!d!%4 Character Set %5!d!",
		0, 0, (LPWSTR) (&buf), 0, (va_list *) args) == 0)
		panic(mainwin->hwnd, "FormatMessage() in reportCurrentFont() failed");
	if (SendMessageW(mainwin->hCurFontLabel, WM_SETTEXT, 0, (LPARAM) buf) == FALSE)
		panic(mainwin->hwnd, "error setting current font label text");
}

static HFONT chooseFont(struct mainwin *mainwin)
{
	CHOOSEFONTW cf;
	LOGFONTW lf;
	HFONT font;

	ZeroMemory(&cf, sizeof (CHOOSEFONTW));
	ZeroMemory(&lf, sizeof (LOGFONTW));
	cf.lStructSize = sizeof (CHOOSEFONTW);
	cf.hwndOwner = mainwin->hwnd;
	cf.lpLogFont = &lf;
	cf.Flags = 0;		// TODO CF_FORCEFONTEXIST?
	cf.rgbColors = RGB(0, 0, 0);
	if (ChooseFontW(&cf) == FALSE) {
		DWORD err;

		err = CommDlgExtendedError();
		if (err == 0)		// user cancelled
			return NULL;
		panic(mainwin->hwnd, "error opening Choose Font dialog");
	}
	font = CreateFontIndirectW(&lf);
	if (font == NULL)
		panic(mainwin->hwnd, "error loading selected font");
	mainwin->font = font;
	mainwin->freeFont = FALSE;
	reportCurrentFont(mainwin, lf);
	return font;
}

static void freefont(struct mainwin *mainwin)
{
	if (!mainwin->freeFont)		// only if we should
		return;
	if (DeleteObject(mainwin->font) == 0)
		panic(mainwin->hwnd, "error freeing previous font");
}

static void recalc(HWND hwnd, struct mainwin *mainwin)
{
	runCalculations(hwnd, mainwin->font, mainwin->x, mainwin->y, mainwin->xs, mainwin->ys);
	refreshResultsListView(mainwin->hResults);
}

INT_PTR CALLBACK mainwinDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct mainwin *mainwin;
	HFONT font;
	NMHDR *nmhdr = (NMHDR *) lParam;
	NMLVDISPINFOW *fill = (NMLVDISPINFO *) lParam;
	UINT entry;
	BOOL valid;
	LOGFONTW lf;

	mainwin = (struct mainwin *) GetWindowLongPtr(hwnd, DWLP_USER);
	if (mainwin == NULL && uMsg != WM_INITDIALOG)		// skip if not ready yet
		return FALSE;
	switch (uMsg) {
	case WM_INITDIALOG:
		mainwin = (struct mainwin *) malloc(sizeof (struct mainwin));
		if (mainwin == NULL)
			panic(hwnd, "error allocating internal mainwin structure");
		ZeroMemory(mainwin, sizeof (struct mainwin));
		mainwin->hwnd = hwnd;
		mainwin->hXCoord = GetDlgItem(hwnd, ecXCoord);
		if (mainwin->hXCoord == NULL)
			panic(hwnd, "error getting width box handle");
		mainwin->hYCoord = GetDlgItem(hwnd, ecYCoord);
		if (mainwin->hYCoord == NULL)
			panic(hwnd, "error getting height box handle");
		mainwin->hResults = GetDlgItem(hwnd, lcResults);
		if (mainwin->hResults == NULL)
			panic(hwnd, "error getting results list handle");
		mainwin->hCurFontLabel = GetDlgItem(hwnd, scCurFontLabel);
		if (mainwin->hCurFontLabel == NULL)
			panic(hwnd, "error getting current font label handle");
		// require a button to be clicked before editing
		EnableWindow(mainwin->hXCoord, FALSE);
		EnableWindow(mainwin->hYCoord, FALSE);
		SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR) mainwin);
		initResultsListView(mainwin->hResults);
		return TRUE;
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case bcChooseFont:
				font = chooseFont(mainwin);
				if (font == NULL) {
					SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, 0);
					return TRUE;
				}
				freefont(mainwin);
				recalc(hwnd, mainwin);
				EnableWindow(mainwin->hXCoord, TRUE);
				EnableWindow(mainwin->hYCoord, TRUE);
				SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, 0);
				return TRUE;
			case bclfMessageFont:
				freefont(mainwin);
				mainwin->font = lfMessageFont;
				mainwin->freeFont = FALSE;
				reportCurrentFont(mainwin, logfontMessageFont);
				recalc(hwnd, mainwin);
				EnableWindow(mainwin->hXCoord, TRUE);
				EnableWindow(mainwin->hYCoord, TRUE);
				SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, 0);
				return TRUE;
			case bcDialogFont:
				freefont(mainwin);
				mainwin->font = (HFONT) SendMessageW(hwnd, WM_GETFONT, 0, 0);
				if (mainwin->font == NULL)
					panic(hwnd, "error grabbing dialog font");
				mainwin->freeFont = FALSE;
				if (GetObjectW(mainwin->font, sizeof (LOGFONTW), &lf) == 0)
					panic(hwnd, "error getting LOGFONT for dialog font");
				reportCurrentFont(mainwin, lf);
				recalc(hwnd, mainwin);
				EnableWindow(mainwin->hXCoord, TRUE);
				EnableWindow(mainwin->hYCoord, TRUE);
				SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, 0);
				return TRUE;
			}
			return FALSE;
		case EN_CHANGE:
			switch (LOWORD(wParam)) {
			case ecXCoord:
				entry = GetDlgItemInt(hwnd, ecXCoord, &valid, FALSE);
				if (!valid) {
					MessageBeep(-1);
					// TODO
					SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, 0);
					return TRUE;
				}
				mainwin->x = (int) entry;
				recalc(hwnd, mainwin);
				SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, 0);
				return TRUE;
			case ecYCoord:
				entry = GetDlgItemInt(hwnd, ecYCoord, &valid, FALSE);
				if (!valid) {
					MessageBeep(-1);
					// TODO
					SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, 0);
					return TRUE;
				}
				mainwin->y = (int) entry;
				recalc(hwnd, mainwin);
				SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, 0);
				return TRUE;
			}
			return FALSE;
		}
		return FALSE;
	case WM_NOTIFY:
		if (nmhdr->code == LVN_GETDISPINFO && nmhdr->idFrom == lcResults) {
			switch (fill->item.iSubItem) {
			case 0:		// mode name
				fill->item.pszText = modenames[fill->item.iItem];
				return TRUE;
			case 1:		// width
				// I'd use swprintf_s() here but it's not available on Windows XP
				// because we use _snwprintf(), we have to pass len - 1 so we can have room for a null terminator
				_snwprintf(fill->item.pszText, fill->item.cchTextMax - 1, L"%d", mainwin->xs[fill->item.iItem]);
				fill->item.pszText[fill->item.cchTextMax - 1] = L'\0';
				return TRUE;
			case 2:		// height
				_snwprintf(fill->item.pszText, fill->item.cchTextMax - 1, L"%d", mainwin->ys[fill->item.iItem]);
				fill->item.pszText[fill->item.cchTextMax - 1] = L'\0';
				return TRUE;
			}
			// else fallthrough
		}
		return FALSE;
	case WM_CLOSE:
		if (EndDialog(hwnd, 1) == 0)
			panic(NULL, "error closing main window");
		return TRUE;
	default:
		return FALSE;
	}
	panic(hwnd, "programmer error: message code in mainwinDlgProc without return value");
	return FALSE;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX icc;
	NONCLIENTMETRICSW ncm;

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_LISTVIEW_CLASSES;
	if (InitCommonControlsEx(&icc) == FALSE)
		panic(NULL, "Error initializing Common Controls (comctl32.dll)");

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW)) == 0)
		panic(NULL, "Failed to get lfMessageFont");
	logfontMessageFont = ncm.lfMessageFont;
	lfMessageFont = CreateFontIndirectW(&ncm.lfMessageFont);
	if (lfMessageFont == NULL)
		panic(NULL, "Failed to load lfMessageFont");

	if (DialogBox(NULL, MAKEINTRESOURCE(rcMainWin), NULL, mainwinDlgProc) != 1)
		panic(NULL, "DialogBox() to open the main window failed");

	return 0;
}
