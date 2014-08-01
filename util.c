// 1 august 2014
#include "windlgunits.h"

void panic(HWND owner, char *msg)
{
	DWORD lasterr;
	DWORD comdlglasterr;
	int lasterrno;
	WCHAR *buf;
	WCHAR *lasterrmsg;
	DWORD_PTR args[4];

	lasterr = GetLastError();
	comdlglasterr = CommDlgExtendedError();
	lasterrno = errno;
	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, lasterr, 0, (LPWSTR) (&lasterrmsg), 0, NULL) == 0)
		goto lasterrfail;
	args[0] = (DWORD_PTR) msg;
	args[1] = (DWORD_PTR) lasterrmsg;
	args[2] = (DWORD_PTR) comdlglasterr;
	args[3] = (DWORD_PTR) strerror(lasterrno);
	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_STRING,
		L"Error: %1!S!\nWindows last error: %2\ncomdlg32.dll last error: %3!d!\nerrno: %4!S!",
		0, 0, (LPWSTR) (&buf), 0, (va_list *) args) == 0)
		goto msgfail;
	MessageBoxW(owner,
		buf,
		L"PANIC",
		MB_OK | MB_ICONSTOP | MB_TASKMODAL);
	exit(1);

lasterrfail:
	MessageBoxW(owner,
		L"FormatMessageW() for producing Windows last error failed.",
		L"PANIC IN PANIC",
		MB_OK | MB_ICONSTOP | MB_TASKMODAL);
	abort();

msgfail:
	MessageBoxW(owner,
		L"FormatMessageW() for producing panic message failed.",
		L"PANIC IN PANIC",
		MB_OK | MB_ICONSTOP | MB_TASKMODAL);
	abort();
}
