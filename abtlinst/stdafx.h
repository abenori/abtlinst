#pragma once

#define _WTL_NEW_PAGE_NOTIFY_HANDLER

#ifndef NDEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#include <crtdbg.h>
#endif

// winsock2.h‚Íwindows.h‚æ‚èæ‚Éinclude‚·‚é•K—v‚ª‚ ‚éH
#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <Shlobj.h>
#include <wininet.h>
#include <shellapi.h>

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atldlgs.h>
#include <atlctrls.h>

#include <ablib/string.h>


//#pragma warning(disable:4005)
#include <functional>
//#pragma warning(default:4005)

#define _TR(s) _T(s)

struct MSGFUNC{
	std::function<void (const ablib::string&)> msg,detail;
};


