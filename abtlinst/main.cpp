#include "stdafx.h"
#include "MainWnd.h"
#include "DlgInitInstaller.h"
#include "Path.h"
#include "Network.h"
#include "Setting.h"
#include "Applications.h"
#include "abnrSetting.h"
#include <system/Process.h>
#include "DlgList.h"
#include "Version.h"


Path g_Path;
Setting g_Setting;
Applications g_Applications;

#ifndef INSTALL_FROM_LOCAL
Network g_Network;
#endif

CAppModule g_module;

#include <boost/filesystem.hpp>


int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow){
#ifndef NDEBUG
	::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//	::_CrtSetBreakAlloc(146);


#endif

/*
	{
		OnigRegion *m_region = onig_region_new();
		regex_t *m_regex;
		char *str = "ab+c";
		OnigErrorInfo m_err;
		onig_new(&m_regex,(OnigUChar*)str,(OnigUChar*)(str + ::lstrlenA(str)),ONIG_OPTION_NONE,ONIG_ENCODING_ASCII,ONIG_SYNTAX_RUBY,&m_err);
		onig_free(m_regex);
		onig_region_free(m_region,1);
	}
	return 0;

*/
	OutputVersionInfo();

	::CoInitialize(NULL);
	AtlInitCommonControls(ICC_BAR_CLASSES);
	g_module.Init(NULL, hInstance);
	HMODULE richdll = LoadLibrary(_T("Riched20.dll"));
	
	g_Setting.GenerateSysINI();

	g_Setting.ReadSetting();
	g_Applications.ReadSetting();

#ifdef INSTALL_FROM_LOCAL
	DlgInitInstaller dii;
	if(dii.DoModal() != TRUE){
		::MessageBox(NULL,(_T("インストーラの初期化に失敗しました．\nabtlinst: Version ") + Version()).c_str(),_T("abtlinst"),0);
		g_Setting.Log(_T("abtlinst: Version ") + Version() + _T(" build : ") + VersionDate() + _T(" ") + VersionTime());
		return 1;
	}
#endif

	MainWnd mw;
	int r = mw.DoModal();
	g_module.Term();
	::CoUninitialize();
	
	g_Path.SetPath();
	if(g_Setting.WriteINI()){
		g_Setting.WriteSetting();
		g_Applications.WriteSetting();
	}
	::FreeLibrary(richdll);
	g_Setting.Log(_T("abtlinst: Version ") + Version() + _T(" build : ") + VersionDate() + _T(" ") + VersionTime());
	return r;
}