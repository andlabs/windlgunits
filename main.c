// 31 july 2014
#include "windlgunits.h"

HFONT lfMessageFont;

struct mainwin {
	HWND hwnd;
	HWND hXCoord;
	HWND hYCoord;
	HWND hResults;
	int x;
	int y;
	int xs[nModes];
	int ys[nModes];
	HFONT font;
	BOOL freeFont;
};

static HFONT chooseFont(HWND parent)
{
	CHOOSEFONTW cf;
	LOGFONTW lf;
	HFONT font;

	ZeroMemory(&cf, sizeof (CHOOSEFONTW));
	ZeroMemory(&lf, sizeof (LOGFONTW));
	cf.lStructSize = sizeof (CHOOSEFONTW);
	cf.hwndOwner = parent;
	cf.lpLogFont = &lf;
	cf.Flags = CF_EFFECTS;		// TODO CF_FORCEFONTEXIST?
	cf.rgbColors = RGB(0, 0, 0);
	if (ChooseFontW(&cf) == FALSE) {
		DWORD err;

		err = CommDlgExtendedError();
		if (err == 0)		// user cancelled
			return NULL;
		panic(parent, "error opening Choose Font dialog");
	}
	font = CreateFontIndirectW(&lf);
	if (font == NULL)
		panic(parent, "error loading selected font");
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
				font = chooseFont(hwnd);
				if (font == NULL) {
					SetWindowLongW(hwnd, DWL_MSGRESULT, 0);
					return TRUE;
				}
				freefont(mainwin);
				mainwin->font = font;
				mainwin->freeFont = TRUE;
				recalc(hwnd, mainwin);
				EnableWindow(mainwin->hXCoord, TRUE);
				EnableWindow(mainwin->hYCoord, TRUE);
				SetWindowLongW(hwnd, DWL_MSGRESULT, 0);
				return TRUE;
			case bclfMessageFont:
				freefont(mainwin);
				mainwin->font = lfMessageFont;
				mainwin->freeFont = FALSE;
				recalc(hwnd, mainwin);
				EnableWindow(mainwin->hXCoord, TRUE);
				EnableWindow(mainwin->hYCoord, TRUE);
				SetWindowLongW(hwnd, DWL_MSGRESULT, 0);
				return TRUE;
			case bcDialogFont:
				freefont(mainwin);
				mainwin->font = (HFONT) SendMessageW(hwnd, WM_GETFONT, 0, 0);
				if (mainwin->font == NULL)
					panic(hwnd, "error grabbing dialog font");
				mainwin->freeFont = FALSE;
				recalc(hwnd, mainwin);
				EnableWindow(mainwin->hXCoord, TRUE);
				EnableWindow(mainwin->hYCoord, TRUE);
				SetWindowLongW(hwnd, DWL_MSGRESULT, 0);
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
					SetWindowLongW(hwnd, DWL_MSGRESULT, 0);
					return TRUE;
				}
				mainwin->x = (int) entry;
				recalc(hwnd, mainwin);
				SetWindowLongW(hwnd, DWL_MSGRESULT, 0);
				return TRUE;
			case ecYCoord:
				entry = GetDlgItemInt(hwnd, ecYCoord, &valid, FALSE);
				if (!valid) {
					MessageBeep(-1);
					// TODO
					SetWindowLongW(hwnd, DWL_MSGRESULT, 0);
					return TRUE;
				}
				mainwin->y = (int) entry;
				recalc(hwnd, mainwin);
				SetWindowLongW(hwnd, DWL_MSGRESULT, 0);
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
	lfMessageFont = CreateFontIndirectW(&ncm.lfMessageFont);
	if (lfMessageFont == NULL)
		panic(NULL, "Failed to load lfMessageFont");

	if (DialogBox(NULL, MAKEINTRESOURCE(rcMainWin), NULL, mainwinDlgProc) != 1)
		panic(NULL, "DialogBox() to open the main window failed");

	return 0;
}
