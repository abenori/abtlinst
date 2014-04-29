// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// winsock2.hはwindows.hより先にincludeする必要がある？
#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <Shlobj.h>
#include <wininet.h>
#include <shellapi.h>

#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atldlgs.h>
#include <atlctrls.h>

#include <ablib/string.h>
#include <functional>

struct MSGFUNC{
	std::function<void (const ablib::string&)> msg,detail;
};

