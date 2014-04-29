#pragma once

#include "resource.h"
#include "WizFirst.h"
#include "WizSetDownload.h"
#include "WizDownload.h"
#include "WizTLSet.h"
#include "WizInstall.h"

class MainWnd :	public CPropertySheetImpl<MainWnd>
{
public:
	MainWnd(ATL::_U_STRINGorID title = reinterpret_cast<LPCTSTR>(NULL),UINT StartPage = 0,HWND hwnd = NULL) : CPropertySheetImpl<MainWnd>(title,StartPage,hwnd){
		SetWizardMode();
		AddPage(m_wizfirst);
#ifndef INSTALL_FROM_LOCAL
		AddPage(m_wizsetdownload);
		AddPage(m_wizdownload);
#endif
		AddPage(m_wiztlset);
		AddPage(m_wizinstall);
	}
	~MainWnd(void){}
private:
	WizFirst m_wizfirst;
#ifndef INSTALL_FROM_LOCAL
	WizSetDownload m_wizsetdownload;
	WizDownload m_wizdownload;
#endif
	WizTLSet m_wiztlset;
	WizInstall m_wizinstall;
};

struct DownloadFileData{
	ablib::string url;
	unsigned int size;
	ablib::string localdir;
};
