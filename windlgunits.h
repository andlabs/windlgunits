// 31 july 2014
#include "winapi.h"
#include <string.h>
#include <stdio.h>		/* apparently wsprintf_s() is here and not in string.h according to MSDN */
#include <stdlib.h>
#include <errno.h>

#include "mainwin.h"

// dlgunits.c
enum {
	mode_noSystem_GetTextExtentPoint32,
	mode_noSystem_tmAveCharWidth,
	mode_System_GetTextExtentPoint32,
	mode_System_tmAveCharWidth,
	mode_MapDialogRect,
	nModes,
};
extern WCHAR *modenames[nModes];
extern void initResultsListView(HWND);
extern void refreshResultsListView(HWND);
extern void runCalculations(HWND, HFONT, int, int, int *, int *);

// util.c
extern void panic(HWND, char *);
