#include "stdafx.h"
#include "WizTLSet.h"
#include <boost/lexical_cast.hpp>
#include <atlstr.h>
#include "Setting.h"
#include "DlgTLAdvSet.h"
#include "Applications.h"

extern Setting g_Setting;
extern Applications g_Applications;

BOOL WizTLSet::OnSetActive(){
	BOOL enable_win = (g_Applications.m_TeXLive.GetDownloadState() ? TRUE : FALSE);
#ifndef INSTALL_FROM_LOCAL
	CButton(this->GetDlgItem(IDC_DIRISSET)).EnableWindow(enable_win);
	CEdit(this->GetDlgItem(IDC_TEXDIR)).EnableWindow(enable_win);
#endif
	CButton(this->GetDlgItem(IDC_PAPERA4)).EnableWindow(enable_win);
	CButton(this->GetDlgItem(IDC_PAPERLETTER)).EnableWindow(enable_win);
	CButton(this->GetDlgItem(IDC_FILEASSOC)).EnableWindow(enable_win);
	CButton(this->GetDlgItem(IDC_SANSYO)).EnableWindow(enable_win);
	CButton(this->GetDlgItem(IDC_BTNTLADVSET)).EnableWindow(enable_win);

	if(g_Applications.m_TeXLive.GetDownloadState()){
		BOOL paperletter = (g_Applications.m_TeXLive.GetVar(_T("option_letter")) != _T("0"));
		CButton(this->GetDlgItem(IDC_PAPERA4)).SetCheck(!paperletter);
		CButton(this->GetDlgItem(IDC_PAPERLETTER)).SetCheck(paperletter);
		CButton(this->GetDlgItem(IDC_FILEASSOC)).SetCheck((g_Applications.m_TeXLive.GetVar(_T("option_file_assocs")) != _T("0")));
#ifdef INSTALL_FROM_LOCAL
		CEdit(this->GetDlgItem(IDC_INSTDIR)).SetWindowText(g_Setting.GetInstallDir().c_str());
		CButton(this->GetDlgItem(IDC_SETPATH)).SetCheck(g_Setting.SetPath() ? TRUE : FALSE);
		CButton(this->GetDlgItem(IDC_ABNRSET)).SetCheck(g_Applications.m_abnrSetting.DoInst() ? TRUE : FALSE);
		CButton(this->GetDlgItem(IDC_TL)).SetCheck(g_Applications.m_TeXLive.DoInst() ? TRUE : FALSE);
		CButton(this->GetDlgItem(IDC_TEX2IMG)).SetCheck(g_Applications.m_TeX2img.DoInst() ? TRUE : FALSE);
		CButton(this->GetDlgItem(IDC_IMAGEMAGICK)).SetCheck(g_Applications.m_ImageMagick.DoInst() ? TRUE : FALSE);
		CButton(this->GetDlgItem(IDC_NKF)).SetCheck(g_Applications.m_nkf.DoInst() ? TRUE : FALSE);
#else
		CStatic(this->GetDlgItem(IDC_MSG)).SetWindowText(_T(""));
		CButton(this->GetDlgItem(IDC_DIRISSET)).SetCheck((g_TeXLive.GetVar(_T("abnr_use_texlivedir_prefix")) != _T("0")));
		CEdit(this->GetDlgItem(IDC_TEXDIR)).SetWindowText(g_TeXLive.GetVar(_T("TEXDIR")).c_str());
#endif
		SetEnabled();
	}else{
#ifndef INSTALL_FROM_LOCAL
		CStatic(this->GetDlgItem(IDC_MSG)).SetWindowText(_T("TeX Live のインストール準備に失敗したため，インストールされません．"));
#endif
	}
	return TRUE;
}

BOOL WizTLSet::OnWizardBack(){
	return IDD_WIZSETDOWNLOAD;
}

BOOL WizTLSet::OnCommand(UINT codeNotify, int id, HWND hwndCtl){
	switch(id){
	case IDC_DIRISSET:
	case IDC_TL:
		SetEnabled();
		return TRUE;
	case IDC_SANSYO:
	{
#ifndef INSTALL_FROM_LOCAL
		if(CButton(this->GetDlgItem(IDC_DIRISSET)).GetCheck())return FALSE;
#endif
		CFolderDialog dl(this->m_hWnd,_T("インストールディレクトリを選択してください．"));
		if(dl.DoModal() == IDOK){
#ifdef INSTALL_FROM_LOCAL
			CEdit(this->GetDlgItem(IDC_INSTDIR)).SetWindowText(dl.GetFolderPath());
#else
			CEdit(this->GetDlgItem(IDC_TEXDIR)).SetWindowText(dl.GetFolderPath());
#endif
		}
		return TRUE;
	}
	case IDC_BTNTLADVSET:
	{
		SetTeXLiveVars();
		DlgTLAdvSet dlg;
		dlg.DoModal(this->m_hWnd);
		return TRUE;
	}
	default:
		return FALSE;
	}
}

BOOL WizTLSet::OnKillActive(){
	SetTeXLiveVars();
#ifdef INSTALL_FROM_LOCAL
	g_Applications.m_abnrSetting.Set(CButton(this->GetDlgItem(IDC_ABNRSET)).GetCheck() ? true : false);
	g_Applications.m_TeXLive.Set(CButton(this->GetDlgItem(IDC_TL)).GetCheck() ? true : false);
	g_Applications.m_TeX2img.Set(CButton(this->GetDlgItem(IDC_TEX2IMG)).GetCheck() ? true : false);
	g_Applications.m_ImageMagick.Set(CButton(this->GetDlgItem(IDC_IMAGEMAGICK)).GetCheck() ? true : false);
	g_Applications.m_nkf.Set(CButton(this->GetDlgItem(IDC_NKF)).GetCheck() ? true : false);
	g_Setting.SetPath(CButton(this->GetDlgItem(IDC_SETPATH)).GetCheck() ? true : false);

#endif

	return CPropertyPageImpl<WizTLSet>::OnKillActive();
}

void WizTLSet::SetEnabled(){
#ifndef INSTALL_FROM_LOCAL
	CEdit(this->GetDlgItem(IDC_TEXDIR)).SetReadOnly(CButton(this->GetDlgItem(IDC_DIRISSET)).GetCheck() ? TRUE : FALSE);
#endif
	BOOL tl = CButton(this->GetDlgItem(IDC_TL)).GetCheck() ? TRUE : FALSE;
	CButton(this->GetDlgItem(IDC_PAPERA4)).EnableWindow(tl);
	CButton(this->GetDlgItem(IDC_PAPERLETTER)).EnableWindow(tl);
	CButton(this->GetDlgItem(IDC_FILEASSOC)).EnableWindow(tl);
	CButton(this->GetDlgItem(IDC_BTNTLADVSET)).EnableWindow(tl);
}

void WizTLSet::SetTeXLiveVars(){
#ifdef INSTALL_FROM_LOCAL
	ATL::CString str;
	CEdit(this->GetDlgItem(IDC_INSTDIR)).GetWindowText(str);
	g_Setting.SetInstallDir(static_cast<const TCHAR*>(str));
#else
	TCHAR buf[4096];
	if(CButton(this->GetDlgItem(IDC_DIRISSET)).GetCheck()){
		g_TeXLive.SetVar(_T("TEXDIR"),g_Setting.GetInstallDir() + boost::lexical_cast<ablib::string>(g_TeXLive.GetYear()));
	}else{
		CEdit(this->GetDlgItem(IDC_TEXDIR)).GetWindowText(buf,4095);
		g_TeXLive.SetVar(_T("TEXDIR"),buf);
	}
#endif

	g_Applications.m_TeXLive.SetVar(_T("option_letter"),CButton(this->GetDlgItem(IDC_PAPERLETTER)).GetCheck() ? _T("1") : _T("0"));
	g_Applications.m_TeXLive.SetVar(_T("option_file_assocs"),CButton(this->GetDlgItem(IDC_FILEASSOC)).GetCheck() ? _T("1") : _T("0"));
	g_Applications.m_TeXLive.UpdateTeXDirs();
}