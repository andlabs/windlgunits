// 31 july 2014
#include "windlgunits.h"

HFONT lfMessageFont;

struct mainwin {
	HWND hXCoord;
	HWND hYCoord;
	HWND hResults;
	UINT xcoord;
	UINT ycoord;
};

#define printf(...) abort()

HFONT chooseFont(HWND parent)
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
		printf("panic TODO\n");
	}
	font = CreateFontIndirectW(&lf);
	if (font == NULL)
		printf("panic TODO\n");
	return font;
}

INT_PTR CALLBACK mainwinDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct mainwin *mainwin;
	HFONT font;

	mainwin = (struct mainwin *) GetWindowLongPtr(hwnd, DWLP_USER);
	if (mainwin == NULL && uMsg != WM_INITDIALOG)		// skip if not ready yet
		return FALSE;
	switch (uMsg) {
	case WM_INITDIALOG:
		mainwin = (struct mainwin *) malloc(sizeof (struct mainwin));
		if (mainwin == NULL)
			printf("TODO panic\n");
		ZeroMemory(mainwin, sizeof (struct mainwin));
		mainwin->hXCoord = GetDlgItem(hwnd, ecXCoord);
		if (mainwin->hXCoord == NULL)
			printf("TODO panic\n");
		mainwin->hYCoord = GetDlgItem(hwnd, ecYCoord);
		if (mainwin->hYCoord == NULL)
			printf("TODO panic\n");
		mainwin->hResults = GetDlgItem(hwnd, lcResults);
		if (mainwin->hResults == NULL)
			printf("TODO panic\n");
		SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR) mainwin);
		return TRUE;
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case bcChooseFont:
				font = chooseFont(hwnd);
				if (font == NULL)
					return TRUE;
				// TODO
				if (DeleteObject(font) == 0)
					printf("panic TODO\n");
				return TRUE;
			case bclfMessageFont:
				// TODO
				return TRUE;
			case bcDialogFont:
				font = (HFONT) SendMessageW(hwnd, WM_GETFONT, 0, 0);
				if (font == NULL)
					printf("panic TODO\n");
				// TODO
				return TRUE;
			}
			return FALSE;
		}
		return FALSE;
	case WM_CLOSE:
		if (EndDialog(hwnd, 1) == 0)
			printf("TODO panic\n");
		return TRUE;
	default:
		return FALSE;
	}
	// TODO panic
	return FALSE;
}

int main(void)
{
	INITCOMMONCONTROLSEX icc;
	NONCLIENTMETRICSW ncm;

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_LISTVIEW_CLASSES;
	if (InitCommonControlsEx(&icc) == FALSE)
		printf("error TODO\n");

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW)) == 0)
		printf("panic TODO\n");
	lfMessageFont = CreateFontIndirectW(&ncm.lfMessageFont);
	if (lfMessageFont == NULL)
		printf("panic TODO\n");

	if (DialogBox(NULL, MAKEINTRESOURCE(rcMainWin), NULL, mainwinDlgProc) != 1)
		printf("error TODO\n");

	return 0;
}