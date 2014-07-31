// 31 july 2014
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
// get Windows version right; right now Windows XP
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501		/* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <commctrl.h>

// TODO split to a separate file?
// constants for preferred control sizes and positions
// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingspacing
#define dialogMargin 7
#define buttonWidth 67		/* originally 50; not wide enough */
#define buttonHeight 14
#define buttonXSpace 4
#define editHeight 14
#define labelHeight 8
#define labelYOff 3
#define labelXSpace 2
#define horizontalSpace 7
#define verticalSpace 4
