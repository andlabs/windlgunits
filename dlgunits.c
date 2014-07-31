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
	if (SendMessageW(hwnd, LVM_SETITEMCOUNT, (WPARAM) nModes, 0) == 0)
		abort();//TODO
}
